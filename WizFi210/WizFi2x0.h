#ifndef _WIZFI2x0_H_
#define _WIZFI2x0_H_

#include <SPI.h>

#define WizFi2x0_RST			2
#define WizFi2x0_DataReady	3
#define WizFi2x0_CS			4

class SPIChar
{
public:
	static const byte SPI_ESC_CHAR = 0xFB;
	static const byte SPI_IDLE_CHAR = 0xF5;
	static const byte SPI_XOFF_CHAR = 0xFA;
	static const byte SPI_XON_CHAR = 0xFD;
	static const byte SPI_INVALID_CHAR_ALL_ONE = 0xFF;
	static const byte SPI_INVALID_CHAR_ALL_ZERO = 0x00;
	static const byte SPI_LINK_READY = 0xF3;
};

class WizFi2x0Class
{
public:
	byte MsgBuf[128];
	SPIChar spichar;
public:
	void init(void);
	boolean Setup(void);
//	uint8_t Connect(IPAddress ip, uint16_t port);
//	uint8_t Disconnect();
//	uint8_t IsConnected();
	virtual uint8_t write(byte ch);
	virtual uint8_t write(byte *buf, size_t length);
	virtual uint8_t read();
	virtual uint8_t read(byte *buf);
	boolean ByteStuff(byte *str);
};

#endif
