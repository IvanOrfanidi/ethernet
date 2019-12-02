/**
 ******************************************************************************
 * @file    lcd.cpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    10-April-2018
 * @brief   This file provides all the RTC firmware method.
 ******************************************************************************
 * @attention
 *
 *
 * 
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "hd44780.hpp"
#include <string.h>

/**
 * @brief  Initialisation LCD.
 * @retval None.
 */
Hd44780::Hd44780(uint8_t sizeLine, uint8_t sizeColumn) :
    _sizeLine(sizeLine),
    _sizeColumn(sizeColumn),
    _numLine(RamAddr::DD_RAM_ADDR1),
    _pointer(0),
    _totalSize(0),
    _systick(Systick::getInstance())
{
}

void Hd44780::init() const
{
    _rw.reset();
    _rs.reset();

    _systick.delay(20);
    _d4.set();
    _d5.set();
    clock();
    _systick.delay(20);
    _d4.set();
    _d5.set();
    clock();
    _systick.delay(20);
    _d4.set();
    _d5.set();
    clock();
    _systick.delay(20);
    _d4.reset();
    _d5.set();
    clock();
    _d4.reset();
    _d5.reset();
    _d6.reset();
    _d7.reset();

    sendCmd(DISP_ON);
    sendCmd(CLR_DISP);
    _systick.delay(100);
}

void Hd44780::setSizeLine(uint8_t size)
{
    _sizeLine = size;
}

void Hd44780::setSizeColumn(uint8_t size)
{
    _sizeColumn = size;
}

void Hd44780::configPortPinRS(GPIO_TypeDef* port, uint8_t pin)
{
    Gpio::Config gpio;
    gpio.mode = Gpio::Mode::OUTPUT_PUSH_PULL;
    gpio.speed = Gpio::Speed::_50mhz;
    _rs.init(port, pin, &gpio);
}

void Hd44780::configPortPinRW(GPIO_TypeDef* port, uint8_t pin)
{
    Gpio::Config gpio;
    gpio.mode = Gpio::Mode::OUTPUT_PUSH_PULL;
    gpio.speed = Gpio::Speed::_50mhz;
    _rw.init(port, pin, &gpio);
}

void Hd44780::configPortPinE(GPIO_TypeDef* port, uint8_t pin)
{
    Gpio::Config gpio;
    gpio.mode = Gpio::Mode::OUTPUT_PUSH_PULL;
    gpio.speed = Gpio::Speed::_50mhz;
    _e.init(port, pin, &gpio);
}

void Hd44780::configPortPinD4(GPIO_TypeDef* port, uint8_t pin)
{
    Gpio::Config gpio;
    gpio.mode = Gpio::Mode::OUTPUT_PUSH_PULL;
    gpio.speed = Gpio::Speed::_50mhz;
    _d4.init(port, pin, &gpio);
}

void Hd44780::configPortPinD5(GPIO_TypeDef* port, uint8_t pin)
{
    Gpio::Config gpio;
    gpio.mode = Gpio::Mode::OUTPUT_PUSH_PULL;
    gpio.speed = Gpio::Speed::_50mhz;
    _d5.init(port, pin, &gpio);
}

void Hd44780::configPortPinD6(GPIO_TypeDef* port, uint8_t pin)
{
    Gpio::Config gpio;
    gpio.mode = Gpio::Mode::OUTPUT_PUSH_PULL;
    gpio.speed = Gpio::Speed::_50mhz;
    _d6.init(port, pin, &gpio);
}

void Hd44780::configPortPinD7(GPIO_TypeDef* port, uint8_t pin)
{
    Gpio::Config gpio;
    gpio.mode = Gpio::Mode::OUTPUT_PUSH_PULL;
    gpio.speed = Gpio::Speed::_50mhz;
    _d7.init(port, pin, &gpio);
}

/**
 * @brief  Send command to LCD
 * @param [in] cmd - command
 * @retval None.
 */
void Hd44780::sendCmd(uint8_t cmd) const
{
    if(cmd & 0x80) {
        _d7.set();
    }
    if(cmd & 0x40) {
        _d6.set();
    }
    if(cmd & 0x20) {
        _d5.set();
    }
    if(cmd & 0x10) {
        _d4.set();
    }

    // set LCD to data mode
    _rs.reset();
    clock();
    _d4.reset();
    _d5.reset();
    _d6.reset();
    _d7.reset();

    if(cmd & 0x8) {
        _d7.set();
    }
    if(cmd & 0x4) {
        _d6.set();
    }
    if(cmd & 0x2) {
        _d5.set();
    }
    if(cmd & 0x1) {
        _d4.set();
    }

    // set LCD to data mode
    _rs.reset();
    clock();
    _d4.reset();
    _d5.reset();
    _d6.reset();
    _d7.reset();
}

/**
 * @brief  Send char to LCD
 * @param [in] data - char
 * @retval None.
 */
void Hd44780::send(char data)
{
    if(data & 0x80) {
        _d7.set();
    }
    if(data & 0x40) {
        _d6.set();
    }
    if(data & 0x20) {
        _d5.set();
    }
    if(data & 0x10) {
        _d4.set();
    }

    _rs.set();
    clock();
    _d4.reset();
    _d5.reset();
    _d6.reset();
    _d7.reset();

    if(data & 0x8) {
        _d7.set();
    }
    if(data & 0x4) {
        _d6.set();
    }
    if(data & 0x2) {
        _d5.set();
    }
    if(data & 0x1) {
        _d4.set();
    }

    _rs.set();
    clock();
    _d4.reset();
    _d5.reset();
    _d6.reset();
    _d7.reset();
}

/**
 * @brief  Go to position LCD
 * @retval None.
 */
bool Hd44780::goTo(uint8_t x, uint8_t y)
{
    if(x >= _sizeLine || y >= _sizeColumn) {
        return true;
    }
    _pointer = y;
    _totalSize = x * _sizeColumn + y;
    switch(x) {
        case 0:
            _numLine = RamAddr::DD_RAM_ADDR1;
            sendCmd(RamAddr::DD_RAM_ADDR1 + y);
            break;

        case 1:
            _numLine = RamAddr::DD_RAM_ADDR2;
            sendCmd(RamAddr::DD_RAM_ADDR2 + y);
            break;

        case 2:
            _numLine = RamAddr::DD_RAM_ADDR3;
            sendCmd(RamAddr::DD_RAM_ADDR3 + y);
            break;

        case 3:
            _numLine = RamAddr::DD_RAM_ADDR4;
            sendCmd(RamAddr::DD_RAM_ADDR4 + y);
            break;

        default:
            return true;
    }

    return false;
}

/**
 * @brief  Send string to LCD
 * @param [in] string - string
 * @retval None.
 */
void Hd44780::send(const char* const str)
{
    if(_totalSize >= _sizeColumn * _sizeLine) {
        clear();
        _totalSize = 0;
        _pointer = 0;
        _numLine = DD_RAM_ADDR1;
    }

    // loop to the end of the string
    size_t i = 0;
    while(str[i] != 0) {
        if((str[i] == '\r') || (str[i] == '\n') ||
            (_pointer > _sizeColumn - 1)) {
            _pointer = 0;
            switch(_numLine) {
                case RamAddr::DD_RAM_ADDR1:
                    _numLine = DD_RAM_ADDR2;
                    sendCmd(DD_RAM_ADDR2 + _pointer);
                    break;

                case RamAddr::DD_RAM_ADDR2:
                    _numLine = DD_RAM_ADDR3;
                    sendCmd(DD_RAM_ADDR3 + _pointer);
                    break;

                case RamAddr::DD_RAM_ADDR3:
                    _numLine = DD_RAM_ADDR4;
                    sendCmd(DD_RAM_ADDR4 + _pointer);
                    break;

                case RamAddr::DD_RAM_ADDR4:
                    _numLine = DD_RAM_ADDR1;
                    sendCmd(DD_RAM_ADDR1 + _pointer);
                    break;

                default:
                    break;
            }

            if((str[i] == '\r') || (str[i] == '\n')) {
                ++i;
                continue;
            }
        }

        send(str[i]);
        _pointer++;
        _totalSize++;
        ++i;
    }
}

/**
 * @brief  clear LCD
 * @retval None.
 */
void Hd44780::clear()
{
    sendCmd(CLR_DISP);
    _systick.delay(1);
    _totalSize = 0;
    _pointer = 0;
    _numLine = RamAddr::DD_RAM_ADDR1;
}

void Hd44780::clock() const
{
    _e.set();
    _systick.delay(1);
    _e.reset();
}
