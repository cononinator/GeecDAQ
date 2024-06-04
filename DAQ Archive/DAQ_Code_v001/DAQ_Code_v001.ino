/*
* Author: Conor Farrell & Breandán Gillanders
* Date: 17/02/2024
*
*  Code to test basic FreeRTOS functionality with the ADC
*  Added functionality for the SD Card saving
*

*/

// This is the code being used for Edgeworth's town test on 27/02/2024

#include <Wire.h>
#include <DFRobot_ADS1115.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

//SD Card Variables
#define REASSIGN_PINS
int sck = 48;
int miso = 47;
int mosi = 38;
int cs = 21;
String filename = "/default.txt"; // pretty sure the part before the dot has to be less than or equal to 8 chars (DOS8.3 naming)

DFRobot_ADS1115 ads(&Wire);

//GPS Variables
int PWRPin = D8;
char sim_response[512];
String nmeaSentence;
int ledState = LOW;
const int ledPin = D9;
String response, values;
int startIndex, endIndex = -1;

//Speed Sensor
#define SPEEDPIN D4
const int debounce = 125; //Minimum time between interrupts
double speed;
const double filter = 0.25; //Change this to change how much a new speed effects the recorded speed
unsigned long currSpeedTime, prevSpeedTime = 1;


struct test{
  double speed,iBatt, iMot, vBat, vMot;
  String GPS;
};

test t = {0, 0, 0, 0, 0, ""};

void setup() {

  adcSetup();
 
  Serial0.begin(115200);
  //Serial1.begin(115200, SERIAL_8N1, D2, D3);

  pinMode(ledPin, OUTPUT);
  pinMode(PWRPin, OUTPUT);

  sdSetup();
  setNextFileName("/dynov"); // DOS8.3 naming, must instert 5 char prefix rn

  attachInterrupt(digitalPinToInterrupt(SPEEDPIN), currSpeed, FALLING);

  delay(1000);
 
  xTaskCreate(
                    readADC,          /* Task function. */
                    "readADC",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
 
  gpsSetup();

  xTaskCreate(
                    pollGPSTask,          /* Task function. */
                    "pollGPSTask",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
 
}
 
void loop() {
  // NOTHING
}

void adcSetup(){
  ads.setAddr_ADS1115(ADS1115_IIC_ADDRESS0);   // 0x48
  ads.setGain(eGAIN_TWOTHIRDS);   // 2/3x gain
  ads.setMode(eMODE_SINGLE);       // single-shot mode
  ads.setRate(eRATE_128);          // 128SPS (default) - REFER TO DFRobot_ADS1115.h FOR DIFFERENT DATA RATES
  ads.setOSMode(eOSMODE_SINGLE);   // Set to start a single-conversion
  ads.init();
}

//RTOS Tasks 
void readADC( void * parameter )
{
    for(;;){
 
      //These values need to be fixed
        t.iBatt = 10*(ads.readVoltage(0)/1000.0 - 0.493);
        t.iMot = 10*(ads.readVoltage(1)/1000.0- 0.493);
        t.vMot = 7.51*((float)ads.readVoltage(2));
        t.vBat = 9.01*((float)ads.readVoltage(3));       
        
        xTaskCreate( SaveToSDTask, "SaveToSDTask", 10000, NULL, 2, NULL);
        
        vTaskDelay(pdMS_TO_TICKS((10)));
    }
 
    // Serial1.println("Ending task 1");
    vTaskDelete( NULL );
 
}

void SaveToSDTask( void * parameter)
{
  for(;;){
    // Serial1.printf("%f,%f,%f,%f \n", t.iBatt, t.iMot,t.vBat, t.vMot);
    String iB = String(t.iBatt,3);
    String iM = String(t.iMot,3);
    String vB = String(t.vBat,3);
    String vM = String(t.vMot,3);
    String message = String(millis()) + "," + iB + "," + iM + "," + vB + "," + vM + "," + String(t.speed, 1) + "," + t.GPS + "\n";
    // char messageArray[64] = message;
    appendFile(SD, filename, message);

    vTaskDelete( NULL); //Task deletes itself
  }
}

void pollGPSTask( void *parameter){
  for(;;){
    response = send_at("AT+CGNSINF");
    startIndex = response.indexOf(",") - 1;
    endIndex = response.indexOf(",,");
    if (startIndex +1 != endIndex ) {
        t.GPS = response.substring(startIndex, endIndex);
    }

    // Serial1.println(t.GPS);
    
    vTaskDelay(pdMS_TO_TICKS(1000));

  }
}


//SD Functions
void sdSetup(){
  #ifdef REASSIGN_PINS
    SPI.begin(sck, miso, mosi, cs);
  #endif
  if(!SD.begin(cs)){ //Change to this function to manually change CS pin
    // Serial1.println("Card Mount Failed");
    while (true){;};
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    // Serial1.println("No SD card attached");
    return;
  }
}

String setNextFileName(String prefix) {
  String next = "";
  int highest = -1;
  File root = SD.open("/");
  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      break;
    }
    String name = entry.name();
    if (name.startsWith("dynov") && name.endsWith(".txt")) {
      String numberStr = name.substring(5, 8); // Extract XXX part
      int number = numberStr.toInt();
      if (number > highest) {
        highest = number;
      }
    }
    entry.close();
  }
  highest++;
  char buffer[4]; // Buffer to hold the padded number
  sprintf(buffer, "%03d", highest); // Format the number with leading zeros
  next = prefix + String(buffer) + ".txt";
  filename = next;
  return next;
}

void appendFile(fs::FS &fs, String path, String message){
    // Serial1.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        // Serial1.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        // Serial1.println("Message appended");
    } else {
        // Serial1.println("Append failed");
    }
    file.close();
}

//GPS Functions

void gpsSetup(){
  // Toggle power
  digitalWrite(PWRPin, HIGH);
  delay(1000);
  digitalWrite(PWRPin, LOW);
  delay(1000);

  send_at("AT+CGNSCOLD");

  // Serial1.println("Sending AT+CGNSHOT:");
  // Serial1.println(send_at("AT+CGNSHOT"));

  bool gpsInitialised = false;
  while (gpsInitialised == false){
    gpsInitialised = validResponse(send_at("AT+CGNSINF"));
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    // Serial1.println("Checking");
    delay(1000);
  }
  digitalWrite(ledPin, HIGH);
}

bool get_ok_response(String at_command){
  bool ok = false;
  String response = send_at(at_command);
  int x = response.indexOf("OK");
  if (x != -1){
    ok = true;
  }
  return ok;
}

bool validResponse(String response){
  // Serial1.println(response);
  if (response.indexOf("+CGNSINF: ") != -1){
    int start = response.indexOf(",") - 1;
    int end = response.lastIndexOf(",");
    String values = response.substring(start, end);
    // Serial1.println(values);
    if (values.charAt(0) == '1'){
      if (values.charAt(2) == '1'){
        return true;
      }
    }
  }
  return false;
}

String send_at(String at_command){
  Serial0.flush();
  Serial0.println(at_command);
  while(!Serial0.available()){
    delay(1);  //No operation 
  }

  int t = 0;
  char c = 0;

  sim_response[0] = 0;

  while (Serial0.available()) {
    c = Serial0.read();
    sim_response[t] = c;
    sim_response[++t] = 0;
  }

  return sim_response;
}

//Speed Sensor
void currSpeed(){
  currSpeedTime = millis();
  
  if ( currSpeedTime - prevSpeedTime > debounce ){
    t.speed = 1.0 / ((currSpeedTime-prevSpeedTime) / 1000.0) * 0.5 * 3.14 * 3.6;
    // Serial1.println(t.speed);
    prevSpeedTime = currSpeedTime;
  }
}

void filterSpeed(){
  currSpeedTime = millis();
  
  if ( currSpeedTime - prevSpeedTime > debounce ){
    speed = 1/(currSpeedTime-prevSpeedTime) * 0.5 * 3.14 * 3.6;
    t.speed = t.speed + filter * (speed - t.speed);
    prevSpeedTime = currSpeedTime;
  }
}