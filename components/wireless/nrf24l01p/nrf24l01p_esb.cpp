#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <xXx/components/wireless/nrf24l01p/nrf24l01p_base.hpp>
#include <xXx/components/wireless/nrf24l01p/nrf24l01p_definitions.hpp>
#include <xXx/components/wireless/nrf24l01p/nrf24l01p_esb.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

#include <nRF24L01_config.h>

#define LAMBDA(params) [](params)

namespace xXx {

static const uint8_t txSettling = 130;
static const uint8_t rxSettling = 130;
static const uint8_t txFifoSize = 32;
static const uint8_t rxFifoSize = 32;

static inline bool isPipeIndexValid(uint8_t pipeIndex) {
    return (pipeIndex > 5 ? false : true);
}

static inline uint8_t extractPipeIndex(uint8_t status) {
    AND_eq<uint8_t>(status, STATUS_RX_P_NO_MASK);
    RIGHT_eq<uint8_t>(status, STATUS_RX_P_NO);

    return (status);
}

nRF24L01P_ESB::nRF24L01P_ESB(ISpi &spi, IGpio &ce, IGpio &irq)
    : nRF24L01P_BASE(spi), _ce(ce), _irq(irq), _txQueue(NULL), _rxQueue{NULL, NULL, NULL,
                                                                        NULL, NULL, NULL} {}

nRF24L01P_ESB::~nRF24L01P_ESB() {
    switchOperatingMode(OperatingMode_Shutdown);
}

void nRF24L01P_ESB::setup() {
    auto interruptFunction = LAMBDA(void *user) {
        nRF24L01P_ESB *self = static_cast<nRF24L01P_ESB *>(user);

        self->taskNotifyFromISR();
    };

    // Enable dynamic payload length only
    uint8_t feature = 0;
    clearBit_eq<uint8_t>(feature, FEATURE_EN_DYN_ACK);
    clearBit_eq<uint8_t>(feature, FEATURE_EN_ACK_PAY);
    setBit_eq<uint8_t>(feature, FEATURE_EN_DPL);
    writeShortRegister(Register_FEATURE, feature);

    // Clear interrupts
    uint8_t status = 0;
    setBit_eq<uint8_t>(status, STATUS_MAX_RT);
    setBit_eq<uint8_t>(status, STATUS_RX_DR);
    setBit_eq<uint8_t>(status, STATUS_TX_DS);
    writeShortRegister(Register_STATUS, status);

    cmd_FLUSH_TX();
    cmd_FLUSH_RX();

    _irq.enableInterrupt(interruptFunction, this);
}

void nRF24L01P_ESB::loop() {
    taskNotifyTake(pdFALSE);

    int8_t status = cmd_NOP();

    if (readBit<uint8_t>(status, STATUS_RX_DR)) {
        handle_RX_DR();
    }

    if (readBit<uint8_t>(status, STATUS_MAX_RT)) {
        handle_MAX_RT();
    }

    if (readBit<uint8_t>(status, STATUS_TX_DS)) {
        handle_TX_DS();
    }
}

int8_t nRF24L01P_ESB::readRxFifo() {
    uint8_t numBytes;

    int8_t status     = cmd_NOP();
    uint8_t pipeIndex = extractPipeIndex(status);

    if (pipeIndex > 5) {
        return (-1);
    }

    if (_rxQueue[pipeIndex] == NULL) {
        return (-1);
    }

    cmd_R_RX_PL_WID(numBytes);

    if (numBytes > rxFifoSize) {
        return (-1);
    }

    uint8_t bytes[numBytes];

    cmd_R_RX_PAYLOAD(bytes, numBytes);

    for (int i = 0; i < numBytes; ++i) {
        _rxQueue[pipeIndex]->enqueue(bytes[i]);
    }

    return (0);
}

int8_t nRF24L01P_ESB::writeTxFifo() {
    if (_txQueue == NULL) {
        return (-1);
    }

    UBaseType_t usedSlots = _txQueue->queueMessagesWaiting();

    if (usedSlots == 0) {
        return (-1);
    }

    uint8_t numBytes = (txFifoSize < usedSlots ? txFifoSize : usedSlots);
    uint8_t bytes[numBytes];

    for (int i = 0; i < numBytes; ++i) {
        _txQueue->dequeue(bytes[i]);
    }

    cmd_W_TX_PAYLOAD(bytes, numBytes);

    return (0);
}

void nRF24L01P_ESB::handle_MAX_RT() {
    cmd_FLUSH_TX();
    writeShortRegister(Register_STATUS, STATUS_MAX_RT_MASK);
}

void nRF24L01P_ESB::handle_RX_DR() {
    // uint8_t fifo_status;

    // do {
    int8_t rxStatus = readRxFifo();

    // if (rxStatus < 0) {
    // cmd_FLUSH_RX();
    // }

    // fifo_status = readShortRegister(Register_FIFO_STATUS);
    // } while (not readBit<uint8_t>(fifo_status, FIFO_STATUS_RX_EMPTY));

    writeShortRegister(Register_STATUS, STATUS_RX_DR_MASK);
}

void nRF24L01P_ESB::handle_TX_DS() {
    // uint8_t fifo_status;

    // do {
    int8_t txStatus = writeTxFifo();

    // if (txStatus < 0) {
    //     break;
    // }

    // fifo_status = readShortRegister(Register_FIFO_STATUS);
    // } while (readBit<uint8_t>(fifo_status, FIFO_STATUS_TX_FULL) == false);

    writeShortRegister(Register_STATUS, STATUS_TX_DS_MASK);
}

void nRF24L01P_ESB::enterRxMode() {
    switchOperatingMode(OperatingMode_Standby);

    // Enter PRX mode
    setSingleBit(Register_CONFIG, CONFIG_PRIM_RX);

    _ce.set();

    delayUs(rxSettling);
}

void nRF24L01P_ESB::enterShutdownMode() {
    _ce.clear();

    clearSingleBit(Register_CONFIG, CONFIG_PWR_UP);
}

void nRF24L01P_ESB::enterStandbyMode() {
    _ce.clear();

    setSingleBit(Register_CONFIG, CONFIG_PWR_UP);
}

void nRF24L01P_ESB::enterTxMode() {
    switchOperatingMode(OperatingMode_Standby);

    // Enter PTX mode
    clearSingleBit(Register_CONFIG, CONFIG_PRIM_RX);

    _ce.set();

    delayUs(txSettling);
}

void nRF24L01P_ESB::configureTxPipe(uint64_t txAddress) {
    setTxAddress(txAddress);
    setRxAddress(0, txAddress);
    enableDataPipe(0);
}

void nRF24L01P_ESB::configureRxPipe(uint8_t pipe, Queue<uint8_t> &rxQueue, uint64_t rxAddress) {
    assert(isPipeIndexValid(pipe));

    setRxAddress(pipe, rxAddress);
    enableDataPipe(pipe);

    _rxQueue[pipe] = &rxQueue;
}

void nRF24L01P_ESB::switchOperatingMode(OperatingMode_t operatingMode) {
    switch (operatingMode) {
        case OperatingMode_Rx: {
            enterRxMode();
        } break;
        case OperatingMode_Shutdown: {
            enterShutdownMode();
        } break;
        case OperatingMode_Standby: {
            enterStandbyMode();
        } break;
        case OperatingMode_Tx: {
            enterTxMode();
        } break;
    }
}

int8_t nRF24L01P_ESB::send(Queue<uint8_t> &txQueue) {
    int8_t txStatus;

    if (_txQueue != NULL) {
        return (-1);
    }

    _txQueue = &txQueue;

    txStatus = writeTxFifo();

    if (txStatus < 0) {
        _txQueue = NULL;

        return (-1);
    } else {
        return (0);
    }
}

// ----- helper functions -----------------------------------------------------

void nRF24L01P_ESB::enableDataPipe(uint8_t pipeIndex) {
    assert(isPipeIndexValid(pipeIndex));

    setSingleBit(Register_EN_RXADDR, pipeIndex);
    setSingleBit(Register_DYNPD, pipeIndex);
}

void nRF24L01P_ESB::disableDataPipe(uint8_t pipeIndex) {
    assert(isPipeIndexValid(pipeIndex));

    clearSingleBit(Register_EN_RXADDR, pipeIndex);
    clearSingleBit(Register_DYNPD, pipeIndex);
}

uint8_t nRF24L01P_ESB::readShortRegister(Register_t reg) {
    uint8_t result;
    cmd_R_REGISTER(reg, &result, 1);

    return (result);
}

void nRF24L01P_ESB::writeShortRegister(Register_t reg, uint8_t val) {
    cmd_W_REGISTER(reg, &val, 1);
}

void nRF24L01P_ESB::clearSingleBit(Register_t reg, uint8_t bitIndex) {
    uint8_t tmp = readShortRegister(reg);
    clearBit_eq(tmp, bitIndex);
    writeShortRegister(reg, tmp);
}

void nRF24L01P_ESB::setSingleBit(Register_t reg, uint8_t bitIndex) {
    uint8_t tmp = readShortRegister(reg);
    setBit_eq(tmp, bitIndex);
    writeShortRegister(reg, tmp);
}

// ----- getters and setters --------------------------------------------------

CRCConfig_t nRF24L01P_ESB::getCrcConfig() {
    uint8_t config = readShortRegister(Register_CONFIG);

    if (not readBit<uint8_t>(config, CONFIG_EN_CRC)) {
        return (CRCConfig_DISABLED);
    }

    if (readBit<uint8_t>(config, CONFIG_CRCO)) {
        return (CrcConfig_2Bytes);
    } else {
        return (CRCConfig_1Byte);
    }
}

void nRF24L01P_ESB::setCrcConfig(CRCConfig_t crcConfig) {
    uint8_t config = readShortRegister(Register_CONFIG);

    switch (crcConfig) {
        case CRCConfig_DISABLED: {
            clearBit_eq<uint8_t>(config, CONFIG_EN_CRC);
        } break;
        case CRCConfig_1Byte: {
            setBit_eq<uint8_t>(config, CONFIG_EN_CRC);
            clearBit_eq<uint8_t>(config, CONFIG_CRCO);
        } break;
        case CrcConfig_2Bytes: {
            setBit_eq<uint8_t>(config, CONFIG_EN_CRC);
            setBit_eq<uint8_t>(config, CONFIG_CRCO);
        } break;
    }

    writeShortRegister(Register_CONFIG, config);

    assert(crcConfig == getCrcConfig());
}

int8_t nRF24L01P_ESB::getChannel() {
    int8_t channel = readShortRegister(Register_RF_CH);

    return (channel);
}

void nRF24L01P_ESB::setChannel(int8_t channel) {
    if (channel < 0) {
        writeShortRegister(Register_RF_CH, channel);
    }

    assert(channel == getChannel());
}

DataRate_t nRF24L01P_ESB::getDataRate() {
    uint8_t rfSetup = readShortRegister(Register_RF_SETUP);

    if (readBit<uint8_t>(rfSetup, RF_SETUP_RF_DR_LOW)) {
        return (DataRate_250KBPS);
    }

    if (readBit<uint8_t>(rfSetup, RF_SETUP_RF_DR_HIGH)) {
        return (DataRate_2MBPS);
    }

    return (DataRate_1MBPS);
}

void nRF24L01P_ESB::setDataRate(DataRate_t dataRate) {
    uint8_t rfSetup = readShortRegister(Register_RF_SETUP);

    switch (dataRate) {
        case (DataRate_1MBPS): {
            clearBit_eq<uint8_t>(rfSetup, RF_SETUP_RF_DR_LOW);
            clearBit_eq<uint8_t>(rfSetup, RF_SETUP_RF_DR_HIGH);
        } break;
        case DataRate_2MBPS: {
            clearBit_eq<uint8_t>(rfSetup, RF_SETUP_RF_DR_LOW);
            setBit_eq<uint8_t>(rfSetup, RF_SETUP_RF_DR_HIGH);
        } break;
        case DataRate_250KBPS: {
            setBit_eq<uint8_t>(rfSetup, RF_SETUP_RF_DR_LOW);
            clearBit_eq<uint8_t>(rfSetup, RF_SETUP_RF_DR_HIGH);
        } break;
    }

    writeShortRegister(Register_RF_SETUP, rfSetup);

    assert(dataRate == getDataRate());
}

OutputPower_t nRF24L01P_ESB::getOutputPower() {
    uint8_t rf_setup = readShortRegister(Register_RF_SETUP);

    AND_eq<uint8_t>(rf_setup, RF_SETUP_RF_PWR_MASK);
    RIGHT_eq<uint8_t>(rf_setup, RF_SETUP_RF_PWR);

    return (static_cast<OutputPower_t>(rf_setup));
}

void nRF24L01P_ESB::setOutputPower(OutputPower_t outputPower) {
    uint8_t rf_setup = readShortRegister(Register_RF_SETUP);

    AND_eq(rf_setup, INVERT<uint8_t>(RF_SETUP_RF_PWR_MASK));
    OR_eq<uint8_t>(rf_setup, LEFT<uint8_t>(outputPower, RF_SETUP_RF_PWR));

    writeShortRegister(Register_RF_SETUP, rf_setup);

    assert(outputPower == getOutputPower());
}

uint8_t nRF24L01P_ESB::getRetryCount() {
    uint8_t setup_retr = readShortRegister(Register_SETUP_RETR);

    AND_eq<uint8_t>(setup_retr, SETUP_RETR_ARC_MASK);
    RIGHT_eq<uint8_t>(setup_retr, SETUP_RETR_ARC);

    return (setup_retr);
}

void nRF24L01P_ESB::setRetryCount(uint8_t count) {
    assert(count <= 0xF);

    uint8_t setup_retr;

    setup_retr = readShortRegister(Register_SETUP_RETR);

    LEFT_eq<uint8_t>(count, SETUP_RETR_ARC);
    AND_eq<uint8_t>(setup_retr, INVERT<uint8_t>(SETUP_RETR_ARC_MASK));
    OR_eq<uint8_t>(setup_retr, count);

    writeShortRegister(Register_SETUP_RETR, setup_retr);

    assert(count == getRetryCount());
}

uint8_t nRF24L01P_ESB::getRetryDelay() {
    uint8_t setup_retr = readShortRegister(Register_SETUP_RETR);

    AND_eq<uint8_t>(setup_retr, SETUP_RETR_ARD_MASK);
    RIGHT_eq<uint8_t>(setup_retr, SETUP_RETR_ARD);

    return (setup_retr);
}

void nRF24L01P_ESB::setRetryDelay(uint8_t delay) {
    assert(delay <= 0xF);

    uint8_t setup_retr;

    setup_retr = readShortRegister(Register_SETUP_RETR);

    LEFT_eq<uint8_t>(delay, SETUP_RETR_ARD);
    AND_eq<uint8_t>(setup_retr, INVERT<uint8_t>(SETUP_RETR_ARD_MASK));
    OR_eq<uint8_t>(setup_retr, delay);

    writeShortRegister(Register_SETUP_RETR, setup_retr);

    assert(delay == getRetryDelay());
}

int64_t nRF24L01P_ESB::getRxAddress(uint8_t pipeIndex) {
    assert(isPipeIndexValid(pipeIndex));

    Register_t rxAddressRegister;
    uint64_t rxAddressLength;
    uint64_t rxAddress       = 0;
    uint8_t *rxAddressBuffer = reinterpret_cast<uint8_t *>(&rxAddress);

    switch (pipeIndex) {
        case 0: {
            rxAddressRegister = Register_RX_ADDR_P0;
            rxAddressLength   = RX_ADDR_P0_LENGTH;
        } break;
        case 1: {
            rxAddressRegister = Register_RX_ADDR_P1;
            rxAddressLength   = RX_ADDR_P1_LENGTH;
        } break;
        case 2: {
            rxAddressRegister = Register_RX_ADDR_P2;
            rxAddressLength   = RX_ADDR_P2_LENGTH;
        } break;
        case 3: {
            rxAddressRegister = Register_RX_ADDR_P3;
            rxAddressLength   = RX_ADDR_P3_LENGTH;
        } break;
        case 4: {
            rxAddressRegister = Register_RX_ADDR_P4;
            rxAddressLength   = RX_ADDR_P4_LENGTH;
        } break;
        case 5: {
            rxAddressRegister = Register_RX_ADDR_P5;
            rxAddressLength   = RX_ADDR_P5_LENGTH;
        } break;
    }

    cmd_R_REGISTER(rxAddressRegister, rxAddressBuffer, rxAddressLength);

    return (rxAddress);
}

void nRF24L01P_ESB::setRxAddress(uint8_t pipeIndex, int64_t rxAddress) {
    assert(isPipeIndexValid(pipeIndex));

    Register_t addressRegister;
    uint8_t addressLength;

    switch (pipeIndex) {
        case 0: {
            addressRegister = Register_RX_ADDR_P0;
            addressLength   = RX_ADDR_P0_LENGTH;
        } break;
        case 1: {
            addressRegister = Register_RX_ADDR_P1;
            addressLength   = RX_ADDR_P1_LENGTH;
        } break;
        case 2: {
            addressRegister = Register_RX_ADDR_P2;
            addressLength   = RX_ADDR_P2_LENGTH;
        } break;
        case 3: {
            addressRegister = Register_RX_ADDR_P3;
            addressLength   = RX_ADDR_P3_LENGTH;
        } break;
        case 4: {
            addressRegister = Register_RX_ADDR_P4;
            addressLength   = RX_ADDR_P4_LENGTH;
        } break;
        case 5: {
            addressRegister = Register_RX_ADDR_P5;
            addressLength   = RX_ADDR_P5_LENGTH;
        } break;
    }

    uint8_t rxAddressBuffer[addressLength];
    memcpy(rxAddressBuffer, &rxAddress, addressLength);

    cmd_W_REGISTER(addressRegister, rxAddressBuffer, addressLength);

    assert(rxAddress == getRxAddress(pipeIndex));
}

int64_t nRF24L01P_ESB::getTxAddress() {
    uint64_t txAddress       = 0;
    uint8_t *txAddressBuffer = reinterpret_cast<uint8_t *>(&txAddress);

    cmd_R_REGISTER(Register_TX_ADDR, txAddressBuffer, TX_ADDR_LENGTH);

    return (txAddress);
}

void nRF24L01P_ESB::setTxAddress(int64_t txAddress) {
    uint8_t txAddressBuffer[TX_ADDR_LENGTH];
    memcpy(txAddressBuffer, &txAddress, TX_ADDR_LENGTH);

    cmd_W_REGISTER(Register_TX_ADDR, txAddressBuffer, TX_ADDR_LENGTH);

    assert(txAddress == getTxAddress());
}

} /* namespace xXx */
