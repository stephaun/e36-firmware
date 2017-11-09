#include <SoftwareSerial.h>

#include <SFE_MG2639_CellShield.h>
#include <MicroNMEA.h>

#include <BLEService.h>
#include <BLECentral.h>
#include <BLEAttributeWithValue.h>
#include <BLECommon.h>
#include <BLEDevice.h>
#include <BLETypedCharacteristic.h>
#include <CurieBLE.h>
#include <BLEDescriptor.h>
#include <BLEPeripheral.h>
#include <BLECharacteristic.h>
#include <BLETypedCharacteristics.h>

#define MAX_SW_SERIAL 57600
//#define DEBUG_LOGS 1

const int ledPin    = 13; // set ledPin to use on-board LED
const int resetPin  = 9;
const int pulseTime = 8;

const int gsmRX     = 2; // 0 for hardware UART, 2 software UART
const int gsmTX     = 3; // 1 for hardware UART, 3 software UART
const int gsmEN     = 10;

const int gpsRX     = 8; // 0 for hardware UART, 8 software UART
const int gpsTX     = 9; // 1 for hardware UART, 9 software UART
const int gpsEN     = 11;

const int gxxReset  = 6;
const int gxxOnOff  = 7;

HardwareSerial& con = Serial;   // Console interface to Braswell
HardwareSerial& hws = Serial1;  // Serial interface on pins 0 & 1 
SoftwareSerial  gsm(gsmRX, gsmTX);
SoftwareSerial  gps(gpsRX, gpsTX);

char nmeaBuffer[128];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

void printUnknownSentence(MicroNMEA& nmea) {
  con.println();
  con.print("Unknown sentence: ");
  con.println(nmea.getSentence());
}

BLEService ledService("a14a0000-cd9c-4a64-90ee-28b22f978bcd");
BLECharCharacteristic switchChar("a14a0001-cd9c-4a64-90ee-28b22f978bcd", BLERead | BLEWrite);

void setup() {
  pinMode(gpsEN, OUTPUT);
  digitalWrite(gpsEN, LOW);
  pinMode(gsmEN, OUTPUT);
  digitalWrite(gsmEN, LOW);
  
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, HIGH);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  pinMode(gxxReset, OUTPUT);
  digitalWrite(gxxReset, LOW); // Dont reset.
  pinMode(gxxOnOff, OUTPUT);
  digitalWrite(gxxOnOff, LOW); // Dont turn on.

  
  while (!con) ;  // Required for Arduino 101 to finish booting;
                  // so we dont miss serial data after a reset.    
  con.begin(9600);
  hws.begin(115200);

  con.println("Checking MG2639 module ...");
  if (!checkModuleOn()) { 
    con.println("It's not on... enabling now.");
    if (turnModuleOn()) {
      con.println("Successfully turned the module on...");
    } else {
      con.println("Failed to turn the module on...");
      // return an error message to Braswell
      // Setup a timer to automatically retry every 10 seconds.
    } 
  }
  
  // Empty the input buffer.
  //  while (gps.available()) gps.read();
  //con.println("... done");

  // Clear the list of messages which are sent.
//  MicroNMEA::sendSentence(gps, "$PORZB");
  // Send only RMC and GGA messages.
//  MicroNMEA::sendSentence(gps, "$PORZB,RMC,1,GGA,1");
  // Disable compatability mode (NV08C-CSM proprietary message) and
  // adjust precision of time and position fields
//  MicroNMEA::sendSentence(gps, "$PNVGNME,2,9,1");
  // MicroNMEA::sendSentence(gps, "$PONME,2,4,1,0");

  con.println("Setting up the Bluetooth interface!");
  BLE.begin();
  BLE.setLocalName("BMW_E36");
  BLE.setAdvertisedService(ledService);

  switchChar.setEventHandler(BLEWritten, switchCharacteristicWritten);
  switchChar.setValue(0);

  ledService.addCharacteristic(switchChar);

  BLE.addService(ledService);
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  BLE.advertise();
  con.println("Bluetooth device active, waiting for connections...");
}
bool skip = true;

void loop() {


  while (gps.available()) {
    char c = gps.read();
    con.print(c);
    nmea.process(c);
    skip = false;
  }
  if (!skip) {
    con.print("Nav. system: ");
    if (nmea.getNavSystem())
      con.println(nmea.getNavSystem());
    else
      con.println("none");

    con.print("Num. satellites: ");
    con.println(nmea.getNumSatellites());

    con.print("HDOP: ");
    con.println(nmea.getHDOP()/10., 1);
    
    con.print("Date/time: ");
    con.print(nmea.getYear());
    con.print('-');
    con.print(int(nmea.getMonth()));
    con.print('-');
    con.print(int(nmea.getDay()));
    con.print('T');
    con.print(int(nmea.getHour()));
    con.print(':');
    con.print(int(nmea.getMinute()));
    con.print(':');
    con.println(int(nmea.getSecond()));

    long latitude_mdeg = nmea.getLatitude();
    long longitude_mdeg = nmea.getLongitude();
    con.print("Latitude (deg): ");
    con.println(latitude_mdeg / 1000000., 6);

    con.print("Longitude (deg): ");
    con.println(longitude_mdeg / 1000000., 6);

    long alt;
    con.print("Altitude (m): ");
    if (nmea.getAltitude(alt))
      con.println(alt / 1000., 3);
    else
      con.println("not available");

    con.print("Speed: ");
    con.println(nmea.getSpeed() / 1000., 3);
    con.print("Course: ");
    con.println(nmea.getCourse() / 1000., 3);

    con.println("-----------------------");    
    nmea.clear();
  } else {
    skip = true;  
  }
  
  // Output GPS information from previous second
  //con.print("Valid fix: ");
  //con.println(nmea.isValid() ? "yes" : "no");
  
  BLE.poll();
}

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

void enableGPStoHWuart() {
  digitalWrite(gsmEN, LOW);
  digitalWrite(gpsEN, HIGH);
}

void enableGSMtoHWuart() {
  digitalWrite(gpsEN, LOW);
  digitalWrite(gsmEN, HIGH);  
}

bool checkGPSsetup() {
//  enableGPStoHWuart();
//  if (hws.) {
//    
//  } else {
//    
//  }      
}

bool checkGSMsetup() {
  enableGSMtoHWuart();
  if (checkATOK()) {
    return true;
  } else {
    return false;
  }
}

void blePeripheralConnectHandler(BLEDevice central) {
  Serial.print("Connected to central: ");
  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
}

void sendPowerSignal() {
  digitalWrite(resetPin, HIGH);
  // This is part of the UDOO spec, need to send at least 5
  // HIGH-LOW transitions within 100ms to power cycle the UDOO
  for (int i=0; i<5; i++) {
    digitalWrite(resetPin, LOW);
    delay(pulseTime);
    digitalWrite(resetPin, HIGH);
    delay(pulseTime);
  }
}

void switchCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
    // central wrote new value to characteristic, update LED
    Serial.print("Characteristic event, written: ");

    if (switchChar.value()) {
      Serial.println("LED on");
      digitalWrite(ledPin, HIGH);
      switchChar.setValue(0);
      delay(1000);
      //sendPowerSignal();  
    } else {
      Serial.println("LED off");
      digitalWrite(ledPin, LOW);
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

