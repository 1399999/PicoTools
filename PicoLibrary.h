typedef char * string;

bool is_stdio_init = false;
bool is_led_init = false;

#ifdef CYW43_WL_GPIO_LED_PIN
    bool is_pico_w = true;
#elif defined(PICO_DEFAULT_LED_PIN)
    bool is_pico_w = false;
#endif

int main();

void write(string str);
void write_line(string str);
void sleep(uint32_t milliseconds);
int led_init();
void set_led(bool led_on);
