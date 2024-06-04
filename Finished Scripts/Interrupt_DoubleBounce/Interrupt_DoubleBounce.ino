  // Code written for arduino uno
// Simple button interrupt with internal pull up resistor
//
// One leg of button attached to digital pin 2
// Other leg of button attached to GND

byte buttonPin = 2;
int debounce = 50; // ms - can afford to have debounce very high since only pressed once per lap
int presses = 0;
long lastPress, currentTime = 0;


void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPin, INPUT);
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPressed, FALLING);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void buttonPressed(){
  currentTime = millis();
  double speed;


  if (currentTime - lastPress > debounce){
    speed = 1/(currentTime-lastPress)*0.5*3.14*3.6;
    Serial.println(speed);
    presses++;  
    lastPress = currentTime;
  } 
}