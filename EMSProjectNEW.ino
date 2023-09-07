#include <LiquidCrystal.h>
#include <Arduino_FreeRTOS.h>

// Include queue support
#include <queue.h>
/* 
 * Declaring a global variable of type QueueHandle_t 
 * 
 */
QueueHandle_t charQueue;

const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);







void setup() {

   /**
   * Create a queue.
   * https://www.freertos.org/a00116.html
   */
  charQueue = xQueueCreate(200, // Queue length
                              sizeof(uint8_t) // Queue item size
                              );
  
  if (charQueue != NULL) {
    
    // Create task that consumes the queue if it was created.
    xTaskCreate(TaskSerial, // Task function
                "Serial", // A name just for humans
                128,  // This stack size can be checked & adjusted by reading the Stack Highwater
                NULL, 
                2, // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
                NULL);


    // Create task that publish data in the queue if it was created.
    xTaskCreate(TaskListen, // Task function
                "Listen", // Task name
                128,  // Stack size
                NULL, 
                1, // Priority
                NULL);

    xTaskCreate(TaskBlink, // Task function
              "Blink", // Task name
              128, // Stack size 
              NULL, 
              0, // Priority
              NULL );
  
}
}

void loop() 
{
  // put your main code here, to run repeatedly: 
}



/**
 * Analog read task
 * Reads an analog input on pin 0 and send the readed value through the queue.
 * See Blink_AnalogRead example.
 */
void TaskListen(void *pvParameters)
{
  (void) pvParameters;
   // Init Arduino serial
  Serial.begin(115200);
  lcd.begin(16, 2);
  lcd.print("**EMS Terminal**");
  delay(2000);

  // Wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  while (!Serial) {
    vTaskDelay(1);
  }
  
  for (;;)
  {
    // Read the input on analog pin 0:
    if(Serial.available()>0){
    int sensorValue = Serial.read();

    /**
     * Post an item on a queue.
     * https://www.freertos.org/a00117.html
     */
    xQueueSend(charQueue, &sensorValue, portMAX_DELAY);
    }
    // One tick delay (15ms) in between reads for stability
    vTaskDelay(1);
  }
}

  // char valueFromQueue = 0;
#define CHUNK_SIZE 16 //16 Number of letters to display at a time
//char valueFromQueue[CHUNK_SIZE + 1]; // +1 for null terminator
char valueFromQueue=0;
  int index = 0;
// void displayChunk(const char* chunk) {
//   lcd.setCursor(0, 1);
//   lcd.print(chunk);
// }

// void displayChunk(const char chunk) {
//   if(index == 15)
//   {
//     index = 0;
//     lcd.setCursor(0, 1);
//     lcd.print("                ");
//   }
//   if(chunk == 13 || chunk == 10){
//     // lcd.setCursor(index, 1);
//     // lcd.write(' ');
//   }else
//   {
//   lcd.setCursor(index, 1);
//   lcd.write(chunk);
//   }
//   index++;
// }
char textBuffer[17];
void displayChunk(const char chunk) {
  /*
  if(index < 16)
  {
    if(chunk == 13 || chunk == 10)
    {
      lcd.setCursor(index, 0);
      //lcd.write('');
    }
    else
    {
      lcd.setCursor(index, 0);
      lcd.write(chunk);
    }
  }
  else if(index > 15 && index < 32)
  {
    if(chunk == 13 || chunk == 10)
    {
      lcd.setCursor(index-16, 1);
      //lcd.write('');
    }
    else
    {
      lcd.setCursor(index-16, 1);
      lcd.write(chunk);
    }
  }
  else
  {
    index = 0;
    delay(2000);
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
  }
  // if(chunk == 13 || chunk == 10){
  //   // lcd.setCursor(index, 1);
  //   // lcd.write(' ');
  // }else
  // {
  // lcd.setCursor(index, 1);
  // lcd.write(chunk);
  // }
  index++;
  */
  for (int i = 0; i < 15; i++) {
    textBuffer[i] = textBuffer[i + 1];
  }
  textBuffer[15] = chunk;
  // Add the new character to the end of the buffer
  if (chunk == 13 || chunk == 10) textBuffer[15] =  ' ';
  textBuffer[16] = '\0';  // Null-terminate the string

  // Print the textBuffer contents to the second row of the LCD
  lcd.setCursor(0, 1);
  lcd.print(textBuffer);
}
/**
 * Serial task.
 * Prints the received items from the queue to the serial monitor.
 */
void TaskSerial(void * pvParameters) {
  (void) pvParameters;

 


  for (;;) 
  {

    /**
     * Read an item from a queue.
     * https://www.freertos.org/a00118.html
     */
    // if (xQueueReceive(charQueue, &valueFromQueue, portMAX_DELAY) == pdPASS) {
    //   Serial.println(valueFromQueue);
    //   lcd.setCursor(0, 1);
    //   lcd.write(valueFromQueue);
    // }

    if (xQueueReceive(charQueue, &valueFromQueue, portMAX_DELAY) == pdPASS) {
      displayChunk(valueFromQueue);
      delay(250);
    }

    // if (xQueueReceive(charQueue, &valueFromQueue[index], portMAX_DELAY) == pdPASS) {
    //   index++;

    //   // Check if a chunk of letters is ready to be displayed
    //   if (index == CHUNK_SIZE) {
    //     valueFromQueue[CHUNK_SIZE] = '\0'; // Null-terminate the string
    //     displayChunk(valueFromQueue);
    //     index = 0; // Reset the index for the next chunk

    //   }
    // }


  }
}

/* 
 * Blink task. 
 * See Blink_AnalogRead example. 
 */
void TaskBlink(void *pvParameters)
{
  (void) pvParameters;

  pinMode(LED_BUILTIN, OUTPUT);

  for (;;)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay( 250 / portTICK_PERIOD_MS );
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay( 250 / portTICK_PERIOD_MS );
  }
}