// MAKE SURE THE PIN NUMBERING IS SET TO GPIO INSTEAD OF DEFAULT ARDUINO

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define REASSIGN_PINS
int sck = 48;
int miso = 47;
int mosi = 38;
int cs = 21;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial) { delay (10); }
  Serial.println("Serial started");

  

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

  
  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
}

void loop() {
  // put your main code here, to run repeatedly:
  appendFile(SD, "/newfile1.txt", "The SD card works.\n");
  Serial.println("Appended to file");
  delay(1000);  
}

void appendFile(fs::FS &fs, const char * path, const char * message){
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
