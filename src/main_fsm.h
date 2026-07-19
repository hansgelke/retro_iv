//
// Created by Hans Gelke on 21.06.2026.
//
#ifndef MAIN_FSM_H
#define MAIN_FSM_H

#include <zephyr/drivers/gpio.h>
#include <zephyr/smf.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>

/* ------------------------------------------------------------------ */
/* int_call — shared one-hot register (common to fsm1 and fsm2)       */
/*                                                                     */
/* Reflects pulse_queue[1] as a one-hot bit, but only when            */
/* pulse_queue[0] == 1.                                               */
/*                                                                     */
/* Encoding: value N → BIT(N-1)                                       */
/*   1 → 0b00000001   2 → 0b00000010   3 → 0b00000100                */
/*   4 → 0b00001000   5 → 0b00010000   6 → 0b00100000                */
/*   7 → 0b01000000   8 → 0b10000000   9 → 0b100000000               */
/*                                                                     */
/* fsm1 and fsm2 OR their bits in simultaneously.                     */
/* Bits are only SET here — clearing is described in later            */
/* instructions.                                                       */
/* ------------------------------------------------------------------ */
extern atomic_t int_call;

/* ------------------------------------------------------------------ */
/* engaged — tracks which FSM instances are active (not in S0)        */
/*   bit 0 : fsm1 not in S0                                           */
/*   bit 1 : fsm2 not in S0                                           */
/*   0b00 = both idle   0b01 = fsm1 active   0b10 = fsm2 active      */
/*   0b11 = both active                                               */
/* Bit is SET when FSM leaves S0, CLEARED when FSM enters S0.         */
/* ------------------------------------------------------------------ */
extern atomic_t engaged;

#define ENGAGED_FSM1_BIT  0
#define ENGAGED_FSM2_BIT  1
#define ENGAGED_FSM3_BIT  2

/* Bit each FSM owns in int_call                                       */
#define INT_CALL_FSM1_BIT 0
#define INT_CALL_FSM2_BIT 1
#define INT_CALL_FSM3_BIT 2

/* ------------------------------------------------------------------ */
/* Events                                                              */
/* ------------------------------------------------------------------ */
#define EVENT_BTN_ACTIVE        BIT(0)  /* button pressed  (active edge)  */
#define EVENT_BTN_INACTIVE      BIT(1)  /* button released (inactive edge) */
#define EVENT_PULSE_EXPIRED     BIT(2)  /* pulse_timer reached zero        */
#define EVENT_HANGUP_EXPIRED    BIT(3)  /* hang_up_timer reached zero      */
#define EVENT_INT_CALL          BIT(4)  /* int_call bit set for this FSM   */
#define EVENT_INT_CALL_CLEARED  BIT(5)  /* int_call bit cleared — S7 → S1 */

#define EVENT_ALL  (EVENT_BTN_ACTIVE       | \
                    EVENT_BTN_INACTIVE     | \
                    EVENT_PULSE_EXPIRED    | \
                    EVENT_HANGUP_EXPIRED   | \
                    EVENT_INT_CALL         | \
                    EVENT_INT_CALL_CLEARED)

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

    struct k_work         status_work;

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

    /* Which bit of engaged this instance owns (ENGAGED_FSM1_BIT or ENGAGED_FSM2_BIT) */
    uint8_t              engaged_bit;

    /* Which bit of int_call this instance monitors (INT_CALL_FSM1_BIT or INT_CALL_FSM2_BIT) */
    uint8_t              int_call_bit;
    /* Which bit of the shared status register (GPIO_B) this instance watches */
    uint8_t              status_bit;      /* <-- ADD THIS LINE */
    /* Pointer to the FSM that initiated the current int_call to us  */
    /* Set by update_int_call, used by S5 to notify the caller       */
    struct fsm_instance  *caller;
};

int fsm_init(struct fsm_instance *inst);
int fsm_run(struct fsm_instance *inst);
/* Register an instance so update_int_call can wake it via EVENT_INT_CALL */
void fsm_register(struct fsm_instance *inst);

#endif /* MAIN_FSM_H */
