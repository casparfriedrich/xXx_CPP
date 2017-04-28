#include <assert.h>
#include <string.h>

#include <xXx/components/wireless/nrf24l01p/nrf24l01p_base.hpp>
#include <xXx/components/wireless/nrf24l01p/nrf24l01p_types.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

namespace xXx {

nRF24L01P_BASE::nRF24L01P_BASE(ISpi &spi) : _spi(spi) {}

nRF24L01P_BASE::~nRF24L01P_BASE() {}

uint8_t nRF24L01P_BASE::transmit(uint8_t command, uint8_t inBytes[], uint8_t outBytes[],
                                 size_t numBytes) {
    uint8_t status;
    uint8_t buffer[numBytes + 1];

    buffer[0] = command;

    if (inBytes != NULL) {
        memcpy(&buffer[1], inBytes, numBytes);
    } else {
        memset(&buffer[1], 0xff, numBytes);
    }

    _spi.transmit_receive(buffer, buffer, numBytes + 1);

    status = buffer[0];

    if (outBytes != NULL) {
        memcpy(outBytes, &buffer[1], numBytes);
    }

    return (status);
}

uint8_t nRF24L01P_BASE::cmd_R_REGISTER(Register_t address, uint8_t bytes[], size_t numBytes) {
    uint8_t command;
    uint8_t status;

    command = OR<uint8_t>(Command_R_REGISTER, address);
    status  = transmit(command, NULL, bytes, numBytes);

    return (status);
}

uint8_t nRF24L01P_BASE::cmd_W_REGISTER(Register_t address, uint8_t bytes[], size_t numBytes) {
    uint8_t command;
    uint8_t status;

    command = OR<uint8_t>(Command_W_REGISTER, address);
    status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

uint8_t nRF24L01P_BASE::cmd_W_TX_PAYLOAD(uint8_t bytes[], size_t numBytes) {
    uint8_t command;
    uint8_t status;

    command = Command_W_TX_PAYLOAD;
    status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

uint8_t nRF24L01P_BASE::cmd_R_RX_PAYLOAD(uint8_t bytes[], size_t numBytes) {
    uint8_t command;
    uint8_t status;

    command = Command_R_RX_PAYLOAD;
    status  = transmit(command, NULL, bytes, numBytes);

    return (status);
}

uint8_t nRF24L01P_BASE::cmd_FLUSH_TX() {
    uint8_t command;
    uint8_t status;

    command = Command_FLUSH_TX;
    status  = transmit(command, NULL, NULL, 0);

    return (status);
}

uint8_t nRF24L01P_BASE::cmd_FLUSH_RX() {
    uint8_t command;
    uint8_t status;

    command = Command_FLUSH_RX;
    status  = transmit(command, NULL, NULL, 0);

    return (status);
}

uint8_t nRF24L01P_BASE::cmd_REUSE_TX_PL() {
    uint8_t command;
    uint8_t status;

    command = Command_REUSE_TX_PL;
    status  = transmit(command, NULL, NULL, 0);

    return (status);
}

uint8_t nRF24L01P_BASE::cmd_R_RX_PL_WID(uint8_t &payloadLength) {
    uint8_t command;
    uint8_t status;

    command = Command_R_RX_PL_WID;
    status  = transmit(command, NULL, &payloadLength, 1);

    return (status);
}

uint8_t nRF24L01P_BASE::cmd_W_ACK_PAYLOAD(uint8_t pipe, uint8_t bytes[], size_t numBytes) {
    uint8_t command;
    uint8_t status;

    command = OR<uint8_t>(Command_W_ACK_PAYLOAD, pipe);
    status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

uint8_t nRF24L01P_BASE::cmd_W_TX_PAYLOAD_NOACK(uint8_t bytes[], size_t numBytes) {
    uint8_t command;
    uint8_t status;

    command = Command_W_TX_PAYLOAD_NOACK;
    status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

uint8_t nRF24L01P_BASE::cmd_NOP() {
    uint8_t command;
    uint8_t status;

    command = Command_NOP;
    status  = transmit(command, NULL, NULL, 0);

    return (status);
}

} /* namespace xXx */
