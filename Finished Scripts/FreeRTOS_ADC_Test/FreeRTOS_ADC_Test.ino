/*
* Author: Conor Farrell & Breandán Gillanders
* Date: 15/02/2024
*
*  Code to test basic FreeRTOS functionality with the ADC
*
*/

#include <Wire.h>
#include <DFRobot_ADS1115.h>

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
        t.vMot = 7.51*(ads.readVoltage(3));
        t.vBat = 9.01*(ads.readVoltage(2));
        xTaskCreate( printTask, "printTask", 10000, NULL, 2, NULL);
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

void printTask( void * parameter)
{
  for(;;){
    Serial.printf("%f,%f,%f,%f \n", t.iBatt, t.iMot,t.vBat, t.vMot);
    vTaskDelete( NULL);
  }
}