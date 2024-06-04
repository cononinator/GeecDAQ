#include <Wire.h>
#include <Adafruit_ADS1X15.h>
// #include "Longan_I2C_CAN_Arduino.h"

Adafruit_ADS1115 ads;

//Speed Sensor
#define SPEEDPIN D4
const int debounce = 125; //Minimum time between interrupts
double speed;
const double filter = 0.25; //Change this to change how much a new speed effects the recorded speed
unsigned long currSpeedTime, prevSpeedTime = 1;

struct data_t{
  long time;
  double speed,iBatt, iMot, vBat, vMot;
  String GPS;
};
data_t data = {0, 0, 0, 0, 0, 0, ""};

static TaskHandle_t I2CactiveTask = NULL;


typedef union{
  float val;
  unsigned char bytes[4];
} FLOATUNION_t;
FLOATUNION_t canBuff; //RENAME To More Descriptive name


#define CANADDRESS 0x25
// I2C_CAN CAN(0x25);                                    // Set CS pin

void setup()
{
    Serial.begin(115200);
    adcSetup();
    xTaskCreate(
                readADC,          /* Task function. */
                "readADC",        /* String with name of task. */
                10000,            /* Stack size in bytes. */
                NULL,             /* Parameter passed as input of the task */
                1,                /* Priority of the task. */
                NULL);            /* Task handle. */
    xTaskCreate(
                canTask,
                "canTask",
                10000,
                NULL,
                1,
                NULL
                );
}

void loop()
{}


void canTask( void * parameter ){
  for(;;){
    if (I2CactiveTask == NULL){
      I2CactiveTask = xTaskGetCurrentTaskHandle();
      // 0x01: Speed id, 0x02: Current id, 0x03: Voltage id
      canBuff.val = data.speed;
      Serial.printf("Speed canBuff.val = %f\n", data.speed);
      sendMsgBuf(0x01, 0,0, 4, canBuff.bytes);

      canBuff.val = data.iBatt;
      Serial.printf("Current canBuff.val = %f\n", data.iBatt);
      sendMsgBuf(0x02, 0,0, 4, canBuff.bytes);

      canBuff.val = data.vBat;
      Serial.printf("Voltage canBuff.val = %f\n", data.vBat);
      sendMsgBuf(0x03, 0,0, 4, canBuff.bytes);


      I2CactiveTask = NULL;
      vTaskDelay(pdMS_TO_TICKS(1000));
    } else{
      vTaskDelay(pdMS_TO_TICKS(8)); //Time ADC task takes to run
    }
  }
}

void adcSetup(){
  ads.setGain(GAIN_TWOTHIRDS);
  ads.setDataRate(RATE_ADS1115_860SPS);
  
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    // digitalWrite(ledPin, HIGH);
    delay(2000);
    // digitalWrite(ledPin, LOW);
    while (1); //TODO ADD ERROR CODE LED
  
  } 
  Serial.println("ADC Setup Complete");
}

//RTOS Tasks 
void readADC( void * parameter )
{
    for(;;){
      Serial.println("Running ADC Task");
      if(I2CactiveTask == NULL){
        I2CactiveTask = xTaskGetCurrentTaskHandle();
        int16_t adc0, adc1, adc2, adc3;
        float v1, v2, v3, v4;

        adc0 = ads.readADC_SingleEnded(0);
        adc1 = ads.readADC_SingleEnded(1);
        adc2 = ads.readADC_SingleEnded(2);
        adc3 = ads.readADC_SingleEnded(3);
        
        data.iBatt = 10*(ads.computeVolts(adc0) - 0.493);
        data.iMot = 10*(ads.computeVolts(adc1)- 0.493);
        data.vMot = 7.51*(ads.computeVolts(adc2));
        data.vBat = 9.01*(ads.computeVolts(adc3));   
        
        I2CactiveTask = NULL;
        vTaskDelay(pdMS_TO_TICKS((100)));
      } else{
        vTaskDelay(pdMS_TO_TICKS(15));
      }
    }
 
    // Serial1.println("Ending task 1");
    vTaskDelete( NULL );
 
}

// CAN Code
void sendMsgBuf(unsigned long id, byte ext, byte rtr, byte len, unsigned char *buf){
  unsigned char dta[16];

  dta[0] = 0xff & (id >> 24);
  dta[1] = 0xff & (id >> 16);
  dta[2] = 0xff & (id >> 8);
  dta[3] = 0xff & (id >> 0);

  dta[4] = ext;
  dta[5] = rtr;

  dta[6] = len;

  for(int i=0; i<len; i++){
    dta[7+i] = buf[i];
  }
  dta[15] = makeCheckSum(dta, 15);

  IIC_CAN_SetReg(0x30, 16, dta);
}

unsigned char makeCheckSum(unsigned char *dta, int len){
    unsigned long sum = 0;
    for(int i=0; i<len; i++)sum += dta[i];

    if(sum > 0xff)
    {
        sum = ~sum;
        sum += 1;
    }

    sum  = sum & 0xff;
    return sum;
}

void IIC_CAN_SetReg(unsigned char __reg, unsigned char __len, unsigned char *__dta)
{
    Wire.beginTransmission(CANADDRESS);
    Wire.write(__reg);
    for(int i=0; i<__len; i++)
    {
        Wire.write(__dta[i]);
    }
    Wire.endTransmission();
    
}
