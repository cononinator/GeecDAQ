// Code written for arduino uno
// Simple button interrupt with internal pull up resistor
//
// One leg of button attached to digital pin 2
// Other leg of button attached to GND

byte buttonPin = 2;
int debounce = 200; // ms - can afford to have debounce very high since only pressed once per lap
int presses = 0;
long lastPress = 0;


void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPressed, FALLING);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void buttonPressed(){
  long currentTime = millis();

  if (currentTime - lastPress > debounce){
    presses++;
    Serial.print("Button pressed: ");
    Serial.print(presses);
    Serial.println("");
    lastPress = currentTime;
  }

}