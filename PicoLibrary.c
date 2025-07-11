#include <stdio.h>
#include "pico/stdlib.h"
#include "PicoLibrary.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

// Pico W devices use a GPIO on the WIFI chip for the LED,
// so when building for Pico W, CYW43_WL_GPIO_LED_PIN will be defined.
#ifdef CYW43_WL_GPIO_LED_PIN
    #include "pico/cyw43_arch.h"
#endif

// For testing purposes.
int main()
{
    while(true)
    {
        write_line("Example 1\n");

        write_line("Hello, world!");

        write_line("\nExample 2\n");

        set_led(true);
        sleep(500);

        set_led(false);
        sleep(500);

        write_line("\nExample 3\n");
    }
}

#pragma region Basic Functions
void write(string str)
{
    if (!is_stdio_init)
    {
        stdio_init_all();
        is_stdio_init = true;
    }

    printf(str);
}

void write_line(string str)
{
    write(str);
    write("\n");
}

void sleep(uint32_t milliseconds)
{
    sleep_ms(milliseconds);
}

int led_init()
{
    #if defined(PICO_DEFAULT_LED_PIN)
        // A device like Pico that uses a GPIO for the LED will define PICO_DEFAULT_LED_PIN
        // so we can use normal GPIO functionality to turn the led on and off.
        gpio_init(PICO_DEFAULT_LED_PIN);
        gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
        return PICO_OK;
    #elif defined(CYW43_WL_GPIO_LED_PIN)
        // For Pico W devices we need to initialise the driver, etc.
        return cyw43_arch_init();
    #endif

    is_led_init = true;
}

void set_led(bool led_on)
{
    if (!is_led_init)
    {
        led_init();
    }

    #if defined(PICO_DEFAULT_LED_PIN)
        // Just set the GPIO on or off
        gpio_put(PICO_DEFAULT_LED_PIN, led_on);
    #elif defined(CYW43_WL_GPIO_LED_PIN)
        // Ask the wifi "driver" to set the GPIO on or off
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
    #endif
}

#pragma endregion

uint16_t read_gpio_pin_adc_raw(uint8_t adc_input)
{
    if (!is_adc_init)
    {
        adc_init();
        is_adc_init = true;
    }

    if (!contains_uint8_t(temp_used_adc_gpio_pins, adc_input + 26))
    {
        // Make sure GPIO is high-impedance, no pullups etc.
        adc_gpio_init(adc_input + 26);

        temp_used_adc_gpio_pins[temp_adc_gpio_index++] = adc_input + 26;
    }

    // Select ADC input 0 (GPIO26), input 1 (GPIO27) ....
    adc_select_input(adc_input);

    return adc_read();
}

float read_gpio_pin_adc_volts(uint8_t adc_input)
{
    const float conversion_factor = 3.3f / (1 << 12);
    return read_gpio_pin_adc_raw(adc_input) * conversion_factor;
}

bool contains_uint8_t(uint8_t array[], uint8_t value)
{
    for (uint8_t i = 0; i < temp_adc_gpio_index; i++)
    {
        if (value == array[i])
        {
            return true;
        }
    }

    return false;
}
