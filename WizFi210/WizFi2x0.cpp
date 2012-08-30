#include "WizFi2x0.h"

////////////////////////////////
// 1. Set IO Pins direction
// 2. SPI Init
// 3. Reset WizFi2x0 
void WizFi2x0Class::init(void)
{
	// Set IO Pins Direction
	pinMode(WizFi2x0_DataReady, INPUT);
	pinMode(WizFi2x0_CS, OUTPUT);
	pinMode(WizFi2x0_RST, OUTPUT);

	// SPI Init
	SPI.setClockDivider(SPI_CLOCK_DIV8); // Max Clock Frequency
	SPI.begin();

	//Reset Wifi2x0
	digitalWrite(WizFi2x0_CS, HIGH);
	digitalWrite(WizFi2x0_RST, HIGH);

	digitalWrite(WizFi2x0_RST, LOW);
	delay(500);

	digitalWrite(WizFi2x0_RST, HIGH);
	delay(6000);

	
}

boolean WizFi2x0Class::Setup(void)
{
	byte MsgBuf[128];

}

uint8_t WizFi2x0Class::write(byte ch)
{
	if(ByteStuff(&ch))
	{
		digitalWrite(WizFi2x0_CS, LOW);
		SPI.transfer(spichar.SPI_ESC_CHAR);
		digitalWrite(WizFi2x0_CS, HIGH);
	}

	digitalWrite(WizFi2x0_CS, LOW);
	SPI.transfer(ch);
	digitalWrite(WizFi2x0_CS, HIGH);

	return 1;
}

uint8_t WizFi2x0Class::write(byte *buf, size_t length)
{
	return (uint8_t)length;
}

uint8_t WizFi2x0Class::read(void)
{
	uint8_t result;

	digitalWrite(WizFi2x0_CS, LOW);
	result = SPI.transfer(0);
	digitalWrite(WizFi2x0_CS, HIGH);

	return result;
}

uint8_t WizFi2x0Class::read(byte *buf)
{
}

boolean WizFi2x0Class::ByteStuff(byte *ch)
{
	if((*ch == spichar.SPI_ESC_CHAR) ||
		(*ch == spichar.SPI_XON_CHAR) ||
		(*ch == spichar.SPI_XOFF_CHAR) ||
		(*ch == spichar.SPI_IDLE_CHAR) ||
		(*ch == spichar.SPI_INVALID_CHAR_ALL_ONE) ||
		(*ch == spichar.SPI_INVALID_CHAR_ALL_ZERO) ||
		(*ch == spichar.SPI_LINK_READY) )
	{
		*ch = *ch^0x20;
		return true;
	}else
		return false;
}
