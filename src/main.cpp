
#include <Wire.h> //For I2C
#include <SD.h>


#include <ESP8266React.h>
#include <TransactionStateService.h>
#include <functional>



#include "Kernel.h"
#include "Card.h"
#include "Terminal.h"
#include "Display.h"
#include "Debug.h"

#include "UI.h"

#define SERIAL_BAUD_RATE 115200



#ifndef DEBUG_DISABLED // Only if debug is not disabled (for production/release)
// Instance of RemoteDebug
RemoteDebug Debug;
void processCmdRemoteDebug() ;
#endif




Kernel  kernel;
Display display(true, true, true);
Card  card(display);

//TODO - Has to come from Login
char autherisationToken[] = "eyJhbGciOiJSUzI1NiIsImtpZCI6IkQyQjY5NDI2RUNFQzk1MDQxMjA1MkU0NDUyRjJERkVIIiwidHlwIjoiYXQrand0In0.eyJuYmYiOjE2MDA4NzQzMTgsImV4cCI6MTYyMDg3NTIxOCwiaXNzIjoic2VjdXJlLnNhZmV3ZWJzZXJ2aWNlcy5jb20iLCJzdWIiOiJ1MTM1NTEwNSIsImlkcCI6ImxvY2FsIiwianRpIjoiYWE3N2FiMTAtNzM2OS00ZjBlLWEyY2YtMGQ0Y2E4ZjZjYTExIiwic2lkIjoiZTAzOTNlOGFmY2QzNDMxODIwOWEyZjRhZWViNTdmY2QiLCJpYXQiOjE2MDA4NzQzMTgsInNjb3BlIjpbInRyYW5zYWN0aW9uLnByb2Nlc3MuYWxsIiwidHJhbnNhY3Rpb24ucXVlcnkuYWxsIl0sImh0dHBzOi8vc2VjdXJlLnNhZmV3ZWJzZXJ2aWNlcy5jb20vand0L2NsYWltcyI6eyJ4X2RldmljZV9pZCI6Ijk5NjU0OSA1YS0wMDUyLTQ5MTAtYjNiNi0zZGQwZjRjMTVieHkiLCJ4X2VudGl0eV9pZCI6IjY3OTcwOCIsInhfZW50aXR5X3R5cGUiOiJtZXJjaGFudCIsInhfY2xpZW50X2lkIjoiOTk2NTQ5NWEtMDA1Mi00OTEwLWIzYjYtM2RkMGY0YzE1YmNjIiwieF9hdXRoX3RpbWUiOjE2MDA4NzQzMTgsInhfcmVxdWVzdF9pcCI6IjEyMi4xNjkuMTAuNzQifX0.H8Fs3JCQqk3T0NWznOQ1YK2GGg1_40JM8zLU3Ntrlf77GPRKuMZsA5CvogutRSR861eIKxR45mRK4aecCImhGqJhCjIqQwBhGHswzw0nYbSJkUx39wppGkbgFP_qSlrzNYZxdQvsqYBoZ67RshetFnBmk9IPVpz8EMMUPnNjSAhPqqKL3XYOb_ML-JdfsogwTanMDVRYEMWN2OpOOPOgzzLRAJp60F1h6Zn4_L7d1VbHeF4v9PSgmEYOsccPolzYavn72SCKmKSNVCNWqz3N8a6EudIBA08UVMz4T8mLSrKnZSerlo0AFP6NM8WCrnevJR0zo-GuoVsz3Ig9HB5jfQeyJhbGciOiJSUzI1NiIsImtpZCI6IkQyQjY5NDI2RUNFQzk1MDQxMjA1MkU0NDUyRjJERkVIIiwidHlwIjoiYXQrand0In0.eyJuYmYiOjE2MDA4NzQzMTgsImV4cCI6MTYxMDg3NTIxOCwiaXNzIjoic2VjdXJlLnNhZmV3ZWJzZXJ2aWNlcy5jb20iLCJzdWIiOiJ1MTM1NTEwNSIsImlkcCI6ImxvY2FsIiwianRpIjoiYWE3N2FiMTAtNzM2OS00ZjBlLWEyY2YtMGQ0Y2E4ZjZjYTExIiwic2lkIjoiZTAzOTNlOGFmY2QzNDMxODIwOWEyZjRhZWViNTdmY2QiLCJpYXQiOjE2MDA4NzQzMTgsInNjb3BlIjpbInRyYW5zYWN0aW9uLnByb2Nlc3MuYWxsIiwidHJhbnNhY3Rpb24ucXVlcnkuYWxsIl0sImh0dHBzOi8vc2VjdXJlLnNhZmV3ZWJzZXJ2aWNlcy5jb20vand0L2NsYWltcyI6eyJ4X2RldmljZV9pZCI6Ijk5NjU0OSA1YS0wMDUyLTQ5MTAtYjNiNi0zZGQwZjRjMTVieHkiLCJ4X2VudGl0eV9pZCI6IjY3OTcwOCIsInhfZW50aXR5X3R5cGUiOiJtZXJjaGFudCIsInhfY2xpZW50X2lkIjoiOTk2NTQ5NWEtMDA1Mi00OTEwLWIzYjYtM2RkMGY0YzE1YmNjIiwieF9hdXRoX3RpbWUiOjE2MDA4NzQzMTgsInhfcmVxdWVzdF9pcCI6IjEyMi4xNjkuMTAuNzQifX0.Xgthdpd79MARm02_7Ph8Bs1AvfQ_DciJv2Dme0n-N99NbkoVxH3ZfT4CwphbAJpMVRk-0-mm6ejZ25lsFrZuEArz_FYXkffcthVhSsy25NuKS30yiAffOX9f_gZQMcMMrnpatIabuGyezhEpREPXQABD0tOBz3itBSgCSThYoUeTX66ZKs53y2SRkcEGAqQRdEgoocQhDOK17F4E6vUaDLE_ug0_LCvf0yWN9UfVB0fxv-drR34W0-Q_kgrsSq9c5OVxcxYgenHFBsEiAip3z9Xv3x759423-MS2j6PCVLlY4S7Q8PU4R6YCsg5WGraS3gqaCNIyHNtkzVg9mbK9Zg";


AsyncWebServer server(80);
ESP8266React esp8266React(&server);


TransactionStateService txnStateService = TransactionStateService(&server,
                    esp8266React.getSecurityManager());

Terminal terminal;


  void txnStateUpadteInfo(const String& originId) {
    Serial.print("The txn's state has been updated by: "); 
    Serial.println(originId); 
    heap_caps_check_integrity_all(true);
    if(originId != "Terminal")
      txnStateService.read(std::bind(&Terminal::startTxn,terminal,std::placeholders::_1));
  }



void setup() {
  // start serial and filesystem
  Serial.begin(SERIAL_BAUD_RATE);

  // start the framework and demo project
  esp8266React.begin();


  #ifndef DEBUG_DISABLED // Only for development
  Debug.begin("CloudPoS"); // Initialize the WiFi server
  //Debug.setPassword("r3m0t0."); // Password for WiFi client connection (telnet or webapp)  ?
  Debug.setResetCmdEnabled(true); // Enable the reset command
  Debug.showProfiler(true); // Profiler (Good to measure times, to optimize codes)
  Debug.showColors(true); // Colors
  Debug.setSerialEnabled(false); // if you wants serial echo - only recommended if ESP is plugged in USB
  String helpCmd = "ProcessEMV - Start the card processing\n";
  //helpCmd.concat("bench2 - Benchmark 2");

  Debug.setHelpProjectsCmds(helpCmd);
  Debug.setCallBackProjectCmds(&processCmdRemoteDebug);

  // End of setup - show IP

  Serial.println("* Arduino RemoteDebug Library");
  Serial.println("*");
  Serial.print("* WiFI connected. IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("*");
  Serial.println("* Please use the telnet client (telnet for Mac/Unix or putty and others for Windows)");
  Serial.println("* or the RemoteDebugApp , download from https://github.com/JoaoLopesF/RemoteDebugApp");
  Serial.println("*");
  Serial.println("* This sample will send messages of debug in all levels.");
  Serial.println("*");
  Serial.println("* Please try change debug level in client (telnet or web app), to see how it works");
  Serial.println("*");

  #endif
  
  display.initDisplay();


  card.initNFCMod();

  
  txnStateService.begin();

  update_handler_id_t myUpdateHandler = txnStateService.addUpdateHandler(txnStateUpadteInfo);

  server.begin();

  delay(500);


}

void loop() {
  // run the framework's loop function
  esp8266React.loop();

  #ifndef DEBUG_DISABLED
  // RemoteDebug handle (for WiFi connections)
  Debug.handle();
#endif
display.loop();
yield();

}


#ifndef DEBUG_DISABLED

// Process commands from RemoteDebug

void processCmdRemoteDebug() {

  String lastCmd = Debug.getLastCommand();

  
}
#endif
