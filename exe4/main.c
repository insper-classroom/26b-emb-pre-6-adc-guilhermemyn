/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

const int PIN_LED_B = 4;
const int PIN_POT = 28;   /* CONFERIR no diagram.json: pino do sinal do potenciometro */

const float conversion_factor = 3.3f / (1 << 12);

/**
 * 0..1.0V: Desligado
 * 1..2.0V: 300 ms
 * 2..3.3V: 500 ms
 * (o enunciado diz 300/500 ms; o comentario original do codigo dizia 150/400)
 */
static const int PERIODO_MS[3] = {0, 300, 500};

volatile int flag_timer = 0;

/* Callback do timer: so levanta a flag, quem mexe no LED e o main */
bool timer_callback(repeating_timer_t *rt) {
    flag_timer = 1;
    return true;   /* continua repetindo */
}

/*
 * Converte a tensao lida na zona (0, 1 ou 2), com histerese de 50 mV.
 * Sem histerese, uma leitura perto de 1,0 V ou 2,0 V oscila entre duas zonas
 * por causa do ruido do ADC; cada troca reinicia o timer e o LED nunca chega
 * a piscar. Com histerese, para mudar de zona a tensao precisa passar do
 * limite por pelo menos 50 mV.
 */
#define HISTERESE 0.05f

int zona_da_tensao(float tensao, int zona_atual) {
    float limite_1 = (zona_atual >= 1) ? 1.0f - HISTERESE : 1.0f + HISTERESE;
    float limite_2 = (zona_atual == 2) ? 2.0f - HISTERESE : 2.0f + HISTERESE;

    if (tensao >= limite_2) {
        return 2;
    }
    if (tensao >= limite_1) {
        return 1;
    }
    return 0;
}

int main() {
    stdio_init_all();

    gpio_init(PIN_LED_B);
    gpio_set_dir(PIN_LED_B, GPIO_OUT);
    gpio_put(PIN_LED_B, 0);

    adc_init();
    adc_gpio_init(PIN_POT);
    adc_select_input(PIN_POT - 26);   /* GP26 = ADC0, GP27 = ADC1, GP28 = ADC2 */

    repeating_timer_t timer;
    int timer_ativo = 0;
    int zona_atual = 0;
    int led = 0;

    while (1) {
        float tensao = adc_read() * conversion_factor;
        int zona = zona_da_tensao(tensao, zona_atual);

        /* Mudou de zona: para o timer antigo e, se nao for a zona 0, cria um novo */
        if (zona != zona_atual) {
            if (timer_ativo) {
                cancel_repeating_timer(&timer);
                timer_ativo = 0;
            }
            flag_timer = 0;       /* descarta um disparo antigo */
            led = 0;
            gpio_put(PIN_LED_B, 0);

            if (zona > 0) {
                add_repeating_timer_ms(PERIODO_MS[zona], timer_callback, NULL, &timer);
                timer_ativo = 1;
            }
            zona_atual = zona;
        }

        /* Cada disparo do timer inverte o LED */
        if (flag_timer) {
            flag_timer = 0;
            led = !led;
            gpio_put(PIN_LED_B, led);
        }
    }
}