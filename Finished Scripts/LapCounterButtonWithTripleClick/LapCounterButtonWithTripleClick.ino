#define lapButtonPin 2

boolean restartTimer = false;
volatile int lapNumber = 0;
int debounce = 100;
volatile long lastPress = 0;
volatile int numPresses = 0;
int resetTimerPressDelay = 1000;
volatile long firstPress = 0; // First press in sequence of 3 (To check if resetting timer)

void setup() {
  // put your setup code here, to run once:
  pinMode(lapButtonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(lapButtonPin), lapButtonPress, FALLING);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (restartTimer){
    Serial.println("LAP COUNTER RESET--------------------------");
    restartTimer = false;
    lapNumber = 0;
    numPresses = 0;
  }
}

void lapButtonPress(){
  if (millis() - lastPress > debounce){
    lapNumber++;
    Serial.print("LAP: ");
    Serial.println(lapNumber);

    if (millis() - firstPress < resetTimerPressDelay){
      numPresses++;
      if (numPresses > 2){
        // Have had 3 consecutive presses (within resetTimerPressDelay)
        Serial.print("RESET TIMER: ");
        Serial.println(millis() - firstPress);
        restartTimer=true;
      }
    }
    else {
      firstPress = millis(); // Set this press to be first in sequence of 3
      numPresses = 1;
    }

    lastPress = millis();

    if (lapNumber == 1){
      timerSetup();
    }
  }
}

void timerSetup(){
  Serial.println("Starting timer");
}
