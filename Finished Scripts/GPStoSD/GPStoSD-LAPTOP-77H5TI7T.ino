// Saves GPS data to SD card once the GPS has obtained
// a fix on satellites. 
//
// Must have pin numbering set to GPIO for SD card to work

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define REASSIGN_PINS // GPIO pin numbering
int sck = 48; // D13
int miso = 47; // D12
int mosi = 38; // D11
int cs = 21; // D10

#define TX1 D3;
#define RX1 D2;

int PWRPin = D6;
char sim_response[512];
String nmeaSentence;
int ledState = LOW;
int ledPin = D9;
String response, values;
int startIndex, endIndex = -1;

void setup() {
  delay(3000);
  Serial0.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, D2, D3);
  Serial1.println("Started serial");

  pinMode(ledPin, OUTPUT);

  #ifdef REASSIGN_PINS
    SPI.begin(sck, miso, mosi, cs);
  #endif

  if(!SD.begin(cs)){ 
    Serial1.println("Card Mount Failed");
    while (true){;};
  }

  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  
  // Currently turning on GPS Power manually (with button)
  // pinMode(PWRPin, OUTPUT);
  // digitalWrite(PWRPin, HIGH);
  // delay(2000);
  // digitalWrite(PWRPin, LOW);
  // delay(1000);

  Serial1.println("Sending AT+CGNSPWR=1:");
  Serial1.println(send_at("AT+CGNSPWR=1"));
  
  Serial1.println("Sending AT+CGNSCOLD:");
  Serial1.println(send_at("AT+CGNSCOLD"));

  bool gpsInitialised = false;
  while (gpsInitialised == false){
    gpsInitialised = validResponse(send_at("AT+CGNSINF"));
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(ledPin, ledState);
    Serial1.println("Checking");
    delay(1000);
  }

}

void loop() {
  response = send_at("AT+CGNSINF");
  startIndex = response.indexOf(",") - 1;
  endIndex = response.lastIndexOf(",");
  values = response.substring(startIndex, endIndex);
  Serial1.println(values);
  values.concat("\n");
  appendFile(SD, "/gpsdata.txt", values);

  delay(1000);
}

String send_at(String at_command){
  Serial0.flush();
  Serial0.println(at_command);
  while(!Serial0.available()){
    ;
  }
  // delay(150);

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
  Serial1.println(response);
  if (response.indexOf("+CGNSINF: ") != -1){
    int start = response.indexOf(",") - 1;
    int end = response.lastIndexOf(",");
    String values = response.substring(start, end);
    Serial1.println(values);
    if (values.charAt(0) == '1'){
      if (values.charAt(2) == '1'){
        return true;
      }
    }
  }
  return false;
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