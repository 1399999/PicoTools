#include <stdio.h>
#include "pico/stdlib.h"
#include "PicoLibrary.h"

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
        write_line("Hello, world!");

        set_led(true);
        sleep(1000);

        set_led(false);
        sleep(1000);
    }
}

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
