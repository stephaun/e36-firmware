#include <SFE_MG2639_CellShield.h>

const enum command {
  
};

bool checkModuleOn() {
  if (checkGPSsetup() || checkGSMsetup()) {
    return true;
  } else {
    return false;
  }
}

bool turnModuleOn() {
  return true;
}

void enableGSMtoHWuart() {
  digitalWrite(gpsEN, LOW);
  digitalWrite(gsmEN, HIGH);
}

bool checkGSMsetup() {
  enableGSMtoHWuart();
  if (checkATOK()) {
    return true;
  } else {
    return false;
  }
}

//bool initMG2639() {
//  // Quick test to see if the module is already on.
//  gsm.begin(MAX_SW_SERIAL);
//  for (int i=0; i<5; i++) {
//    if(checkATOK()) return true;
//    delay(100);
//  }
//  gsm.end(); // That failed
//
//  con.println("Turning on the MG2639 Module...");
//  digitalWrite(gxxOnOff, HIGH); // Turn the module on
//  delay(3000);
//  digitalWrite(gxxOnOff, LOW); // Leave the module on
//  con.println("The MG2639 Module should now be booted...");
//
//  gsm.begin(115200);
//  delay(10);
//  String nbCommand = String("AT+IPR=") + MAX_SW_SERIAL + String("\r\n");
//  gsm.print(nbCommand); // Set the new baud rate
//  delay(10);
//  gsm.end();
//  gsm.begin(MAX_SW_SERIAL);
//  delay(10);
//
//  // Test again to verify the module is on.
//  for (int i=0; i<5; i++) {
//    if(checkATOK()) return true;
//    delay(100);
//  }
//
//  return false;
//}

bool checkATOK() {
  if (sendCommand("") == "OK") {
    return true;
  } else {
    return false;
  }
}

//This sends a command and checks that the response is valid (not error)
//This function is not pretty but it works. Improvments are welcomed.
//For each AT command sent, two to three reponses come back. See ATQuery-Response_Example.png for more info

//Response example: "AT+CSQ\r\r\n+CSQ: 4, 99\r\n\r\nOK\r\n"
//We need to strip off the echoed command, check that we got an OK on the 3rd chunk
//And return the 2nd chunk

//Chunk 1: copy/echo of the command
//Chunk 2: The response data or OK or ERROR
//Chunk 3: OK or nothing

#define ERROR_NOCHARACTERS  "NOCHAR"
#define ERROR_COMMAND_STRING_MISMATCH "MISMATCH"
#define ERROR_RESPONSE2  "REPONSE2ERROR"
#define ERROR_RESPONSE3  "REPONSE3ERROR"

String sendCommand(String command) {
  String firstChunk; //No command should be longer than 20
  String secondChunk; //Not sure but responses can be large
  String thirdChunk; //Should always be 'OK' or empty

  boolean breakFlag = false;
  int newlineCount = 0;

  String strToSend = "AT" + command + "\r"; //Commands get sent with *only* a \r.
  //Adding a \n will cause the parser to break

  con.print(F("Sending command: "));
  con.println(strToSend);

  while(gsm.available()) gsm.read(); //Remove everything in the buffers
  gsm.print(strToSend); //Send this string to the module

  for(byte x=0; x<100 ; x++) {
    if(gsm.available()) break; //Wait until we have some characters
    delay(1);
  }

  delay(55); //We need a few ms at 9600 to get. 15ms works well

  while(breakFlag == false) {
    if(gsm.available() == false) return(ERROR_NOCHARACTERS); //This shouldn't happen
    char incoming = gsm.read();

    switch(incoming) {
    case '\r':  // We're done!
        breakFlag = true;
        firstChunk += incoming;
        break;
    default:    // continue
        firstChunk += incoming;
        break;
    }
  }

  //Serial.print("Chunk1: ");
  //Serial.println(firstChunk);

  //First test
  if(firstChunk != strToSend) {
    con.print("Chunk1: ");
    con.println(firstChunk);
    return(ERROR_COMMAND_STRING_MISMATCH);
  }

  //Reset the variables
  breakFlag = false;
  newlineCount = 0;

  while(breakFlag == false) {
    if(gsm.available() == false) return(ERROR_NOCHARACTERS); //This shouldn't happen
    char incoming = gsm.read();

    switch(incoming) {
    case '\r':  //Ignore it
        break;
    case '\n':
        newlineCount++;
        if(newlineCount == 1) {
          //Ignore it
        } else if(newlineCount == 2) {
          //We're done!
          breakFlag = true;
          secondChunk += incoming;
        }
        break;
    default:
        secondChunk += incoming;
        break;
    }
  }

  //Serial.print("Chunk2: ");
  //Serial.println(secondChunk);

  //Second test
  if(secondChunk == "OK\n") {
    //We're ok, this command just doesn't return any data
    return("OK"); //Ignore trailing \n
  } else if (secondChunk == "ERROR") {
    //This might be an error
    return(ERROR_RESPONSE2);
  }

  //Reset the variables
  breakFlag = false;
  newlineCount = 0;

  while(breakFlag == false) {
    if(gsm.available() == false) return(ERROR_NOCHARACTERS); //This shouldn't happen
    char incoming = gsm.read();

    switch(incoming) {
    case '\r': //Ignore it
        break;
    case '\n':
        newlineCount++;
        if(newlineCount == 1) {
          //Ignore it
        } else if(newlineCount == 2) {
          //We're done!
          breakFlag = true;
          //thirdChunk += incoming; //Ignore the final \n
        }
        break;
    default:
        thirdChunk += incoming;
        break;
    }
  }

  if(thirdChunk == "OK") {
    //We're so good!
    command.replace("?", ""); //Some commands (+CREG?) use ? but reponses do not have them. This removes the ? from the command
    //Now remove the original command from the response.
    //For example: +CSQ: 4, 99 should become " 4, 99"
    secondChunk.replace(command + ":", "");
    secondChunk.trim(); //Get rid of any leading white space
    return(secondChunk); //Report this chunk to the caller
  } else {
    //This is bad. Probably 'Error'
    con.print("Chunk 3: ");
    con.print(thirdChunk);
    return(ERROR_RESPONSE3);
  }
}

bool gsm_setup() {
  #ifdef DEBUG
    con.println("[GSM_SETUP] INFO: Begin");
  #endif

  #ifdef DEBUG
    con.println("[GSM_SETUP] INFO: End");
  #endif
  return true;
}

bool gsm_loop() {
  #ifdef DEBUG
    con.println("[GSM_LOOP] INFO: Begin");
  #endif

  #ifdef DEBUG
    con.println("[GSM_LOOP] INFO: End");
  #endif
  return true;
}
