#ifndef _WIZFI210_H_
#define _WIZFI210_H_


#define WIZFI210_INIT_STATE_IDLE					1
#define WIZFI210_INIT_STATE_AT_SENDING			2
#define WIZFI210_INIT_STATE_AT_SENT				3
#define WIZFI210_INIT_STATE_ATE0_SENDING			4
#define WIZFI210_INIT_STATE_ATE0_SENT			5
#define WIZFI210_INIT_STATE_ATWD_SENDING		6
#define WIZFI210_INIT_STATE_ATWD_SENT			7
#define WIZFI210_INIT_STATE_ATWWEP1_SENDING	8
#define WIZFI210_INIT_STATE_ATWWEP1_SENT		9
#define WIZFI210_INIT_STATE_ATNDHCP_SENDING		10
#define WIZFI210_INIT_STATE_ATNDHCP_SENT			11
#define WIZFI210_INIT_STATE_ATNSET_SENDING		12
#define WIZFI210_INIT_STATE_ATNSET_SENT			13
#define WIZFI210_INIT_STATE_ATWAUTO_SENDING	14
#define WIZFI210_INIT_STATE_ATWAUTO_SENT		15
#define WIZFI210_INIT_STATE_ATNAUTO_SENDING		16
#define WIZFI210_INIT_STATE_ATNAUTO_SENT		17
#define WIZFI210_INIT_STATE_ATA_SENDING			18
#define WIZFI210_INIT_STATE_ATA_SENT				19
#define WIZFI210_INIT_STATE_COMPLETED			20
#define WIZFI210_INIT_STATE_REPLY_RCVD			21
#define WIZFI210_INIT_STATE_ATNCTCP_SENDING		22
#define WIZFI210_INIT_STATE_ATNCTCP_SENT			23
#define WIZFI210_INIT_STATE_ATNCLOSE_SENDING	24
#define WIZFI210_INIT_STATE_ATNCLOSE_SENT		25
#define WIZFI210_INIT_STATE_ATNCLOSEALL_SENDING	26
#define WIZFI210_INIT_STATE_ATNCLOSEALL_SENT	27
#define WIZFI210_INIT_STATE_WAIT_TIMEOUT			28


typedef struct UART_SET_TAG{
	unsigned char baudrate;
	unsigned char bitsperchar;
	unsigned char parity;
	unsigned char stopbits;
	unsigned char SWFlowCtrlFlag;
	unsigned char HWFlowCtrlFlag;
}UART_SET;

typedef struct WIZFI210_TIMEOUT_VALUES_TAG{
	unsigned short NetworkConnectionTimeout;
	unsigned short AutoAssociateTimeout;
	unsigned short TCPConnectionTimeout;
	unsigned short AssociationRetryCountTimeout;
	unsigned short NagleAlgorithmWaitTime;
	unsigned short ScanTime;
}WIZFI210_TIMEOUT_VALUES;

typedef struct WIZFI210_NETWORK_INFO_TAG{
	unsigned char SourceIPAddr[16];
	unsigned char SubnetMask[16];
	unsigned char GatewayIPAddr[16];
	unsigned short SourcePortNum;
	unsigned char PeerIPAddr[16];
	unsigned short PeerPortNum;
}WIZFI210_NETWORK_INFO;

typedef struct WIZFI210_CONNECTION_INFO_TAG{
	unsigned char SOCK_TYPE;			//TCP, UDP
	unsigned char CONNECTION_TYPE;	//Client, Server in WIZFI210
	unsigned char Current_CID;
}WIZFI210_CONNECTION_INFO;

typedef struct DATE_TAG{
	unsigned short Year;
	unsigned char Month;
	unsigned char Day;
}SYSTEM_DATE_STRUCT;

typedef struct TIME_TAG{
	unsigned char Hour;
	unsigned char Minute;
	unsigned char Second;
}SYSTEM_TIME_STRUCT;

typedef struct WIZFI210_INFO_TAG{
	unsigned char EchoFlag;
	unsigned char ResponseFlag;
	UART_SET UARTInfo;
	WIZFI210_TIMEOUT_VALUES TimeoutValues;
	unsigned char AdapterID;
	unsigned char MacAddr[18];
	unsigned char SSID[33];
	unsigned char BSSID[33];
	unsigned char Channel;
	int RSSI;
	unsigned char StationOperatingMode;
	unsigned char AuthenMode;
	unsigned char WEPKey[21];
	unsigned char WPAPassphrase[33];
	unsigned char RxEnable;
	unsigned char PowerSaveMode;
	unsigned char MulticastRxEnable;
	unsigned char TxPower;
	unsigned short SyncLossInterval;
	unsigned char ExternalPAEnable;
	unsigned char KeepAliveTimerInterval;
	unsigned char DHCPEnable;
	WIZFI210_NETWORK_INFO NetworkInfo;
	WIZFI210_CONNECTION_INFO ConnectionInfo;
	unsigned char DNSLookupURL[33];
	unsigned char DNSServerIPAddr[16];
	SYSTEM_DATE_STRUCT SystemDate;
	SYSTEM_TIME_STRUCT SystemTime;
}WIZFI210_INFO;

typedef struct APINFO_TAG{
	unsigned char BSSID[33];
	unsigned char SSID[33];
	unsigned char Channel[3];
	unsigned char Type[10];
	unsigned char RSSI[5];
	unsigned char Security[33];
	unsigned char nodata[3];
}APINFO;

enum {TO_NetworkConnection=0, TO_AutoAssociate=1, TO_TCPConnection=2, TO_NagleAlgorithmWait=4, TO_ScanTime=5};

enum {Authen_None=0, Authen_Open=1, Authen_SharedWEP=2};

enum {OP_AT = 1, OP_ATE = 2, OP_WS = 3, OP_WD = 4, OP_WAUTO = 5, OP_NAUTO = 6, OP_NDHCP = 7, OP_WWEP = 8, OP_WWPA = 9, 
		OP_ATA = 10, OP_NSET = 11, OP_ATCID = 12, OP_ATO = 13, OP_NCTCP = 14, OP_NCLOSE = 15, OP_NCLOSEALL = 16, OP_CMDMODE = 17};

enum {ORIGIN_CLIENT=0, ORIGIN_SERVER=1};

enum {PROTO_UDP=0, PROTO_TCP=1};

enum {TCP_DISCONNECT, TCP_CONNECT};

enum {AP_DISCONNECT, AP_CONNECT};

enum {RS232_SETTING, RS232_RUN};

void WIZFI210_APINFO_Init(void);
void WIZFI210_SetBSSID(unsigned char index, unsigned char* BSSID);
unsigned char * WIZFI210_GetBSSID(unsigned char index);
void WIZFI210_SetSSID(unsigned char index, unsigned char* SSID);
unsigned char * WIZFI210_GetSSID(unsigned char index);
void WIZFI210_SetChannel(unsigned char index, unsigned char* Channel);
unsigned char * WIZFI210_GetChannel(unsigned char index);
void WIZFI210_SetType(unsigned char index, unsigned char* Type);
unsigned char * WIZFI210_GetType(unsigned char index);
void WIZFI210_SetRSSI(unsigned char index, unsigned char* RSSI);
unsigned char * WIZFI210_GetRSSI(unsigned char index);
void WIZFI210_SetSecurity(unsigned char index, unsigned char* Security);
unsigned char * WIZFI210_GetSecurity(unsigned char index);
unsigned char WIZFI210_GetAPListCount();
void WIZFI210_SetAPListCount(unsigned char Count);
int WIZFI210_GetAPIndex(unsigned char * SSID);

void Init_WIZFI210(void);
unsigned char CheckReply_WIZFI210(void);
unsigned char Check_ModePin(void);
void Change_Command_Mode(void);
unsigned char Check_Network_Status(void);
unsigned char Check_AP_Association_Status(void);
void Site_Survey(void);
unsigned char Parse_Reply(void);
int GetToken(unsigned char * Token);
void MakeCommand_WIZFI210();

void Set_Current_WIZFI210_Init_State(unsigned char state);
unsigned char Get_Current_WIZFI210_Init_State(void);
void Set_Current_WIZFI210_Connect_State(unsigned char state);
unsigned char Get_Current_WIZFI210_Connect_State(void);

unsigned char Get_CID(void);
void Set_CID(unsigned char cid);
void Set_Current_CommandCode(unsigned char commandcode);

#endif
