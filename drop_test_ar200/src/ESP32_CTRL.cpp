#include "BluetoothSerial.h"

// constant
#define SPP_SERVER_NAME "ESP32_SOLENOID_CTRL"
#define GPIO_SOLENOID 2
#define RUN_DURATION_S 5 // The duration in seconds to keep the solenoid active

// global
BluetoothSerial SerialBT;

TaskHandle_t run_task_handle = NULL;

void run_solenoid_task(void *pvParameters)
{
    // Energize the solenoid
    digitalWrite(GPIO_SOLENOID, HIGH);

    // Wait for RUN_DURATION_S
    vTaskDelay((RUN_DURATION_S * 1000) / portTICK_PERIOD_MS);

    // Deenergize the solenoid
    digitalWrite(GPIO_SOLENOID, LOW);

    run_task_handle = NULL; // Clear the handle to allow a new run to start
    vTaskDelete(NULL);      // Delete the task when it's finished
}

void bt_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    if (event == ESP_SPP_SRV_OPEN_EVT)
    {
        Serial.println("Client Connected");
    }
    else if (event == ESP_SPP_CLOSE_EVT)
    {
        Serial.println("Client Disconnected");
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(GPIO_SOLENOID, OUTPUT);
    digitalWrite(GPIO_SOLENOID, LOW); // Initialize magnet to inactive state
    
    SerialBT.begin(SPP_SERVER_NAME);
    Serial.println("Bluetooth device is ready to pair.");

    SerialBT.register_callback(bt_callback);
}

void loop()
{
    if (SerialBT.available())
    {
        String command = SerialBT.readStringUntil('\n');
        command.trim(); // Remove any whitespace
        
        if (command == "start")
        {
            // Prevent starting a new task if one is already in progress
            if (run_task_handle == NULL)
            {
                // Create a new task to run the solenoid activation
                xTaskCreate(
                    run_solenoid_task,
                    "run_solenoid_task",
                    2048,
                    NULL,
                    1,
                    &run_task_handle
                );
            }
            else
            {
                const char* busyMsg = "A run is already in progress.";
                Serial.println(busyMsg);
                SerialBT.println(busyMsg);
            }
        } 
        else
        {
            // Handle any other commands
            const char* part1 = "Unknown command: '";
            const char* part2 = "'";
            Serial.print(part1);
            Serial.print(command);
            Serial.println(part2);
            SerialBT.print(part1);
            SerialBT.print(command);
            SerialBT.println(part2);
        }
    }
    delay(20); 
}