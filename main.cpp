

/* Standart lib */
#include <iostream>
#include <time.h>

/* Driver lib */
#include "spi.hpp"
#include "ethernet/enc28j60.hpp"
#include "exti.hpp"
#include "gpio.hpp"

int main()
{
    auto& systick = Systick::getInstance();
    // Configure 1 tick - 1 msec
    systick.init(SystemCoreClock, 1000);

    // Create SPI interface class
    Spi* spi = Spi::getInstance(SPI1);

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

    interface.cePort = GPIOC;
    interface.cePin = 4;

    Enc28j60::Config config;
    constexpr uint8_t MAC[] = { 0x00, 0x2F, 0x68, 0x12, 0xAC, 0x30 };
    memcpy(config.macAddr, MAC, Enc28j60::MAC_ADDR_SIZE);
    constexpr uint8_t IP[] = { 192, 168, 0, 200 };
    memcpy(config.macAddr, IP, Enc28j60::IP_ADDR_SIZE);
    config.tcpPort = 80;

    // Create NET class
    Enc28j60* net = new Enc28j60(&interface, &config);

    while(true) {
    }
}
