#include <Wire.h>
#include "Longan_I2C_CAN_Arduino.h"
#include "rgb_lcd.h"
//Variables for LCD
rgb_lcd lcd;
const int colorR = 0;
const int colorG = 8;
const int colorB = 0;
//Variables for interrupt
const byte interruptPin = 2;
volatile int lapNumber = 0;
volatile unsigned long lastInterruptTime = 0;
bool buttonPressed = false;
const int debounceTime = 500;


typedef union{
  float val;
  byte bytes[4];
} FLOATUNION_t;

FLOATUNION_t buff;
float speedRx;
float currRx;
float voltRx;

I2C_CAN CAN(0x25);  // Set I2C Address
unsigned char len = 0;
unsigned long id;



void setup() {
  Serial.begin(9600);
  //while(!Serial);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  lcd.setRGB(colorR, colorG, colorB);

  Serial.println("begin to init can");

  while (CAN_OK != CAN.begin(CAN_500KBPS))  // init can bus : baudrate = 500k
  {
    Serial.println("CAN BUS FAIL!");
    delay(100);
  }
  Serial.println("CAN BUS OK!");

  // Initialize interrupt pin
  pinMode(interruptPin, INPUT_PULLUP);

  //Attatch interrupt
  // attachInterrupt(digitalPinToInterrupt(interruptPin), lap, FALLING);
}


void loop() {
  interruptCheck();
  readMsg(); //CAN Bus code

  float power = currRx * voltRx;
  String powerString = String(power, 1);
  powerString = stringFormat(powerString, 3);

  lcd.setCursor(0, 0);
  lcd.print(powerString);
  lcd.print("W");

   // Calculate elapsed time since last lap
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - lastInterruptTime;


  // Convert milliseconds to minutes and seconds
  
  unsigned long seconds = (elapsedTime / 1000) ;
  unsigned long minutes=seconds/60;

  lcd.setCursor(0,1);
  // Display minutes and seconds
  lcd.print(minutes);
  lcd.print(":");

  
  if (seconds < 10||(seconds % 60)<10) {
    lcd.print("0");
  }
  
  lcd.print(seconds % 60);
  delay(100);
}

void readMsg(){
   while ( CAN_MSGAVAIL == CAN.checkReceive()){
    CAN.readMsgBuf(&len, buff.bytes);
    id = CAN.getCanId();
    Serial.print("CAN ID: ");
    Serial.println(id);
    switch (CAN.getCanId()){
      case 1:
        speedRx = buff.val;
        Serial.println(speedRx);
        break;
      case 2:
        currRx = buff.val;
        Serial.println(currRx);
        break;
      case 3:
        voltRx = buff.val;
        Serial.println(voltRx);
        break;
    }
  }
}

String stringFormat(String number, int len){
  while(number.length() < len){
    number = "0" + number;
  }
  return number;
}

void interruptCheck(){
  //   //Interrupt check
//   if (buttonPressed) {
//     // the button has been pressed since the loop last ran

//     buttonPressed = false;


//     // check how recent the last button press was
//     unsigned long currentTime = millis();
//     unsigned long elapsedTime = currentTime - lastInterruptTime;


//     // if the last button press was more than 100ms ago, it's a valid press
//     if (elapsedTime > debounceTime) {
//       lastInterruptTime = currentTime;
//       // Increment lap number
//       lapNumber++;
//     }
//   }
}

void lap() {
  // Update the buttonPressed boolean
  buttonPressed = true;
}