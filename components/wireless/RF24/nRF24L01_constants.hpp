#ifndef NRF24L01_CONSTANTS_HPP_
#define NRF24L01_CONSTANTS_HPP_

static const uint8_t max_payload_size   = 32;
static const uint8_t max_address_length = 5;
static const uint8_t dummy              = 0xFF;
static const uint8_t RW_REGISTER_MASK   = 0b00011111;

enum RF24_Command_t {
    RF24_Command_R_REGISTER          = 0x00,
    RF24_Command_W_REGISTER          = 0x20,
    RF24_Command_R_RX_PAYLOAD        = 0x61,
    RF24_Command_W_TX_PAYLOAD        = 0xA0,
    RF24_Command_FLUSH_TX            = 0xE1,
    RF24_Command_FLUSH_RX            = 0xE2,
    RF24_Command_REUSE_TX_PL         = 0xE3,
    RF24_Command_R_RX_PL_WID         = 0x60,
    RF24_Command_W_ACK_PAYLOAD       = 0xA8,
    RF24_Command_W_TX_PAYLOAD_NO_ACK = 0xB0,
    RF24_Command_NOP                 = 0xFF,
    RF24_Command_ACTIVATE            = 0x50,
    RF24_Command_REGISTER_MASK       = 0x1F,
};

// XXX
enum class RF24_Command : uint8_t {
    R_REGISTER          = 0x00,
    W_REGISTER          = 0x20,
    R_RX_PAYLOAD        = 0x61,
    W_TX_PAYLOAD        = 0xA0,
    FLUSH_TX            = 0xE1,
    FLUSH_RX            = 0xE2,
    REUSE_TX_PL         = 0xE3,
    R_RX_PL_WID         = 0x60,
    W_ACK_PAYLOAD       = 0xA8,
    W_TX_PAYLOAD_NO_ACK = 0xB0,
    NOP                 = 0xFF,
    ACTIVATE            = 0x50 // TODO: Explain from data sheet
};

enum RF24_MemoryMap_t {
    RF24_MM_CONFIG      = 0x00,
    RF24_MM_EN_AA       = 0x01,
    RF24_MM_EN_RXADDR   = 0x02,
    RF24_MM_SETUP_AW    = 0x03,
    RF24_MM_SETUP_RETR  = 0x04,
    RF24_MM_RF_CH       = 0x05,
    RF24_MM_RF_SETUP    = 0x06,
    RF24_MM_STATUS      = 0x07,
    RF24_MM_OBSERVE_TX  = 0x08,
    RF24_MM_CD          = 0x09,
    RF24_MM_RX_ADDR_P0  = 0x0A,
    RF24_MM_RX_ADDR_P1  = 0x0B,
    RF24_MM_RX_ADDR_P2  = 0x0C,
    RF24_MM_RX_ADDR_P3  = 0x0D,
    RF24_MM_RX_ADDR_P4  = 0x0E,
    RF24_MM_RX_ADDR_P5  = 0x0F,
    RF24_MM_TX_ADDR     = 0x10,
    RF24_MM_RX_PW_P0    = 0x11,
    RF24_MM_RX_PW_P1    = 0x12,
    RF24_MM_RX_PW_P2    = 0x13,
    RF24_MM_RX_PW_P3    = 0x14,
    RF24_MM_RX_PW_P4    = 0x15,
    RF24_MM_RX_PW_P5    = 0x16,
    RF24_MM_FIFO_STATUS = 0x17,
    RF24_MM_DYNPD       = 0x1C,
    RF24_MM_FEATURE     = 0x1D
};

// XXX
enum class RF24_MemoryMap : uint8_t {
    CONFIG      = 0x00,
    EN_AA       = 0x01,
    EN_RXADDR   = 0x02,
    SETUP_AW    = 0x03,
    SETUP_RETR  = 0x04,
    RF_CH       = 0x05,
    RF_SETUP    = 0x06,
    STATUS      = 0x07,
    OBSERVE_TX  = 0x08,
    CD          = 0x09,
    RX_ADDR_P0  = 0x0A,
    RX_ADDR_P1  = 0x0B,
    RX_ADDR_P2  = 0x0C,
    RX_ADDR_P3  = 0x0D,
    RX_ADDR_P4  = 0x0E,
    RX_ADDR_P5  = 0x0F,
    TX_ADDR     = 0x10,
    RX_PW_P0    = 0x11,
    RX_PW_P1    = 0x12,
    RX_PW_P2    = 0x13,
    RX_PW_P3    = 0x14,
    RX_PW_P4    = 0x15,
    RX_PW_P5    = 0x16,
    FIFO_STATUS = 0x17,
    DYNPD       = 0x1C,
    FEATURE     = 0x1D
};

enum RF24_Config_t {
    RF24_Config_PRIM_RX,
    RF24_Config_PWR_UP,
    RF24_Config_CRCO,
    RF24_Config_EN_CRC,
    RF24_Config_MASK_MAX_RT,
    RF24_Config_MASK_TX_DS,
    RF24_Config_MASK_RX_DR
};

enum RF24_EN_AA_t {
    RF24_ENAA_P0,
    RF24_ENAA_P1,
    RF24_ENAA_P2,
    RF24_ENAA_P3,
    RF24_ENAA_P4,
    RF24_ENAA_P5
};

enum RF24_EN_RXADDR_t {
    RF24_ERX_P0,
    RF24_ERX_P1,
    RF24_ERX_P2,
    RF24_ERX_P3,
    RF24_ERX_P4,
    RF24_ERX_P5
};

enum RF24_SETUP_AW_t {
    RF24_SETUP_AW,
};

enum RF24_SETUP_RETR_t {
    RF24_SETUP_RETR_ARC = 0,
    RF24_SETUP_RETR_ARD = 4,
};

// clang-format off

/* Bit Mnemonics */
#define PLL_LOCK      (4)
#define RF_DR         (3)
#define RF_PWR        (6)
#define RX_DR         (6)
#define TX_DS         (5)
#define MAX_RT        (4)
#define RX_P_NO       (1)
#define TX_FULL       (0)
#define PLOS_CNT      (4)
#define ARC_CNT       (0)
#define TX_REUSE      (6)
#define FIFO_FULL     (5)
#define TX_EMPTY      (4)
#define RX_FULL       (1)
#define RX_EMPTY      (0)
#define DPL_P5        (5)
#define DPL_P4        (4)
#define DPL_P3        (3)
#define DPL_P2        (2)
#define DPL_P1        (1)
#define DPL_P0        (0)
#define EN_DPL        (2)
#define EN_ACK_PAY    (1)
#define EN_DYN_ACK    (0)

/* Non-P omissions */
#define LNA_HCURR     (0)

/* P model memory Map */
#define RPD           (0x09)

/* P model bit Mnemonics */
#define RF_DR_LOW     (5)
#define RF_DR_HIGH    (3)
#define RF_PWR_LOW    (1)
#define RF_PWR_HIGH   (2)

// clang-format on

#endif // NRF24L01_CONSTANTS_HPP_