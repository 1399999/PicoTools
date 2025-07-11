#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

// Pico W devices use a GPIO on the WIFI chip for the LED,
// so when building for Pico W, CYW43_WL_GPIO_LED_PIN will be defined.
#ifdef CYW43_WL_GPIO_LED_PIN
    #include "pico/cyw43_arch.h"
#endif

#ifndef PICOLIBRARY_H_
#define PICOLIBRARY_H_

typedef char * string;

bool is_led_init = false;

#ifdef CYW43_WL_GPIO_LED_PIN
    bool is_pico_w = true;
#elif defined(PICO_DEFAULT_LED_PIN)
    bool is_pico_w = false;
#endif

bool is_adc_init = false;

uint8_t temp_used_adc_gpio_pins[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t * used_adc_gpio_pins = temp_used_adc_gpio_pins;
uint8_t temp_adc_gpio_index = 0;

int main();

// Basic functions
void sleep(uint32_t milliseconds);
int led_init();
void set_led(bool led_on);

// ADC functions
uint16_t read_gpio_pin_adc_raw(uint8_t adc_input);
float read_gpio_pin_adc_volts(uint8_t adc_input);

// Utility functions
bool contains_uint8_t(uint8_t array[], uint8_t value);

// Examples
void one_without_library();
void one_with_library();
void two_without_library();
void two_with_library();
void three_without_library();
void three_with_library();
void four_without_library();
void four_with_library();

#endif
