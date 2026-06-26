//
// Created by Hans Gelke on 21.06.2026.
//
#ifndef MAIN_FSM_H
#define MAIN_FSM_H

#include <zephyr/drivers/gpio.h>
#include <zephyr/smf.h>

#define EVENT_BTN_PRESS  BIT(0)

struct fsm_instance {
    struct smf_ctx       ctx;         /* must be first */

    struct k_event       event;
    int32_t              events;

    struct gpio_dt_spec  button;
    struct gpio_callback button_cb;

    struct gpio_dt_spec  led;
};

int fsm_init(struct fsm_instance *inst);
int fsm_run(struct fsm_instance *inst);

#endif /* MAIN_FSM_H */