/*
   Funcao: Uso do dual core no ESP32
   Autor: Paulo Cesar Menegon de Castro
   Criado em: 10.08.2024
   Modificado em: 10.08.2024
   Referencias: https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/
                https://elcereza.com/multiprocessamento-esp32-como-usar-corretamente/
                https://www.crescerengenharia.com/post/usando-o-dual-core-do-esp32-processamento-paralelo
*/

#include <stdio.h>
#include "driver\gpio.h"
#include "freeRTOS\freeRTOS.h"
#include "freeRTOS\task.h"

#define LED_STRIP1_G GPIO_NUM_32
#define LED_STRIP1_R GPIO_NUM_33
#define LED_STRIP1_B GPIO_NUM_25

#define LED_STRIP2_G GPIO_NUM_26
#define LED_STRIP2_R GPIO_NUM_27
#define LED_STRIP2_B GPIO_NUM_2

#define LED_STRIP3_G GPIO_NUM_13
#define LED_STRIP3_R GPIO_NUM_14
#define LED_STRIP3_B GPIO_NUM_15

#define LED_STRIP1_BIT_MASK (1ULL << LED_STRIP1_G | 1ULL << LED_STRIP1_R | 1ULL << LED_STRIP1_B)
#define LED_STRIP2_BIT_MASK (1ULL << LED_STRIP2_G | 1ULL << LED_STRIP2_R | 1ULL << LED_STRIP2_B)
#define LED_STRIP3_BIT_MASK (1ULL << LED_STRIP3_G | 1ULL << LED_STRIP3_R | 1ULL << LED_STRIP3_B)

#define PUSH_BUTTON_1 GPIO_NUM_4
#define PUSH_BUTTON_2 GPIO_NUM_35
#define PUSH_BUTTON_3 GPIO_NUM_36

#define PUSH_BUTTON_1_BIT_MASK (1ULL << PUSH_BUTTON_1)
#define PUSH_BUTTON_2_BIT_MASK (1ULL << PUSH_BUTTON_2)
#define PUSH_BUTTON_3_BIT_MASK (1ULL << PUSH_BUTTON_3)


gpio_config_t myGPIOconfig;

void config_ports(void);

void Led_strip_task1(void *parameter);
void Led_strip_task2(void *parameter);
void Led_strip_task3(void *parameter);

void app_main(void)
{
    config_ports();

    xTaskCreate(Led_strip_task1, "led_strip1_task", 2048, NULL, 2, NULL);
    xTaskCreate(Led_strip_task2, "led_strip2_task", 2048, NULL, 2, NULL);
    xTaskCreate(Led_strip_task3, "led_strip3_task", 2048, NULL, 2, NULL);


    while(1)
    {
        // no return from here
    }

}

void config_ports(void)
{
    // Configure Digital I/O for LEDs
    myGPIOconfig.pin_bit_mask = (LED_STRIP1_BIT_MASK | LED_STRIP2_BIT_MASK | LED_STRIP3_BIT_MASK);
    myGPIOconfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    myGPIOconfig.pull_up_en = GPIO_PULLUP_DISABLE;
    myGPIOconfig.mode = GPIO_MODE_OUTPUT;
    myGPIOconfig.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&myGPIOconfig);

    myGPIOconfig.pin_bit_mask = (PUSH_BUTTON_2_BIT_MASK | PUSH_BUTTON_3_BIT_MASK);
    myGPIOconfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    myGPIOconfig.pull_up_en = GPIO_PULLUP_DISABLE;
    myGPIOconfig.mode = GPIO_MODE_INPUT;
    myGPIOconfig.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&myGPIOconfig);

    gpio_set_direction(PUSH_BUTTON_1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PUSH_BUTTON_1, GPIO_PULLUP_ONLY);

}

void Led_strip_task1(void *parameters)
{
    for(;;)
    {
            gpio_set_level(LED_STRIP1_G, 1);
            vTaskDelay(1);
            gpio_set_level(LED_STRIP1_G, 0);

            gpio_set_level(LED_STRIP1_R, 1);
            vTaskDelay(1);
            gpio_set_level(LED_STRIP1_R, 0);

            gpio_set_level(LED_STRIP1_B, 1);
            vTaskDelay(1);
            gpio_set_level(LED_STRIP1_B, 0);
            vTaskDelay(1);
    }
}

void Led_strip_task2(void *parameters)
{
    for(;;)
    {
            gpio_set_level(LED_STRIP2_G, 1);
            vTaskDelay(100);
            gpio_set_level(LED_STRIP2_G, 0);

            gpio_set_level(LED_STRIP2_R, 1);
            vTaskDelay(100);
            gpio_set_level(LED_STRIP2_R, 0);

            gpio_set_level(LED_STRIP2_B, 1);
            vTaskDelay(100);
            gpio_set_level(LED_STRIP2_B, 0);
            vTaskDelay(100);
    }
}

void Led_strip_task3(void *parameters)
{
    for(;;)
    {
            gpio_set_level(LED_STRIP3_G, 1);
            vTaskDelay(1000);
            gpio_set_level(LED_STRIP3_G, 0);

            gpio_set_level(LED_STRIP3_R, 1);
            vTaskDelay(1000);
            gpio_set_level(LED_STRIP3_R, 0);

            gpio_set_level(LED_STRIP3_B, 1);
            vTaskDelay(1000);
            gpio_set_level(LED_STRIP3_B, 0);
            vTaskDelay(1000);
    }
}