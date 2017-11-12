#include <MicroNMEA.h>

char nmeaBuffer[128];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

void printUnknownSentence(MicroNMEA& nmea) {
  con.print("Unknown sentence: ");
  con.println(nmea.getSentence());
}

void enableGPStoHWuart() {
  digitalWrite(gsmEN, LOW);
  digitalWrite(gpsEN, HIGH);
}

bool checkGPSsetup() {
  if (nmea.isValid()) {
    return true;
  } else {
    enableGPStoHWuart();

  }

//  enableGPStoHWuart();
//  if (hws.) {
//
//  } else {
//
//  }
}


bool gps_setup() {
  #ifdef DEBUG
    con.println("[GPS_SETUP] INFO: Begin");
  #endif
  // Empty the input buffer.
  //while (gps.available()) gps.read();
  //con.println("... done");

  // Clear the list of messages which are sent.
  // MicroNMEA::sendSentence(gps, "$PORZB");
  // Send only RMC and GGA messages.
  // MicroNMEA::sendSentence(gps, "$PORZB,RMC,1,GGA,1");
  // Disable compatability mode (NV08C-CSM proprietary message) and
  // adjust precision of time and position fields
  // MicroNMEA::sendSentence(gps, "$PNVGNME,2,9,1");
  // MicroNMEA::sendSentence(gps, "$PONME,2,4,1,0");

  #ifdef DEBUG
    con.println("[GPS_SETUP] INFO: End");
  #endif
  return true;
}

bool skip = true;

bool gps_loop() {
  #ifdef DEBUG
    con.println("[GPS_LOOP] INFO: Begin");
  #endif

  // while (gps.available()) {
  //   char c = gps.read();
  //   con.print(c);
  //   nmea.process(c);
  //   skip = false;
  // }
  // if (!skip) {
  //   con.print("Nav. system: ");
  //   if (nmea.getNavSystem())
  //     con.println(nmea.getNavSystem());
  //   else
  //     con.println("none");
  //
  //   con.print("Num. satellites: ");
  //   con.println(nmea.getNumSatellites());
  //
  //   con.print("HDOP: ");
  //   con.println(nmea.getHDOP()/10., 1);
  //
  //   con.print("Date/time: ");
  //   con.print(nmea.getYear());
  //   con.print('-');
  //   con.print(int(nmea.getMonth()));
  //   con.print('-');
  //   con.print(int(nmea.getDay()));
  //   con.print('T');
  //   con.print(int(nmea.getHour()));
  //   con.print(':');
  //   con.print(int(nmea.getMinute()));
  //   con.print(':');
  //   con.println(int(nmea.getSecond()));
  //
  //   long latitude_mdeg = nmea.getLatitude();
  //   long longitude_mdeg = nmea.getLongitude();
  //   con.print("Latitude (deg): ");
  //   con.println(latitude_mdeg / 1000000., 6);
  //
  //   con.print("Longitude (deg): ");
  //   con.println(longitude_mdeg / 1000000., 6);
  //
  //   long alt;
  //   con.print("Altitude (m): ");
  //   if (nmea.getAltitude(alt))
  //     con.println(alt / 1000., 3);
  //   else
  //     con.println("not available");
  //
  //   con.print("Speed: ");
  //   con.println(nmea.getSpeed() / 1000., 3);
  //   con.print("Course: ");
  //   con.println(nmea.getCourse() / 1000., 3);
  //
  //   con.println("-----------------------");
  //   nmea.clear();
  // } else {
  //   skip = true;
  // }

  // Output GPS information from previous second
  //con.print("Valid fix: ");
  //con.println(nmea.isValid() ? "yes" : "no");
  #ifdef DEBUG
    con.println("[GPS_LOOP] INFO: End");
  #endif
  return true;
}
