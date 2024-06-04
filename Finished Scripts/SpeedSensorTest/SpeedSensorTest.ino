struct data_t{
  long time;
  double speed,iBatt, iMot, vBat, vMot;
  String GPS;
};

data_t data = {0, 0, 0, 0, 0, 0, ""};

//Speed Sensor
#define SPEEDPIN D4
const int debounce = 125000; //Minimum time between interrupts
double speed;
double minSpeedTime = 1400000;
const double filter = 0.7; //Change this to change how much a new speed effects the recorded speed
unsigned long currSpeedTime, prevSpeedTime = 1;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(SPEEDPIN), currSpeed, FALLING);
  xTaskCreate(
                speedTask,
                "speedTask",
                10000,
                NULL,
                3,
                NULL);

    xTaskCreate(
              printTask,
              "printTask",
              10000,
              NULL,
              3,
              NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
  //
}

void printTask(void * parameters){
  for(;;){
    Serial.println(data.speed);
    vTaskDelay(pdMS_TO_TICKS(700));
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

//Speed Sensor
void currSpeed(){
  currSpeedTime = micros();
  
  if ( currSpeedTime - prevSpeedTime > debounce ){
    data.speed = 1.0 / ((currSpeedTime-prevSpeedTime) / 1000000.0) * 5.625; // 5.625 is 3.6*3.14*0.5
    // Serial1.println(t.speed);
    prevSpeedTime = currSpeedTime;
  }
}

void filterSpeed(){
  currSpeedTime = micros();
  
  if ( currSpeedTime - prevSpeedTime > debounce ){
    speed = 1.0 / ((currSpeedTime-prevSpeedTime) / 1000000.0) * 5.625; // 5.625 is 3.6*3.14*0.5
    data.speed = data.speed + filter * (speed - data.speed);
    prevSpeedTime = currSpeedTime;
  }
}