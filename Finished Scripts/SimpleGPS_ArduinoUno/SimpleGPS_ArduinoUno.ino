/*
 * Adapted from Aoife Prendergast project
 * This version works on the Arduino Uno
 * Should be able to convert to Nano ESP32 by only using hardware serial port - can't print to serial monitor
 *
 * Code for reading GPS data off SIM7070G IoTHat
 * 
 * Arduino pin 7 connected to SIM7070G TX pin
 * Arduino pin 8 connected to SIM7070G RX pin
 * Arduino pin 9 connected to SIM7070G PWR pin
 * 
 */

#include "SoftwareSerial.h"
#include "Wire.h"
#include "elapsedMillis.h"

#define COMMANDMAX 512

#define SIM7070G_GPS_PWR 9

#define LED_PIN 13

/*
     TELEMETRY DATA GLOBAL VARIABLES

*/

SoftwareSerial sim7070G_gps(7, 8); // Arduino RX, TX  --  SIM7070G TX, RX

char sim_response[COMMANDMAX];  // AT command result buffer

bool blinkState = false;

int x;
elapsedMillis everySecond;

// individual strings to store daq data w/ default values
String gps_info;
String connection;
String ueInfo;
String daq2020data;
String accelerometerData;
String signalQuality;

// Combination the 6 above
String liveSDData;

/*
    End of defition of global variables
*/

// *************************************************************************************************//

void setup() {
  pinMode(SIM7070G_GPS_PWR, OUTPUT);
  pinMode(LED_PIN, OUTPUT);           // configure LED pin

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("\n\t  --- START OF PROGRAM -- \n");

  // ------------------ GPS IoT Hat Setup ------------------- //
  Serial.println("\nStarting IoT Boards setup... ");
  Serial.print("\n\n \t *** GPS SET UP *** \n");

  // set the data rate for the SoftwareSerial port
  sim7070G_gps.begin(9600);

  sim7070G_gps.flush();
  Serial.flush();
  delay(50);

  Serial.println(F("Configuring to 9600 baud"));
  send_at(&sim7070G_gps, "AT+IPR=9600"); // Set baud rate

  get_ok_response(&sim7070G_gps, "AT", SIM7070G_GPS_PWR);

  send_at(&sim7070G_gps, "AT+CGNSPWR=1");

  sim7070G_gps.flush();
  Serial.flush();
  delay(50);

  // Initialize variables values
  gps_info = "";
  everySecond = 0;
  x = 0;
}

void loop() { // run over and over

  if ( everySecond >= 1000 ) {

    everySecond = everySecond - 1000;

    Serial.print("\n\t**************");
    Serial.print( (String)(millis() / 1000) );
    Serial.println("**************");

    // blink LED to indicate activity
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);

    
    gps_info = send_at(&sim7070G_gps, "AT+CGNSINF");
    Serial.println(gps_info);
    gps_info.remove(0, 23);
    x = gps_info.indexOf("\r\n\r\nOK\r\n");
    if ( x != -1) {
      gps_info.remove(x, 14);
    }
  }
}

// *************************************************************************************************//
//                                          FUNCTIONS                                               //
// *************************************************************************************************//

void power(int power_pin) {
  digitalWrite(power_pin, HIGH);
  delay(2000);
  digitalWrite(power_pin, LOW);
  delay(5000);
}


String send_at(SoftwareSerial * sim7070G, String at_command) {
  // By default, the last intialized port is listening.
  // when you want to listen on a port, explicitly select it
  sim7070G->listen();

  Serial.flush();
  sim7070G->flush();
  delay(10);

  int t = 0;
  char c = 0;

  //Serial.print("Sending AT Command: ");
  //Serial.println(at_command);

  sim7070G->println(at_command);
  delay(150);

  if ( at_command == "AT+SMCONN") {
    //Serial.println("Waiting longer");
    delay(8000);
  }

  sim_response[0] = 0;

  while ( sim7070G->available() ) {
    c = sim7070G->read();
    sim_response[t] = c;
    sim_response[++t] = 0;

  }
  //Serial.print("Sim7070 Response: ");
  //Serial.println(sim_response);

  return sim_response;
}

void get_ok_response(SoftwareSerial * sim7070G, String comm, int pin) {
  // By default, the last intialized port is listening.
  // when you want to listen on a port, explicitly select it
  sim7070G->listen();

  bool ok = false;
  int count = 0;

  while (!ok) {

    String response = send_at(sim7070G, comm);

    int x = response.indexOf("OK");
    if (x != -1) {
      ok = true;
    }
    count++;
    if ( (ok == false) && (count % 3 == 0) ) {
      Serial.println("Attempting hitting power button!");
      power(pin);
    }
  }
}
