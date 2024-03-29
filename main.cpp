/**
 ******************************************************************************
 * @file    main.cpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    10-2019
 * @brief
 ******************************************************************************
 * @attention
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.hpp"

int main()
{
    Main();
}

Main::Main() : _systick(Systick::getInstance()), _lcd(4, 20), _net(nullptr)
{
    // Configure 1 tick - 1 msec
    _systick.init(SystemCoreClock, 1000);

    initNet();
}

void Main::initNet()
{
    // Create SPI interface class
    static Spi* spi = Spi::getInstance(SPI1);

    // Configure
    Spi::Config spiConfig;
    spiConfig.mode = Spi::Mode::MASTER;
    spiConfig.direction = Spi::Direction::TWO_LINE_FULL_DUPLEX;
    spiConfig.prescaler = Spi::Prescaler::_64P;
    spiConfig.size = Spi::Size::_8B;
    spiConfig.phase = Spi::Phase::_1E;
    spiConfig.polarity = Spi::Polarity::LOW;
    spiConfig.firstBit = Spi::FirstBit::MSB;
    spi->init(&spiConfig);
    spi->createInterrupt();

    // Config interface
    SpiInterface::Config interface;
    interface.virtualPort = spi;

    interface.interruptPort = GPIOC;
    interface.interruptPin = 6;
    interface.resetPort = GPIOC;
    interface.resetPin = 5;
    interface.csPort = GPIOC;
    interface.csPin = 4;

    Enc28j60::Config config;
    constexpr uint8_t MAC[] = { 0x00, 0x2F, 0x68, 0x12, 0xAC, 0x30 };
    memcpy(config.macAddr, MAC, Enc28j60::MAC_ADDR_SIZE);
    constexpr uint8_t IP[] = { 192, 168, 0, 200 };
    memcpy(config.ipAddr, IP, Enc28j60::IP_ADDR_SIZE);
    config.tcpPort = 80;

    // Create NET class
    Enc28j60* _net = new Enc28j60(&interface, &config);
}