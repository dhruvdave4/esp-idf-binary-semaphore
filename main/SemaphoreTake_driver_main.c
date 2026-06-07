/*  
    * PWM LED Brightness Control and UART Communication on ESP32    
    * 
    * This code demonstrates how to use the LEDC (LED Control) driver on an ESP32 to control the brightness of an onboard LED using PWM (Pulse Width Modulation).
    * The LED will gradually increase in brightness and then decrease in a loop.
    * 
    * Note: Make sure to connect an appropriate resistor in series with the LED to prevent damage.
*/   

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"   
#include"driver/uart.h" 
#include"driver/gpio.h"  
#include "freertos/semphr.h"

 
static const char *TAG = "semaphore_task_with_pwm_driver"; 
// onboard LED GPIO
#define LED_GPIO 2  
#define BUTTON_PIN 4
// #define UART_TX_GPIO 17
// #define UART_RX_GPIO 16  
#define UART_BAUD_RATE 115200
#define UART_BUFFER_SIZE 1024 
#define UART_PORT_NUM UART_NUM_0   
#define dealy_time 20
#define button_dealy  (dealy_time/portTICK_PERIOD_MS)
// PWM SETTINGS
#define PWM_FREQ       6000 // 6 hz   
#define PWM_RESOLUTION LEDC_TIMER_13_BIT
#define PWM_MODE       LEDC_HIGH_SPEED_MODE
#define PWM_TIMER      LEDC_TIMER_0
#define PWM_CHANNEL    LEDC_CHANNEL_0
 
// max duty for 13-bit resolution
#define PWM_DUTY_MAX   8191 
// QueueHandle_t button_queue;
 SemaphoreHandle_t binary_sem;
uint8_t data[UART_BUFFER_SIZE];
static int value_of_duty;  
bool button_state= false;   
uint8_t pwm_run_state=1; 
uint8_t current; 
uint8_t previous =1 ;   
static  int duty = 0;  
uint32_t count;
TaskHandle_t button_task_handle = NULL;

void button_init(void) 
{ 
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE, // No interrupt
        .mode = GPIO_MODE_INPUT,        // Set as input mode
        .pin_bit_mask = (1ULL << BUTTON_PIN), // Bit mask for the button pin this shit bit toware left side so the  i have  4 then we shift 4 time this  give  me  
        // 1ull is  unsigin 16 bit is  only thie  0 and  positve value  
        //BUTTON_PIN -->4 
        // we shit  1--->4 time  
        // 00001000
        .pull_down_en = 0,              // Disable pull-down
        .pull_up_en = 1                 // Enable pull-up
    };
    gpio_config(&io_conf);  
    ESP_LOGI("TAG","ye button is set hahahaha!\r\n");   
} 
static void IRAM_ATTR button_isr_handler(void* arg) 
{   

    BaseType_t higher_priority_task_woken = pdFALSE;

    xSemaphoreGiveFromISR(
        binary_sem,
        &higher_priority_task_woken
    );

    if(higher_priority_task_woken)
    {
        portYIELD_FROM_ISR();
    }
}


/* uart init and  desing program*/
void uart_init(void)
{
    const uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(UART_PORT_NUM, &uart_config);
//    / uart_set_pin(UART_PORT_NUM, UART_TX_GPIO, UART_RX_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT_NUM, UART_BUFFER_SIZE * 2, 0, 0, NULL, 0);
}   
 
void button_task(void *pvParameters)
{ 

    while(1)
    {
        if(xSemaphoreTake(binary_sem, portMAX_DELAY))
        {
            ESP_LOGI(TAG, "Semaphore Received");

            // static bool led = false;
            // led = !led;

            // gpio_set_level(LED_GPIO, led); 


                 current = gpio_get_level(BUTTON_PIN);
                    if(current == 0)
                    {
                        duty = 0; 
                        pwm_run_state= false;   
                        ledc_set_duty(PWM_MODE, PWM_CHANNEL, duty);   
                        ledc_update_duty(PWM_MODE, PWM_CHANNEL);   
                        ESP_LOGI(TAG,"button pressed! pwm duty off &LED is OFF duty_cycle:%d \r\n",duty);  
                    }
   
            
            else 
            {  
                duty = value_of_duty; 
                pwm_run_state= true; 
                ledc_set_duty(PWM_MODE, PWM_CHANNEL, duty);   
                ledc_update_duty(PWM_MODE, PWM_CHANNEL);  
                current = gpio_get_level(BUTTON_PIN);
                ESP_LOGI(TAG,"button Released! pwm duty on &LED is ON duty_cycle:%d \r\n",duty);  

             }
    } 

 } 
}
void uart_receive_task(void *pvParameters)
{
    
    while (1)
    {
        int len = uart_read_bytes(UART_PORT_NUM, data, UART_BUFFER_SIZE, pdMS_TO_TICKS(2000));
        if (len > 0)
        {
            data[len] = '\0'; // Null-terminate the received data
            ESP_LOGI(TAG, "Received: %s", data);  
            // value_of_duty = atoi((const char *)data); // Convert received string to integer  
            
        }
    }
}
void cmd_task(void *pvParameters)
{
      while (1)
    {
        if (strcmp((char *)data, "exit") == 0)
        { 
            duty = 0;   
            ledc_set_duty(PWM_MODE, PWM_CHANNEL, duty);
            ledc_update_duty(PWM_MODE, PWM_CHANNEL);

            ESP_LOGI(TAG, "PWM OFF"); 
            //   memset(data, 0, sizeof(data));
        }

        else if (strcmp((char *)data, "help") == 0)
        {
            ESP_LOGI(TAG, "Commands:");
            ESP_LOGI(TAG, "pwm <0-8191>");
            ESP_LOGI(TAG, "on");
            ESP_LOGI(TAG, "off");  
            ESP_LOGI(TAG,"count");
            //   memset(data, 0, sizeof(data));
        }

        else if (strncmp((char *)data, "pwm ", 4) == 0)
        {
            value_of_duty = atoi((char *)&data[4]);

            if (value_of_duty >= 0 && value_of_duty <= PWM_DUTY_MAX )
            {  
                if(pwm_run_state==false) 
                { 
                  duty = 0;   
                ledc_set_duty(PWM_MODE, PWM_CHANNEL, duty);
                ledc_update_duty(PWM_MODE, PWM_CHANNEL);

                // ESP_LOGI(TAG, "Duty = %d", duty);
                } 
                else 
                { 
                    pwm_run_state= true;
                duty = value_of_duty;   
                ledc_set_duty(PWM_MODE, PWM_CHANNEL, duty);
                ledc_update_duty(PWM_MODE, PWM_CHANNEL);

                ESP_LOGI(TAG, "Duty_cycle = %d", duty); 
                }  

            } 

//   memset(data, 0, sizeof(data));
            
        }

        else if (strcmp((char *)data, "on") == 0)
        {
            duty = PWM_DUTY_MAX; 
            pwm_run_state= true;
            ledc_set_duty(PWM_MODE, PWM_CHANNEL, duty);
            ledc_update_duty(PWM_MODE, PWM_CHANNEL); 
            // memset(data, 0, sizeof(data));
        }

        else if (strcmp((char *)data, "off") == 0)
        {
            duty = 0; 
            pwm_run_state= false;   
            ledc_set_duty(PWM_MODE, PWM_CHANNEL, duty);
            ledc_update_duty(PWM_MODE, PWM_CHANNEL); 
            // memset(data, 0, sizeof(data));
        }  
       

        // vTaskDelay(10/portTICK_PERIOD_MS); 
        vTaskDelay(pdMS_TO_TICKS(100)); // Add a small delay to prevent task starvation 
    } 
  

}
void pwm_init(void)
{
    ESP_LOGI(TAG, "PWM Init Start");

    // Configure PWM timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = PWM_MODE,
        .timer_num        = PWM_TIMER,
        .duty_resolution  = PWM_RESOLUTION,
        .freq_hz          = PWM_FREQ,
        .clk_cfg          = LEDC_AUTO_CLK
    };

    ledc_timer_config(&ledc_timer);

    // Configure PWM channel
    ledc_channel_config_t ledc_channel = {
        .gpio_num   = LED_GPIO,
        .speed_mode = PWM_MODE,
        .channel    = PWM_CHANNEL,
        .timer_sel  = PWM_TIMER,
        .duty       = 0,
        .hpoint     = 0
    };

    ledc_channel_config(&ledc_channel);

    ESP_LOGI(TAG, "PWM Init Done");
}
 


void app_main(void)
{ 
//    ESP_LOGI(TAG, "pwm and  uart base code start ==========================================");


    //    button_queue = xQueueCreate(10, sizeof(int)); 
    

    /* Create Task */


    /* Install ISR Service */ 
    /* in free rtos setup in system */  



    binary_sem =
        xSemaphoreCreateBinary();

    if(binary_sem == NULL)
    {
        ESP_LOGE(TAG,
                 "Semaphore Create Failed");
        return;
    }
 

     pwm_init(); 
    uart_init();    
    button_init ();
   
    xTaskCreate(uart_receive_task, "uart_receive_task", 2048, NULL, 5, NULL);   
    xTaskCreate(cmd_task, "cmd_task", 2048, NULL, 5, NULL);   
    xTaskCreate( button_task,"button_task", 2048, NULL,  4,&button_task_handle);
     gpio_install_isr_service(0);

    /* Add ISR Handler */
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler,(void *)BUTTON_PIN);
//  take  create or nt  checking conduction // 

   ESP_LOGI(TAG,"Binary Semaphore Ready"); 
  
 if(button_task_handle == NULL)
{
    ESP_LOGE(TAG, "Button Task Creation Failed!");
}
else
{
    ESP_LOGI(TAG, "Button Task Created Successfully");
}
 

}