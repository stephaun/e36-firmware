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

BLEService ledService("a14a0000-cd9c-4a64-90ee-28b22f978bcd");
BLECharCharacteristic switchChar("a14a0001-cd9c-4a64-90ee-28b22f978bcd", BLERead | BLEWrite);



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

bool bluetooth_setup() {
  #ifdef DEBUG
    con.println("[BLU_SETUP] INFO: Begin");
  #endif

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

  #ifdef DEBUG
    con.println("[BLU_SETUP] INFO: End");
  #endif
  return true;
}

bool bluetooth_loop() {
  #ifdef DEBUG
    con.println("[BLU_LOOP] INFO: Begin");
  #endif


  BLE.poll();

  #ifdef DEBUG
    con.println("[BLU_LOOP] INFO: End");
  #endif
  return true;
}
