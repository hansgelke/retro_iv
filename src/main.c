    #include <zephyr/logging/log.h>

    #include "gpio_i2c.h"
    #include "main.h"
#include "tones.h"
#include "slic.h"

    LOG_MODULE_REGISTER(i2c_example, LOG_LEVEL_DBG);

int main(void)
    {
init_gpios();
    init_slic();
        initCMX865();

        return 0; // Return from main.c
    }

