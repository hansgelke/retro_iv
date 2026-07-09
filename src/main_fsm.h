//
// Created by Hans Gelke on 21.06.2026.
//
#ifndef MAIN_FSM_H
#define MAIN_FSM_H

#include <zephyr/drivers/gpio.h>
#include <zephyr/smf.h>
#include <zephyr/kernel.h>

/* ------------------------------------------------------------------ */
/* Events                                                              */
/* ------------------------------------------------------------------ */
#define EVENT_BTN_ACTIVE        BIT(0)  /* button pressed  (active edge)  */
#define EVENT_BTN_INACTIVE      BIT(1)  /* button released (inactive edge) */
#define EVENT_PULSE_EXPIRED     BIT(2)  /* pulse_timer reached zero        */
#define EVENT_HANGUP_EXPIRED    BIT(3)  /* hang_up_timer reached zero      */

#define EVENT_ALL  (EVENT_BTN_ACTIVE    | \
                    EVENT_BTN_INACTIVE  | \
                    EVENT_PULSE_EXPIRED | \
                    EVENT_HANGUP_EXPIRED)

/* ------------------------------------------------------------------ */
/* Timer values                                                        */
/* ------------------------------------------------------------------ */
#define PULSE_TIMER_MS    500   /* inter-pulse timeout                  */
#define HANGUP_TIMER_MS    80   /* open loop > 80 ms = hang up          */

/* ------------------------------------------------------------------ */
/* Per-instance context                                                */
/* ------------------------------------------------------------------ */
struct fsm_instance {
    struct smf_ctx       ctx;             /* must be first               */

    struct k_event       event;
    int32_t              events;

    struct gpio_dt_spec  button;
    struct gpio_callback button_cb;

    struct gpio_dt_spec  led;

    struct k_timer       pulse_timer;     /* inter-pulse countdown       */
    bool                 pulse_expired;   /* set by pulse_timer callback */

    struct k_timer       hang_up_timer;   /* open-loop detection         */

    uint32_t             pulse_count;                      /* pulses in current S2/S3 series, reset in S1     */

    /* pulse_queue — array of completed series counts                  */
    /* One entry per S2/S3 burst, separated by S1 transitions          */
    /* Cleared when FSM enters S0 or S1 (new call / new digit group)   */
#define PULSE_QUEUE_SIZE  16
    uint32_t             pulse_queue[PULSE_QUEUE_SIZE];   /* stored series counts                            */
    uint8_t              pulse_queue_idx;                 /* next free slot, also == number of entries so far */
};

int fsm_init(struct fsm_instance *inst);
int fsm_run(struct fsm_instance *inst);

#endif /* MAIN_FSM_H */
