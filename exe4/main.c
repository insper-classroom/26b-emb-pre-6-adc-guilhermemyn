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
 
 const float conversion_factor = 3.3f / (1 << 12);

 volatile int flag_timer = 0;

 bool timer_callback(repeating_timer_t *rt) {
    flag_timer = 1;
    return true;
 }

 int zone_calculator(float tension) {
    if (tension < 1) {
        return 0;
    } else if (tension < 2) {
        return 1;
    } else {
        return 2;
    }
 }
 /**
  * 0..1.0V: Desligado
  * 1..2.0V: 150 ms
  * 2..3.3V: 400 ms
  */
 int main() {
    stdio_init_all();
    adc_init();
    gpio_init(PIN_LED_B);
    gpio_set_dir(PIN_LED_B, GPIO_OUT);

    adc_gpio_init(28);
    adc_select_input(2);

    int zona = 0;
    int led_state = 0;
    int timer_on = 0;
    repeating_timer_t timer;

    while (1) {
        uint16_t result = adc_read();
        float tensao = result * conversion_factor;
        int nova_zona;

        nova_zona = zone_calculator(tensao);

        if(nova_zona != zona) {
            if (timer_on == 1) {
                cancel_repeating_timer(&timer);
                timer_on = 0;
            }
            if (nova_zona == 0) {
                gpio_put(PIN_LED_B, 0);
                led_state = 0;
                flag_timer = 0;
            } else if (nova_zona == 1) {
                if (add_repeating_timer_ms(300, timer_callback, NULL, &timer)) {
                    timer_on = 1;
                }
            } else {
                if (add_repeating_timer_ms(500, timer_callback, NULL, &timer)) {
                    timer_on = 1;
                }            
            }
            zona = nova_zona;
        }

        if (flag_timer) {
            led_state = !led_state;
            gpio_put(PIN_LED_B, led_state);
            flag_timer = 0;
        }
    
    }
 }