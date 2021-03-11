#include <TransactionStateService.h>

TransactionStateService::TransactionStateService(AsyncWebServer* server,
                                     SecurityManager* securityManager) :
    _httpEndpoint(TransactionState::read,
                  TransactionState::update,
                  this,
                  server,
                  TRANSACTION_SETTINGS_ENDPOINT_PATH,
                  securityManager,
                  AuthenticationPredicates::IS_AUTHENTICATED),
    _webSocket(TransactionState::read,
               TransactionState::update,
               this,
               server,
               TRANSACTION_SETTINGS_SOCKET_PATH,
               securityManager,
               AuthenticationPredicates::IS_AUTHENTICATED) {
  
}

void TransactionStateService::begin() {
  _state.txn_state = String("Start Txn");

}
  