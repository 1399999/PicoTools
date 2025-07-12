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

#ifdef CYW43_WL_GPIO_LED_PIN
    bool is_pico_w = true;
#elif defined(PICO_DEFAULT_LED_PIN)
    bool is_pico_w = false;
#endif

bool is_led_init = false;
bool is_adc_init = false;
bool is_gpio_init = false;

uint8_t temp_used_adc_gpio_pins[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t * used_adc_gpio_pins = temp_used_adc_gpio_pins;
uint8_t temp_adc_gpio_index = 0;

enum temperature_enum
{
    CELCIUS,
    FAHRENHEIT,
    KELVIN
};

int main();

// Basic functions
void sleep(uint32_t milliseconds);
int led_init();
void set_led(bool led_on);

// ADC functions
uint16_t adc_read_gpio_pin_raw(uint8_t adc_input);
float adc_read_gpio_pin_volts(uint8_t adc_input);
void adc_set_temperature_sensor(bool on);
void adc_select_pin(uint8_t pin);
uint16_t adc_read_selected_raw();
float adc_read_selected_volts();
void __not_in_flash_func(adc_capture)(uint16_t *buf, size_t count);
float acd_read_onboard_temperature(enum temperature_enum temperature, uint8_t pin);

// GPIO functions
void gpio_pins_change_all(uint32_t function);
void gpio_pins_set_all_directions(uint32_t value);
void gpio_pin_set_function(uint8_t pin, gpio_function_t function);
void gpio_pin_disable_pulls(uint8_t pin);
void gpio_pin_set_input_output(uint8_t pin, bool is_input);
void gpio_pin_set_mode(uint8_t pin, bool is_input);
void gpio_pin_set_high_low(uint8_t pin, bool is_high);

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
void five_without_library();
void five_with_library();
void six_without_library();
void six_with_library();

// Utilities for Examples
void printhelp();
void __not_in_flash_func(adc_capture)(uint16_t *buf, size_t count);
float read_onboard_temperature(const char unit);

#endif
