|struct test{
  int v1;
  int v2;
};

test t = {0, 0};

void setup() {
 
  Serial.begin(112500);
  delay(1000);
 
  xTaskCreate(
                    taskOne,          /* Task function. */
                    "TaskOne",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
 
  xTaskCreate(
                    taskTwo,          /* Task function. */
                    "TaskTwo",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
 
}
 
void loop() {
  delay(1000);
  
}
 
void taskOne( void * parameter )
{
 
    for(;;){ 
        t.v1++;
        xTaskCreate( printTask, "printTask", 10000, NULL, 2, NULL);
        vTaskDelay(pdMS_TO_TICKS((250)));
    }
 
    Serial.println("Ending task 1");
    vTaskDelete( NULL );
 
}
 
void taskTwo( void * parameter)
{
 
    for(;;){
 
        t.v2++;
        vTaskDelay(pdMS_TO_TICKS((750)));
    }
    Serial.println("Ending task 2");
    vTaskDelete( NULL );
 
}

void printTask( void * parameter)
{
  for(;;){
    Serial.printf("Task 1 %d, Task 2 %d \n", t.v1, t.v2);
    vTaskDelete( NULL);
  }
}