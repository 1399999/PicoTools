#include "PicoLibrary.h"

// For testing purposes.
int main()
{
    four_with_library();
}

#pragma region Basic Functions

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
#pragma region ADC Functions
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
#pragma endregion

#pragma region Example 1 (Hello World)
    void one_without_library() 
    {
        stdio_init_all();
        while (true) 
        {
            printf("Hello, world!\n");
            sleep_ms(1000);
        }
    }

    void one_with_library() 
    {
        stdio_init_all();
        while (true) 
        {
            printf("Hello, world!\n");
            sleep(1000);
        }
    }
#pragma endregion
#pragma region Example 2 (Blink)
    #ifndef LED_DELAY_MS
        #define LED_DELAY_MS 250
    #endif

    void two_without_library() 
    {
        #if defined(PICO_DEFAULT_LED_PIN)
            // A device like Pico that uses a GPIO for the LED will define PICO_DEFAULT_LED_PIN
            // so we can use normal GPIO functionality to turn the led on and off
            gpio_init(PICO_DEFAULT_LED_PIN);
            gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
            hard_assert(PICO_OK == PICO_OK);
        #elif defined(CYW43_WL_GPIO_LED_PIN)
            // For Pico W devices we need to initialise the driver etc
            hard_assert(cyw43_arch_init() == PICO_OK);
        #endif

        while (true) 
        {
            #if defined(PICO_DEFAULT_LED_PIN)
                // Just set the GPIO on or off
                gpio_put(PICO_DEFAULT_LED_PIN, true);
            #elif defined(CYW43_WL_GPIO_LED_PIN)
                // Ask the wifi "driver" to set the GPIO on or off
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
            #endif

            sleep_ms(LED_DELAY_MS);

            #if defined(PICO_DEFAULT_LED_PIN)
                // Just set the GPIO on or off
                gpio_put(PICO_DEFAULT_LED_PIN, false);
            #elif defined(CYW43_WL_GPIO_LED_PIN)
                // Ask the wifi "driver" to set the GPIO on or off
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
            #endif

            sleep_ms(LED_DELAY_MS);
        }
    }

    void two_with_library() 
    {
        while (true)
        {
            set_led(true);
            sleep(LED_DELAY_MS);
            set_led(false);
            sleep(LED_DELAY_MS);
        }
    }
#pragma endregion
#pragma region Example 3 (Hello ADC)

void three_without_library() 
{
    stdio_init_all();
    printf("ADC Example, measuring GPIO26\n");

    adc_init();

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);
    // Select ADC input 0 (GPIO26)
    adc_select_input(0);

    while (1) 
    {
        // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
        const float conversion_factor = 3.3f / (1 << 12);
        uint16_t result = adc_read();
        printf("Raw value: 0x%03x, voltage: %f V\n", result, result * conversion_factor);
        sleep_ms(500);
    }
}

void three_with_library() 
{
    stdio_init_all();
    printf("ADC Example, measuring GPIO26\n");

    while (true) 
    {
        // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
        const float conversion_factor = 3.3f / (1 << 12);
        uint16_t result = read_gpio_pin_adc_raw(0);
        printf("Raw value: 0x%03x, voltage: %f V\n", result, result * conversion_factor);
        sleep_ms(500);
    }
}

#pragma endregion
#pragma region Example 4 (Joystick Display)

void four_without_library() 
{
    stdio_init_all();
    adc_init();
    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);
    adc_gpio_init(27);

    while (true) 
    {
        adc_select_input(0);
        uint adc_x_raw = adc_read();
        adc_select_input(1);
        uint adc_y_raw = adc_read();

        // Display the joystick position something like this:
        // X: [            o             ]  Y: [              o         ]

        const uint bar_width = 40;
        const uint adc_max = (1 << 12) - 1;

        uint bar_x_pos = adc_x_raw * bar_width / adc_max;
        uint bar_y_pos = adc_y_raw * bar_width / adc_max;

        printf("\rX: [");

        for (uint i = 0; i < bar_width; ++i)
        {
            putchar( i == bar_x_pos ? 'o' : ' ');
        }

        printf("]  Y: [");

        for (uint i = 0; i < bar_width; ++i)
        {
            putchar( i == bar_y_pos ? 'o' : ' ');
        }

        printf("]");
        sleep_ms(50);
    }
}

void four_with_library() 
{
    stdio_init_all();

    while (true) 
    {
        uint adc_x_raw = read_gpio_pin_adc_raw(0);
        uint adc_y_raw = read_gpio_pin_adc_raw(1);

        // Display the joystick position something like this:
        // X: [            o             ]  Y: [              o         ]

        const uint bar_width = 40;
        const uint adc_max = (1 << 12) - 1;

        uint bar_x_pos = adc_x_raw * bar_width / adc_max;
        uint bar_y_pos = adc_y_raw * bar_width / adc_max;

        printf("\rX: [");

        for (uint i = 0; i < bar_width; ++i)
        {
            putchar( i == bar_x_pos ? 'o' : ' ');
        }

        printf("]  Y: [");

        for (uint i = 0; i < bar_width; ++i)
        {
            putchar( i == bar_y_pos ? 'o' : ' ');
        }

        printf("]");
        sleep(50);
    }
}

#pragma endregion
#pragma region Example 5 (ADC Console)

#define N_SAMPLES 1000
uint16_t sample_buf[N_SAMPLES];

void printhelp() 
{
    puts("\nCommands:");
    puts("c0, ...\t: Select ADC channel n");
    puts("s\t: Sample once");
    puts("S\t: Sample many");
    puts("w\t: Wiggle pins");
}

void __not_in_flash_func(adc_capture)(uint16_t *buf, size_t count) 
{
    adc_fifo_setup(true, false, 0, false, false);
    adc_run(true);
    for (size_t i = 0; i < count; i = i + 1)
        buf[i] = adc_fifo_get_blocking();
    adc_run(false);
    adc_fifo_drain();
}

void five_without_library() 
{
    stdio_init_all();
    adc_init();
    adc_set_temp_sensor_enabled(true);

    // Set all pins to input (as far as SIO is concerned)
    gpio_set_dir_all_bits(0);
    for (int i = 2; i < 30; ++i) 
    {
        gpio_set_function(i, GPIO_FUNC_SIO);
        if (i >= 26) 
        {
            gpio_disable_pulls(i);
            gpio_set_input_enabled(i, false);
        }
    }

    printf("\n===========================\n");
    printf("RP2040 ADC and Test Console\n");
    printf("===========================\n");
    printhelp();

    while (true) 
    {
        char c = getchar();
        printf("%c", c);
        switch (c) 
        {
            case 'c':
                c = getchar();
                printf("%c\n", c);
                if (c < '0' || c > '7') 
                {
                    printf("Unknown input channel\n");
                    printhelp();
                } 
                
                else 
                {
                    adc_select_input(c - '0');
                    printf("Switched to channel %c\n", c);
                }

                break;
            case 's': 
            {
                uint32_t result = adc_read();
                const float conversion_factor = 3.3f / (1 << 12);
                printf("\n0x%03x -> %f V\n", result, result * conversion_factor);
                break;
            }
            
            case 'S': 
            {
                printf("\nStarting capture\n");
                adc_capture(sample_buf, N_SAMPLES);
                printf("Done\n");
                for (int i = 0; i < N_SAMPLES; i = i + 1)
                {
                    printf("%03x\n", sample_buf[i]);
                }
                break;
            }

            case 'w': 
            {
                printf("\nPress any key to stop wiggling\n");
                int i = 1;
                gpio_set_dir_all_bits(-1);
                while (getchar_timeout_us(0) == PICO_ERROR_TIMEOUT) {
                    // Pattern: Flash all pins for a cycle,
                    // Then scan along pins for one cycle each
                    i = i ? i << 1 : 1;
                    gpio_put_all(i ? i : ~0);
                }
                gpio_set_dir_all_bits(0);
                printf("Wiggling halted.\n");
                break;
            }

            case '\n':
            case '\r':
                break;
            case 'h':
                printhelp();
                break;
            default:
                printf("\nUnrecognised command: %c\n", c);
                printhelp();
                break;
        }
    }
}

#pragma endregion
