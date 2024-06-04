
struct test{
  int speed, iBatt;
  double iMot, vBat, vMot;
  char GPS[70];
};

QueueHandle_t buffer;//Message buffer

test t = {0, 0, 0, 0, 0, "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"};


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print(sizeof(t));

  buffer = xQueueCreate(100, sizeof(struct test *));
  xTaskCreate( regularTask, "regularTask", 10000, NULL, 1, NULL);
  xTaskCreate( printTask,"printTask", 10000, NULL, 2, NULL);

}

void loop() {
  // put your main code here, to run repeatedly:

}


void regularTask( void *parameter){
  for(;;){
    // Serial.println("a");
    test v = t;
    // t.speed = t.speed+1;
    // t.iBatt = t.iBatt - 1;
    xQueueSend(buffer, (void * ) &v, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void printTask(void *parameter){
  for(;;){
    // Serial.println("b");
    test v = {0, 0, 0, 0, 0, ""};

    while(xQueueReceive(buffer, &v, 0)){
      String message = String(millis() + ",");
      Serial.println(ESP.getFreeHeap() );
    }

    vTaskDelay(pdMS_TO_TICKS(900));
  }
}
