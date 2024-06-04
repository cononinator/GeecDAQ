/*
* Author: Conor Farrell & Breandán Gillanders
* Date: 17/02/2024
*
*  Code to test basic FreeRTOS functionality with the ADC
*  Added functionality for the SD Card saving
*
*/

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
String filename = "/fRTOSAD1.txt"; // pretty sure the part before the dot has to be less than or equal to 8 chars (DOS8.3 naming)

DFRobot_ADS1115 ads(&Wire);

struct test{
  double iBatt, iMot, vBat, vMot;
};

test t = {0, 0, 0, 0};

void setup() {

  ads.setAddr_ADS1115(ADS1115_IIC_ADDRESS0);   // 0x48
  ads.setGain(eGAIN_TWOTHIRDS);   // 2/3x gain
  ads.setMode(eMODE_SINGLE);       // single-shot mode
  ads.setRate(eRATE_128);          // 128SPS (default) - REFER TO DFRobot_ADS1115.h FOR DIFFERENT DATA RATES
  ads.setOSMode(eOSMODE_SINGLE);   // Set to start a single-conversion
  ads.init();
 
  Serial.begin(115200);

  sdSetup();

  delay(1000);
 
  xTaskCreate(
                    taskOne,          /* Task function. */
                    "TaskOne",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
 
  // xTaskCreate(
  //                   taskTwo,          /* Task function. */
  //                   "TaskTwo",        /* String with name of task. */
  //                   10000,            /* Stack size in bytes. */
  //                   NULL,             /* Parameter passed as input of the task */
  //                   1,                /* Priority of the task. */
  //                   NULL);            /* Task handle. */
 
}
 
void loop() {
  delay(1000);
  
}
 
void taskOne( void * parameter )
{
 
    for(;;){
 
      //These values need to be fixed
        t.iBatt = 10*(ads.readVoltage(0) - 0.493);
        t.iMot = 10 * (ads.readVoltage(1)- 0.493);
        t.vMot = 7.51*(ads.readVoltage(2));
        t.vBat = 9.01*(ads.readVoltage(3));
        xTaskCreate( SaveToSDTask, "SaveToSDTask", 10000, NULL, 2, NULL);
        vTaskDelay(pdMS_TO_TICKS((50)));
    }
 
    Serial.println("Ending task 1");
    vTaskDelete( NULL );
 
}
 
void taskTwo( void * parameter)
{
 
    for(;;){
 

        vTaskDelay(pdMS_TO_TICKS((750)));
    }
    Serial.println("Ending task 2");
    vTaskDelete( NULL );
 
}

void SaveToSDTask( void * parameter)
{
  for(;;){
    Serial.printf("%f,%f,%f,%f \n", t.iBatt, t.iMot,t.vBat, t.vMot);
    String iB = String(t.iBatt,3);
    String iM = String(t.iMot,3);
    String vB = String(t.vBat,3);
    String vM = String(t.vMot,3);
    String message = iB + "," + iM + "," + vB + "," + vM + "\n";
    // char messageArray[64] = message;
    appendFile(SD, filename, message);

    vTaskDelete( NULL);
  }
}

void sdSetup(){
  #ifdef REASSIGN_PINS
    SPI.begin(sck, miso, mosi, cs);
  #endif
  if(!SD.begin(cs)){ //Change to this function to manually change CS pin
    Serial.println("Card Mount Failed");
    while (true){;};
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }
}

void setFileName(String name){
  Serial.printf("Filename: %s", name);
  filename = name;  
}

void appendFile(fs::FS &fs, String path, String message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}