/*
Purpose - The purpose of Card is to represent the contactless card , its stata and 
its data.

Following are functions -

1. Identify when the card is in the field and goes out of field
2. Interface with the card and send the commands as identified by the kernel
3. Handle response from card and communicate that to the kernel


*/


#ifndef __CARD_H__
#define __CARD_H__


#include "Display.h"
#include "Kernel.h"
#include "Pins.h"

#define ALTERNATE_PINS

  

#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>
#include "tlv.h"
#include "hashtable.h"
#include "emvTagList.h"
#include "Debug.h" 

#define MAX_FUNCTIONS_IN_FLOW 10
#define CALL_MEMBER_FN(object,ptrToMember)  ((object)->*(ptrToMember))
#define MAX_CARD_DATA    1024
#define CMD_LEN          128
#define MAX_AID_LEN      16


class Card;
class Kernel;

typedef bool (Card::*CardFunction)(Kernel &,bool&,uint8_t & errorCode);

struct GenericCmdProp{
 uint8_t cmd[CMD_LEN];
 uint8_t cmdLength;
 bool appendResponse;
};

#ifndef DEBUG_DISABLED // Only if debug is not disabled (for production/release)

extern RemoteDebug Debug;
#endif

class Card{

private:
	Display  * display;
	uint8_t cardCmdIndex;
	uint8_t cardCmdEndIndex;
	PN532_SPI * pn532_bus;
	PN532  * nfc;
  SPIClass * alternateSPI;

	uint8_t data[MAX_CARD_DATA];
  dict_t *dict[HASHSIZE];
  uint8_t aid[MAX_AID_LEN] ;
  uint8_t sw1sw2[2];
  uint16_t dataLength;
  uint8_t cmdLength;
  unsigned long dataPosition ;


	uint8_t selectPPSECmd[20] = {0x00,                                     /* CLA */
                          0xA4,                                     /* INS */
                          0x04,                                     /* P1  */
                          0x00,                                     /* P2  */
                          0x0e,                                     /* Length of AID  */
                          0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, /* 2PAY.SYS.DDF01 */ /*TODO - Create EMVCommands Class*/
                          0x00 /* Le  */};

	 uint8_t selectApplicationCmd[4] = {0x00,                                     /* CLA */
                        0xA4,                                     /* INS */
                        0x04,                                     /* P1  */
                        0x00                                     /* P2  */
                        };  
            

	
  bool selectPPSE(Kernel &,bool &,uint8_t & errorCode);
  bool selectApplication(Kernel &,bool& ,uint8_t & errorCode);
  bool genericCardRequest(Kernel & kernel,bool & lastCmd,uint8_t & errorCode);
	
	CardFunction cardFunctions [MAX_FUNCTIONS_IN_FLOW] ={&Card::selectPPSE,&Card::selectApplication};
	GenericCmdProp nextCmd [MAX_FUNCTIONS_IN_FLOW];

	
	uint8_t getSW1();
	uint8_t getSW2();
	void parseAndStoreResponse(tlvInfo_t *t,int index);
	void setAid(uint8_t * aid,uint8_t len);
  
  uint8_t aidLen;
  void putData(uint8_t *  response, uint8_t length,bool append);
  void putData( uint8_t length,bool append);



public:
	Card(Display & display);
  bool initNFCMod();
	void cleanUp();
	bool stopRF();
	bool startRF();

	bool initCard(uint8_t & errorCode);
	
  uint8_t getAid(uint8_t * val);
	bool nextCardCommand(Kernel &,bool & lastCmd,uint8_t & errorCode);
	uint8_t * getData(uint16_t & lengthOut,bool withStatusWord,unsigned long & positionArray);
  void putCmd(const char *  cmd, uint8_t length,bool appendResponse); 


};

class Utils{

  public:

  static char convertCharToHex(char ch);
  
};

#endif
