#ifndef _WIZFI2x0_H_
#define _WIZFI2x0_H_

#include <SPI.h>

#define WizFi2x0_RST			2
#define WizFi2x0_DataReady	3
#define WizFi2x0_CS			4

#define WizFi2x0_CmdState_IDLE	0x01
#define WizFi2x0_CmdState_Sent	0x02
#define WizFi2x0_CmdState_Rcvd	0x03

enum CMDOP {
	OP_AT = 1, 
	OP_ATE = 2, 
	OP_WS = 3, 
	OP_WD = 4, 
	OP_WAUTO = 5, 
	OP_NAUTO = 6, 
	OP_NDHCP = 7, 
	OP_WWEP = 8, 
	OP_WWPA = 9, 
	OP_ATA = 10, 
	OP_NSET = 11, 
	OP_ATCID = 12, 
	OP_ATO = 13, 
	OP_NCTCP = 14, 
	OP_NCLOSE = 15, 
	OP_NCLOSEALL = 16, 
	OP_CMDMODE = 17
};

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

class TimeoutClass
{
private:
	boolean IsTimerStarted;
	uint16_t TimerCount;
	uint16_t TimerValue;
	boolean IsTimeout;
public:
	void init(void);
	virtual void TimerStart(uint16_t timevalue);
	virtual void TimerStart(void);
	void TimerStop(void);
	boolean GetIsTimeout(void);	
	void CheckIsTimeout();
};

class WizFi2x0Class
{
public:
	byte MsgBuf[128];
	uint16_t RxIdx;
	uint8_t SendByte;
	
	SPIChar spichar;
	TimeoutClass ReplyCheckTimer;
	uint8_t Current_CmdState;
	uint8_t Current_Command_Code;
	uint16_t Current_Ptr;
private:
	
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
	uint8_t CheckReply(void);
	uint8_t CheckSyncReply(void);
	uint8_t ParseReply(byte *buf);
	void SendSync(void);
	int GetToken(uint8_t * Token);
	void SendCommand(void);

};

#endif
