/**
 ******************************************************************************
 * @file    enc28j60.cpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    07/01/2019
 * @brief   This file provides all the enc28j60 firmware method.
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; </center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "enc28j60.hpp"

/**
 * @brief Constructor
 * @param [in] port - virtual port (SPI)
 */
Enc28j60::Enc28j60(const SpiInterface::Config* interfaceConfig,
    const Config* config) :
    _interface(interfaceConfig),
    _enc28j60Bank(0),
    _nextPacketPtr(RXSTART_INIT),
    _tcpPort(0),
    _buffer(nullptr),
    _bufSize(0),
    _isError(false)
{
    writeOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
    _interface.delayUs(100);

    // Rx start
    writeReg(ERXSTL, RXSTART_INIT & 0xFF);
    writeReg(ERXSTH, RXSTART_INIT >> 8);
    // set receive pointer address
    writeReg(ERXRDPTL, RXSTART_INIT & 0xFF);
    writeReg(ERXRDPTH, RXSTART_INIT >> 8);
    // RX end
    writeReg(ERXNDL, RXSTOP_INIT & 0xFF);
    writeReg(ERXNDH, RXSTOP_INIT >> 8);
    // TX start
    writeReg(ETXSTL, TXSTART_INIT & 0xFF);
    writeReg(ETXSTH, TXSTART_INIT >> 8);
    // TX end
    writeReg(ETXNDL, TXSTOP_INIT & 0xFF);
    writeReg(ETXNDH, TXSTOP_INIT >> 8);
    // do bank 1 stuff, packet filter:
    // For broadcast packets we allow only ARP packtets
    // All other packets should be unicast only for our mac (MAADR)

    // The pattern to match on is therefore
    // Type     ETH.DST
    // ARP      BROADCAST
    // 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
    // in binary these poitions are:11 0000 0011 1111
    // This is hex 303F->EPMM0=0x3F,EPMM1=0x30
    writeReg(ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN);
    writeReg(EPMM0, 0x3F);
    writeReg(EPMM1, 0x30);

    writeReg(EPMCSL, 0xF9);
    writeReg(EPMCSH, 0xF7);

    // do bank 2 stuff
    // enable MAC receive
    writeReg(MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
    // bring MAC out of reset
    writeReg(MACON2, 0x00);
    // enable automatic padding to 60bytes and CRC operations
    writeOp(ENC28J60_BIT_FIELD_SET,
        MACON3,
        MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);
    // set inter-frame gap (non-back-to-back)
    writeReg(MAIPGL, 0x12);
    writeReg(MAIPGH, 0x0C);
    // set inter-frame gap (back-to-back)
    writeReg(MABBIPG, 0x12);
    // Set the maximum packet size which the controller will accept
    // Do not send packets longer than MAX_FRAMELEN:
    writeReg(MAMXFLL, MAX_FRAMELEN & 0xFF);
    writeReg(MAMXFLH, MAX_FRAMELEN >> 8);
    // do bank 3 stuff
    // write MAC address
    // NOTE: MAC address in ENC28J60 is byte-backward
    writeReg(MAADR5, config->macAddr[0]);
    writeReg(MAADR4, config->macAddr[1]);
    writeReg(MAADR3, config->macAddr[2]);
    writeReg(MAADR2, config->macAddr[3]);
    writeReg(MAADR1, config->macAddr[4]);
    writeReg(MAADR0, config->macAddr[5]);
    // no loopback of transmitted frames
    phyWrite(PHCON2, PHCON2_HDLDIS);
    // switch to bank 0
    setBank(ECON1);
    // enable interrutps
    writeOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);
    // enable packet reception
    writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

    initPhy();

    memcpy(_macAddr, config->macAddr, MAC_ADDR_SIZE);
    memcpy(_ipAddr, config->ipAddr, IP_ADDR_SIZE);

    if(_bufSize > MAX_FRAMELEN) {
        _bufSize = MAX_FRAMELEN;
    }

    // Create memory
    _buffer = ::new uint8_t[config->sizeBuf];

    // Attacgh interrupt, calling update
    _interface.attach(this);
}

Enc28j60::~Enc28j60()
{
    ::delete[] _buffer;
}

void Enc28j60::initPhy()
{
    /* Magjack leds configuration, see enc28j60 datasheet, page 11 */
    // LEDA=green LEDB=yellow
    //
    // 0x880 is PHLCON LEDB=on, LEDA=on
    // enc28j60PhyWrite(PHLCON,0b0000 1000 1000 00 00);
    phyWrite(PHLCON, 0x880);
    _interface.delayUs(500);
    //
    // 0x990 is PHLCON LEDB=off, LEDA=off
    // enc28j60PhyWrite(PHLCON,0b0000 1001 1001 00 00);
    phyWrite(PHLCON, 0x990);
    _interface.delayUs(500);
    //
    // 0x880 is PHLCON LEDB=on, LEDA=on
    // enc28j60PhyWrite(PHLCON,0b0000 1000 1000 00 00);
    phyWrite(PHLCON, 0x880);
    _interface.delayUs(500);
    //
    // 0x990 is PHLCON LEDB=off, LEDA=off
    // enc28j60PhyWrite(PHLCON,0b0000 1001 1001 00 00);
    phyWrite(PHLCON, 0x990);
    _interface.delayUs(500);
    //
    // 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit
    // enc28j60PhyWrite(PHLCON,0b0000 0100 0111 01 10);
    phyWrite(PHLCON, 0x476);
    _interface.delayUs(100);
}

uint8_t Enc28j60::readReg(uint8_t address)
{
    // set the bank
    setBank(address);
    // do the read
    return readOp(ENC28J60_READ_CTRL_REG, address);
}

void Enc28j60::writeReg(uint8_t address, uint8_t data)
{
    // set the bank
    setBank(address);
    // do the write
    writeOp(ENC28J60_WRITE_CTRL_REG, address, data);
}

void Enc28j60::setBank(uint8_t address)
{
    // set the bank (if needed)
    if((address & BANK_MASK) != _enc28j60Bank) {
        writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1 | ECON1_BSEL0));
        writeOp(ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK) >> 5);
        _enc28j60Bank = (address & BANK_MASK);
    }
}

void Enc28j60::writeOp(uint8_t oper, uint8_t address, uint8_t data)
{
    _interface.setSelect(true);
    _interface.sendByte(oper | (address & ADDR_MASK));
    _interface.sendByte(data);
    _interface.setSelect(false);
}

uint8_t Enc28j60::readOp(uint8_t oper, uint8_t address)
{
    _interface.setSelect(true);
    _interface.sendByte(oper | (address & ADDR_MASK));
    uint8_t data = _interface.getByte();

    // do dummy read if needed (for mac and mii, see datasheet page 29)
    if(address & 0x80) {
        data = _interface.getByte();
    }
    _interface.setSelect(false);
    return data;
}

void Enc28j60::writeBuffer(const uint8_t* data, size_t len)
{
    _interface.setSelect(true);
    _interface.sendByte(ENC28J60_WRITE_BUF_MEM);
    for(size_t i = 0; i < len; ++i) {
        _interface.sendByte(*data++);
    }
    _interface.setSelect(false);
}

void Enc28j60::readBuffer(uint8_t* data, size_t len)
{
    _interface.setSelect(true);
    _interface.sendByte(ENC28J60_READ_BUF_MEM);

    for(size_t i = 0; i < len; ++i) {
        *data++ = _interface.getByte();
    }

    _interface.setSelect(false);
}

void Enc28j60::phyWrite(uint8_t address, uint16_t data)
{
    // set the PHY register address
    writeReg(MIREGADR, address);
    // write the PHY data
    writeReg(MIWRL, data);
    writeReg(MIWRH, data >> 8);
    // wait until the PHY write completes
    while(readReg(MISTAT) & MISTAT_BUSY) {
        _interface.delayUs(1);
    }
}

/**
 * @brief Gets a packet from the network receive buffer, if one is available
 * @param [in] packet - pointer where packet data should be stored
 * @param [in] maxLen  - maximum acceptable length of a retrieved packet
 * @retval packet length in bytes if a packet was retrieved, zero otherwise
 */
size_t Enc28j60::packetReceive(uint8_t* packet, size_t maxLen)
{
    // check if a packet has been received and buffered
    //if( !(enc28j60Read(EIR) & EIR_PKTIF) ){
    // The above does not work. See Rev. B4 Silicon Errata point 6.
    if(readReg(EPKTCNT) == 0) {
        return 0;
    }

    // Set the read pointer to the start of the received packet
    writeReg(ERDPTL, _nextPacketPtr);
    writeReg(ERDPTH, _nextPacketPtr >> 8);

    // read the next packet pointer
    _nextPacketPtr = readOp(ENC28J60_READ_BUF_MEM, 0);
    _nextPacketPtr |= readOp(ENC28J60_READ_BUF_MEM, 0) << 8;

    // read the packet length (see datasheet page 43)
    size_t len = readOp(ENC28J60_READ_BUF_MEM, 0);
    len |= readOp(ENC28J60_READ_BUF_MEM, 0) << 8;
    len -= 4;    //remove the CRC count

    // read the receive status (see datasheet page 43)
    uint16_t rxstat = readOp(ENC28J60_READ_BUF_MEM, 0);
    rxstat |= readOp(ENC28J60_READ_BUF_MEM, 0) << 8;

    // limit retrieve length
    const size_t limit = maxLen - 1;
    if(len > limit) {
        len = limit;
    }
    // check CRC and symbol errors (see datasheet page 44, table 7-3):
    // The ERXFCON.CRCEN is set by default. Normally we should not
    // need to check this.
    if((rxstat & 0x80) == 0) {
        // invalid
        len = 0;
    }
    else {
        // copy the packet from the receive buffer
        readBuffer(packet, len);
    }

    // Move the RX read pointer to the start of the next received packet
    // This frees the memory we just read out
    writeReg(ERXRDPTL, _nextPacketPtr);
    writeReg(ERXRDPTH, _nextPacketPtr >> 8);

    // decrement the packet counter indicate we are done with this packet
    writeOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
    return len;
}

void Enc28j60::packetSend(const uint8_t* packet, size_t len)
{
    // Set the write pointer to start of transmit buffer area
    writeReg(EWRPTL, TXSTART_INIT & 0xFF);
    writeReg(EWRPTH, TXSTART_INIT >> 8);
    // Set the TXND pointer to correspond to the packet size given
    writeReg(ETXNDL, (TXSTART_INIT + len) & 0xFF);
    writeReg(ETXNDH, (TXSTART_INIT + len) >> 8);
    // write per-packet control byte (0x00 means use macon3 settings)
    writeOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
    // copy the packet into the transmit buffer
    writeBuffer(packet, len);
    // send the contents of the transmit buffer onto the network
    writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
    // Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
    if((readReg(EIR) & EIR_TXERIF)) {
        writeOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
    }
}

bool Enc28j60::isError() const
{
    return _isError;
}

void Enc28j60::update()
{
    while(true) {
        const size_t pacLen = packetReceive(_buffer, _bufSize);
        if(0 == pacLen) {
            return;
        }

        // arp is broadcast if unknown but a host may also verify the mac address by sending it to a unicast address
        if(Ethernet::ethTypeIsArp(_buffer, pacLen, _ipAddr)) {
            const size_t ansLel = Ethernet::MakeArpAnswerFromRequest(
                _buffer, pacLen, _macAddr, _ipAddr);
            packetSend(_buffer, ansLel);
            continue;
        }

        // check if the ip packet is for us
        if(!(Ethernet::ethTypeIsIp(_buffer, pacLen, _ipAddr))) {
            continue;
        }

        // ICMP Echo (ping)
        if(Ethernet::ethTypeIsIcmpEcho(_buffer, pacLen)) {
            const size_t ansLel = Ethernet::MakeIcmpEchoAnswerFromRequest(
                _buffer, pacLen, _macAddr, _ipAddr);
            packetSend(_buffer, ansLel);
            continue;
        }
    }
}