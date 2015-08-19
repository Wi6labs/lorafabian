#ifndef _FROGGYFACTORY_LORA_H_
#define _FROGGYFACTORY_LORA_H_

#if defined(ARDUINO) && ARDUINO >= 100
  #include <Arduino.h>
#else
  #include <WProgram.h>
#endif

#include "coap.h"

// Arduino commands
#define ARDUINO_CMD_AVAILABLE 0x00
#define ARDUINO_CMD_READ      0x01
#define ARDUINO_CMD_WRITE     0x02

#define ARDUINO_CMD_DEBUG     0x20
#define ARDUINO_CMD_HOSTNAME  0x21
#define ARDUINO_CMD_GET_MAC   0x22

#define ARDUINO_CMD_FREQ       0x30
#define ARDUINO_CMD_GET_FREQ   0x31
#define ARDUINO_CMD_RF_CFG     0x32
#define ARDUINO_CMD_BW_CFG     0x33
#define ARDUINO_CMD_GET_BW_CFG 0x34
#define ARDUINO_CMD_SF_CFG     0x35
#define ARDUINO_CMD_GET_SF_CFG 0x36
#define ARDUINO_CMD_CR_CFG     0x37
#define ARDUINO_CMD_GET_CR_CFG 0x38

#define ARDUINO_CMD_TEST       0xFF

//Delay
#define WAIT_TIME_BETWEEN_SPI_MSG 200
#define WAIT_TIME_BETWEEN_BYTES_SPI 20

//Radio
#define FREQ_8680 868000000
#define FREQ_8681 868100000
#define FREQ_8682 868200000
#define FREQ_8683 868300000
#define FREQ_8695 869500000
#define FREQ_8696 869600000
#define FREQ_8698 869800000
#define FREQ_8699 869900000
#define FREQ_MIN  863000000
#define FREQ_MAX  870000000
#define FREQ_DEFAULT FREQ_MAX

//SF : between 7 & 12
#define SF_MIN     7
#define SF_MAX     12
#define SF_DEFAULT SF_MIN

//CR : 1(4/5), 2(4/6), 3(4/7), 4(4/8)
#define CR_MIN     1
#define CR_MAX     4
#define CR_DEFAULT 3

//BW : 0(125kHz), 1(250kHz), 2(500kHz) 
#define BW_MIN     0
#define BW_MAX     2
#define BW_DEFAULT 1

#define CONF_MIN     0
#define CONF_MAX     4
#define CONF_DEFAULT CONF_MIN

//SPI
#define SS_PIN 10

#if defined(F_CPU) && F_CPU == 8000000L
    //MCU 8MHz
    #define SPI_CLK_DIVIDER SPI_CLOCK_DIV16
#elif defined(F_CPU) && F_CPU == 16000000L
    //MCU 16 MHz
    #define SPI_CLK_DIVIDER SPI_CLOCK_DIV32
#elif defined(F_CPU) && F_CPU == 84000000L
    //Due board
    #define DUEBOARD
    #define SPI_CLK_DIVIDER 168
#endif

/**
 * Class to use the Wi6Labs Shield
 */
class LoraShield
{
  public:
    LoraShield();

    void init();
    void begin(String name);
    int dataAvailable();
    String read(bool verbose = false);
    void write(byte buff[], int bufflen);

    void setContikiDebug(bool setcontikidebug);

    void setFreq(long freq);
    unsigned long getFreq();
    void setBandwidth(int bw);
    int  getBandwidth();
    void setCodingRate(int cr);
    int  getCodingRate();
    void setSpreadingFactor(int sf);
    int  getSpreadingFactor();
    void setRFConfig(int rfconfig);
    String getMAC();

  private:
    String hex8ToString(uint8_t *data, uint8_t length);
    boolean _debug;
};

#endif
