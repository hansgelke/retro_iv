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
/* FSM instances — initialised from devicetree aliases                 */
/* ------------------------------------------------------------------ */

static struct fsm_instance fsm1 = {
    .button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios),
    .led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios),
};

static struct fsm_instance fsm2 = {
    .button = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios),
    .led = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios),
};

/* ------------------------------------------------------------------ */
/* One thread per FSM instance                                         */
/* ------------------------------------------------------------------ */

#define FSM_STACK_SIZE  1024
#define FSM_PRIORITY    5

/* Semaphore: threads wait here until main() has called fsm_init */
static K_SEM_DEFINE(fsm_ready, 0, 2);

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

K_THREAD_DEFINE(fsm1_tid, FSM_STACK_SIZE, fsm1_thread,
                NULL, NULL, NULL, FSM_PRIORITY, 0, 0);
K_THREAD_DEFINE(fsm2_tid, FSM_STACK_SIZE, fsm2_thread,
                NULL, NULL, NULL, FSM_PRIORITY, 0, 0);

int main(void)
{
    int ret;

    init_gpios();
    init_slic();
    initCMX865();

    ret = fsm_init(&fsm1);
    if (ret) LOG_ERR("fsm1 init failed: %d", ret);

    ret = fsm_init(&fsm2);
    if (ret) LOG_ERR("fsm2 init failed: %d", ret);

    /* Release both threads now that init is complete */
    k_sem_give(&fsm_ready);
    k_sem_give(&fsm_ready);

    return 0;
}

