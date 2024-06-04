// Sketch finds next filename

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define REASSIGN_PINS
int sck = 48;
int miso = 47;
int mosi = 38;
int cs = 21;

String filename = "";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(2000);
  sdSetup();

  String highest = findHighestNumber();

  setFileName("/etest" + highest + ".txt");
  appendFile(SD, filename, "Appended to file");
  Serial.println("filename");
}

void loop() {
  // put your main code here, to run repeatedly:

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
    if (name.startsWith("etest") && name.endsWith(".txt")) {
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
  filename = next
  return next;
}

//SD Functions
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