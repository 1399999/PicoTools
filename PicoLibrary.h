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
