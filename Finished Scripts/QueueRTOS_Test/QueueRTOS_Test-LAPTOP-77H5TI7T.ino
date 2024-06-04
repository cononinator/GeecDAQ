
struct data_t {
  int speed, iBatt;
  double iMot, vBat, vMot;
  char GPS[70];
} ;

QueueHandle_t buffer;//Message buffer

data_t data = {0, 0, 0, 0, 0, "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"};


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print(sizeof(data));

  buffer = xQueueCreate(100, sizeof( data_t));
  xTaskCreate( regularTask, "regularTask", 10000, NULL, 1, NULL);
  xTaskCreate( printTask,"printTask", 10000, NULL, 2, NULL);

}

void loop() {
  // put your main code here, to run repeatedly:

}


void regularTask( void *parameter){
  for(;;){
    // Serial.println("a");
    data_t duplicate = data;
    // t.speed = t.speed+1;
    // t.iBatt = t.iBatt - 1;
    xQueueSend(buffer, &duplicate, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void printTask(void *parameter){
  for(;;){
    // Serial.println("b");
    data_t recieved = {0, 0, 0, 0, 0, ""};

    while(xQueueReceive(buffer, &recieved, 0)){
        Serial.println(recieved.GPS);
    }

    vTaskDelay(pdMS_TO_TICKS(900));
  }
}
