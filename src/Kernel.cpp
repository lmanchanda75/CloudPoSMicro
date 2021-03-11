
#include "kernel.h"


//#define SECURE true
//#define JSON true

Kernel::Kernel()
{

  #ifdef SECURE
	  client = new WiFiClientSecure;
	  if(client) 
	    client -> setCACert(rootCACertificate); 
	  #else
	    client = new WiFiClient;
  #endif

  sessionId = NULL;
  
}


bool Kernel::nextKernelCommand(Card & card,uint8_t & errorCode){

   kernelCmdIndex +=1;

	
	KernelFunction kernelFunc = kernelFunctions[kernelIndex][kernelCmdIndex-1];


	if(kernelFunc != NULL)
	{
		return CALL_MEMBER_FN(this,kernelFunc) (card,errorCode);
	}
	else
	{
		return true; //If there is no command to  execute 
	}
}

bool Kernel::sendHTTP(char* requestURI, uint8_t* requestBody, uint16_t requestLength,bool ifHttps, String & responsePayload,uint8_t & errorCode)
{
	if(client)
  {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
      HTTPClient https; //if HTTP or S depends on the type of WiFiClient

    #ifndef DEBUG_DISABLED
      debugV("[HTTPS] begin...\n");
    #endif
    #ifndef SERIAL_DEBUG_DISABLED
      Serial.println("[HTTPS] begin...\n");  
    #endif  
      if (https.begin(*client,SERVER_HOST,SERVER_PORT,requestURI,ifHttps)) {  // HTTPS

        https.addHeader("Content-Type", "application/json");
        https.setAuthorization(autherisationToken);
        if(sessionId)
        {
          https.addHeader("Cookie", sessionId);
        }
        else
        {
          const char * headersToGet[1] = {"Set-Cookie"};

          https.collectHeaders(headersToGet,1);
         
        }

        #ifndef DEBUG_DISABLED
          debugV("[HTTPS] POST...\n");
        #endif
        #ifndef SERIAL_DEBUG_DISABLED
          Serial.println("[HTTPS] POST...\n");
        #endif    

        
        
        // start connection and send HTTP header
        int httpCode = https.POST(requestBody,requestLength);
  
        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
         #ifndef DEBUG_DISABLED 
          debugD("[HTTP] POST... code: %d\n", httpCode);
         #endif
         #ifndef SERIAL_DEBUG_DISABLED
          Serial.printf("[HTTP] POST... code: %d\n", httpCode); 
         #endif  
  
          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {



             if(sessionId == NULL && https.hasHeader("Set-Cookie")) //Because its the first request
             {
        
               sessionId = (char *) malloc(https.header("Set-Cookie").length()+1);
               
                #ifndef DEBUG_DISABLED 
                debugV("SessionID %s\n", https.header("Set-Cookie"));
               #endif
               #ifndef SERIAL_DEBUG_DISABLED
                Serial.printf("SessionID: %s,length:%d\n:", https.header("Set-Cookie").c_str(),https.header("Set-Cookie").length()); 
               #endif 
               
               strcpy(sessionId,https.header("Set-Cookie").c_str());
             }
              
             responsePayload = https.getString();

             #ifndef DEBUG_DISABLED
                debugV("ResponsePayload: %s\n", responsePayload.c_str());
             #endif
             #ifndef SERIAL_DEBUG_DISABLED
                Serial.println("responsePayload: " + responsePayload);
             #endif   
             
             https.end();
             return true;
          }
          else
          {
            #ifndef DEBUG_DISABLED
                debugE("HTTP Failed \n");
             #endif
             #ifndef SERIAL_DEBUG_DISABLED
                Serial.println("HTTP Failed " );
             #endif   
             return false;
          }
        } else {

          #ifndef DEBUG_DISABLED
            debugE("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
          #endif
          #ifndef SERIAL_DEBUG_DISABLED
            Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());  
          #endif  
          https.end();
          return false;
        }
  
        
      } else {
         #ifndef DEBUG_DISABLED
         debugE("[HTTPS] Unable to connect\n");
        #endif
        #ifndef SERIAL_DEBUG_DISABLED
          Serial.println("[HTTPS] Unable to connect\n"); 
        #endif  
        return false;
      }

    
    //delete client;
  } else {
    #ifndef DEBUG_DISABLED
      debugE("Unable to create client");
    #endif
    #ifndef SERIAL_DEBUG_DISABLED
      Serial.println("Unable to create client");  
    #endif
      
    return false;
  }
}


bool Kernel::initRequest(uint8_t & errorCode,uint16_t txnAmount)
{
    

    uint8_t initRequestBody[INIT_BODY_SIZE];

    size_t requestSize;

    String payload; 

    char txnAmt[16];

    //Initialise the functions 

    kernelCmdIndex = 0;//Usage point
    kernelCmdEndIndex = 1;//Insertion point
    sessionId = NULL; //New session

    sprintf(txnAmt,"012%d",txnAmount*100);
        
    DynamicJsonDocument doc(768);//TODO, find the dynamic size of this request

    JsonObject initRequest = doc.createNestedObject("initRequest");
    initRequest["readerProfileName"] = "PPS_MChip1";
    initRequest["actProfileName"] = "MCTerm-Default";
    initRequest["amountAuthorised"] = txnAmt;
    initRequest["amountOther"] = "000000000000";
    initRequest["transactionCategoryCode"] = "52";
    initRequest["transactionType"] = "00";
    
    JsonArray actSignal = doc.createNestedArray("actSignal");
    
    JsonObject actSignal_0 = actSignal.createNestedObject();
    actSignal_0["tag"] = "9F02";
    actSignal_0["value"] = "000000001500";
    
    JsonObject actSignal_1 = actSignal.createNestedObject();
    actSignal_1["tag"] = "9F03";
    actSignal_1["value"] = "000000000000";
    
    JsonObject actSignal_2 = actSignal.createNestedObject();
    actSignal_2["tag"] = "9F7C";
    actSignal_2["value"] = "1234567890EFEFEFEFEFEFEFEFEFEFEFEFEFEFEF";
    
    JsonObject actSignal_3 = actSignal.createNestedObject();
    actSignal_3["tag"] = "9F53";
    actSignal_3["value"] = "52";
    
    JsonObject actSignal_4 = actSignal.createNestedObject();
    actSignal_4["tag"] = "5F2A";
    actSignal_4["value"] = "0840";
    
    JsonObject actSignal_5 = actSignal.createNestedObject();
    actSignal_5["tag"] = "5F36";
    actSignal_5["value"] = "02";
    
    JsonObject actSignal_6 = actSignal.createNestedObject();
    actSignal_6["tag"] = "9A";
    actSignal_6["value"] = "170420";
    
    JsonObject actSignal_7 = actSignal.createNestedObject();
    actSignal_7["tag"] = "9F21";
    actSignal_7["value"] = "153605";
    
    JsonObject actSignal_8 = actSignal.createNestedObject();
    actSignal_8["tag"] = "9C";
    actSignal_8["value"] = "00";
  
    requestSize = serializeJson(doc, (char*)initRequestBody,sizeof(initRequestBody)); //TODO, check for any error in this request

   #ifndef DEBUG_DISABLED
    debugV("Init Request body after serialise\n -  %s", initRequestBody);
  #endif
  #ifndef SERIAL_DEBUG_DISABLED
    Serial.printf("Init Request body after serialise\n -  %s\n", initRequestBody);  
  #endif  

    if (sendHTTP(initURI, initRequestBody, requestSize,false, payload,errorCode))
      {
        
        #ifndef DEBUG_DISABLED
          debugI("Init Request Successful\n ");
        #endif
        #ifndef SERIAL_DEBUG_DISABLED
          Serial.printf("Init Request Successful\n");  
        #endif  
        return true;
      }
      else
      {
        return false;
      }

    
}


bool Kernel::activateRequest(Card & card,uint8_t & errorCode)
{
    
    uint8_t  activateRequestBody[INIT_BODY_SIZE];
 
    size_t requestSize;

    uint8_t * fciData;

    uint16_t fciDataLen;

    uint8_t cardAid[MAX_AID_LEN];
    uint8_t cardAidLen;

    uint16_t orgStrArrLen ;
    bool success ;

    String payload;
   
    char kernelId[3] = "02";
    unsigned long positionArray;

    
    fciData=card.getData(fciDataLen,false,positionArray);
    cardAidLen = card.getAid(cardAid);

    char * cardDataAsString = (char*)calloc(2*cardAidLen+1,sizeof(char));

    for(uint8_t i=0;i<cardAidLen;i++)
    {
     sprintf(cardDataAsString+2*i,"%02X",*((cardAid+i)));
    }

    #ifndef DEBUG_DISABLED 
      debugV("CardId in activate :%s, Len:%d\n ", cardDataAsString,cardAidLen);
    #endif
    #ifndef SERIAL_DEBUG_DISABLED
      Serial.printf("CardId in activate :%s, Len:%d\n ", cardDataAsString,cardAidLen);
    #endif
      
   

  #ifdef JSON //When the request will be sent as JSON and encrypted
    DynamicJsonDocument doc(1024);

    JsonObject data = doc.createNestedObject("data");
    data["initReaderData"] = "initTCC56IDSN";
    
    JsonArray data_fci = data.createNestedArray("fci");

    for(uint16_t i=0;i<(fciDataLen);i++){ //Not copying the response word
      data_fci.add(fciData[i]);
    }

    
    data.getOrAddMember("aid").set(cardDataAsString);
    data["kernelID"] = "02";

    
    requestSize = serializeJson(doc, (char*)activateRequestBody,sizeof(activateRequestBody));
   #endif

    activateRequestBody[0] = '\0';

    strcpy((char*)activateRequestBody,"{\"data\":\"{\\\"initReaderData\\\":\\\"initTCC56IDSN\\\",\\\"fci\\\":[");
    
    
    orgStrArrLen = strlen((const char*)activateRequestBody);

    
    
     for(uint16_t i=0;i<fciDataLen;i++){ //Not copying the response word
         orgStrArrLen += sprintf((char*)activateRequestBody+orgStrArrLen,"%d,",fciData[i]);
    }
    
   
   
    orgStrArrLen +=sprintf((char*)(activateRequestBody+orgStrArrLen-1),"%s%s%s%s%s","],\\\"aid\\\":\\\"",cardDataAsString,"\\\",\\\"kernelID\\\":\\\"",kernelId,"\\\"}\"}");
  
    free(cardDataAsString);

    requestSize = orgStrArrLen-1;

  #ifndef DEBUG_DISABLED
    debugV("Activate Request body after serialise\n -  %s", activateRequestBody);
  #endif
  #ifndef SERIAL_DEBUG_DISABLED
    Serial.printf("Activate Request body after serialise\n -  %s\n", activateRequestBody);  
  #endif  
  

    if(sendHTTP(activateURI, activateRequestBody, requestSize,false, payload,errorCode))
    {
  
		  //debugI("Payload for Activate Response \n -  %s", payload);
      
		  DynamicJsonDocument doc(2*payload.length());
	    deserializeJson(doc, payload);
	    
	    const char* data_nextCommand_0 = doc["data"]["nextCommand"][0]; 
	    
	    success = doc["success"]; 

	    if(success)
	    {
       #ifndef DEBUG_DISABLED
        debugI("Successful response from server for Activate Request");
      #endif
      #ifndef SERIAL_DEBUG_DISABLED
        Serial.println("Successful response from server for Activate Request");  
      #endif   
	      card.putCmd(data_nextCommand_0,strlen(data_nextCommand_0),false);
	      this->putFunction(&Kernel::genericAPIRequest);
	      return true;
	    }
	    else
	    {
       #ifndef DEBUG_DISABLED
        debugE("Response for Activate request was a fail from server");
      #endif
      #ifndef SERIAL_DEBUG_DISABLED
          Serial.println("Response for Activate request was a fail from server");  
        #endif    
	    	return false;
	    }
  	}
  	else
  	{
    #ifndef DEBUG_DISABLED
      debugE("Send HTTP for ActivateRequest failed");
     #endif
     #ifndef SERIAL_DEBUG_DISABLED
      Serial.println("Send HTTP for ActivateRequest failed");  
    #endif
      
  		return false;
  	}
  
}


bool Kernel::genericAPIRequest(Card & card,uint8_t & errorCode)
{
    
    uint8_t  genericRequestBody[GENERIC_BODY_SIZE];

    size_t requestSize;

    uint8_t * responseData;

    uint16_t responseDataLen;

    uint16_t orgStrArrLen ;
    bool success ;
    char statusCode[2] = "0";
    unsigned long lastCommandExecutionTime = 198000;
    String payload;
    unsigned long  positionArray;

      
    responseData=card.getData(responseDataLen,true,positionArray);
 
  #ifdef JSON //When the request will be sent as JSON and encrypted
  
    DynamicJsonDocument doc(96);

    JsonObject data = doc.createNestedObject("data");
    
    data["rApdu"][0] = "7716820203809410080101001001010118010100200102009000";
    data["statusCode"] = 0;
    data["lastCommandExecutionTime"] = 198000;
    
    requestSize = serializeJson(doc, (char*)genericRequestBody,sizeof(genericRequestBody));
   #endif

   //{"data":"{\"rApdu\":[\"7716820203809410080101001001010118010100200102009000\"],\"statusCode\":0,\"lastCommandExecutionTime\":198000}"}
   
    genericRequestBody[0] = '\0';

    strcpy((char*)genericRequestBody,"{\"data\":\"{\\\"rApdu\\\":[");
    
    
    orgStrArrLen = strlen((const char*)genericRequestBody);

    
  
    uint16_t masterCnt=0;
    for(uint8_t i=0;i<sizeof(positionArray);i++)
    {
      uint8_t dataLength = ((uint8_t*)&positionArray)[i];
      if(dataLength > 0)
      {
        orgStrArrLen += sprintf((char*)genericRequestBody+orgStrArrLen,"%s","\\\"");
        
        for(uint8_t j=0;j<dataLength;j++)
        {
          orgStrArrLen += sprintf((char*)genericRequestBody+orgStrArrLen,"%02X",responseData[masterCnt++]);
        }
    
        orgStrArrLen += sprintf((char*)genericRequestBody+orgStrArrLen,"%s","\\\",");
      }
      
    } 
         
    if( genericRequestBody[orgStrArrLen-1] == ',')
      genericRequestBody[orgStrArrLen-1]= ']'; //Replace the ',' with closing ]
    else
      genericRequestBody[orgStrArrLen++]= ']';
   
   
    orgStrArrLen +=sprintf((char*)(genericRequestBody+orgStrArrLen),"%s%s%s%u%s",",\\\"statusCode\\\":",statusCode,",\\\"lastCommandExecutionTime\\\":",lastCommandExecutionTime,"}\"}");
   
    requestSize = orgStrArrLen;
    
  #ifndef DEBUG_DISABLED

    debugV("Generic Request body after serialise\n -  %s", genericRequestBody);
  #endif
  #ifndef SERIAL_DEBUG_DISABLED
    Serial.printf("Generic Request body after serialise\n -  %s", genericRequestBody);  
  #endif   
   

     if(sendHTTP(genericURI, genericRequestBody, requestSize,false, payload,errorCode))
    {
        //debugI("Payload for Genric Response \n -  %s", payload);
		    DynamicJsonDocument doc(3*payload.length());
        DeserializationError err =deserializeJson(doc, payload);

        if(err)
        {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(err.c_str());
          return false;
        }
        
        success = doc["success"]; 

        if(success)
        {
         #ifndef DEBUG_DISABLED
          debugV("Successful response from server for Generic Request");
         #endif
         #ifndef SERIAL_DEBUG_DISABLED
          Serial.println("Successful response from server for Generic Request"); 
         #endif
          
          JsonArray data_nextCommand = doc["data"]["nextCommand"];
          JsonObject data_readerOutcome = doc["data"]["readerOutcome"];

         
        #ifndef DEBUG_DISABLED
          debugV("No of commands returned:%d",data_nextCommand.size());
        #endif
        #ifndef SERIAL_DEBUG_DISABLED
          Serial.printf("No of commands returned:%d\n",data_nextCommand.size());  
        #endif  
           uint8_t i=0;

         if(data_nextCommand.size())
         {
          if(data_nextCommand.size() > 1)
          {
           
            card.putCmd(data_nextCommand[i],strlen(data_nextCommand[i]),false);
            this->putFunction(NULL);

            for(i=1;i<data_nextCommand.size()-1;i++)//TODO: Bad hack, assuming that for just the last command real API will be added
            {
              card.putCmd(data_nextCommand[i],strlen(data_nextCommand[i]),true);
              this->putFunction(NULL);
            }
            
            card.putCmd(data_nextCommand[i],strlen(data_nextCommand[i]),true);
            this->putFunction(&Kernel::genericAPIRequest);
          }
          else 
          {
            card.putCmd(data_nextCommand[i],strlen(data_nextCommand[i]),false);
            this->putFunction(&Kernel::genericAPIRequest);
          }
          #ifndef DEBUG_DISABLED
            debugI("Success in response of Generic API:%02X%02X\n",responseData[0],responseData[1]);
          #endif
          #ifndef SERIAL_DEBUG_DISABLED
            Serial.printf("Success in response of Generic API:%02X%02X\n",responseData[0],responseData[1]);  
          #endif
          return true;
         }
          else if(data_readerOutcome.size())
          {
            JsonObject data_readerOutcome_mOutcomeParamSet = data_readerOutcome["mOutcomeParamSet"];
            const char* data_readerOutcome_mOutcomeParamSet_mStatus = data_readerOutcome_mOutcomeParamSet["mStatus"]; // "ONLINE_REQUEST" 
            if(!strncmp(data_readerOutcome_mOutcomeParamSet_mStatus,"ONLINE_REQUEST",strlen(data_readerOutcome_mOutcomeParamSet_mStatus)))
            {
              #ifndef DEBUG_DISABLED
                debugI("Success in response of Generic API:%02X%02X\n",responseData[0],responseData[1]);
              #endif
              #ifndef SERIAL_DEBUG_DISABLED
                Serial.printf("Success in response of Generic API:%02X%02X\n",responseData[0],responseData[1]);  
              #endif
              return true;
            }  
            else
            {
              #ifndef DEBUG_DISABLED
                debugE("Failed in response of Generic API:%02X%02X\n",responseData[0],responseData[1]);
              #endif
              #ifndef SERIAL_DEBUG_DISABLED
                Serial.printf("Failed in response of Generic API:%02X%02X\n",responseData[0],responseData[1]);  
              #endif
              return false;  
            }
              
          }
          else
          {
            Serial.printf("No ligitimate response in Generic API\n");  
            return false;
          }
        }
        else
        {
          #ifndef DEBUG_DISABLED
            debugE("Failed in response of Generic API:%02X%02X\n",responseData[0],responseData[1]);
          #endif
          #ifndef SERIAL_DEBUG_DISABLED
            Serial.printf("Failed in response of Generic API:%02X%02X\n",responseData[0],responseData[1]);  
          #endif
            
          
          return false;
        }
            
  	}
  	else
  	{
    #ifndef DEBUG_DISABLED
      debugE("HTTP request failed for Generic API:%02X%02X\n",responseData[0],responseData[1]);
    #endif
    #ifndef SERIAL_DEBUG_DISABLED
      Serial.printf("HTTP request failed for Generic API:%02X%02X\n",responseData[0],responseData[1]);  
    #endif  
  		return false;
  	}
  
  
  
}


bool Kernel::checkAuthorization(uint8_t & errorCode,char *authorisationCodeOut,char *transactionOutcomeOut,char * transactionIDOut  )
{
   String payload;
   
   #ifndef DEBUG_DISABLED
    debugI("Authorization Request..");
  #endif
  #ifndef SERIAL_DEBUG_DISABLED
    Serial.printf("Authorization Request..");  
  #endif  

    if(sendHTTP(authorizeURI, NULL, 0,false, payload,errorCode))
    {
      DynamicJsonDocument doc(2*payload.length());
      DeserializationError err =deserializeJson(doc, payload);

    if(err)
      {
         #ifndef DEBUG_DISABLED
        debugE("deserializeJson failed for checkAuthorization\n");
        #endif
        #ifndef SERIAL_DEBUG_DISABLED
          Serial.printf("deserializeJson failed for checkAuthorization\n");  
        #endif  
        return false;
        
      }
      else
      {
      
        const char* authorisationCode = doc["authorisationCode"]; // "3265461235471236"
        const char* transactionOutcome = doc["transactionOutcome"]; // "APPROVED"
        const char* transactionID = doc["transactionID"]; // "111111111111111"
        bool isSuccess = doc["isSuccess"]; //true

        strcpy(authorisationCodeOut,authorisationCode);
        strcpy(transactionOutcomeOut,transactionOutcome);

        if(isSuccess)
        {
          
          strcpy(transactionIDOut,transactionID);
           #ifndef DEBUG_DISABLED
          debugI("Authorisation successful \n");
          #endif
          #ifndef SERIAL_DEBUG_DISABLED
            Serial.printf("Authorisation successful\n");  
          #endif  
          return true;
        }
        else
        {
          #ifndef DEBUG_DISABLED
          debugE("authorisation failed \n");
          #endif
          #ifndef SERIAL_DEBUG_DISABLED
            Serial.printf("authorisation failed\n");  
          #endif  
        return false;
        }
      }
    }
    else
    {
      #ifndef DEBUG_DISABLED
      debugE("HTTP request failed for checkAuthorization\n");
      #endif
      #ifndef SERIAL_DEBUG_DISABLED
        Serial.printf("HTTP request failed for checkAuthorization\n");  
      #endif  
        return false;
    }
     
}

bool Kernel::kernelSelection(){
  kernelIndex = 1; //TODO
	return true;
	
}

void Kernel::putFunction(KernelFunction funcPtr)
{

  #ifndef DEBUG_DISABLED
   debugV("Function pointer: %X,kernelCmdEndIndex:%d",funcPtr,kernelCmdEndIndex);
  #endif
  #ifndef SERIAL_DEBUG_DISABLED
    Serial.printf("Function pointer: %X,kernelCmdEndIndex:%d\n",funcPtr,kernelCmdEndIndex); 
  #endif   
  kernelCmdEndIndex +=1;
 
	kernelFunctions [kernelIndex][kernelCmdEndIndex] = funcPtr;
 
  #ifndef DEBUG_DISABLED
    debugV("Cmd Func:%X\n",kernelFunctions [kernelIndex][kernelCmdEndIndex]);
  #endif
  #ifndef SERIAL_DEBUG_DISABLED
    Serial.printf("Cmd Func:%X\n",kernelFunctions [kernelIndex][kernelCmdEndIndex]);  
  #endif  
}

void Kernel::cleanUp()
{
    kernelIndex = 0;
    kernelCmdIndex = 0;
    kernelCmdEndIndex = 2;
    if(sessionId)
    {
      free (sessionId);
      sessionId = NULL; 
    }
}
