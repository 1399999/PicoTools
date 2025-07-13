#include "PicoLibrary.h"

// For testing purposes.
int main()
{
    six_with_library();
}

#pragma region Basic Functions

void sleep(uint32_t milliseconds)
{
    sleep_ms(milliseconds);
}

void led_set(bool led_on)
{
    if (!is_led_init)
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

uint16_t adc_read_gpio_pin_raw(uint8_t adc_input)
{
    int pin = adc_input + 26;

    if (!is_adc_init)
    {
        adc_init();
        is_adc_init = true;
    }

    if (!is_gpio_init)
    {
        gpio_init(pin);
        is_adc_init = true;
    }

    if (!contains_uint8_t(temp_used_adc_gpio_pins, pin))
    {
        // Make sure GPIO is high-impedance, no pullups etc.
        adc_gpio_init(pin);

        temp_used_adc_gpio_pins[temp_adc_gpio_index++] = pin;
    }

    // Select ADC input 0 (GPIO26), input 1 (GPIO27) ....
    adc_select_input(adc_input);

    return adc_read();
}

float adc_read_gpio_pin_volts(uint8_t adc_input)
{
    const float conversion_factor = 3.3f / (1 << 12);
    return adc_read_gpio_pin_raw(adc_input) * conversion_factor;
}

void adc_set_temperature_sensor(bool on)
{
    if (!is_adc_init)
    {
        adc_init();
        is_adc_init = true;
    }

    adc_set_temp_sensor_enabled(on);
}

void adc_select_pin(uint8_t pin)
{
    if (!is_adc_init)
    {
        adc_init();
        is_adc_init = true;
    }

    adc_select_input(pin);
}

uint16_t adc_read_selected_raw()
{
    if (!is_adc_init)
    {
        adc_init();
        is_adc_init = true;
    }

    return adc_read();
}

float adc_read_selected_volts()
{
    const float conversion_factor = 3.3f / (1 << 12);
    return adc_read_selected_raw() * conversion_factor;
}

void __not_in_flash_func(adc_capture)(uint16_t *buf, size_t count) 
{
    if (!is_adc_init)
    {
        adc_init();
        is_adc_init = true;
    }

    adc_fifo_setup(true, false, 0, false, false);
    adc_run(true);

    for (size_t i = 0; i < count; i = i + 1)
    {
        buf[i] = adc_fifo_get_blocking();
    }

    adc_run(false);
    adc_fifo_drain();
}

float acd_read_onboard_temperature(enum temperature_enum temperature, uint8_t pin) 
{
    if (!is_adc_init)
    {
        adc_init();
        is_gpio_init = true;
    }

    /* Initialize hardware AD converter, enable onboard temperature sensor and
     *   select its channel (do this once for efficiency, but beware that this
     *   is a global operation). */

    adc_set_temp_sensor_enabled(true);
    adc_select_input(pin);

    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.3f / (1 << 12);

    float adc = (float) adc_read() * conversionFactor;
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

    if (temperature == CELCIUS) 
    {
        return tempC;
    } 
    
    else if (temperature == FAHRENHEIT) 
    {
        return tempC * 9 / 5 + 32;
    }

    else if (temperature == KELVIN) 
    {
        return tempC + 273.15;
    }

    return -1.0f;
}

#pragma endregion
#pragma region GPIO Functions

void gpio_pins_change_all(uint32_t function)
{
    gpio_put_all(function);
}

void gpio_pins_set_all_directions(uint32_t value)
{
    gpio_set_dir_all_bits(value);
}

void gpio_pin_set_function(uint8_t pin, gpio_function_t function)
{
    if (!is_gpio_init)
    {
        gpio_init(pin);
        is_gpio_init = true;
    }

    gpio_set_function(pin, function);
}

void gpio_pin_disable_pulls(uint8_t pin)
{
    if (!is_gpio_init)
    {
        gpio_init(pin);
        is_gpio_init = true;
    }

    gpio_disable_pulls(pin);
}

void gpio_pin_set_input_output(uint8_t pin, bool is_input)
{
    if (!is_gpio_init)
    {
        gpio_init(pin);
        is_gpio_init = true;
    }

    gpio_set_input_enabled(pin, is_input);
}

void gpio_pin_set_mode(uint8_t pin, bool is_input)
{
    if (!is_gpio_init)
    {
        gpio_init(pin);
        is_gpio_init = true;
    }

    gpio_set_input_enabled(pin, is_input);
}

void gpio_pin_set_high_low(uint8_t pin, bool is_high)
{
    if (!is_gpio_init)
    {
        gpio_init(pin);
        is_gpio_init = true;
    }

    gpio_put(pin, is_high);
}

#pragma region Miscellaneous Functions

int power_get_status(bool * is_battery_powered) 
{
    // Pico W uses a CYW43 pin to get VBUS so we need to initialize it.
    #if CYW43_USES_VSYS_PIN
        if (!is_pico_w_init)
        {
            cyw43_arch_init();
            is_pico_w_init = true;
        }
    #endif

    #if defined CYW43_WL_GPIO_VBUS_PIN
        *is_battery_powered = !cyw43_arch_gpio_get(CYW43_WL_GPIO_VBUS_PIN);
        return PICO_OK;
    #elif defined PICO_VBUS_PIN
        gpio_set_function(PICO_VBUS_PIN, GPIO_FUNC_SIO);
        *is_battery_powered = !gpio_get(PICO_VBUS_PIN);
        return PICO_OK;
    #else
        return PICO_ERROR_NO_DATA;
    #endif
}

int power_get_voltage_status(float * voltage_result, uint8_t pin, int power_sample_count) 
{
    // Pico W uses a CYW43 pin to get VBUS so we need to initialize it.
    #if CYW43_USES_VSYS_PIN
        if (!is_pico_w_init)
        {
            cyw43_arch_init();
            is_pico_w_init = true;
        }
    #endif

    #ifndef PICO_VSYS_PIN
        return PICO_ERROR_NO_DATA;
    #else
    #if CYW43_USES_VSYS_PIN
        cyw43_thread_enter();
        // Make sure cyw43 is awake
        cyw43_arch_gpio_get(CYW43_WL_GPIO_VBUS_PIN);
    #endif

    // setup adc
    adc_gpio_init(PICO_VSYS_PIN);
    adc_select_input(PICO_VSYS_PIN - pin);
 
    adc_fifo_setup(true, false, 0, false, false);
    adc_run(true);

    // We seem to read low values initially - this seems to fix it
    int ignore_count = power_sample_count;

    while (!adc_fifo_is_empty() || ignore_count-- > 0) 
    {
        (void)adc_fifo_get_blocking();
    }

    // read vsys
    uint32_t vsys = 0;

    for(int i = 0; i < power_sample_count; i++) 
    {
        uint16_t val = adc_fifo_get_blocking();
        vsys += val;
    }

    adc_run(false);
    adc_fifo_drain();

    vsys /= power_sample_count;
    #if CYW43_USES_VSYS_PIN
        cyw43_thread_exit();
    #endif
        // Generate voltage
        const float conversion_factor = 3.3f / (1 << 12);
        *voltage_result = vsys * 3 * conversion_factor;
        return PICO_OK;
    #endif
}

void pico_w_deinit()
{
    #if CYW43_USES_VSYS_PIN
        cyw43_arch_deinit();
    #endif
}

#pragma endregion

#pragma endregion
#pragma region Utility Functions

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
            led_set(true);
            sleep(LED_DELAY_MS);
            led_set(false);
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
        uint16_t result = adc_read_gpio_pin_raw(0);
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
        uint adc_x_raw = adc_read_gpio_pin_raw(0);
        uint adc_y_raw = adc_read_gpio_pin_raw(1);

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
            {
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
            }

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
                while (getchar_timeout_us(0) == PICO_ERROR_TIMEOUT) 
                {
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

void five_with_library() 
{
    stdio_init_all();
    adc_set_temperature_sensor(true);

    // Set all pins to input (as far as SIO is concerned).
    gpio_pins_set_all_directions(0);

    for (int i = 2; i < 30; ++i) 
    {
        gpio_pin_set_function(i, GPIO_FUNC_SIO);
        if (i >= 26) 
        {
            gpio_pin_disable_pulls(i);
            gpio_pin_set_input_output(i, false);
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
            {
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
            }

            case 's': 
            {
                uint32_t result = adc_read_selected_raw();
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
                gpio_pins_set_all_directions(-1);

                while (getchar_timeout_us(0) == PICO_ERROR_TIMEOUT) 
                {
                    // Pattern: Flash all pins for a cycle,
                    // Then scan along pins for one cycle each
                    i = i ? i << 1 : 1;
                    gpio_pins_change_all(i ? i : ~0);
                }

                gpio_pins_set_all_directions(0);
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
#pragma region Example 6 (Onboard Temperature)

/* Choose 'C' for Celsius or 'F' for Fahrenheit. */
#define TEMPERATURE_UNITS 'C'

/* References for this implementation:
 * raspberry-pi-pico-c-sdk.pdf, Section '4.1.1. hardware_adc'
 * pico-examples/adc/adc_console/adc_console.c */
float read_onboard_temperature(const char unit) 
{
    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.3f / (1 << 12);

    float adc = (float)adc_read() * conversionFactor;
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

    if (unit == 'C') 
    {
        return tempC;
    } 
    
    else if (unit == 'F') 
    {
        return tempC * 9 / 5 + 32;
    }

    return -1.0f;
}

void six_without_library() 
{
    stdio_init_all();
    #ifdef PICO_DEFAULT_LED_PIN
        gpio_init(PICO_DEFAULT_LED_PIN);
        gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    #endif

    /* Initialize hardware AD converter, enable onboard temperature sensor and
     *   select its channel (do this once for efficiency, but beware that this
     *   is a global operation). */
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);

    while (true) 
    {
        float temperature = read_onboard_temperature(TEMPERATURE_UNITS);
        printf("Onboard temperature = %.02f %c\n", temperature, TEMPERATURE_UNITS);

        #ifdef PICO_DEFAULT_LED_PIN
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            sleep_ms(10);

            gpio_put(PICO_DEFAULT_LED_PIN, 0);
        #endif

        sleep_ms(990);
    }
}

void six_with_library() 
{
    stdio_init_all();

    #ifdef PICO_DEFAULT_LED_PIN
        gpio_pin_set_mode(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    #endif

    while (true) 
    {
        float temperature = acd_read_onboard_temperature(CELCIUS, 4);
        printf("Onboard temperature = %.02f %c\n", temperature, TEMPERATURE_UNITS);

        #ifdef PICO_DEFAULT_LED_PIN
            gpio_pin_set_high_low(PICO_DEFAULT_LED_PIN, 1);
            sleep(10);

            gpio_pin_set_high_low(PICO_DEFAULT_LED_PIN, 0);
        #endif

        sleep(990);
    }
}

#pragma endregion
#pragma region Example 7 (ADC Microphone)

void seven_without_library() 
{
    #define ADC_NUM 0
    #define ADC_PIN (26 + ADC_NUM)
    #define ADC_VREF 3.3
    #define ADC_RANGE (1 << 12)
    #define ADC_CONVERT (ADC_VREF / (ADC_RANGE - 1))

    stdio_init_all();
    printf("Beep boop, listening...\n");

    bi_decl(bi_program_description("Analog microphone example for Raspberry Pi Pico")); // for picotool
    bi_decl(bi_1pin_with_name(ADC_PIN, "ADC input pin"));

    adc_init();
    adc_gpio_init( ADC_PIN);
    adc_select_input( ADC_NUM);

    uint adc_raw;
    
    while (true) 
    {
        adc_raw = adc_read(); // raw voltage from ADC
        printf("%.2f\n", adc_raw * ADC_CONVERT);
        sleep_ms(10);
    }
}

void seven_with_library() 
{
    #define ADC_NUM 0
    #define ADC_PIN (26 + ADC_NUM)
    #define ADC_VREF 3.3
    #define ADC_RANGE (1 << 12)
    #define ADC_CONVERT (ADC_VREF / (ADC_RANGE - 1))

    stdio_init_all();
    printf("Beep boop, listening...\n");
    
    // Binary info canot be made into functions.
    binary_info_add_description("Analog microphone example for Raspberry Pi Pico"); // for picotool
    binary_info_name_pin(ADC_PIN, "ADC input pin");

    uint adc_raw;
    
    while (true) 
    {
        adc_raw = adc_read_gpio_pin_raw(ADC_NUM); // raw voltage from ADC
        printf("%.2f\n", adc_raw * ADC_CONVERT);
        sleep(10);
    }
}

#pragma endregion
#pragma region Example 8 (Read VSYS)

#ifndef PICO_POWER_SAMPLE_COUNT
    #define PICO_POWER_SAMPLE_COUNT 3
#endif

// Pin used for ADC 0
#define PICO_FIRST_ADC_PIN 26

int power_source(bool *battery_powered) 
{
    #if defined CYW43_WL_GPIO_VBUS_PIN
        *battery_powered = !cyw43_arch_gpio_get(CYW43_WL_GPIO_VBUS_PIN);
        return PICO_OK;
    #elif defined PICO_VBUS_PIN
        gpio_set_function(PICO_VBUS_PIN, GPIO_FUNC_SIO);
        *battery_powered = !gpio_get(PICO_VBUS_PIN);
        return PICO_OK;
    #else
        return PICO_ERROR_NO_DATA;
    #endif
}

int power_voltage(float *voltage_result) 
{
    #ifndef PICO_VSYS_PIN
        return PICO_ERROR_NO_DATA;
    #else
    #if CYW43_USES_VSYS_PIN
        cyw43_thread_enter();
        // Make sure cyw43 is awake
        cyw43_arch_gpio_get(CYW43_WL_GPIO_VBUS_PIN);
    #endif

    // setup adc
    adc_gpio_init(PICO_VSYS_PIN);
    adc_select_input(PICO_VSYS_PIN - PICO_FIRST_ADC_PIN);
 
    adc_fifo_setup(true, false, 0, false, false);
    adc_run(true);

    // We seem to read low values initially - this seems to fix it
    int ignore_count = PICO_POWER_SAMPLE_COUNT;

    while (!adc_fifo_is_empty() || ignore_count-- > 0) 
    {
        (void)adc_fifo_get_blocking();
    }

    // read vsys
    uint32_t vsys = 0;

    for(int i = 0; i < PICO_POWER_SAMPLE_COUNT; i++) 
    {
        uint16_t val = adc_fifo_get_blocking();
        vsys += val;
    }

    adc_run(false);
    adc_fifo_drain();

    vsys /= PICO_POWER_SAMPLE_COUNT;
    #if CYW43_USES_VSYS_PIN
        cyw43_thread_exit();
    #endif
        // Generate voltage
        const float conversion_factor = 3.3f / (1 << 12);
        *voltage_result = vsys * 3 * conversion_factor;
        return PICO_OK;
    #endif
}

void eight_without_library() 
{
    stdio_init_all();

    adc_init();
    adc_set_temp_sensor_enabled(true);

    // Pico W uses a CYW43 pin to get VBUS so we need to initialise it
    #if CYW43_USES_VSYS_PIN
    if (cyw43_arch_init()) 
    {
        printf("failed to initialise\n");
        return 1;
    }
    #endif

    bool old_battery_status = false;
    bool battery_status = true;
    float old_voltage = -1;
    char *power_str = "UNKNOWN";

    while(true) 
    {
        // Get battery status
        if (power_source(&battery_status) == PICO_OK) 
        {
            power_str = battery_status ? "BATTERY" : "POWERED";
        }

        // Get voltage
        float voltage = 0;
        int voltage_return = power_voltage(&voltage);
        voltage = floorf(voltage * 100) / 100;

        // Display power if it's changed
        if (old_battery_status != battery_status || old_voltage != voltage) 
        {
            char percent_buf[10] = {0};

            if (battery_status && voltage_return == PICO_OK) 
            {
                const float min_battery_volts = 3.0f;
                const float max_battery_volts = 4.2f;
                int percent_val = (int) (((voltage - min_battery_volts) / (max_battery_volts - min_battery_volts)) * 100);
                snprintf(percent_buf, sizeof(percent_buf), " (%d%%)", percent_val);
            }

            // Also get the temperature
            adc_select_input(4);
            const float conversionFactor = 3.3f / (1 << 12);
            float adc = (float)adc_read() * conversionFactor;
            float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

            // Display power and remember old vales
            printf("Power %s, %.2fV%s, temp %.1f DegC\n", power_str, voltage, percent_buf, tempC);
            old_battery_status = battery_status;
            old_voltage = voltage;
        }

        sleep_ms(1000);
    }

    #if CYW43_USES_VSYS_PIN
        cyw43_arch_deinit();
    #endif  
}

void eight_with_library() 
{
    stdio_init_all();

    bool old_battery_status = false;
    bool battery_status = true;
    float old_voltage = -1;
    string power_str = "UNKNOWN";

    while(true) 
    {
        // Get battery status
        if (power_get_status(&battery_status) == PICO_OK) 
        {
            power_str = battery_status ? "BATTERY" : "POWERED";
        }

        // Get voltage
        float voltage = 0;
        int voltage_return = power_get_voltage_status(&voltage, PICO_FIRST_ADC_PIN, PICO_POWER_SAMPLE_COUNT);
        voltage = floorf(voltage * 100) / 100;

        // Display power if it's changed
        if (old_battery_status != battery_status || old_voltage != voltage) 
        {
            char percent_buf[10] = {0};

            if (battery_status && voltage_return == PICO_OK) 
            {
                const float min_battery_volts = 3.0f;
                const float max_battery_volts = 4.2f;
                int percent_val = (int) (((voltage - min_battery_volts) / (max_battery_volts - min_battery_volts)) * 100);
                snprintf(percent_buf, sizeof(percent_buf), " (%d%%)", percent_val);
            }

            // Also get the temperature
            float adc = adc_read_gpio_pin_volts(4);
            float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

            // Display power and remember old vales
            printf("Power %s, %.2fV%s, temp %.1f DegC\n", power_str, voltage, percent_buf, tempC);
            old_battery_status = battery_status;
            old_voltage = voltage;
        }

        sleep(1000);
    }

    pico_w_deinit();
}

#pragma endregion
#pragma region Example 9 (Blink Any)

// Set an LED_TYPE variable - 0 is default, 1 is connected to WIFI chip
// Note that LED_TYPE == 1 is only supported when initially compiled for
// a board with PICO_CYW43_SUPPORTED (eg pico_w), else the required
// libraries won't be present
bi_decl(bi_program_feature_group(0x1111, 0, "LED Configuration"));
#if defined(PICO_DEFAULT_LED_PIN)
    // the tag and id are not important as picotool filters based on the
    // variable name, so just set them to 0
    bi_decl(bi_ptr_int32(0x1111, 0, LED_TYPE, 0));
    bi_decl(bi_ptr_int32(0x1111, 0, LED_PIN, PICO_DEFAULT_LED_PIN));
#elif defined(CYW43_WL_GPIO_LED_PIN)
    bi_decl(bi_ptr_int32(0x1111, 0, LED_TYPE, 1));
    bi_decl(bi_ptr_int32(0x1111, 0, LED_PIN, CYW43_WL_GPIO_LED_PIN));
#else
    bi_decl(bi_ptr_int32(0x1111, 0, LED_TYPE, 0));
    bi_decl(bi_ptr_int32(0x1111, 0, LED_PIN, 25));
#endif

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 250
#endif

// Perform initialisation
int pico_led_init(void) 
{
    if (LED_TYPE == 0) 
    {
        // A device like Pico that uses a GPIO for the LED so we can
        // use normal GPIO functionality to turn the led on and off
        gpio_init(LED_PIN);
        gpio_set_dir(LED_PIN, GPIO_OUT);
        return PICO_OK;

        #ifdef CYW43_WL_GPIO_LED_PIN
            } 
            
            else if (LED_TYPE == 1) 
            {
                // For Pico W devices we need to initialise the driver etc
                return cyw43_arch_init();
        #endif
    } 
    
    else 
    {
        return PICO_ERROR_INVALID_DATA;
    }
}

void nine_without_library() 
{
    int rc = 0;

    if (LED_TYPE == 0) 
    {
        // A device like Pico that uses a GPIO for the LED so we can
        // use normal GPIO functionality to turn the led on and off
        gpio_init(LED_PIN);
        gpio_set_dir(LED_PIN, GPIO_OUT);
        rc = PICO_OK;

        #ifdef CYW43_WL_GPIO_LED_PIN
            } 
            
            else if (LED_TYPE == 1) 
            {
                // For Pico W devices we need to initialise the driver etc
                rc = cyw43_arch_init();
        #endif
    } 
    
    else 
    {
        rc = PICO_ERROR_INVALID_DATA;
    }

    hard_assert(rc == PICO_OK);

    while (true) 
    {
        if (LED_TYPE == 0) 
        {
            // Just set the GPIO on or off
            gpio_put(LED_PIN, true);
            #ifdef CYW43_WL_GPIO_LED_PIN
                } 
                
                else if (LED_TYPE == 1) 
                {
                    // Ask the wifi "driver" to set the GPIO on or off
                    cyw43_arch_gpio_put(LED_PIN, led_on);
            #endif
        }

        sleep_ms(LED_DELAY_MS);

        if (LED_TYPE == 0) 
        {
            // Just set the GPIO on or off
            gpio_put(LED_PIN, false);
            #ifdef CYW43_WL_GPIO_LED_PIN
                } 
                
                else if (LED_TYPE == 1) 
                {
                    // Ask the wifi "driver" to set the GPIO on or off
                    cyw43_arch_gpio_put(LED_PIN, led_on);
            #endif
        }

        sleep_ms(LED_DELAY_MS);
    }
}

void nine_with_library()
{
    while (true) 
    {
        led_set(true);
        sleep(LED_DELAY_MS);
        led_set(false);
        sleep(LED_DELAY_MS);
    }
}

#pragma endregion
