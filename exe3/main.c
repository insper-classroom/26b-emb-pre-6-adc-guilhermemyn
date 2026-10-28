#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

#include "data.h"
QueueHandle_t xQueueData;

#define JANELA 5   /* tamanho da media movel */

// não mexer! Alimenta a fila com os dados do sinal
void data_task(void *p) {
    vTaskDelay(pdMS_TO_TICKS(400));

    int data_len = sizeof(sine_wave_four_cycles) / sizeof(sine_wave_four_cycles[0]);
    for (int i = 0; i < data_len; i++) {
        xQueueSend(xQueueData, &sine_wave_four_cycles[i], 1000000);
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void process_task(void *p) {
    int data = 0;

    /*
     * Media movel com vetor circular: guarda as ultimas 5 amostras e a soma
     * delas. A cada amostra nova, tira da soma a mais antiga e soma a nova.
     * O vetor comeca zerado, entao as primeiras saidas "sobem" ate a janela
     * encher.
     */
    int janela[JANELA] = {0};
    int indice = 0;
    int soma = 0;

    while (true) {
        if (xQueueReceive(xQueueData, &data, 100)) {
            // implementar filtro aqui!
            soma -= janela[indice];       /* descarta a amostra mais antiga */
            janela[indice] = data;        /* guarda a nova no lugar dela */
            soma += data;
            indice = (indice + 1) % JANELA;

            printf("%d \n", soma / JANELA);

            // deixar esse delay!
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}

int main() {
    stdio_init_all();

    xQueueData = xQueueCreate(64, sizeof(int));

    xTaskCreate(data_task, "Data task ", 4096, NULL, 1, NULL);
    xTaskCreate(process_task, "Process task", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}