/*
 WizFi210 Arduino test code
 
 Circuit:
 WizFi210 connected to Arduino via SPI
 
 RST: pin 2  // Output
 DRDY: pin 3  // Input
 CSB: pin 4  // output

 MOSI: pin 11  // output
 MISO: pin 12
 SCK: pin 13  // out
 
 Created 15 May 2012
 by Jinbuhm Kim  (jbkim@wiznet, Jinbuhm.Kim@gmail.com)

 */

// WizFi210 communicates using SPI, so include the SPI library:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SPI.h>
#include <WizFi2x0.h>
#include <TimerOne.h>

byte IsTimer1Expired = 0;
uint16_t CurrentTime = 0;

#define DHCP_ENABLE

#define SSID 		"WIZnetCisco"        // SSID of your AP
#define WPA_PASS 	"0557564860"         // WPA Password

#define Infrastructure	0
#define Ad_hoc		1

#define Client_Mode	0
#define Server_Mode	1

#define Protocol_UDP	0
#define Protocol_TCP	1

// for static IP address
unsigned char IP[4] 		= {192, 168, 88, 123};
unsigned char  Sub[4] 	        = {255, 255, 255, 0};
unsigned char  Gw[4] 		= {192, 168, 88, 1};
unsigned char  SIP[4] 	        = {192, 168, 88, 228};
unsigned int ListenPort = 5000;
unsigned int ServerPort = 5000;

WizFi2x0Class myWizFi;
SPIChar spichar;

// pins used for the connection with the WizFi210

boolean Wifi_setup = false;

///////////////////////////////
// 1msec Timer
void Timer1_ISR()
{
  myWizFi.ReplyCheckTimer.CheckIsTimeout();
}
//
//////////////////////////////

void setup() {
  byte key, retval, i;
  byte retry_count = 0;
  
  Serial.begin(9600);
  Serial.println("\r\nSerial Init");
  
  
  // initalize the  data ready and chip select pins:
  myWizFi.init();
 
  
   // Timer1 Initialize
  Timer1.initialize(1000); // 1msec
  Timer1.attachInterrupt(Timer1_ISR);
 
  myWizFi.SendSync();
  myWizFi.ReplyCheckTimer.TimerStart(1000);
  
  Serial.println("Send Sync data");
  
  while(1)
  {
    if(myWizFi.CheckSyncReply())
    {
      myWizFi.ReplyCheckTimer.TimerStop();
      break;
    }
    if(myWizFi.ReplyCheckTimer.GetIsTimeout())
      break;
  }
  
  Serial.println("Rcvd Sync data");
  
  while(1)
  {
    if(myWizFi.Current_CmdState == WizFi2x0_CmdState_IDLE)
    {
      memset(myWizFi.MsgBuf, 0, sizeof(myWizFi.MsgBuf));
      sprintf((char *)myWizFi.MsgBuf, "AT\r");
      myWizFi.SendByte = 3;
      
      myWizFi.SendCommand();
      Serial.println("Sent AT");
    }else if(myWizFi.Current_CmdState == WizFi2x0_CmdState_Sent)
    {
      retval = myWizFi.CheckReply();
      if(retval == 1)
      {
        PrintRCVDData();
        Serial.println("OK rcvd");
        myWizFi.Current_CmdState = WizFi2x0_CmdState_Rcvd;
        myWizFi.ReplyCheckTimer.TimerStop();
      }else if(retval == 2)
      {
        PrintRCVDData();
        Serial.println("ERROR rcvd");
        myWizFi.ReplyCheckTimer.TimerStop();
        retry_count++;
        if(retry_count > 2)
        {
          break;
        }else
        {
          myWizFi.Current_CmdState = WizFi2x0_CmdState_IDLE;
        }
      }
      if(myWizFi.ReplyCheckTimer.GetIsTimeout())
      {
        Serial.println("NO valid reply. Timeout!!");
        myWizFi.ReplyCheckTimer.TimerStop();
        retry_count++;
        
        if(retry_count > 2)
        {
          break;
        }else
        {
          myWizFi.Current_CmdState = WizFi2x0_CmdState_IDLE;
        }
      }
    }else if(myWizFi.Current_CmdState == WizFi2x0_CmdState_Rcvd)
    {
      break;
    }
  }
  
  Serial.println("Wi-Fi Init Complete");
}

void loop() {
}

void PrintRCVDData(void)
{
      Serial.print("RCVD Data: ");
      Serial.println((char *)(myWizFi.MsgBuf));
      Serial.println("");
}

