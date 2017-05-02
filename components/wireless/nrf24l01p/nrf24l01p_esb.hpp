#ifndef NRF24L01P_ESB_HPP_
#define NRF24L01P_ESB_HPP_

#include <stdint.h>

#include <xXx/components/wireless/nrf24l01p/nrf24l01p_base.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/os/simpletask.hpp>
#include <xXx/templates/queue.hpp>

namespace xXx {

class RF24_ESB : public nRF24L01P_BASE, public SimpleTask {
   private:
    IGpio &ce;
    IGpio &irq;

    Queue_Handle_t<RF24_Package_t> rxQueue[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
    Queue_Handle_t<RF24_Package_t> txQueue    = NULL;

    void setup();
    void loop();

    void handle_MAX_RT(uint8_t status);
    void handle_RX_DR(uint8_t status);
    void handle_TX_DS(uint8_t status);

    RF24_Status readRxFifo(uint8_t status);
    RF24_Status writeTxFifo(uint8_t status);
    RF24_Status enableDynamicPayloadLength(uint8_t pipe);
    RF24_Status disableDynamicPayloadLength(uint8_t pipe);

    uint8_t getRetransmissionCounter();

   public:
    RF24_ESB(ISpi &spi, IGpio &ce, IGpio &irq);
    ~RF24_ESB();

    void enterRxMode();
    void enterShutdownMode();
    void enterStandbyMode();
    void enterTxMode();

    RF24_Status enableRxDataPipe(uint8_t pipe, Queue<RF24_Package_t> &rxQueue);
    RF24_Status disableRxDataPipe(uint8_t pipe);
    RF24_Status enableTxDataPipe(Queue<RF24_Package_t> &txQueue);
    RF24_Status disableTxDataPipe();

    RF24_Address_t getRxAddress(uint8_t pipe);
    RF24_Status setRxAddress(uint8_t pipe, RF24_Address_t address);
    RF24_Address_t getTxAddress();
    RF24_Status setTxAddress(RF24_Address_t address);

    uint8_t getChannel();
    RF24_Status setChannel(uint8_t channel);
    RF24_CRCConfig getCrcConfig();
    RF24_Status setCrcConfig(RF24_CRCConfig crc);
    RF24_DataRate getDataRate();
    RF24_Status setDataRate(RF24_DataRate dataRate);
    RF24_OutputPower getOutputPower();
    RF24_Status setOutputPower(RF24_OutputPower level);
    uint8_t getRetryCount();
    RF24_Status setRetryCount(uint8_t count);
    uint8_t getRetryDelay();
    RF24_Status setRetryDelay(uint8_t delay);

    uint8_t getPackageLossCounter();
};

} /* namespace xXx */

#endif  // NRF24L01P_ESB_HPP_
