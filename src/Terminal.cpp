#include <Arduino.h>
#include "Terminal.h"

#define AUTHORIZATION_CODE_LEN 20 
#define TRANSACTION_OUTCOME_LEN 20 
#define TRANSACTION_ID_LEN 20


extern Terminal terminal;
extern TransactionStateService txnStateService;

int  Terminal::txnAmt =0;
TaskHandle_t Terminal::terminalKernelCardTask = 0;
String Terminal::txnStatusStr = "";

 struct ErrorCodeMessage errorList[] = {{NO_CARD_FOUND,"Card not found in field"},{KERNEL_INIT_FAILED,"Init Kernel Failed"},{NO_CARD_PRESENTED,"PLease try again.."},
{CARD_COMMAND_FAILED,"Card command Failed"}};



StateUpdateResult Terminal::startTxn(TransactionState & state)
{
  
   
   Serial.println("txn_state in read:" + state.txn_state);
   Serial.printf("txn_amt in read: %d\n" , state.txn_amt);
   
   if(state.txn_amt > 0 && Terminal::txnAmt==0)//TODO: Bad check to see if the transaction was started
   {
    Terminal::txnAmt = state.txn_amt;
    
    //  TxnProcessingInfo txnInfo;

    // txnInfo.txnAmt = state.txn_amt;
    // Serial.printf("txn Amount:%d\n", txnInfo.txnAmt);
    // txnInfo.server = &txnStateService;
  
    xTaskCreatePinnedToCore(
    Terminal::txnLoop,   /* Task function. */
    "Terminal",     /* name of task. */
    10000,       /* Stack size of task */
    nullptr,        /* parameter of the task */
    1,           /* priority of the task */
    &terminalKernelCardTask,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */

   }
   heap_caps_check_integrity_all(true);
   return StateUpdateResult::UNCHANGED;
}

 StateUpdateResult Terminal::updateTxnState(TransactionState & state)
 {
      heap_caps_check_integrity_all(true);
      state.txn_state = Terminal::txnStatusStr;
      state.txn_amt = Terminal::txnAmt;
      return StateUpdateResult::CHANGED;
 }



void Terminal::txnLoop(void * pvParameters)
{

#ifndef DEBUG_DISABLED
        debugV("txnLoop running on core,%d",xPortGetCoreID());
#endif
  uint8_t errorCode;  
  const char * msg;

  heap_caps_check_integrity_all(true);

  txnStatusStr = "Transaction in progress...";
  txnStateService.update(std::bind(&Terminal::updateTxnState,terminal,std::placeholders::_1),"Terminal");
  
   if(!kernel.initRequest(errorCode,Terminal::txnAmt))
    {
       txnStatusStr = "Init Kernel Failed";
       txnStateService.update(std::bind(&Terminal::updateTxnState,terminal,std::placeholders::_1),"Terminal");

       #ifndef DEBUG_DISABLED
       debugE("Init Kernel Failed");
      #endif
      #ifndef SERIAL_DEBUG_DISABLED
        Serial.println("Init Kernel Failed"); 
      #endif  
      //Signal the failure to the UI
       display.alertUser(getErrorMessage(KERNEL_INIT_FAILED),RED_BLINK,FREQ_50HZ_100ms); //TODO,provide proper LCD, LED status and audio frequency
    }
    else
    {
       #ifndef DEBUG_DISABLED
        debugV("Init Kernel Successful");
       #endif
       #ifndef SERIAL_DEBUG_DISABLED
        Serial.printf("Init Kernel Successful"); 
      #endif
        
      //Start polling the card and let display know about it
      display.alertUser(TAP_CARD_MESSAGE,RED_BLINK,FREQ_50HZ_100ms);//TODO,provide proper LED status and audio frequency
      if(!card.initCard(errorCode))
      {
        txnStatusStr = "Init Card Failed";
        txnStateService.update(std::bind(&Terminal::updateTxnState,terminal,std::placeholders::_1),"Terminal");
         #ifndef DEBUG_DISABLED
           debugE("Init Card Failed");
         #endif
         #ifndef SERIAL_DEBUG_DISABLED
           Serial.println("Init Card Failed");  
         #endif
           
        msg = getErrorMessage(errorCode); //TODO - Use this error message to alert user
        display.alertUser(msg,RED_BLINK,FREQ_50HZ_100ms);
        //User did not present card for allowed time
        cleanUp(NO_CARD_FOUND);

        

      }
      else
      {
        bool lastCommand = false; //Check if the last command for the card has reached
        bool failure = false;
        //Do rest of the processing
        while(1) //Continue till the last command is reached or an error occurs
        {
          if(!card.nextCardCommand(kernel,lastCommand,errorCode)) //Card command
          {
            txnStatusStr = "Card command Failed";
            txnStateService.update(std::bind(&Terminal::updateTxnState,terminal,std::placeholders::_1),"Terminal");
            #ifndef DEBUG_DISABLED
              debugE("Card command Failed");
            #endif
            #ifndef SERIAL_DEBUG_DISABLED
              Serial.println("Card command Failed");  
            #endif
              
            msg = getErrorMessage(errorCode); //TODO - Use this error message to alert user
            display.alertUser(msg,RED_BLINK,FREQ_50HZ_100ms); //TODO: Proper error for command failure
            cleanUp(NO_CARD_FOUND);//TODO: Proper error for command failure
            failure = true;
            break;
          }
          else if(!kernel.nextKernelCommand(card,errorCode))
          {
            txnStatusStr = "Kernel command Failed";
            txnStateService.update(std::bind(&Terminal::updateTxnState,terminal,std::placeholders::_1),"Terminal");
             #ifndef DEBUG_DISABLED
              debugE("Kernel command Failed");
             #endif
             #ifndef SERIAL_DEBUG_DISABLED
                Serial.println("Kernel command Failed"); 
             #endif   
            msg = getErrorMessage(errorCode); //TODO - Use this error message to alert user
            display.alertUser(msg,RED_BLINK,FREQ_50HZ_100ms); //TODO: Proper error for command failure
            cleanUp(NO_CARD_FOUND);//TODO: Proper error for command failure
            failure = true;
            break;
          }

          if(lastCommand)
            break;

        }

        if(lastCommand && !failure)
        {
          txnStatusStr = "Remove card...";
          txnStateService.update(std::bind(&Terminal::updateTxnState,terminal,std::placeholders::_1),"Terminal");
          #ifndef DEBUG_DISABLED
           debugI("Success in Kernnel processing!");
          #endif
          #ifndef SERIAL_DEBUG_DISABLED
            Serial.println("Success in Kernnel processing!"); 
          #endif  

          //Alert user to remove card
           display.alertUser("Remove card...",RED_BLINK,FREQ_50HZ_100ms); //TODO: Proper Success 
          //Stop RF
          card.cleanUp();

          //Lets get the outcome
           char authorisationCodeOut[AUTHORIZATION_CODE_LEN],transactionOutcomeOut[TRANSACTION_OUTCOME_LEN],transactionIDOut[TRANSACTION_ID_LEN] ; //TODO USE THIS IN DISPLAY
            
           if(kernel.checkAuthorization(errorCode,authorisationCodeOut,transactionOutcomeOut,transactionIDOut))
           {
             txnStatusStr = "Authorisation Success!";
             txnStateService.update(std::bind(&Terminal::updateTxnState,terminal,std::placeholders::_1),"Terminal");
            #ifndef DEBUG_DISABLED
             debugI("Authorisation Success!");
            #endif
            #ifndef SERIAL_DEBUG_DISABLED
              Serial.println("Authorisation Success!"); 
            #endif 
            display.alertUser("Authorisation Success!",RED_BLINK,MA_SONIC); //TODO: Proper Success 
           }
           else
           {
             txnStatusStr = "Authorisation Failed";
              txnStateService.update(std::bind(&Terminal::updateTxnState,terminal,std::placeholders::_1),"Terminal");
              #ifndef DEBUG_DISABLED
               debugE("Authorisation Failed");
              #endif
              #ifndef SERIAL_DEBUG_DISABLED
                Serial.println("Authorisation Failed"); 
              #endif 
              display.alertUser("Authorisation Failed",RED_BLINK,FREQ_50HZ_100ms); //TODO: Proper Success
           }
          
           
          //kernel.cleanUp();//TODO: Proper cleanup after success
          cleanUp(SUCCESS);
        }

      }
    }
  vTaskDelete(NULL);
}


void Terminal::cleanUp(TerminaCleanupReasons reason)
{
  Terminal::txnAmt=0;
	card.cleanUp();
	kernel.cleanUp();
	display.alertUser(WAITING_FOR_AMOUNT,RED_BLINK,FREQ_50HZ_100ms);
}

const char * Terminal::getErrorMessage(uint8_t  errorCode)//TODO: Create a mapping structure to pass a proper message across
{
  for(uint8_t counter=0; counter < sizeof(errorList);counter++)
  {
    if(errorList[counter].code == errorCode)
      return errorList[counter].msg;
  }
  return "Error";
}
