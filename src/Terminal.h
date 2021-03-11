/*
Purpose - The purpose of Terminal is to be the orchestrator among various 
entities in cloudPoS and deliver the funcationality. 

Terminal is expected to function in 2 separate tasks, one to hand the Display and Input
While the other to work with the Kenel and card.

Following are the functionalities -

1. Wait for user input to start a transaction
2. Work with card to read data and kernel for the processing
3. Work with UI elements to notify the status and prompt for any action

*/




#ifndef __TERMINAL_H__
#define __TERMINAL_H__


#include "Kernel.h"
#include "Card.h"
#include "Display.h"
#include "Debug.h"       
#include <TransactionStateService.h>

#define TAP_CARD_MESSAGE "Tap the card to the terminal.."
#define WAITING_FOR_AMOUNT "Please enter amount.."



extern Kernel kernel;
extern Display display;
extern Card card;


struct ErrorCodeMessage{
uint8_t code;
const char * msg;
};

enum TerminaCleanupReasons{
	NO_CARD_FOUND,
  KERNEL_INIT_FAILED,
  NO_CARD_PRESENTED,
  CARD_COMMAND_FAILED,
  SUCCESS

};

 

struct TxnProcessingInfo
{
  TransactionStateService * server;
  uint16_t txnAmt;
};


#ifndef DEBUG_DISABLED // Only if debug is not disabled (for production/release)

extern RemoteDebug Debug;

#endif

class Terminal
{



public:
  Terminal(){}
  
	static void txnLoop(void * pvParameters);
  static int txnAmt;
  static TaskHandle_t terminalKernelCardTask;
  static String txnStatusStr;

  
  
	static void cleanUp( TerminaCleanupReasons);
  static const char * getErrorMessage(uint8_t errorCode);
  StateUpdateResult  startTxn(TransactionState & state);
  StateUpdateResult updateTxnState(TransactionState & state);

};




#endif
