#ifndef TransactionStateService_h
#define TransactionStateService_h



#include <HttpEndpoint.h>

#include <WebSocketTxRx.h>


// Note that the built-in LED is on when the pin is low on most NodeMCU boards.
// This is because the anode is tied to VCC and the cathode to the GPIO 4 (Arduino pin 2).
#ifdef ESP32
#define LED_ON 0x1
#define LED_OFF 0x0
#elif defined(ESP8266)
#define LED_ON 0x0
#define LED_OFF 0x1
#endif

#define TRANSACTION_SETTINGS_ENDPOINT_PATH "/rest/transtat"
#define TRANSACTION_SETTINGS_SOCKET_PATH "/ws/transtat"

class TransactionState {
 public:
  String txn_state;
  int txn_amt;

  static void read(TransactionState& settings, JsonObject& root) {
    root["txn_state"] = settings.txn_state;
    //Serial.println("txn_state in read:" + settings.txn_state);
  }

  static StateUpdateResult update(JsonObject& root, TransactionState& txnState) {
    txnState.txn_state = String((const char*)root["txn_state"]) ;
    txnState.txn_amt = atoi((const char*)root["txn_amt"]);
   // Serial.println("txn_state in Update:" + txnState.txn_state);
    // Serial.println("txn_amt in Update:" + String((const char*)root["txn_amt"]));
    return StateUpdateResult::CHANGED;
    
  }

  static void haRead(TransactionState& settings, JsonObject& root) {
    root["txn_state"] = settings.txn_state;
  }

  static StateUpdateResult haUpdate(JsonObject& root, TransactionState& txnState) {
   txnState.txn_state = String((const char*)root["txn_state"]) ;
   txnState.txn_amt = atoi((const char*)root["txn_amt"]);
    return StateUpdateResult::CHANGED;
  }
};

class TransactionStateService : public StatefulService<TransactionState> {
 public:
  TransactionStateService(AsyncWebServer* server,
                    SecurityManager* securityManager);
  void begin();

  
 private:
  HttpEndpoint<TransactionState> _httpEndpoint;
  
  WebSocketTxRx<TransactionState> _webSocket;
  

  
};

#endif
