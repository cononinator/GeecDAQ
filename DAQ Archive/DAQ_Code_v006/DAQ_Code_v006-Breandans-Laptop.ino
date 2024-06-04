/*
* Author: Conor Farrell & Breandán Gillanders
* Date: 17/02/2024
*
*  Code to data_t basic FreeRTOS functionality with the ADC
*  Added functionality for the SD Card saving
*
* Date: 11/03/2024
* Changed ADS setup from DFROBOT library to Adafruit library to improve speeds
* 
* Date: 12/03/2024
* Added Queue for saving data
*
* Date: 21/03/2024
* Fixed Queue Bug - now sample rate is roughly 95 Sa/s
*
* Date: 25/05/2024
* Fixed Speed Sensor code, added minimum Timing
*
* Date: 09/05/2024
* Added CAN BUS FUNCTION
*/


#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// Filename prefix:
// Max 5 chars (excluding /), should take form: "/XXXXX"
// Only use standard ASCII characters
String prefix = "/poste"; 

//SD Card Variables
#define REASSIGN_PINS
int sck = 48;
int miso = 47;
int mosi = 38;
int cs = 21;
String filename = "/default.txt"; // pretty sure the part before the dot has to be less than or equal to 8 chars (DOS8.3 naming)

Adafruit_ADS1115 ads;

//GPS Variables
int PWRPin = D8; //This pin needs to be soldered to the PCB
char sim_response[512];
String nmeaSentence;
int ledState = LOW;
const int ledPin = D9;
String response, values;
int startIndex, endIndex = -1;

//Speed Sensor
//Speed Sensor
#define SPEEDPIN D4
const int debounce = 125000; //Minimum time between interrupts
double speed;
double minSpeedTime = 1400000;
const double filter = 0.7; //Change this to change how much a new speed effects the recorded speed
unsigned long currSpeedTime, prevSpeedTime = 1;

//CAN Bus
typedef union{
  float val;
  unsigned char bytes[4];
} FLOATUNION_t;
FLOATUNION_t canBuff; 
#define CANADDRESS 0x25


// FreeRTOS Variables
#define configTICK_RATE_HZ 600
struct data_t{
  long time;
  double speed,iBatt, iMot, vBat, vMot;
  String GPS;
};

QueueHandle_t buffer; //Message Queue
data_t data = {0, 0, 0, 0, 0, 0, ""};
static TaskHandle_t I2CactiveTask = NULL; // Activity Semaphore for I2C bus 

void setup() {


  Serial0.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, D2, D3);


  pinMode(ledPin, OUTPUT);
  pinMode(PWRPin, OUTPUT);


  adcSetup();

  sdSetup();

  setNextFileName(prefix); // DOS8.3 naming, must instert 5 char prefix rn

  attachInterrupt(digitalPinToInterrupt(SPEEDPIN), currSpeed, FALLING);

  delay(1000);

  buffer = xQueueCreate(250, sizeof(data_t));
 
  xTaskCreate(
                    readADC,          /* Task function. */
                    "readADC",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */

  xTaskCreate(
                speedTask,
                "speedTask",
                10000,
                NULL,
                3,
                NULL);

  xTaskCreate(
              SaveToSDTask, 
              "SaveToSDTask", 
              10000,
              NULL, 
              10,
              NULL);
  
  xTaskCreate(
              canTask,
              "canTask",
              10000,
              NULL,
              1,
              NULL
              );
 
  gpsSetup();
  
  xTaskCreate(
                    pollGPSTask,          /* Task function. */
                    "pollGPSTask",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    5,                /* Priority of the task. */
                    NULL);            /* Task handle. */
 

}
 
void loop() {
  // NOTHING
}

void adcSetup(){
  ads.setGain(GAIN_TWOTHIRDS);
  ads.setDataRate(RATE_ADS1115_860SPS);
  
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    digitalWrite(ledPin, HIGH);
    delay(2000);
    digitalWrite(ledPin, LOW);
    while (1); //TODO ADD ERROR CODE LED
  
  } 
}

//RTOS Tasks 
void readADC( void * parameter )
{
    for(;;){
      if(I2CactiveTask == NULL){
        I2CactiveTask = xTaskGetCurrentTaskHandle();
        int16_t adc0, adc1, adc2, adc3;
        float v1, v2, v3, v4;

        adc0 = ads.readADC_SingleEnded(0);
        adc1 = ads.readADC_SingleEnded(1);
        adc2 = ads.readADC_SingleEnded(2);
        adc3 = ads.readADC_SingleEnded(3);

        data.time = millis();
        data.iBatt = 10*(ads.computeVolts(adc0) - 0.493);
        data.iMot = 10*(ads.computeVolts(adc1)- 0.493);
        data.vMot = 7.51*(ads.computeVolts(adc2));
        data.vBat = 9.01*(ads.computeVolts(adc3));   


        data_t duplicate = data;
        xQueueSend(buffer, &duplicate, 0); //Send pointer to queue if queue full do nothing

        I2CactiveTask = NULL;
        // The above code takes approx. 7ms    
        vTaskDelay(pdMS_TO_TICKS((400)));  //Delay Can be removed or added to change speed
        
        } else{
          vTaskDelay(pdMS_TO_TICKS(15));
      }
    }
 
    // Serial1.println("Ending task 1");
    vTaskDelete( NULL );
 
}

void SaveToSDTask( void * parameter)
{
  for(;;){
   data_t messageStruct;
    File file = SD.open(filename, FILE_APPEND);
    while(xQueueReceive(buffer, &messageStruct, 0)){
            String iB = String(messageStruct.iBatt,3);
            String iM = String(messageStruct.iMot,3);
            String vB = String(messageStruct.vBat,3);
            String vM = String(messageStruct.vMot,3);
            String message = String(messageStruct.time) + "," + iB + "," + iM + "," + vB + "," + vM + "," + String(messageStruct.speed, 1) + "," + messageStruct.GPS + "\n";
            Serial1.print(message);
            appendFile(file, message);
    }
    file.close();
    vTaskDelay(pdMS_TO_TICKS((900)));
  }
}

void pollGPSTask( void *parameter){
  for(;;){
    response = send_at("AT+CGNSINF");
    startIndex = response.indexOf(",") - 1;
    endIndex = response.indexOf(",,");
    if (startIndex +1 != endIndex ) {
        data.GPS = response.substring(startIndex, endIndex);
        // ADD CODE TO GET GPS SPEED HERE, USE lastIndexOf(",") function
    }

    // Serial1.println(t.GPS);
    
    vTaskDelay(pdMS_TO_TICKS(1000));

  }
}

void speedTask( void * parameter){
  for(;;){
    if (micros() - prevSpeedTime > minSpeedTime){
        data.speed = 0;
    }
    vTaskDelay(pdMS_TO_TICKS(700));
  }
}

//TODO Add Method to take GPS speed
void canTask( void * parameter ){
  for(;;){
    if (I2CactiveTask == NULL){
      I2CactiveTask = xTaskGetCurrentTaskHandle();
      // 0x01: Speed id, 0x02: Current id, 0x03: Voltage id
      canBuff.val = data.speed; 
      sendMsgBuf(0x01, 0,0, 4, canBuff.bytes);

      canBuff.val = data.iBatt;
      sendMsgBuf(0x02, 0,0, 4, canBuff.bytes);

      canBuff.val = data.vBat;
      sendMsgBuf(0x03, 0,0, 4, canBuff.bytes);
      I2CactiveTask = NULL;
      vTaskDelay(pdMS_TO_TICKS(1000));
    } else{
      vTaskDelay(pdMS_TO_TICKS(8)); //Time ADC task takes to run
    }
  }
}

//SD Functions
void sdSetup(){

  #ifdef REASSIGN_PINS
    
    SPI.begin(sck, miso, mosi, cs);
  #endif
  if(!SD.begin(cs)){ //Change to this function to manually change CS pin
    Serial1.println("Card Mount Failed");
    while (true){
          digitalWrite(ledPin, HIGH);
          delay(2000);
          digitalWrite(ledPin, LOW);
          delay(3000);
    };
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
  int prefixLength = prefix.length() - 1;
  File root = SD.open("/");
  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      break;
    }
    String name = entry.name();
    if (name.startsWith(prefix.substring(1, prefixLength + 1)) && name.endsWith(".txt")) {
      String numberStr = name.substring(prefixLength, prefixLength + 3); // Extract XXX part
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

void appendFile(File file, String message){
    // Serial1.printf("Appending to file: %s\n", path);

    // File file = fs.open(path, FILE_APPEND);
    if(!file){
        // Serial1.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        // Serial1.println("Message appended");
    } else {
        // Serial1.println("Append failed");
    }
    // file.close();
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

//CAN Bus
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

//Speed Sensor
void currSpeed(){
  currSpeedTime = micros();
  
  if ( currSpeedTime - prevSpeedTime > debounce ){
    data.speed = 1.0 / ((currSpeedTime-prevSpeedTime) / 1000000.0) * 0.5 * 3.14 * 3.6;
    // Serial1.println(t.speed);
    prevSpeedTime = currSpeedTime;
  }
}

void filterSpeed(){
  currSpeedTime = micros();
  
  if ( currSpeedTime - prevSpeedTime > debounce ){
    speed = 1.0 / ((currSpeedTime-prevSpeedTime) / 1000000.0) * 0.5 * 3.14 * 3.6;
    data.speed = data.speed + filter * (speed - data.speed);
    prevSpeedTime = currSpeedTime;
  }
}