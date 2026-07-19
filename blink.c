/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"    // VSCode will red squiggle underline this (and how two errors in 'prblems' tab) 
                            //  but that's just vscode not getting the weird .pico_sdk setup

#include <stdio.h>      // printf() and friends


// Pico W devices use a GPIO on the WIFI chip for the LED,
// so when building for Pico W, CYW43_WL_GPIO_LED_PIN will be defined
#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 250
#endif

// Define the GPIO pins based on your breadboard layout
// these are gpIO numbers, not index of pins on chip, so led gp15 is bottom pin left hand size
#define BUTTON_PIN 14
#define LED_PIN    15

// Perform initialisation
int pico_led_init(void) {
    printf("Init onboard LED");

#if defined(PICO_DEFAULT_LED_PIN)
    // A device like Pico that uses a GPIO for the LED will define PICO_DEFAULT_LED_PIN
    // so we can use normal GPIO functionality to turn the led on and off
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    return PICO_OK;
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // For Pico W devices we need to initialise the driver etc
    return cyw43_arch_init();
#endif
}

// Turn the led on or off
void pico_set_led(bool led_on) {
#if defined(PICO_DEFAULT_LED_PIN)
    // Just set the GPIO on or off
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // Ask the wifi "driver" to set the GPIO on or off
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
#endif
}

void blink_led() {
        pico_set_led(true);
        sleep_ms(LED_DELAY_MS);
        pico_set_led(false);
        sleep_ms(LED_DELAY_MS);
}

void init_buttons() {
    printf("Init buttons...");

    // Configure the LED pin as an Output
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Configure the Button pin as an Input
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);    
    gpio_pull_up(BUTTON_PIN);       // 'pull up' adds a resistor so when not connected it's not floating (is 1 instead)
}

void button_led() {
    // Read the current state of the button pin
    // Since we use a pull-up, gpio_get returns 0 (false) when pressed!
    if (gpio_get(BUTTON_PIN) == 0) {
        gpio_put(LED_PIN, 1);  // Turn LED ON
    } else {
        gpio_put(LED_PIN, 0);  // Turn LED OFF
    }

    // Small delay to prevent the CPU from spinning at 100% capacity
    sleep_ms(10);
}

int main() {
    stdio_init_all();
    int rc = pico_led_init();
    hard_assert(rc == PICO_OK);
    init_buttons();
    while (true) {
        // blink_led(); waits in tight loopp doesn't work with button reading...
        button_led();
    }

    printf("How did we get here?");
}
