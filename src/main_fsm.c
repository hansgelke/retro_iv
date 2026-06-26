//
// Created by Hans Gelke on 21.06.2026.
//

#include "main_fsm.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/smf.h>
#include "main.h"
#include "tones.h"
#include "slic.h"
#include "gpio_i2c.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* ------------------------------------------------------------------ */
/* State table — shared, read-only, lives in flash                     */
/* ------------------------------------------------------------------ */

static const struct smf_state fsm_states[];
enum fsm_state_id { S0, S1 };

/* ------------------------------------------------------------------ */
/* State S0 — LED off                                                  */
/* ------------------------------------------------------------------ */

static void s0_entry(void *o)
{
        struct fsm_instance *inst = o;
        LOG_INF("FSM %p -> S0 (LED off)", (void *)inst);
        gpio_pin_set_dt(&inst->led, 0);
}

static enum smf_state_result s0_run(void *o)
{
        struct fsm_instance *inst = o;
        if (inst->events & EVENT_BTN_PRESS) {
                smf_set_state(SMF_CTX(inst), &fsm_states[S1]);
        }
        return SMF_EVENT_HANDLED;
}
/* ------------------------------------------------------------------ */
/* State S1 — LED on                                                   */
/* ------------------------------------------------------------------ */

static void s1_entry(void *o)
{
        struct fsm_instance *inst = o;
        LOG_INF("FSM %p -> S1 (LED on)", (void *)inst);
        gpio_pin_set_dt(&inst->led, 1);
}

static enum smf_state_result s1_run(void *o)
{
        struct fsm_instance *inst = o;
        if (inst->events & EVENT_BTN_PRESS) {
                smf_set_state(SMF_CTX(inst), &fsm_states[S0]);
        }
        return SMF_EVENT_HANDLED;
}



/* ------------------------------------------------------------------ */
/* Shared state table                                                  */
/* ------------------------------------------------------------------ */

static const struct smf_state fsm_states[] = {
        [S0] = SMF_CREATE_STATE(s0_entry, s0_run, NULL, NULL, NULL),
        [S1] = SMF_CREATE_STATE(s1_entry, s1_run, NULL, NULL, NULL),
    };

/* ------------------------------------------------------------------ */
/* ISR — one function serves all instances via CONTAINER_OF            */
/* ------------------------------------------------------------------ */

static void button_pressed(const struct device *dev,
                            struct gpio_callback *cb, uint32_t pins)
{
        struct fsm_instance *inst =
            CONTAINER_OF(cb, struct fsm_instance, button_cb);
        k_event_post(&inst->event, EVENT_BTN_PRESS);
}
/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

int fsm_init(struct fsm_instance *inst)
{
        int ret;
        /* ------------------------------------------------------------------ */
        /* Button */
        /* ------------------------------------------------------------------ */

        if (!gpio_is_ready_dt(&inst->button)) {
                LOG_ERR("FSM %p: button not ready", (void *)inst);
                return -ENODEV;
        }
        ret = gpio_pin_configure_dt(&inst->button, GPIO_INPUT);
        if (ret) return ret;

        ret = gpio_pin_interrupt_configure_dt(&inst->button,
                                              GPIO_INT_EDGE_TO_ACTIVE);
        if (ret) return ret;

        gpio_init_callback(&inst->button_cb, button_pressed,
                           BIT(inst->button.pin));
        gpio_add_callback(inst->button.port, &inst->button_cb);
        /* ------------------------------------------------------------------ */
        /* LED */
        /* ------------------------------------------------------------------ */

        if (!gpio_is_ready_dt(&inst->led)) {
                LOG_ERR("FSM %p: LED not ready", (void *)inst);
                return -ENODEV;
        }
        ret = gpio_pin_configure_dt(&inst->led, GPIO_OUTPUT_INACTIVE);
        if (ret) return ret;

        /* ------------------------------------------------------------------ */
        /* SMF */
        /* ------------------------------------------------------------------ */
        k_event_init(&inst->event);
        smf_set_initial(SMF_CTX(inst), &fsm_states[S0]);

        return 0;
}
int fsm_run(struct fsm_instance *inst)
{
    int ret;
    while (1) {
        inst->events = k_event_wait(&inst->event,
                                    EVENT_BTN_PRESS,
                                    true, K_FOREVER);
        ret = smf_run_state(SMF_CTX(inst));
        if (ret) {
            LOG_ERR("FSM %p terminated (%d)", (void *)inst, ret);
            return ret;
        }
    }
}
