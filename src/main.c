#include "main.h"
#include "main_fsm.h"
#include "tones.h"
#include "slic.h"
#include "gpio_i2c.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/smf.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(main, LOG_LEVEL_DBG);

/* ------------------------------------------------------------------ */
/* FSM instances                                                       */
/* ------------------------------------------------------------------ */

static struct fsm_instance fsm1 = {
    .button       = GPIO_DT_SPEC_GET(DT_ALIAS(sw0),  gpios),
    .engaged_bit  = ENGAGED_FSM1_BIT,
    .int_call_bit = INT_CALL_FSM1_BIT,
};

static struct fsm_instance fsm2 = {
    .button       = GPIO_DT_SPEC_GET(DT_ALIAS(sw1),  gpios),
    .engaged_bit  = ENGAGED_FSM2_BIT,
    .int_call_bit = INT_CALL_FSM2_BIT,
};

/* NOTE: sw2 must be defined in the devicetree overlay               */
static struct fsm_instance fsm3 = {
    .button       = GPIO_DT_SPEC_GET(DT_ALIAS(sw2),  gpios),
    .engaged_bit  = ENGAGED_FSM3_BIT,
    .int_call_bit = INT_CALL_FSM3_BIT,
};

/* ------------------------------------------------------------------ */
/* Threads                                                             */
/* ------------------------------------------------------------------ */

#define FSM_STACK_SIZE  1024
#define FSM_PRIORITY    5

static K_SEM_DEFINE(fsm_ready, 0, 3);

static void fsm1_thread(void *a, void *b, void *c)
{
    k_sem_take(&fsm_ready, K_FOREVER);
    fsm_run(&fsm1);
}

static void fsm2_thread(void *a, void *b, void *c)
{
    k_sem_take(&fsm_ready, K_FOREVER);
    fsm_run(&fsm2);
}

static void fsm3_thread(void *a, void *b, void *c)
{
    k_sem_take(&fsm_ready, K_FOREVER);
    fsm_run(&fsm3);
}

K_THREAD_DEFINE(fsm1_tid, FSM_STACK_SIZE, fsm1_thread,
                NULL, NULL, NULL, FSM_PRIORITY, 0, 0);
K_THREAD_DEFINE(fsm2_tid, FSM_STACK_SIZE, fsm2_thread,
                NULL, NULL, NULL, FSM_PRIORITY, 0, 0);
K_THREAD_DEFINE(fsm3_tid, FSM_STACK_SIZE, fsm3_thread,
                NULL, NULL, NULL, FSM_PRIORITY, 0, 0);

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(void)
{
    int ret;

    init_gpios();
    init_slic();
    // set spio cs pin on address decoder for spi
    set_slic(i2c_bus0,
             PERIPH_ADDR_24,
             GPIO_B,
             0x01,//Set Address decoder to 1 for CMX no 1
             MASK_HIGH);
    initCMX865();
    initCMX865();
    initCMX865();

    ret = fsm_init(&fsm1);
    if (ret) LOG_ERR("fsm1 init failed: %d", ret);

    ret = fsm_init(&fsm2);
    if (ret) LOG_ERR("fsm2 init failed: %d", ret);

    ret = fsm_init(&fsm3);
    if (ret) LOG_ERR("fsm3 init failed: %d", ret);

    /* Register instances so update_int_call can wake them            */
    fsm_register(&fsm1);
    fsm_register(&fsm2);
    fsm_register(&fsm3);

    k_sem_give(&fsm_ready);
    k_sem_give(&fsm_ready);
    k_sem_give(&fsm_ready);

    return 0;
}
