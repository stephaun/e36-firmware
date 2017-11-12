#include <SoftwareSerial.h>

#define MAX_SW_SERIAL 57600
#define DEBUG 1

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

  //con.begin(9600);
  con.begin(115200);
  hws.begin(115200);

  #ifdef DEBUG
  con.println("[SETUP] INFO: Begin");
  #endif

  if ( ! gsm_setup() ) {
    #ifdef DEBUG
      con.println("[SETUP] ERROR: gsm_setup");
    #endif
  }
  if ( ! gps_setup() ) {
    #ifdef DEBUG
      con.println("[SETUP] ERROR: gps_setup");
    #endif
  }
  if ( ! bluetooth_setup() ) {
    #ifdef DEBUG
      con.println("[SETUP] ERROR: bluetooth_setup");
    #endif
  }

  #ifdef DEBUG
  con.println("[SETUP] INFO: End");
  #endif

}

#define GSM_WATCH_TIME 250
#define GPS_WATCH_TIME 1000
#define BLU_WATCH_TIME 400

unsigned long lastGsmMillis = 0;
unsigned long lastGpsMillis = 0;
unsigned long lastBluMillis = 0;

void loop() {

    unsigned long currentMillis = millis();
    if (currentMillis - lastGsmMillis >= GSM_WATCH_TIME) {
      lastGsmMillis = currentMillis;
      if ( ! gsm_loop() ) {
        #ifdef DEBUG
          con.println("[MAIN] ERROR: gsm_loop");
        #endif
      }
    }

    unsigned long currentMillis = millis();
    if (currentMillis - lastGpsMillis > GPS_WATCH_TIME) {
      lastGpsMillis = currentMillis;
      if ( ! gps_loop() ) {
        #ifdef DEBUG
          con.println("[MAIN] ERROR: gps_loop");
        #endif
      }
    }

    unsigned long currentMillis = millis();
    if (currentMillis - lastBluMillis > BLU_WATCH_TIME) {
      lastBluMillis = currentMillis;
      if ( ! bluetooth_loop() ) {
        #ifdef DEBUG
          con.println("[MAIN] ERROR: bluetooth_loop");
        #endif
      }
    }

}
