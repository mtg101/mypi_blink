/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"    // VSCode will red squiggle underline this (and how two errors in 'prblems' tab) 
                            //  but that's just vscode not getting the weird .pico_sdk setup
#include "hardware/gpio.h"
#include "hardware/sync.h"

#include <stdio.h>      // printf() and friends


// Pico W devices use a GPIO on the WIFI chip for the LED,
// so when building for Pico W, CYW43_WL_GPIO_LED_PIN will be defined
#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

// Define the GPIO pins based on your breadboard layout
// these are gpIO numbers, not index of pins on chip, so led gp15 is bottom pin left hand size
#define BUTTON_PIN 14
#define LED_PIN    15

// Perform initialisation
int pico_led_init(void) {
    printf("Init onboard LED...\n");

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

// Turn the internal led on or off
void pico_set_internal_led(bool led_on) {
#if defined(PICO_DEFAULT_LED_PIN)
    // Just set the GPIO on or off
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // Ask the wifi "driver" to set the GPIO on or off
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
#endif
}

// Timer Interrupt Callback Routine (Hardware Timer Interrupt)
bool repeating_timer_callback(struct repeating_timer *t) {
    // Keep track of the internal LED state between function calls
    static bool internal_led_state = false;
    
    // Toggle the state and write it to the pin
    internal_led_state = !internal_led_state;
    pico_set_internal_led(internal_led_state);
    
    // Returning true tells the SDK to automatically schedule the next 250ms alarm
    return true; 
}

void init_buttons() {
    printf("Init buttons...\n");

    // Configure the LED pin as an Output
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Configure the Button pin as an Input
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);    
    gpio_pull_up(BUTTON_PIN);       // 'pull up' adds a resistor so when not connected it's not floating (is 1 instead)
}

bool led_state = false;      // Tracks whether the LED should be on or off
volatile uint32_t last_interrupt_time = 0;

void button_led() {
    // Toggle the state variable
    led_state = !led_state; 

    if(led_state) {
        printf("Light goes on\n");
    } else {
        printf("Light goes off\n");
    }
    
    // Apply the new state to the physical LED pin
    gpio_put(LED_PIN, led_state);
}


// Interrupt Service Routine (ISR)
void button_isr_handler(uint gpio, uint32_t events) {
    // Basic hardware debouncing: check time elapsed since last interrupt
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Only trigger if 50ms have passed since the last bounce noise
    if (current_time - last_interrupt_time > 50) {
        // Toggle the state
        button_led();
        
        last_interrupt_time = current_time;
    }
}
int main() {
    stdio_init_all();
    int rc = pico_led_init();
    hard_assert(rc == PICO_OK);
    init_buttons();

    // Configure the interrupt: trigger on the falling edge (when button goes 1 -> 0)
    gpio_set_irq_enabled_with_callback(
        BUTTON_PIN, 
        GPIO_IRQ_EDGE_FALL, 
        true, 
        &button_isr_handler
    );

    // Configure the Hardware Timer Interrupt (1000ms interval)
    struct repeating_timer timer;
    add_repeating_timer_ms(1000, repeating_timer_callback, NULL, &timer);

    // The main loop can now sit completely idle or process other tasks!
    while (true) {
        __wfi(); // Wait For Interrupt - puts the CPU into a low-power sleep state
    }

    printf("How did we get here?\n");
}
