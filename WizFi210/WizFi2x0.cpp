#include <stdio.h>
#include <string.h>
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
	delay(3000);

	ReplyCheckTimer.init();

	Current_CmdState = WizFi2x0_CmdState_IDLE;

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
	uint16_t i;

	for(i=0; i<(uint16_t)length; i++)
	{
		write(buf[i]);
		//delay(10);
	}

	memset((char *)MsgBuf, 0, sizeof(MsgBuf));
	
	return (uint8_t)i;
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

void WizFi2x0Class::SendCommand()
{
	write(MsgBuf, SendByte);
	RxIdx = 0;
	ReplyCheckTimer.TimerStart(6000);
	Current_CmdState = WizFi2x0_CmdState_Sent;
	Current_Command_Code = OP_AT;
}

uint8_t WizFi2x0Class::CheckReply(void)
{
	byte tmp;

	tmp = read();
	if((tmp != spichar.SPI_IDLE_CHAR) && (tmp != spichar.SPI_INVALID_CHAR_ALL_ZERO) && (tmp != spichar.SPI_INVALID_CHAR_ALL_ONE))
	{
		MsgBuf[RxIdx++] = tmp;
	}

	if(tmp == ']')
	{
		/////////////////////////////////////////	
		//Flush rx data
		while(digitalRead(WizFi2x0_DataReady) == HIGH)
		{
			tmp = read();
		}
		//
		////////////////////////////////////////
		
		if(ParseReply(MsgBuf))
			return 1; // Reply is OK
		else
			return 2; // Reply is FAILED
	}

	return 0; // Reply is in progress
	
}

uint8_t WizFi2x0Class::ParseReply(byte * buf)
{
	int retval;
	uint8_t Token[33];
	uint8_t tmpToken[33];
	uint8_t APListIndex;
	
	switch(Current_Command_Code)
	{
	case OP_AT: 
	case OP_ATE:
	case OP_WD:
	case OP_WWEP:
	case OP_WWPA:
	case OP_NDHCP:
	case OP_WAUTO:
	case OP_NAUTO:
	case OP_NSET:
	case OP_ATO:
		Current_Ptr = 0;
		retval = GetToken(Token);

		while(1)
		{
			if(!strcmp((char const *)Token, "[OK]"))
				return 1;
			else
			{
				retval = GetToken(Token);
//				printf("\r\n Token Length: %d, Token : %s", retval, Token);
				if(!strcmp((char const *)Token, "[OK]"))
					return 1;
				if(retval == -1)
					return 0;

			}
		}
	case OP_WS:

		Current_Ptr = 0;
		retval = GetToken(Token);

		return 1;
#if 0
		while(1)
		{
			if(!strcmp((char const *)Token, "BSSIDSSIDChannelTypeRSSISecurity"))
			{
				retval = GetToken(Token);
//				printf("\r\n Token Length: %d, Token : %s", retval, Token);
				memset(tmpToken, 0, 33);
				memcpy(tmpToken, Token, 13);
				APListIndex = WIZFI210_GetAPListCount();
				while(strcmp((char const *)tmpToken, "No.OfAPFound:")!=0)
				{
//					printf("\r\nBSSI: %s", Token);
					WIZFI210_SetBSSID(APListIndex, Token);
					
					retval = GetToken(Token);
//					printf("\tSSID: %s", Token);
					WIZFI210_SetSSID(APListIndex, Token);
					
					retval = GetToken(Token);
//					printf("\t\tChannel: %s", Token);
					WIZFI210_SetChannel(APListIndex, Token);
					
					retval = GetToken(Token);
//					printf("\tType: %s", Token);
					WIZFI210_SetType(APListIndex, Token);

					retval = GetToken(Token);
//					printf("\tRSSI: %s", Token);
					WIZFI210_SetRSSI(APListIndex, Token);

					retval = GetToken(Token);
//					printf("\t\tSecurity: %s", Token);
					WIZFI210_SetSecurity(APListIndex, Token);

					APListIndex++;
					WIZFI210_SetAPListCount(APListIndex);
					
					retval = GetToken(Token);
					memset(tmpToken, 0, 33);
					memcpy(tmpToken, Token, 13);
				}
				retval = GetToken(Token);
//				printf("\r\n %s", Token);
				if(!strcmp((char const *)Token, "[OK]"))
					return 1;
				else
					return 0;
			}else
			{
				retval = GetToken(Token);
				if(retval == -1)
					return 0;
			}
		}
#endif
	case OP_ATA:
		Current_Ptr = 0;
		retval = GetToken(Token);
//		printf("\r\n Token Length: %d, Token : %s", retval, Token);

		while(1)
		{
			if(!strcmp((char const *)Token, "[OK]"))
			{
				return 1;
			}
			else
			{
				memset(tmpToken, 0, 33);
				memcpy(tmpToken, Token, 7);
				if(!strcmp((char const *)tmpToken, "[ERROR]"))
					return 0;
				retval = GetToken(Token);
//				printf("\r\n Token Length: %d, Token : %s", retval, Token);
				if(!strcmp((char const *)Token, "[OK]"))
				{
					return 1;
				}
				if(retval == -1)
					return 0;
			}
		}
	case OP_ATCID:
		Current_Ptr = 0;
		retval = GetToken(Token);

		while(1)
		{
			if(!strcmp((char const *)Token, "NovalidCids"))
			{				
				return 1;
			}
			else if(!strcmp((char const *)Token, "CIDTYPEMODELOCALPORTREMOTEPORTREMOTEIP"))
			{
				return 1;
			}
			retval = GetToken(Token);
//			printf("\r\n Token Length: %d, Token : %s", retval, Token);			
			if(retval == -1)
				return 0;
		}
	}
	return 0;
}

int WizFi2x0Class::GetToken(uint8_t * Token)
{
	uint8_t i = 0;
	memset(Token, 0, 33);

	while(Current_Ptr < RxIdx)
	{
		if(MsgBuf[Current_Ptr] != ',' && MsgBuf[Current_Ptr] != 0x0D && MsgBuf[Current_Ptr] != 0x0A && MsgBuf[Current_Ptr] != '\0')
		{
			if(MsgBuf[Current_Ptr] != ' ' && MsgBuf[Current_Ptr]!= '\t')
				Token[i++] = MsgBuf[Current_Ptr++];
			else
				Current_Ptr++;
		}
		else
		{
			if(MsgBuf[Current_Ptr] == ',')
			{
				Current_Ptr++;
				return i;
			}else if(MsgBuf[Current_Ptr] == 0x0D)
			{
				if(MsgBuf[Current_Ptr + 1] == 0x0A)
				{
					Current_Ptr += 2;
					return i;
				}else
				{
					Current_Ptr++;
					return i;
				}
			}else if(MsgBuf[Current_Ptr] == 0x0A)
			{
				if(MsgBuf[Current_Ptr + 1] == 0x0D)
				{
					Current_Ptr += 2;
					return i;
				}else
				{
					Current_Ptr++;
					return i;
				}
			}else if(MsgBuf[Current_Ptr] == '\0')
			{
				Current_Ptr++;
				return i;
			}
		}
	}

	return -1;
}

uint8_t WizFi2x0Class::CheckSyncReply(void)
{
	byte key;

	key = read();
	if((key != spichar.SPI_IDLE_CHAR) && ( key != spichar.SPI_INVALID_CHAR_ALL_ZERO) && (key != spichar.SPI_INVALID_CHAR_ALL_ONE))
	{
		;
	}

	if(digitalRead(WizFi2x0_DataReady) == LOW)
		return 1;

	delay(10); // wait during 10ms
	
	return 0;
}

void WizFi2x0Class::SendSync(void)
{
	write(spichar.SPI_IDLE_CHAR);	
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


void TimeoutClass::init(void)
{
	TimerValue = 0;
	IsTimeout = false;
	TimerCount = 0;
	IsTimerStarted = false;
}


void TimeoutClass::TimerStart(uint16_t timevalue)
{
	TimerValue = timevalue;
	TimerStart();
}

void TimeoutClass::TimerStart(void)
{
	IsTimeout = false;
	TimerCount = 0;
	IsTimerStarted = true;
}

void TimeoutClass::TimerStop(void)
{
	IsTimerStarted = false;
}

boolean TimeoutClass::GetIsTimeout(void)
{
	return IsTimeout;
}

void TimeoutClass::CheckIsTimeout(void)
{
	if(IsTimerStarted)
	{
		if(TimerCount++ >= TimerValue)
		{
			IsTimeout = true;
			TimerStop();
		}
	}
}


