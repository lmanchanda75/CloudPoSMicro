/*
Purpose - The purpose of the kennel is to represent functionality of the 
Kernel, it needs to do the same by calling the APIs from the cloud and 
taking some of the decisions locally on behalf of the remote kernel.

The kernel functionality here is expected to do the following

1. Analyse the data from the card
2. Keep the data as necessary locally 
3. Send the data from the card to the cloud
4. Be ready with the next set of commands which need to sent to card
5. Provide the state of processing to terminal, which can further use that  state for its functions.

*/

#ifndef __KERNEL_H__
#define __KERNEL_H__


#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "Card.h"
#include "Debug.h" 

//#define SECURE 
 #define SERVER_HOST "192.168.1.31"
 #define SERVER_PORT 8080
 

#define initURI "/payment/test/init"
#define activateURI "/payment/activate"
#define genericURI "/payment/command"
#define authorizeURI "/payment/authorization"

#define INIT_BODY_SIZE 1024
#define GENERIC_BODY_SIZE 2048
#define REQUEST_BODY_SIZE 1024
#define MAX_FUNCTIONS_IN_FLOW 10
#define MAX_SCHEME_NAME_SIZE 20

extern char autherisationToken[];

#ifndef DEBUG_DISABLED // Only if debug is not disabled (for production/release)
extern RemoteDebug Debug;
#endif

struct SupportedKernel{
	char kernelCode[3];
	char scheme[MAX_SCHEME_NAME_SIZE];

};

class Kernel;
class Card;
typedef bool (Kernel::*KernelFunction)(Card &,uint8_t & errorCode);

class Kernel{

private:
  
	WiFiClient * client;
	char *sessionId;
	char currentKenel[3];
	uint8_t kernelCmdIndex;
	uint8_t kernelCmdEndIndex;
	struct SupportedKernel supportedKernels[3] = {{.kernelCode="00",.scheme="UnDefined"},{.kernelCode="02",.scheme="MasterCard"},{.kernelCode="03",.scheme="Visa"}};
	
	bool activateRequest(Card & card,uint8_t & errorCode);
  bool genericAPIRequest(Card & card,uint8_t & errorCode);
	
	KernelFunction kernelFunctions [3][MAX_FUNCTIONS_IN_FLOW] ={{NULL,NULL,NULL},{NULL,&Kernel::activateRequest,&Kernel::genericAPIRequest},{NULL,&Kernel::activateRequest,&Kernel::genericAPIRequest}};//Same order as in supported kernels

	uint8_t kernelIndex;
	void putFunction(KernelFunction funcPtr);
  bool sendHTTP(char* requestURI, uint8_t* requestBody, uint16_t requestLength,bool ifHttps, String & responsePayload,uint8_t & errorCode);//Send the request to cloudKernel
	

public:

	Kernel();
	void cleanUp(); //Clean the context after transction is over
	bool kernelSelection();
	
	bool initRequest(uint8_t & errorCode,uint16_t txnAmount);

	bool nextKernelCommand(Card &,uint8_t & errorCode);

  bool checkAuthorization(uint8_t & errorCode,char *authorisationCodeOut,char *transactionOutcomeOut,char * transactionIDOut);

};



#endif
