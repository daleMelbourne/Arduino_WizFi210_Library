#include "wizfi210.h"
#include "stm32f10x_lib.h"
#include "config.h"
#include "DigitalIO.h"
#include <stdio.h>
#include <string.h>

WIZFI210_INFO Wifi_Config;

unsigned char APListCount;
APINFO APList[20];

unsigned char Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_IDLE;
unsigned char Current_WIZFI210_Connect_State = WIZFI210_INIT_STATE_IDLE;

unsigned char Current_Command_Code = OP_AT;
unsigned char SecurityType = 1;
unsigned char Key[33] = "ec56d9bfb3";
u8 SSID[33] = "benein";
unsigned char DHCP_Flag = 1;

u8 SrcIPAddr[16] = "192.168.123.100";
u8 SrcSubnet[16] = "255.255.255.0";
u8 SrcGateway[16] = "192.168.123.254";
u8 Connection_Type = 0;

u8 PeerIPAddr[16] = "192.168.123.144";
u8 PeerPortNum[6] = "5000";

u8 Origin_Type = ORIGIN_CLIENT;

u8 Socket_Type = PROTO_TCP;
u8 Current_CID;

extern u16 RXBUF_read[MAX_CH];
extern vu16 RXBUF_write[MAX_CH];

extern u16 TXBUF_Len[MAX_CH];
extern u16 TXBUF_TxCounter[MAX_CH]; 

extern u8 TX_BUF[MAX_CH*MAX_SBUF_SIZE];
extern u8 RX_BUF[MAX_CH*MAX_SBUF_SIZE];

extern u8 Reply_Buf[1024];
extern u16 Reply_Buf_Length;
extern u16 Current_Ptr;
extern u8 TmpToken[33];

extern u8 IsReplyTimeoutCheck;
extern u8 IsReplyTimeout;
extern u8 ReplyTimeoutValue;
extern u8 ReplyTimeoutCount;


void WIZFI210_APINFO_Init(void)
{
	memset(&APList[0].BSSID, 0, sizeof(APINFO)*20);
	APListCount = 0;
}

void WIZFI210_SetBSSID(unsigned char index, unsigned char* BSSID)
{
	memset(&APList[index].BSSID, 0, 33);
	memcpy(&APList[index].BSSID, BSSID, 33);

	printf("\r\n APList[%d].BSSID: %s", index, APList[index].BSSID);
}

unsigned char * WIZFI210_GetBSSID(unsigned char index)
{
	return APList[index].BSSID;
}

void WIZFI210_SetSSID(unsigned char index, unsigned char* SSID)
{
	memset(&APList[index].SSID, 0, 33);
	memcpy(&APList[index].SSID, SSID, 33);

	printf("\r\n APList[%d].SSID: %s", index, APList[index].SSID);
}

unsigned char * WIZFI210_GetSSID(unsigned char index)
{
	return APList[index].SSID;
}

void WIZFI210_SetChannel(unsigned char index, unsigned char* Channel)
{
	memset(&APList[index].Channel, 0, 33);
	memcpy(&APList[index].Channel, Channel, 33);

	printf("\r\n APList[%d].Channel: %s", index, APList[index].Channel);
}

unsigned char * WIZFI210_GetChannel(unsigned char index)
{
	return APList[index].Channel;
}

void WIZFI210_SetType(unsigned char index, unsigned char* Type)
{
	memset(&APList[index].Type, 0, 33);
	memcpy(&APList[index].Type, Type, 33);

	printf("\r\n APList[%d].Type: %s", index, APList[index].Type);
}

unsigned char * WIZFI210_GetType(unsigned char index)
{
	return APList[index].Type;
}

void WIZFI210_SetRSSI(unsigned char index, unsigned char* RSSI)
{
	memset(&APList[index].RSSI, 0, 33);
	memcpy(&APList[index].RSSI, RSSI, 33);

	printf("\r\n APList[%d].RSSI: %s", index, APList[index].RSSI);
}

unsigned char * WIZFI210_GetRSSI(unsigned char index)
{
	return APList[index].RSSI;
}

void WIZFI210_SetSecurity(unsigned char index, unsigned char* Security)
{
	memset(&APList[index].Security, 0, 33);
	memcpy(&APList[index].Security, Security, 33);

	printf("\r\n APList[%d].Security: %s", index, APList[index].Security);
}

unsigned char * WIZFI210_GetSecurity(unsigned char index)
{
	return APList[index].Security;
}

unsigned char WIZFI210_GetAPListCount()
{
	return APListCount;
}

void WIZFI210_SetAPListCount(unsigned char Count)
{
	APListCount = Count;
}

int WIZFI210_GetAPIndex(unsigned char * SSID)
{
	int i;

	for(i=0; i<APListCount; i++)
	{
		if(!strcmp(SSID, APList[i].SSID))
			return i;
	}

	return -1;
}

void Init_WIZFI210(void)
{
	unsigned char retval;
	
	switch(Current_WIZFI210_Init_State)
	{
		
	case WIZFI210_INIT_STATE_IDLE:
		//AT 명령을 전송한다.
		Current_Command_Code = OP_AT;
		MakeCommand_WIZFI210();
		Reply_Buf_Clear();

		SetIsWifiTimeout(0);
		SetWifiTimeoutCount(0);
		SetWifiTimeoutValue(2000);
		SetIsWifiTimeoutStart(1);
		
		break;
		
	case WIZFI210_INIT_STATE_AT_SENDING:
	case WIZFI210_INIT_STATE_ATE0_SENDING:
	case WIZFI210_INIT_STATE_ATWD_SENDING:
	case WIZFI210_INIT_STATE_ATWWEP1_SENDING:
	case WIZFI210_INIT_STATE_ATNDHCP_SENDING:
	case WIZFI210_INIT_STATE_ATNSET_SENDING	:
	case WIZFI210_INIT_STATE_ATWAUTO_SENDING:
	case WIZFI210_INIT_STATE_ATNAUTO_SENDING:
	case WIZFI210_INIT_STATE_ATA_SENDING:
		//Timeout Check
		if(GetIsWifiTimeout())
		{
			printf("\r\n Reply Timeout");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_WAIT_TIMEOUT;

			Current_Command_Code = OP_CMDMODE;
			MakeCommand_WIZFI210();
			Reply_Buf_Clear();

			SetIsWifiTimeout(0);
			SetWifiTimeoutCount(0);
			SetWifiTimeoutValue(2000);
			SetIsWifiTimeoutStart(1);		
		}
		break;
	case WIZFI210_INIT_STATE_WAIT_TIMEOUT:
		if(GetIsWifiTimeout())
		{
			printf("\r\n Command Mode Timeout");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_IDLE;			
		}
		break;
	case WIZFI210_INIT_STATE_AT_SENT:
		retval = CheckReply_WIZFI210();
		if(retval == 1)
		{
			printf("\r\n Reply [1]");
			Current_Command_Code = OP_ATE;
			MakeCommand_WIZFI210();
			Reply_Buf_Clear();

			SetIsWifiTimeout(0);
			SetWifiTimeoutCount(0);
			SetWifiTimeoutValue(2000);
			SetIsWifiTimeoutStart(1);
		}else if(retval == 2)
		{
			printf("\r\n Reply [2]");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_IDLE;
		}
		if(GetIsWifiTimeout())
		{
			printf("\r\n Reply Timeout");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_WAIT_TIMEOUT;

			Current_Command_Code = OP_CMDMODE;
			MakeCommand_WIZFI210();
			Reply_Buf_Clear();

			SetIsWifiTimeout(0);
			SetWifiTimeoutCount(0);
			SetWifiTimeoutValue(2000);
			SetIsWifiTimeoutStart(1);		
		}
		break;
	case WIZFI210_INIT_STATE_ATE0_SENT:
		retval = CheckReply_WIZFI210();
		if(retval == 1)
		{
			printf("\r\n Reply [1]");
			Current_Command_Code = OP_WD;
			MakeCommand_WIZFI210();
			Reply_Buf_Clear();

			SetIsWifiTimeout(0);
			SetWifiTimeoutCount(0);
			SetWifiTimeoutValue(2000);
			SetIsWifiTimeoutStart(1);
		}else if(retval == 2)
		{
			printf("\r\n Reply [2]");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_IDLE;
		}
		if(GetIsWifiTimeout())
		{
			printf("\r\n Reply Timeout");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_WAIT_TIMEOUT;

			Current_Command_Code = OP_CMDMODE;
			MakeCommand_WIZFI210();
			Reply_Buf_Clear();

			SetIsWifiTimeout(0);
			SetWifiTimeoutCount(0);
			SetWifiTimeoutValue(2000);
			SetIsWifiTimeoutStart(1);		
		}
		break;
	case WIZFI210_INIT_STATE_ATWD_SENT:
		retval = CheckReply_WIZFI210();
		if(retval == 1)
		{
			printf("\r\n Reply [1]");
			if(SecurityType == 0)
				Current_Command_Code = OP_NDHCP;
			else
				Current_Command_Code = OP_WWEP;
			MakeCommand_WIZFI210();
			Reply_Buf_Clear();

			SetIsWifiTimeout(0);
			SetWifiTimeoutCount(0);
			SetWifiTimeoutValue(2000);
			SetIsWifiTimeoutStart(1);
		}else if(retval == 2)
		{
			printf("\r\n Reply [2]");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_IDLE;
		}
		if(GetIsWifiTimeout())
		{
			printf("\r\n Reply Timeout");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_WAIT_TIMEOUT;

			Current_Command_Code = OP_CMDMODE;
			MakeCommand_WIZFI210();
			Reply_Buf_Clear();

			SetIsWifiTimeout(0);
			SetWifiTimeoutCount(0);
			SetWifiTimeoutValue(2000);
			SetIsWifiTimeoutStart(1);		
		}
		break;
	case WIZFI210_INIT_STATE_ATWWEP1_SENT:
		retval = CheckReply_WIZFI210();
		if(retval == 1)
		{
			printf("\r\n Reply [1]");
			Current_Command_Code = OP_NDHCP;
			MakeCommand_WIZFI210();
			Reply_Buf_Clear();

			SetIsWifiTimeout(0);
			SetWifiTimeoutCount(0);
			SetWifiTimeoutValue(2000);
			SetIsWifiTimeoutStart(1);
		}else if(retval == 2)
		{
			printf("\r\n Reply [2]");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_IDLE;
		}
		if(GetIsWifiTimeout())
		{
			printf("\r\n Reply Timeout");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_WAIT_TIMEOUT;

			Current_Command_Code = OP_CMDMODE;
			MakeCommand_WIZFI210();
			Reply_Buf_Clear();

			SetIsWifiTimeout(0);
			SetWifiTimeoutCount(0);
			SetWifiTimeoutValue(2000);
			SetIsWifiTimeoutStart(1);		
		}
		break;
	case WIZFI210_INIT_STATE_ATNDHCP_SENT:
		retval = CheckReply_WIZFI210();
		if(retval == 1)
		{
			printf("\r\n Reply [1]");
			if(!DHCP_Flag)
			{
				Current_Command_Code = OP_NSET;
				MakeCommand_WIZFI210();
				Reply_Buf_Clear();

				SetIsWifiTimeout(0);
				SetWifiTimeoutCount(0);
				SetWifiTimeoutValue(2000);
				SetIsWifiTimeoutStart(1);
			}else
			{
				Current_Command_Code = OP_WAUTO;
				MakeCommand_WIZFI210();
				Reply_Buf_Clear();

				SetIsWifiTimeout(0);
				SetWifiTimeoutCount(0);
				SetWifiTimeoutValue(2000);
				SetIsWifiTimeoutStart(1);
			}
		}else if(retval == 2)
		{
			printf("\r\n Reply [2]");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_IDLE;
		}
		if(GetIsWifiTimeout())
		{
			printf("\r\n Reply Timeout");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_WAIT_TIMEOUT;

			Current_Command_Code = OP_CMDMODE;
			MakeCommand_WIZFI210();
			Reply_Buf_Clear();

			SetIsWifiTimeout(0);
			SetWifiTimeoutCount(0);
			SetWifiTimeoutValue(2000);
			SetIsWifiTimeoutStart(1);		
		}
		break;
	case WIZFI210_INIT_STATE_ATNSET_SENT:
		retval = CheckReply_WIZFI210();
		if(retval == 1)
		{
			printf("\r\n Reply [1]");
			Current_Command_Code = OP_WAUTO;
			MakeCommand_WIZFI210();
			Reply_Buf_Clear();

			SetIsWifiTimeout(0);
			SetWifiTimeoutCount(0);
			SetWifiTimeoutValue(2000);
			SetIsWifiTimeoutStart(1);
		}else if(retval == 2)
		{
			printf("\r\n Reply [2]");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_IDLE;
		}
		if(GetIsWifiTimeout())
		{
			printf("\r\n Reply Timeout");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_WAIT_TIMEOUT;

			Current_Command_Code = OP_CMDMODE;
			MakeCommand_WIZFI210();
			Reply_Buf_Clear();

			SetIsWifiTimeout(0);
			SetWifiTimeoutCount(0);
			SetWifiTimeoutValue(2000);
			SetIsWifiTimeoutStart(1);		
		}
		break;
	case WIZFI210_INIT_STATE_ATWAUTO_SENT:
		retval = CheckReply_WIZFI210();
		if(retval == 1)
		{
			printf("\r\n Reply [1]");
			Current_Command_Code = OP_NAUTO;
			MakeCommand_WIZFI210();
			Reply_Buf_Clear();

			SetIsWifiTimeout(0);
			SetWifiTimeoutCount(0);
			SetWifiTimeoutValue(2000);
			SetIsWifiTimeoutStart(1);
		}else if(retval == 2)
		{
			printf("\r\n Reply [2]");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_IDLE;
		}
		if(GetIsWifiTimeout())
		{
			printf("\r\n Reply Timeout");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_WAIT_TIMEOUT;

			Current_Command_Code = OP_CMDMODE;
			MakeCommand_WIZFI210();
			Reply_Buf_Clear();

			SetIsWifiTimeout(0);
			SetWifiTimeoutCount(0);
			SetWifiTimeoutValue(2000);
			SetIsWifiTimeoutStart(1);		
		}
		break;
	case WIZFI210_INIT_STATE_ATNAUTO_SENT:
		retval = CheckReply_WIZFI210();
		if(retval == 1)
		{
			printf("\r\n Reply [1]");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_COMPLETED;
		}else if(retval == 2)
		{
			printf("\r\n Reply [2]");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_IDLE;
		}
		if(GetIsWifiTimeout())
		{
			printf("\r\n Reply Timeout");
			Current_WIZFI210_Init_State = WIZFI210_INIT_STATE_WAIT_TIMEOUT;

			Current_Command_Code = OP_CMDMODE;
			MakeCommand_WIZFI210();
			Reply_Buf_Clear();

			SetIsWifiTimeout(0);
			SetWifiTimeoutCount(0);
			SetWifiTimeoutValue(2000);
			SetIsWifiTimeoutStart(1);		
		}
		break;
	case WIZFI210_INIT_STATE_ATA_SENT:		
		break;
	case WIZFI210_INIT_STATE_REPLY_RCVD:
		break;
	case WIZFI210_INIT_STATE_COMPLETED:
		break;
	}


	//Timeout 발생 시, Command Mode로 진입

	// 2초간의 Wait를 준다.


	//Reply를 체크한다.

	//AT+WD 명령을 전송한다. Wifi Disassociate

	//Reply를 체크한다.

	//AT+WWEP1 = @Key 명령을 전송한다. Wifi Security Type

	//Reply를 체크한다.

	//AT+NDHCP=X, X=1 이면 Enable, X=0 이면 Disable

	//Reply를 체크한다.

	////////////////////////////////////////////
	//만약 DHCP 설정이 Disable이면 

	// 1. AT+NSET 명령을 전송한다.

	// Reply를 체크한다.

	//
	///////////////////////////////////////////

	//AT+WAUTO=0,[SSID],,0 명령을 전송한다. Wifi Connection Setting

	//Reply를 체크한다.

	//AT+NAUTO=[Origin_type],[Protocol],[Peer IP Address],[Peer Port Number]

	//Reply를 체크한다.

	//ATA 명령을 전송한다. Auto Connection

	//Reply를 체크한다.
}

unsigned char CheckReply_WIZFI210(void)
{
	u8 i, ch;
	u8 APListCount;
	
	
	
	if(RXBUF_read[WIFI_COMM_CH] != RXBUF_write[WIFI_COMM_CH])
	{
		ch = *(RX_BUF + MAX_SBUF_SIZE*WIFI_COMM_CH + RXBUF_read[WIFI_COMM_CH]++);
			printf("%c", ch);
		Reply_Buf[Reply_Buf_Length++] = ch;
		
		if(RXBUF_read[WIFI_COMM_CH] >= MAX_SBUF_SIZE)
			RXBUF_read[WIFI_COMM_CH] -= MAX_SBUF_SIZE;

		if(ch == ']')
		{
			if(Parse_Reply())
			{
				if(Current_Command_Code == OP_WS)
				{
					APListCount = WIZFI210_GetAPListCount();
					for(i=0; i<APListCount; i++)
						printf("\r\n SSID[%d] : %s", i, WIZFI210_GetSSID(i));
				}
				return 1;	//Reply OK
			}else
				return 2;	//Reply Fail
		}
	}

	return 0; // In Processing
}

unsigned char Check_ModePin(void)
{
	return 1;
}

void Change_Command_Mode(void)
{
}

unsigned char Check_Network_Status(void)
{
	if(GPIO_ReadInputDataBit(GPIOB, TCP_STATUS) == 0)
	{
		printf("\r\nTCP_STATUS : 0 => Connected");
		return TCP_CONNECT;
	}
	else
	{
		printf("\r\nTCP_STATUS : 1 => Disconnected");
		return TCP_DISCONNECT;
	}
}

unsigned char Check_AP_Association_Status(void)
{
	if(GPIO_ReadInputDataBit(GPIOB, AP_STATUS) == 0)
	{
		printf("\r\nAP_STATUS : 0 => Associated");
		return AP_CONNECT;
	}
	else
	{
		printf("\r\nAP_STATUS : 1 => Disassociated");
		return AP_DISCONNECT;
	}
}

void Site_Survey(void)
{
	u8 ret, len = 0;
	u8 IsNoReply = 0;


	WIZFI210_APINFO_Init();
	
//	memset(sock2_buf, 0, 1460);
	//AT command
	TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
	TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
	TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\r';
	TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\n';

	TXBUF_Len[WIFI_COMM_CH] = len;
	TXBUF_TxCounter[WIFI_COMM_CH] = 0;
	
	IsReplyTimeoutCheck = 1;
	IsReplyTimeout = 0;
	ReplyTimeoutCount = 0;
	ReplyTimeoutValue = 2;
	
	USART_ITConfig(USART3, USART_IT_TXE, ENABLE);


	printf("\r\n Reply : ");	
	Current_Command_Code = OP_AT;
	
	while(1)
	{
		ret = CheckReply_WIZFI210();

		if(ret == 1)
			break;

		if(IsReplyTimeout)
			IsNoReply = 1;
	}
//	wait_1ms(2000);
	
	if(IsNoReply)
	{
		len = 0;
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '+';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '+';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '+';

		TXBUF_Len[WIFI_COMM_CH] = len;
		TXBUF_TxCounter[WIFI_COMM_CH] = 0;
		
		USART_ITConfig(USART3, USART_IT_TXE, ENABLE);


		wait_1ms(2000);
		
	}

	//ATE0: Echo Disable
	len = 0;
	TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
	TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
	TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'E';
	TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '0';
	TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\r';
	TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++]= '\n';

	TXBUF_Len[WIFI_COMM_CH] = len;
	TXBUF_TxCounter[WIFI_COMM_CH] = 0;
	
	IsReplyTimeoutCheck = 1;
	IsReplyTimeout = 0;
	ReplyTimeoutCount = 0;
	ReplyTimeoutValue = 2;
	
	USART_ITConfig(USART3, USART_IT_TXE, ENABLE);

	
	printf("\r\n Reply : ");	
	Current_Command_Code = OP_ATE;
	
	while(1)
	{
		ret = CheckReply_WIZFI210();

		if(ret == 1)
			break;

		if(IsReplyTimeout)
			return;
	}

	//AT+WS: Site Survey
	len = 0;
	TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
	TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
	TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '+';
	TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'W';
	TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'S';
	TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\r';
	TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\n';
	
	TXBUF_Len[WIFI_COMM_CH] = len;
	TXBUF_TxCounter[WIFI_COMM_CH] = 0;
	
	IsReplyTimeoutCheck = 1;
	IsReplyTimeout = 0;
	ReplyTimeoutCount = 0;
	ReplyTimeoutValue = 20;
	
	USART_ITConfig(USART3, USART_IT_TXE, ENABLE);

	
	printf("\r\n Reply : ");	
	Current_Command_Code = OP_WS;
	
	while(1)
	{
		ret = CheckReply_WIZFI210();

		if(ret == 1)
			break;
		
		if(IsReplyTimeout)
			return;
	}
}

unsigned char Parse_Reply(void)
{
	int retval;
	u8 Token[33];
	u8 tmpToken[33];
	u8 APListIndex;
	
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
				printf("\r\n Token Length: %d, Token : %s", retval, Token);
				if(!strcmp((char const *)Token, "[OK]"))
					return 1;
				if(retval == -1)
					return 0;

			}
		}
	case OP_WS:

		Current_Ptr = 0;
		retval = GetToken(Token);

		while(1)
		{
			if(!strcmp((char const *)Token, "BSSIDSSIDChannelTypeRSSISecurity"))
			{
				retval = GetToken(Token);
				printf("\r\n Token Length: %d, Token : %s", retval, Token);
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
				printf("\r\n Token Length: %d, Token : %s", retval, Token);
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
			printf("\r\n Token Length: %d, Token : %s", retval, Token);			
			if(retval == -1)
				return 0;
		}
	}
	return 0;
}

int GetToken(unsigned char * Token)
{
	u8 i = 0;
	memset(Token, 0, 33);

	while(Current_Ptr < Reply_Buf_Length)
	{
		if(Reply_Buf[Current_Ptr] != ',' && Reply_Buf[Current_Ptr] != 0x0D && Reply_Buf[Current_Ptr] != 0x0A && Reply_Buf[Current_Ptr] != '\0')
		{
			if(Reply_Buf[Current_Ptr] != ' ' && Reply_Buf[Current_Ptr]!= '\t')
				Token[i++] = Reply_Buf[Current_Ptr++];
			else
				Current_Ptr++;
		}
		else
		{
			if(Reply_Buf[Current_Ptr] == ',')
			{
				Current_Ptr++;
				return i;
			}else if(Reply_Buf[Current_Ptr] == 0x0D)
			{
				if(Reply_Buf[Current_Ptr + 1] == 0x0A)
				{
					Current_Ptr += 2;
					return i;
				}else
				{
					Current_Ptr++;
					return i;
				}
			}else if(Reply_Buf[Current_Ptr] == 0x0A)
			{
				if(Reply_Buf[Current_Ptr + 1] == 0x0D)
				{
					Current_Ptr += 2;
					return i;
				}else
				{
					Current_Ptr++;
					return i;
				}
			}else if(Reply_Buf[Current_Ptr] == '\0')
			{
				Current_Ptr++;
				return i;
			}
		}
	}

	return -1;
}

void MakeCommand_WIZFI210(void)
{
	unsigned char i, len = 0;

	memset(TX_BUF, WIFI_COMM_CH*MAX_SBUF_SIZE, MAX_SBUF_SIZE);
	
	switch(Current_Command_Code)
	{
	case OP_AT:
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\r';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\n';

		Set_Current_WIZFI210_Init_State(WIZFI210_INIT_STATE_AT_SENDING);
		printf("\r\n AT command");
		break;
	case OP_ATE:
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'E';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '0';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\r';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\n';

		Set_Current_WIZFI210_Init_State(WIZFI210_INIT_STATE_ATE0_SENDING);
		printf("\r\n ATE0 command");
		break;
	case OP_WD:
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '+';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'W';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'D';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\r';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\n';

		Set_Current_WIZFI210_Init_State(WIZFI210_INIT_STATE_ATWD_SENDING);
		printf("\r\n AT+WD command");
		break;
	case OP_WWEP:
		if(SecurityType == 1)
		{
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '+';
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'W';
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'W';
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'E';
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'P';
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '1';
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '=';
			for(i=0; Key[i] != '\0'; i++)
				TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = Key[i];				
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\r';
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\n';
			printf("\r\n AT+WWEP1= command");
		}else if(SecurityType == 2)
		{
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '+';
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'W';
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'W';
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'P';
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '=';
			for(i=0; Key[i] != '\0'; i++)
				TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = Key[i];				
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\r';
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\n';
			printf("\r\n AT+WWPA= command");
		}else
		{
			return;
		}

		Set_Current_WIZFI210_Init_State(WIZFI210_INIT_STATE_ATWWEP1_SENDING);
		break;
	case OP_NDHCP:
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '+';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'N';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'D';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'H';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'C';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'P';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '=';
		if(DHCP_Flag)
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '1';
		else
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '0';			
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\r';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\n';

		printf("\r\n AT+NDHCP= command");
		Set_Current_WIZFI210_Init_State(WIZFI210_INIT_STATE_ATNDHCP_SENDING);
		break;
	case OP_NSET:
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '+';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'N';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'S';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'E';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '=';
		for(i=0; SrcIPAddr[i]!='\0'; i++)
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = SrcIPAddr[i];
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = ',';
		for(i=0; SrcSubnet[i]!='\0'; i++)
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = SrcSubnet[i];
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = ',';
		for(i=0; SrcGateway[i]!='\0'; i++)
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = SrcGateway[i];
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\r';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\n';

		printf("\r\n AT+NSET= command");
		Set_Current_WIZFI210_Init_State(WIZFI210_INIT_STATE_ATNSET_SENDING);
		break;
	case OP_WAUTO:
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '+';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'W';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'U';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'O';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '=';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '0';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = ',';
		for(i=0; SSID[i]!='\0'; i++)
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = SSID[i];
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = ',';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = ',';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '0';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\r';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\n';

		printf("\r\n AT+WAUTO= command");
		Set_Current_WIZFI210_Init_State(WIZFI210_INIT_STATE_ATWAUTO_SENDING);
		break;
	case OP_NAUTO:
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '+';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'N';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'U';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'O';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '=';
		if(Connection_Type == 0)
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '0';
		else
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '1';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = ',';
		if(Socket_Type == PROTO_UDP)
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '0';
		else
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '1';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = ',';
		for(i=0; PeerIPAddr[i]!='\0'; i++)
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = PeerIPAddr[i];
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = ',';
		for(i=0; PeerPortNum[i]!='\0'; i++)
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = PeerPortNum[i];
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\r';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\n';

		printf("\r\n AT+NAUTO= command");
		Set_Current_WIZFI210_Init_State(WIZFI210_INIT_STATE_ATNAUTO_SENDING);
		break;
	case OP_ATA:
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\r';

		printf("\r\n ATA command");
		Set_Current_WIZFI210_Connect_State(WIZFI210_INIT_STATE_ATA_SENDING);
		break;
	case OP_NCTCP:
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '+';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'N';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'C';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'C';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'P';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '=';
		for(i=0; PeerIPAddr[i]!='\0'; i++)
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = PeerIPAddr[i];
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = ',';
		for(i=0; PeerPortNum[i]!='\0'; i++)
			TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = PeerPortNum[i];
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\r';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\n';

		printf("\r\n AT+NCTCP= command");
		Set_Current_WIZFI210_Connect_State(WIZFI210_INIT_STATE_ATNCTCP_SENDING);
		break;
	case OP_NCLOSE:
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '+';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'N';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'C';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'L';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'O';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'S';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'E';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '=';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '1';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\r';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\n';

		printf("\r\n AT+NCLOSE= command");
		Set_Current_WIZFI210_Connect_State(WIZFI210_INIT_STATE_ATNCLOSE_SENDING);
		break;
	case OP_NCLOSEALL:
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'T';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '+';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'N';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'C';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'L';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'O';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'S';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'E';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'A';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'L';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = 'L';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\r';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '\n';

		printf("\r\n AT+NCLOSEALL command");
		Set_Current_WIZFI210_Connect_State(WIZFI210_INIT_STATE_ATNCLOSEALL_SENDING);
		break;
	case OP_CMDMODE:
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '+';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '+';
		TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + len++] = '+';

		printf("\r\n +++ command");	
	}

	RXBUF_read[WIFI_COMM_CH] = RXBUF_write[WIFI_COMM_CH] = 0;
	TXBUF_Len[WIFI_COMM_CH] = len;
	TXBUF_TxCounter[WIFI_COMM_CH] = 0;

	printf("\r\nSending Data: \r\n");
	for(i=0; i<len; i++)
	{
		printf("%c", TX_BUF[WIFI_COMM_CH*MAX_SBUF_SIZE + i]);
	}
	
	USART_ITConfig(USART3, USART_IT_TXE, ENABLE);

}

void Set_Current_WIZFI210_Init_State(unsigned char state)
{
	Current_WIZFI210_Init_State = state;

#ifdef DEBUG_MSG
	switch(Get_Current_WIZFI210_Init_State())
	{
	case WIZFI210_INIT_STATE_IDLE:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_IDLE");
		break;
	case WIZFI210_INIT_STATE_ATA_SENDING:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATA_SENDING");
		break;
	case WIZFI210_INIT_STATE_ATA_SENT:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATA_SENT");
		break;
	case WIZFI210_INIT_STATE_ATE0_SENDING:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATE0_SENDING");
		break;
	case WIZFI210_INIT_STATE_ATE0_SENT:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATE0_SENT");
		break;
	case WIZFI210_INIT_STATE_ATWD_SENDING:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATWD_SENDING");
		break;
	case WIZFI210_INIT_STATE_ATWD_SENT:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATWD_SENT");
		break;
	case WIZFI210_INIT_STATE_ATWWEP1_SENDING:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATWWEP1_SENDING");
		break;
	case WIZFI210_INIT_STATE_ATWWEP1_SENT:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATWWEP1_SENT");
		break;
	case WIZFI210_INIT_STATE_ATNDHCP_SENDING:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATNDHCP_SENDING");
		break;
	case WIZFI210_INIT_STATE_ATNDHCP_SENT:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATNDHCP_SENT");
		break;
	case WIZFI210_INIT_STATE_ATNSET_SENDING	:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATNSET_SENDING");
		break;
	case WIZFI210_INIT_STATE_ATNSET_SENT:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATNSET_SENT");
		break;
	case WIZFI210_INIT_STATE_ATWAUTO_SENDING:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATWAUTO_SENDING");
		break;
	case WIZFI210_INIT_STATE_ATWAUTO_SENT:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATWAUTO_SENT");
		break;
	case WIZFI210_INIT_STATE_ATNAUTO_SENDING:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATNAUTO_SENDING");
		break;
	case WIZFI210_INIT_STATE_ATNAUTO_SENT:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATNAUTO_SENT");
		break;
	case WIZFI210_INIT_STATE_COMPLETED:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_COMPLETED");
		break;
	case WIZFI210_INIT_STATE_REPLY_RCVD	:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_REPLY_RCVD");
		break;
	case WIZFI210_INIT_STATE_ATNCTCP_SENDING:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATNCTCP_SENDING");
		break;
	case WIZFI210_INIT_STATE_ATNCTCP_SENT:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATNCTCP_SENT");
		break;
	case WIZFI210_INIT_STATE_ATNCLOSE_SENDING:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATNCLOSE_SENDING");
		break;
	case WIZFI210_INIT_STATE_ATNCLOSE_SENT:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATNCLOSE_SENT");
		break;
	case WIZFI210_INIT_STATE_ATNCLOSEALL_SENDING:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATNCLOSEALL_SENDING");
		break;
	case WIZFI210_INIT_STATE_ATNCLOSEALL_SENT:
		printf("\r\nCurrent_WIZFI210_Init_State: WIZFI210_INIT_STATE_ATNCLOSEALL_SENT");
		break;		
	}
#endif
}

unsigned char Get_Current_WIZFI210_Init_State(void)
{
	return Current_WIZFI210_Init_State;
}

void Set_Current_WIZFI210_Connect_State(unsigned char state)
{
	Current_WIZFI210_Connect_State = state;
}

unsigned char Get_Current_WIZFI210_Connect_State(void)
{
	return Current_WIZFI210_Connect_State;
}

unsigned char Get_CID(void)
{
	return Current_CID;
}

void Set_CID(unsigned char cid)
{
	Current_CID = cid;
}

void Set_Current_CommandCode(unsigned char commandcode)
{
	Current_Command_Code = commandcode;
}


