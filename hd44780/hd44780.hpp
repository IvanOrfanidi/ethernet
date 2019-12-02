/**
 ******************************************************************************
 * @file    rtc.hpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    28-March-2018
 * @brief   This file contains all the methods prototypes for the RTC
 *          firmware library.
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HD44780_HPP
#define __HD44780_HPP

#ifdef __cplusplus

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include <stdint.h>

/* Drivers periphl */
#include "gpio.hpp"
#include "systick.hpp"

/////////////////////////////////////////////////////////

/**
 * @brief Class Lcd HD44780
 */
class Hd44780 {
  public:
    explicit Hd44780(uint8_t, uint8_t);

    void setSizeLine(uint8_t);

    void setSizeColumn(uint8_t);

    void configPortPinRS(GPIO_TypeDef*, uint8_t);

    void configPortPinRW(GPIO_TypeDef*, uint8_t);

    void configPortPinE(GPIO_TypeDef*, uint8_t);

    void configPortPinD4(GPIO_TypeDef*, uint8_t);

    void configPortPinD5(GPIO_TypeDef*, uint8_t);

    void configPortPinD6(GPIO_TypeDef*, uint8_t);

    void configPortPinD7(GPIO_TypeDef*, uint8_t);

    void init() const;

    ///
    enum LcdCmd : uint8_t {
        DISP_ON = 0x0C,     ///< LCD on
        DISP_OFF = 0x08,    ///< LCD off
        CLR_DISP = 0x01,    ///< Clear LCD
        CUR_HOME = 0x02,    ///< 0x0D & 0x0A
    };

    void clear();    /// Clear LCD

    void send(char);    /// Send char to LCD

    void send(const char* const);    /// Send string to LCD

    bool goTo(uint8_t, uint8_t);    /// Go to position LCD

  private:
    enum RamAddr : uint8_t {
        DD_RAM_ADDR1 = 128,
        DD_RAM_ADDR3 = 148,
        DD_RAM_ADDR2 = 192,
        DD_RAM_ADDR4 = 212,
    };

    Hd44780() = default;

    void sendCmd(uint8_t) const;    /// Send command to LCD

    void clock() const;

    uint8_t _sizeLine;
    uint8_t _sizeColumn;

    RamAddr _numLine;
    uint8_t _pointer;
    uint8_t _totalSize;

    Systick& _systick;

    Gpio _rs;
    Gpio _rw;
    Gpio _e;

    Gpio _d4;
    Gpio _d5;
    Gpio _d6;
    Gpio _d7;
};

#endif

#endif