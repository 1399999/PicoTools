#include <stdio.h>
#include "pico/stdlib.h"
#include "PicoLibrary.h"

// For testing purposes.
int main()
{
    for (int i = 0; true; i++)
    {
        write_line("Hello, world!");

        sleep(1000);
    }
}

void write(string str)
{
    if (!is_stdio_init)
    {
        stdio_init_all();
        is_stdio_init = 1;
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
