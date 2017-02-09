#if not defined(NRF24L01P_ESB_HPP_)
#define NRF24L01P_ESB_HPP_

#include <stdint.h>

#include <xXx/components/wireless/nrf24l01p/nrf24l01p_base.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/os/arduinotask.hpp>
#include <xXx/templates/queue.hpp>

enum DataRate_t : uint8_t { DataRate_1MBPS, DataRate_2MBPS, DataRate_250KBPS };

enum CRCConfig_t : uint8_t { CRCConfig_DISABLED, CRCConfig_1Byte, CrcConfig_2Bytes };

enum OutputPower_t : uint8_t {
    OutputPower_18dBm,
    OutputPower_12dBm,
    OutputPower_6dBm,
    OutputPower_0dBm
};

enum OperatingMode_t : uint8_t {
    OperatingMode_Shutdown,
    OperatingMode_Standby,
    OperatingMode_Rx,
    OperatingMode_Tx
};

namespace xXx {

class nRF24L01P_ESB : public nRF24L01P_BASE, public ArduinoTask {
  private:
    IGpio &_ce;
    IGpio &_irq;
    ISpi &_spi;

    Queue_Handle_t<uint8_t> _rxQueue[6];
    Queue_Handle_t<uint8_t> _txQueue;

    OperatingMode_t _operatingMode;

    void transmit_receive(Queue<uint8_t> &queue);

    uint8_t readShortRegister(Register_t reg);
    void writeShortRegister(Register_t reg, uint8_t regValue);
    void clearSingleBit(Register_t reg, uint8_t bitIndex);
    void setSingleBit(Register_t reg, uint8_t bitIndex);

    void setup();
    void loop();

    void enterRxMode();
    void enterShutdownMode();
    void enterStandbyMode();
    void enterTxMode();

    void handle_MAX_RT(uint8_t status);
    void handle_RX_DR(uint8_t status);
    void handle_TX_DS(uint8_t status);

    uint8_t readRxFifo(uint8_t status);
    uint8_t writeTxFifo(uint8_t status);

    void enableDataPipe(uint8_t pipe);
    void disableDataPipe(uint8_t pipe);

  public:
    nRF24L01P_ESB(ISpi &spi, IGpio &ce, IGpio &irq);
    ~nRF24L01P_ESB();

    void configureTxPipe(Queue<uint8_t> &txQueue, uint64_t address = 0);
    void configureRxPipe(uint8_t pipe, Queue<uint8_t> &rxQueue, uint64_t address = 0);
    void switchOperatingMode(OperatingMode_t mode);

    uint8_t getChannel();
    void setChannel(uint8_t channel);
    CRCConfig_t getCrcConfig();
    void setCrcConfig(CRCConfig_t crc);
    void setDataRate(DataRate_t dataRate);
    DataRate_t getDataRate();
    OutputPower_t getOutputPower();
    void setOutputPower(OutputPower_t level);
    uint8_t getRetryCount();
    void setRetryCount(uint8_t count);
    uint8_t getRetryDelay();
    void setRetryDelay(uint8_t delay);
    uint64_t getRxAddress(uint8_t pipe);
    void setRxAddress(uint8_t pipe, uint64_t address);
    uint64_t getTxAddress();
    void setTxAddress(uint64_t address);
};

} /* namespace xXx */

#endif // NRF24L01P_ESB_HPP_