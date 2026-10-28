#include <stdio.h>

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "pico/stdlib.h"

SemaphoreHandle_t xPrintfMutex;

void task_a(void *p) {
    while (true) {
        /* Toma o mutex: so uma task por vez usa o printf */
        if (xSemaphoreTake(xPrintfMutex, portMAX_DELAY) == pdTRUE) {
            printf("Task A: inicio da mensagem\n");
            vTaskDelay(pdMS_TO_TICKS(50));   /* continua com o mutex: faz parte da secao critica */
            printf("Task A: fim da mensagem\n");
            xSemaphoreGive(xPrintfMutex);    /* libera so depois do segundo printf */
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void task_b(void *p) {
    while (true) {
        if (xSemaphoreTake(xPrintfMutex, portMAX_DELAY) == pdTRUE) {
            printf("Task B: inicio da mensagem\n");
            vTaskDelay(pdMS_TO_TICKS(50));
            printf("Task B: fim da mensagem\n");
            xSemaphoreGive(xPrintfMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

int main() {
    stdio_init_all();

    xPrintfMutex = xSemaphoreCreateMutex();

    xTaskCreate(task_a, "Task A", 1024, NULL, 1, NULL);
    xTaskCreate(task_b, "Task B", 1024, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true) {
    }
}