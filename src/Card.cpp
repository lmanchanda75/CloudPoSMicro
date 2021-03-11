#include "card.h"




Card::Card(Display & display) //TODO: Store and use the error in here and use in the main app
{

  this->display = &display;

  #ifdef ALTERNATE_PINS
  alternateSPI = new SPIClass();
  #endif

}


bool Card::initNFCMod()
{

  #ifndef ALTERNATE_PINS
    pn532_bus = new PN532_SPI(SPI, SS);
  #else
    pn532_bus = new PN532_SPI(alternateSPI,(uint8_t)VSPI_SCLK,(uint8_t)VSPI_MISO,(uint8_t)VSPI_MOSI, (uint8_t)VSPI_SS);
  #endif
  nfc = new PN532(*pn532_bus);

  memset(dict, 0, sizeof(dict));
  emvInit(dict);

  nfc->begin();

  uint32_t versiondata = nfc->getFirmwareVersion();
  if (!versiondata)
  {
    Serial.println("Didn't find PN53x board");
    while (1)
      ; // halt
  }

  // Got ok data, print it out!
  Serial.println("Found chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.');
  Serial.println((versiondata >> 8) & 0xFF, DEC);

  // configure board to read RFID tags
  nfc->SAMConfig();


  if(!nfc->setParameters(PARAM_AUTO_ATR_RES | PARAM_AUTO_RATS, true))
  {
    Serial.println("Set parameter PARAM_AUTO_ATR_RES | PARAM_AUTO_RATS failed!"); 
  }

  uint16_t  cmd [] = { PN53X_REG_CIU_TxMode, PN53X_REG_CIU_RxMode, PN53X_REG_CIU_ManualRCV, PN53X_REG_CIU_Status2, PN53X_REG_CIU_BitFraming};
  uint8_t  *cmdRes ;
  uint32_t cmdStatus;
  
  cmdRes = nfc->readRegisters(cmd,5);

  uint16_t  cmd2 []={ PN53X_REG_CIU_TxMode, PN53X_REG_CIU_RxMode};
  uint8_t  cmd2Val []={  0x80,  0x80};
  
  
  cmdStatus = nfc->writeRegisters(cmd2, cmd2Val, 2);

  cmdStatus = nfc->setRFField(0x00,  0x00);

  //cmdStatus = nfc->setRFField(0x00,  0x01);

  
  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc->setPassiveActivationRetries(0xFF);



  uint16_t  cmd3 []={ PN53X_REG_CIU_TxAuto, PN53X_REG_CIU_Control};
  uint8_t  cmd3Val []={  0x40,  0x10};
  
  
  cmdStatus = nfc->writeRegisters(cmd3, cmd3Val, 2);
  
  cmdRes = nfc->readRegisters(cmd,5);
}

bool Card::stopRF()
{
	return nfc->setRFField(0x00,  0x00);
}

bool Card::startRF()
{
	return nfc->setRFField(0x00,  0x01);
}

bool Card::initCard(uint8_t & errorCode)
{
	if(this->startRF())
	{

	 cardCmdIndex = 0;
	 dataLength = 0;
	 cardCmdEndIndex = 1;
 	 // set shield to inListPassiveTarget
 	 return nfc->inListPassiveTarget();
  
	}
	else
	{
    #ifndef DEBUG_DISABLED
    debugE("Starting RF failed in the init card request");
    #endif
    
    #ifndef SERIAL_DEBUG_DISABLED
      Serial.println("Starting RF failed in the init card request");
    #endif
    
		return false;
	}
}


bool Card::nextCardCommand(Kernel & kernel,bool & lastCmd,uint8_t & errorCode){

	CardFunction cardFunc = cardFunctions[cardCmdIndex++];

	if(cardFunc != NULL)
	{
		return CALL_MEMBER_FN(this,cardFunc) (kernel,lastCmd,errorCode);
	}
	else
	{
		return true; //If there is no command to  execute 
	}
}

void Card::putData(uint8_t *  response, uint8_t length,bool append)
{
	 dataLength = length;        
   memcpy(data,response,length);    
   sw1sw2[0] = response[length - 2];
   sw1sw2[1] = response[length - 1];
}


void Card::putData( uint8_t length,bool append)
{
	if(append)
	{
		dataLength += length;

    Serial.printf("Length:%d,DataLength:%d\n",length,dataLength);

		for(uint8_t i=1;i<sizeof(dataPosition);i++) //The first data is already there
		{
			if(((uint8_t*)&dataPosition)[i] == 0)
     {
				((uint8_t*)&dataPosition)[i] = length;
        break;
     }
			else
				continue;

		}	
	}
	else
	{
		dataLength = length;
		dataPosition = 0;
		((uint8_t*)&dataPosition)[0] = length;
	}
}


bool Card::selectPPSE(Kernel & kernel,bool & lastCmd,uint8_t & errorCode)
{
      
   
   bool success;
   uint8_t responseLength = (sizeof(data)-dataLength < 255)?sizeof(data)-dataLength:255;//TODO Handle , as same variable is being used across

   
    success = nfc->inDataExchange(selectPPSECmd, sizeof(selectPPSECmd), data, &responseLength); 


    if (success) //PN532 was succeessful
    {
      #ifndef DEBUG_DISABLED
      debugI("selectPPSE is successful,response length:%d",responseLength);
      #endif
      
      #ifndef SERIAL_DEBUG_DISABLED
      Serial.printf("selectPPSE is successful,response length:%d\n",responseLength);
      #endif
      
      putData( responseLength,false);
      
      nfc->PrintHexChar(data, responseLength);

      //card.putData(response,responseLength);

      if(getSW1() == 0x90 && getSW2() == 0x00) //Card responded successfully 
     {
       #ifndef DEBUG_DISABLED
        debugI("Response word from card was a success");
       #endif
        
       #ifndef SERIAL_DEBUG_DISABLED
       Serial.println("Response word from card was a success"); 
       #endif
       
        tlvInfo_t *t=(tlvInfo_t *)calloc(responseLength,sizeof(tlvInfo_t));
        //memset(t,0,responseLength);
        tlvInfo_init(t);
        int tindex =0;
        emvparse(data, responseLength, t, &tindex , 0, dict);
        //emvPrint_result(t, tindex);
        parseAndStoreResponse(t, tindex);
        free(t);

        //Data for applications is now availble, select the Kernel to process 

        if(!kernel.kernelSelection()) //If no Kernel found to process the application
        {
           #ifndef DEBUG_DISABLED
            debugE("Kernel selection failed");
           #endif
             
           #ifndef SERIAL_DEBUG_DISABLED
            Serial.println("Kernel selection failed");
           #endif 
        	  return false;
        }

        return true;
        
        }
        else
        {
          #ifndef DEBUG_DISABLED
            debugE("Response word from card was a failure");
          #endif
            
          #ifndef SERIAL_DEBUG_DISABLED
            Serial.println("Response word from card was a failure");  
          #endif  
          return false;
        }

    }
    else
    {
      #ifndef DEBUG_DISABLED
      debugI("selectPPSE failed");

      #endif 
      
      #ifndef SERIAL_DEBUG_DISABLED
      
    
       Serial.println("selectPPSE failed"); 
     #endif  
      return false;
    }
}



bool Card::selectApplication(Kernel & kernel,bool & lastCmd,uint8_t & errorCode)
{
       
   //uint8_t response[128];
   bool success;

   dataLength = 0 ; //Over write the existing data
   uint8_t responseLength =  (sizeof(data)-dataLength < 255)?sizeof(data)-dataLength:255;//TODO Handle , as same variable is being used across
           
    
    //Make Select application APDU
  
    uint8_t apduSize = sizeof(selectApplicationCmd)+1+aidLen+1;
    
    byte * apdu = (byte*)malloc(apduSize);

    memcpy(apdu,selectApplicationCmd,sizeof(selectApplicationCmd));
    apdu[sizeof(selectApplicationCmd)] = aidLen;
    memcpy(apdu +sizeof(selectApplicationCmd)+1,aid,aidLen);
    apdu[sizeof(selectApplicationCmd)+1+aidLen] = 0x00;

    success = nfc->inDataExchange(apdu, apduSize, data, &responseLength);

    free(apdu);
   
    if (success) //PN532 was succeessful
    {
      #ifndef DEBUG_DISABLED
        debugI("selectApplication is successful,response length:%d",responseLength);   
      #endif
      #ifndef SERIAL_DEBUG_DISABLED
        Serial.printf("selectApplication is successful,response length:%d\n",responseLength);  
      #endif  

      
      putData( responseLength,false);

      nfc->PrintHexChar(data, responseLength);

      //card.putData(response,responseLength);

      if(getSW1() == 0x90 && getSW2() == 0x00) //Card responded successfully 
     {
        #ifndef DEBUG_DISABLED
         debugI("Response word from card was a success");
        #endif
        #ifndef SERIAL_DEBUG_DISABLED
          Serial.println("Response word from card was a success");
        #endif  
          
        //tlvInfo_t *t=(tlvInfo_t *)calloc(responseLength,sizeof(tlvInfo_t));
        //memset(t,0,responseLength);
        //tlvInfo_init(t);
       // int tindex =0;
        //emvparse(response, responseLength, t, &tindex , 0, dict);
        //emvPrint_result(t, tindex);
        //card.parseAndStoreResponse(t, tindex);
        //free(t);
        return true;
        
        }
        else
        {
          #ifndef DEBUG_DISABLED
           debugI("Response word from card was a failure");
         #endif
         #ifndef SERIAL_DEBUG_DISABLED
           Serial.println("Response word from card was a failure");
         #endif  
           
          return false;
        }

    }
    else
    {
      #ifndef DEBUG_DISABLED
        debugI("selectApplication failed");
      #endif
      #ifndef SERIAL_DEBUG_DISABLED
        Serial.println("selectApplication failed");  
      #endif  
      return false;
    }
}



bool Card::genericCardRequest(Kernel & kernel,bool & lastCmd,uint8_t & errorCode)
{
   uint8_t nextCmdSize;
   uint8_t * response;
   bool success;
   uint8_t responseLength ;

   if(nextCmd[cardCmdIndex - 1].appendResponse)
   {
   	response = data + dataLength;
   	responseLength = (sizeof(data)-dataLength < 255)?sizeof(data)-dataLength:255;
   }
   else
   {
   	response = data;
   	dataLength =0;
   	responseLength = (sizeof(data) < 255)?sizeof(data):255;
   }


   if(nextCmd[cardCmdIndex - 1].cmd[0] == 0x80 && nextCmd[cardCmdIndex - 1].cmd[1] == 0xAE)//GAC
   {
      #ifndef DEBUG_DISABLED
       debugD("Last Command");
      #endif
      #ifndef SERIAL_DEBUG_DISABLED
        Serial.printf("Last Command");  
      #endif 
   		lastCmd = true;
   }
 

   success = nfc->inDataExchange(nextCmd[cardCmdIndex - 1].cmd, nextCmd[cardCmdIndex - 1].cmdLength, response, &responseLength);

   
    if (success) //PN532 was succeessful
    {
      #ifndef DEBUG_DISABLED
       debugI("genericCardRequest is successful,response length:%d",responseLength);
      #endif
      #ifndef SERIAL_DEBUG_DISABLED
        Serial.printf("genericCardRequest is successful,response length:%d\n",responseLength);  
      #endif  
      
      nfc->PrintHexChar(response, responseLength);

      putData( responseLength,nextCmd[cardCmdIndex - 1].appendResponse);

      //card.putData(response,responseLength);

      if(getSW1() == 0x90 && getSW2() == 0x00) //Card responded successfully 
     {
      #ifndef DEBUG_DISABLED
        debugI("Response word from card was a success");
      #endif
      #ifndef SERIAL_DEBUG_DISABLED
        Serial.println ("Response word from card was a success");
      #endif
       
        return true;
        
        }
        else
        {
          #ifndef DEBUG_DISABLED
            debugE("Response word from card was a failure");
          #endif
          #ifndef SERIAL_DEBUG_DISABLED
            Serial.println("Response word from card was a failure");  
          #endif  
          return false;
        }

    }
    else
    {
      #ifndef DEBUG_DISABLED
        debugE("Generic card request failed");
      #endif
      #ifndef SERIAL_DEBUG_DISABLED
        Serial.println("Generic card request failed");  
      #endif  
      return false;
    }
}

void Card::cleanUp()
{
	this->stopRF();
}

uint8_t Card::getSW1()
{
	 return data[dataLength - 2];
}
uint8_t Card::getSW2()
{
	 return data[dataLength - 1];
}



void Card::parseAndStoreResponse(tlvInfo_t *t,int index){

//Iterate through and  get the tags available 
while(index >= 0)
{

    switch (t[index].tlv.Tag){

        case 0x84: //DEDICATED_FILE_NAME
            //ddf.setName((byte*)t[index].tlv.Val,(uint8_t)t[index].tlv.Len);
            

        break;

        case 0x88: //SFI
            //ddf.setSFI(t[index].tlv.Val[0]);

        break;

        case 0x9f11: //ISSUER_CODE_TABLE_INDEX

            //ddf.setIssuerCodeTableIndex(t[index].tlv.Val[0]);
        break;

        case 0x4f: //AID_CARD
            setAid((byte*)t[index].tlv.Val,(uint8_t)t[index].tlv.Len);
            

        break;

        case 0x50: //APPLICATION_LABEL
            //app.setAppLabel((byte*)t[index].tlv.Val,(uint8_t)t[index].tlv.Len);
            
        break;

        case 0x87: //APPLICATION_PRIORITY_INDICATOR
            //app.setAppApi(t[index].tlv.Val[0]);
            
        break;

        

    }

    index--;
}



}

void Card::setAid(uint8_t * aid,uint8_t len){
          
    memcpy(this->aid,aid,len);
    aidLen = len;
    
 }
uint8_t Card::getAid(uint8_t * val){

 memcpy(val,this->aid,aidLen);
 
 return aidLen;
    
}

 uint8_t * Card::getData(uint16_t & lengthOut,bool withStatusWord,unsigned long & positionArray) {
    positionArray = dataPosition;
    
 		if(withStatusWord)
        	lengthOut = dataLength;
        else
        	lengthOut = dataLength-2;
        return data;
    }

void Card::putCmd(const char *  cmd, uint8_t length,bool appendResponse) {

		cardFunctions[++cardCmdEndIndex] = &Card::genericCardRequest;
		nextCmd[cardCmdEndIndex].appendResponse = appendResponse;
		nextCmd[cardCmdEndIndex].cmdLength =  length/2;  
        
              
      
         for(uint8_t i = 0; i < length/2; i++)
          {
            byte extract;
            char a = cmd[2*i];
            char b = cmd[2*i + 1];
            extract = Utils::convertCharToHex(a)<<4 | Utils::convertCharToHex(b);
            nextCmd[cardCmdEndIndex].cmd[i] = extract;

          }

          

  }

char Utils::convertCharToHex(char ch)
{
  char returnType;
  switch(ch)
  {
    case '0':
    returnType = 0;
    break;
    case  '1' :
    returnType = 1;
    break;
    case  '2':
    returnType = 2;
    break;
    case  '3':
    returnType = 3;
    break;
    case  '4' :
    returnType = 4;
    break;
    case  '5':
    returnType = 5;
    break;
    case  '6':
    returnType = 6;
    break;
    case  '7':
    returnType = 7;
    break;
    case  '8':
    returnType = 8;
    break;
    case  '9':
    returnType = 9;
    break;
    case  'A':
    returnType = 10;
    break;
    case  'B':
    returnType = 11;
    break;
    case  'C':
    returnType = 12;
    break;
    case  'D':
    returnType = 13;
    break;
    case  'E':
    returnType = 14;
    break;
    case  'F' :
    returnType = 15;
    break;
    default:
    returnType = 0;
    break;
  }
  return returnType;
}
 
