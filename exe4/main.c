#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;

const int LED_PIN_R = 4;
const int LED_PIN_G = 6;

QueueHandle_t xQueueButId;
SemaphoreHandle_t xSemaphore_r;

QueueHandle_t xQueueButId2;
SemaphoreHandle_t xSemaphore_g;

void btn_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        if (gpio == BTN_PIN_R) {
            xSemaphoreGiveFromISR(xSemaphore_r, 0);
        } else if (gpio == BTN_PIN_G) {
            xSemaphoreGiveFromISR(xSemaphore_g, 0);
        }
    }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int delay = 0;
    while (true) {
        if (xQueueReceive(xQueueButId, &delay, 0)) {
            printf("LED R novo delay: %d\n", delay);
        }
        if (delay > 0) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}

void btn_1_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, btn_callback);

    int delay = 0;
    while (true) {
        if (xSemaphoreTake(xSemaphore_r, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (delay < 1000)
                delay += 100;
            else
                delay = 100;
            printf("BTN_R delay: %d\n", delay);
            xQueueSend(xQueueButId, &delay, 0);
        }
    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    int delay = 0;
    while (true) {
        if (xQueueReceive(xQueueButId2, &delay, 0)) {
            printf("LED delay: %d\n", delay);
        }
        if (delay > 0) {
            gpio_put(LED_PIN_G, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_G, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}

void btn_2_task(void *p) {
    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);
    gpio_set_irq_enabled(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true);

    int delay = 0;
    while (true) {
        if (xSemaphoreTake(xSemaphore_g, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (delay < 1000)
                delay += 100;
            else
                delay = 100;
            printf("BTN_G delay: %d\n", delay);
            xQueueSend(xQueueButId2, &delay, 0);
        }
    }
}

int main() {
    stdio_init_all();
    printf("Start RTOS\n");

    xQueueButId = xQueueCreate(32, sizeof(int));
    xSemaphore_r = xSemaphoreCreateBinary();

    xQueueButId2 = xQueueCreate(32, sizeof(int));
    xSemaphore_g = xSemaphoreCreateBinary();

    xTaskCreate(led_1_task, "LED_Task_R", 256, NULL, 1, NULL);
    xTaskCreate(btn_1_task, "BTN_Task_R", 256, NULL, 1, NULL);

    xTaskCreate(led_2_task, "LED_Task_G", 256, NULL, 1, NULL);
    xTaskCreate(btn_2_task, "BTN_Task_G", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true) { }
    return 0;
}