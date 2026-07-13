//
// Created by Hans Gelke on 21.06.2026.
//

#include "main_fsm.h"
#include <zephyr/kernel.h>
#include <string.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/smf.h>
#include <zephyr/sys/atomic.h>
#include "main.h"
#include "tones.h"
#include "slic.h"
#include "gpio_i2c.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* ------------------------------------------------------------------ */
/* int_call — defined here, declared extern in main_fsm.h             */
/* ------------------------------------------------------------------ */
atomic_t int_call = ATOMIC_INIT(0);
atomic_t engaged  = ATOMIC_INIT(0);

/* ------------------------------------------------------------------ */
/* Instance registry — maps int_call bit index → fsm_instance pointer */
/* Populated via fsm_register() called from main() before threads run */
/* ------------------------------------------------------------------ */
#define FSM_MAX_INSTANCES  8
static struct fsm_instance *fsm_registry[FSM_MAX_INSTANCES];

void fsm_register(struct fsm_instance *inst)
{
    if (inst->int_call_bit < FSM_MAX_INSTANCES) {
        fsm_registry[inst->int_call_bit] = inst;
    }
}


/* ------------------------------------------------------------------ */
/* State table — shared, read-only, lives in flash                     */
/* ------------------------------------------------------------------ */

static const struct smf_state fsm_states[];
enum fsm_state_id { S0, S1, S2, S3, S4, S5, S6, S7, S8 };

/* ------------------------------------------------------------------ */
/* pulse_timer expiry callback                                         */
/* Runs in ISR context — posts EVENT_PULSE_EXPIRED                     */
/* ------------------------------------------------------------------ */

static void pulse_timer_expired(struct k_timer *timer)
{
    struct fsm_instance *inst =
        CONTAINER_OF(timer, struct fsm_instance, pulse_timer);

    inst->pulse_expired = true;
    k_event_post(&inst->event, EVENT_PULSE_EXPIRED);
}

/* ------------------------------------------------------------------ */
/* hang_up_timer expiry callback                                       */
/* Runs in ISR context — posts EVENT_HANGUP_EXPIRED                    */
/* ------------------------------------------------------------------ */

static void hang_up_timer_expired(struct k_timer *timer)
{
    struct fsm_instance *inst =
        CONTAINER_OF(timer, struct fsm_instance, hang_up_timer);

    k_event_post(&inst->event, EVENT_HANGUP_EXPIRED);
}

/* ------------------------------------------------------------------ */
/* Helper — print full pulse_queue array contents to console          */
/* Called after every new entry is written                            */
/* ------------------------------------------------------------------ */

static void log_pulse_queue(struct fsm_instance *inst)
{
    char buf[PULSE_QUEUE_SIZE * 5 + 4];  /* enough for "NNN, " * N   */
    int  pos = 0;

    pos += snprintk(buf + pos, sizeof(buf) - pos, "[ ");
    for (uint8_t i = 0; i < inst->pulse_queue_idx; i++) {
        if (i < inst->pulse_queue_idx - 1) {
            pos += snprintk(buf + pos, sizeof(buf) - pos,
                            "%u, ", inst->pulse_queue[i]);
        } else {
            pos += snprintk(buf + pos, sizeof(buf) - pos,
                            "%u", inst->pulse_queue[i]);
        }
    }
    snprintk(buf + pos, sizeof(buf) - pos, " ]");

    LOG_INF("FSM %p pulse_queue(%u): %s",
            (void *)inst, inst->pulse_queue_idx, buf);
}

/* ------------------------------------------------------------------ */
/* Helper — update int_call after each new pulse_queue entry          */
/*                                                                     */
/* Condition: pulse_queue[0] == 1 AND pulse_queue_idx >= 2            */
/* Encoding:  pulse_queue[1] value N → BIT(N-1) ORed into int_call   */
/* Bits are only SET — clearing is handled by a later mechanism       */
/* ------------------------------------------------------------------ */

static int update_int_call(struct fsm_instance *inst)
{
    if (inst->pulse_queue[0] != 1) {
        return -1;
    }

    if (inst->pulse_queue_idx < 2) {
        return -1;
    }

    uint32_t digit = inst->pulse_queue[1];

    if (digit == 0 || digit > 10) {
        LOG_WRN("FSM %p: pulse_queue[1]=%u out of range, int_call not updated",
                (void *)inst, digit);
        return -1;
    }

    /* One-hot: value N → BIT(N-1), OR into shared register          */
    uint8_t target_bit = (uint8_t)(digit - 1);
    atomic_or(&int_call, (atomic_val_t)BIT(target_bit));

    long val = (long)atomic_get(&int_call);
    LOG_INF("FSM %p: int_call updated  pulse_queue[1]=%u  int_call=0x%03lx",
            (void *)inst, digit, val);

    /* Wake the FSM that owns this bit so s0_run fires immediately    */
    if (target_bit < FSM_MAX_INSTANCES && fsm_registry[target_bit] != NULL) {
        fsm_registry[target_bit]->caller = inst;
        k_event_post(&fsm_registry[target_bit]->event, EVENT_INT_CALL);
        LOG_INF("FSM %p: posted EVENT_INT_CALL to FSM %p",
                (void *)inst, (void *)fsm_registry[target_bit]);
    }

    return (int)target_bit;   /* caller uses this to know which bit was set */
}

/* ------------------------------------------------------------------ */
/* State S0 — idle, wait for BTN_ACTIVE                               */
/* ------------------------------------------------------------------ */

static void s0_entry(void *o)
{
    struct fsm_instance *inst = o;
    LOG_DBG("FSM %p -> S0 (idle)", (void *)inst);

    /* Ensure both timers are stopped when returning to idle          */
    k_timer_stop(&inst->pulse_timer);
    k_timer_stop(&inst->hang_up_timer);
    inst->pulse_expired = false;

    /* Clear pulse queue — fresh start                                */
    memset(inst->pulse_queue, 0, sizeof(inst->pulse_queue));
    inst->pulse_queue_idx = 0;

    /* This FSM is now idle — clear its engaged bit                   */
    atomic_clear_bit(&engaged, inst->engaged_bit);
    long val = (long)atomic_get(&engaged);
    LOG_INF("FSM %p -> S0  engaged=0x%02lx", (void *)inst, val);
}

static enum smf_state_result s0_run(void *o)
{
    struct fsm_instance *inst = o;

    if (inst->events & EVENT_BTN_ACTIVE) {
        /* Off-hook — mark as engaged, go to S1                       */
        atomic_set_bit(&engaged, inst->engaged_bit);
        long val = (long)atomic_get(&engaged);
        LOG_INF("FSM %p leaving S0 (BTN) -> S1  engaged=0x%02lx",
                (void *)inst, val);
        smf_set_state(SMF_CTX(inst), &fsm_states[S1]);
        return SMF_EVENT_HANDLED;
    }

    if (inst->events & EVENT_INT_CALL) {
        /* int_call woke us — verify our bit is actually set          */
        if (atomic_test_bit(&int_call, inst->int_call_bit)) {
            atomic_set_bit(&engaged, inst->engaged_bit);
            long val = (long)atomic_get(&engaged);
            LOG_INF("FSM %p leaving S0 (int_call) -> S5  engaged=0x%02lx",
                    (void *)inst, val);
            smf_set_state(SMF_CTX(inst), &fsm_states[S5]);
        }
    }

    return SMF_EVENT_HANDLED;
}

/* ------------------------------------------------------------------ */
/* State S1 — loop closed, wait for BTN_INACTIVE                      */
/* ------------------------------------------------------------------ */

static void s1_entry(void *o)
{
    struct fsm_instance *inst = o;
    LOG_DBG("FSM %p -> S1 (digits so far=%u)",
            (void *)inst, inst->pulse_queue_idx);

    /* Stop pulse timer and clear flag when entering S1               */
    k_timer_stop(&inst->pulse_timer);
    inst->pulse_expired = false;

    /* Reset pulse counter for the new digit in this sequence         */
    inst->pulse_count     = 0;
}

static enum smf_state_result s1_run(void *o)
{
    struct fsm_instance *inst = o;
    if (inst->events & EVENT_BTN_INACTIVE) {
        smf_set_state(SMF_CTX(inst), &fsm_states[S2]);
    }
    return SMF_EVENT_HANDLED;
}

/* ------------------------------------------------------------------ */
/* State S2 — loop open, hang_up_timer running                        */
/*                                                                     */
/* Entered from S1 (first open) or S3 (inter-pulse open).             */
/* hang_up_timer starts on every entry.                                */
/*   BTN_ACTIVE before 80 ms  → S3 (pulse), stop hang_up_timer        */
/*   hang_up_timer expires     → S0 (hang up)                         */
/* ------------------------------------------------------------------ */

static void s2_entry(void *o)
{
    struct fsm_instance *inst = o;
    LOG_DBG("FSM %p -> S2 (hang_up_timer started)", (void *)inst);

    inst->pulse_expired = false;

    /* Start hang_up_timer — one-shot, 80 ms                          */
    k_timer_start(&inst->hang_up_timer,
                  K_MSEC(HANGUP_TIMER_MS),
                  K_NO_WAIT);
}

static enum smf_state_result s2_run(void *o)
{
    struct fsm_instance *inst = o;

    /* Hang-up takes priority over a button event                     */
    if (inst->events & EVENT_HANGUP_EXPIRED) {
        LOG_DBG("FSM %p: hang_up_timer expired -> S0 (hang up)", (void *)inst);
        smf_set_state(SMF_CTX(inst), &fsm_states[S0]);
        return SMF_EVENT_HANDLED;
    }

    if (inst->events & EVENT_BTN_ACTIVE) {
        /* Loop closed within 80 ms — valid pulse start               */
        k_timer_stop(&inst->hang_up_timer);
        smf_set_state(SMF_CTX(inst), &fsm_states[S3]);
    }

    return SMF_EVENT_HANDLED;
}

static void s2_exit(void *o)
{
    struct fsm_instance *inst = o;
    /* Safety stop — timer is already stopped on BTN_ACTIVE path,     */
    /* but this ensures it is always stopped when leaving S2          */
    k_timer_stop(&inst->hang_up_timer);
}

/* ------------------------------------------------------------------ */
/* State S3 — pulse active, pulse_timer counting down                  */
/*   BTN_INACTIVE + pulse not expired → S2                            */
/*   BTN_INACTIVE + pulse expired     → S1                            */
/*   EVENT_PULSE_EXPIRED              → S1                            */
/* ------------------------------------------------------------------ */

static void s3_entry(void *o)
{
    struct fsm_instance *inst = o;
    inst->pulse_count++;
    LOG_DBG("FSM %p -> S3 (pulse %u, timer running)", (void *)inst, inst->pulse_count);

    /* Start one-shot pulse countdown                                  */
    k_timer_start(&inst->pulse_timer,
                  K_MSEC(PULSE_TIMER_MS),
                  K_NO_WAIT);
}

static enum smf_state_result s3_run(void *o)
{
    struct fsm_instance *inst = o;

    /* Pulse timer expiry takes priority                               */
    if (inst->events & EVENT_PULSE_EXPIRED) {
        /* Append pulse_count to queue if space remains               */
        if (inst->pulse_queue_idx < PULSE_QUEUE_SIZE) {
            inst->pulse_queue[inst->pulse_queue_idx] = inst->pulse_count;
            inst->pulse_queue_idx++;
        } else {
            LOG_WRN("FSM %p: pulse_queue full (%u entries), entry dropped",
                    (void *)inst, PULSE_QUEUE_SIZE);
        }
        log_pulse_queue(inst);
        int called_bit = update_int_call(inst);

        /* If an int_call was placed, go to S7 to wait for pick-up   */
        if (called_bit >= 0) {
            LOG_INF("FSM %p: int_call set (bit %d) -> S7",
                    (void *)inst, called_bit);
            smf_set_state(SMF_CTX(inst), &fsm_states[S7]);
        } else {
            smf_set_state(SMF_CTX(inst), &fsm_states[S1]);
        }
        return SMF_EVENT_HANDLED;
    }

    if (inst->events & EVENT_BTN_INACTIVE) {
        if (!inst->pulse_expired) {
            LOG_DBG("FSM %p: pulse end, timer active -> S2", (void *)inst);
            smf_set_state(SMF_CTX(inst), &fsm_states[S2]);
        } else {
            LOG_DBG("FSM %p: pulse end, timer already expired -> S1", (void *)inst);
            smf_set_state(SMF_CTX(inst), &fsm_states[S1]);
        }
    }

    return SMF_EVENT_HANDLED;
}

static void s3_exit(void *o)
{
    struct fsm_instance *inst = o;
    /* Stop pulse timer whenever leaving S3                           */
    k_timer_stop(&inst->pulse_timer);
}

/* ------------------------------------------------------------------ */
/* S4–S8 — placeholders, expand as needed                             */
/* ------------------------------------------------------------------ */

static void s4_entry(void *o) { struct fsm_instance *inst = o; LOG_DBG("FSM %p -> S4", (void *)inst); }
static enum smf_state_result s4_run(void *o)
{
    struct fsm_instance *inst = o;
    if (inst->events & EVENT_BTN_ACTIVE) {
        smf_set_state(SMF_CTX(inst), &fsm_states[S5]);
    }
    return SMF_EVENT_HANDLED;
}

static void s5_entry(void *o)
{
    struct fsm_instance *inst = o;
    LOG_INF("FSM %p -> S5 (ringing, waiting for BTN_ACTIVE)", (void *)inst);
}

static enum smf_state_result s5_run(void *o)
{
    struct fsm_instance *inst = o;

    if (inst->events & EVENT_BTN_ACTIVE) {
        /* Recipient picked up — clear the int_call bit               */
        atomic_clear_bit(&int_call, inst->int_call_bit);
        long val = (long)atomic_get(&int_call);
        LOG_INF("FSM %p: int_call bit %u cleared  int_call=0x%03lx",
                (void *)inst, inst->int_call_bit, val);

        /* Notify the caller that int_call has been cleared           */
        if (inst->caller != NULL) {
            k_event_post(&inst->caller->event, EVENT_INT_CALL_CLEARED);
            LOG_INF("FSM %p: notified caller FSM %p",
                    (void *)inst, (void *)inst->caller);
            inst->caller = NULL;
        }

        smf_set_state(SMF_CTX(inst), &fsm_states[S6]);
        return SMF_EVENT_HANDLED;
    }

    /* Caller hung up — int_call bit was cleared by S7, go to S0     */
    if (inst->events & EVENT_INT_CALL) {
        if (!atomic_test_bit(&int_call, inst->int_call_bit)) {
            LOG_INF("FSM %p: caller hung up, int_call cleared -> S0",
                    (void *)inst);
            inst->caller = NULL;
            smf_set_state(SMF_CTX(inst), &fsm_states[S0]);
        }
    }

    return SMF_EVENT_HANDLED;
}

/* ------------------------------------------------------------------ */
/* State S6 — button pressed, wait for BTN_INACTIVE then go to S2    */
/* ------------------------------------------------------------------ */

static void s6_entry(void *o)
{
    struct fsm_instance *inst = o;
    LOG_INF("FSM %p -> S6 (waiting for BTN_INACTIVE)", (void *)inst);
}

static enum smf_state_result s6_run(void *o)
{
    struct fsm_instance *inst = o;
    /* Wait for button release, then enter pulse-counting path via S2 */
    if (inst->events & EVENT_BTN_INACTIVE) {
        smf_set_state(SMF_CTX(inst), &fsm_states[S2]);
    }
    return SMF_EVENT_HANDLED;
}

/* ------------------------------------------------------------------ */
/* State S7 — caller waiting for recipient to pick up                 */
/* Entered from S0 when BTN_ACTIVE fires and int_call_bit is set.     */
/* Waits until S5 of the recipient clears the int_call bit, which     */
/* posts EVENT_INT_CALL_CLEARED to this instance. Then goes to S1.    */
/* ------------------------------------------------------------------ */

static void s7_entry(void *o)
{
    struct fsm_instance *inst = o;
    LOG_INF("FSM %p -> S7 (caller waiting for recipient)", (void *)inst);
}

static enum smf_state_result s7_run(void *o)
{
    struct fsm_instance *inst = o;

    if (inst->events & EVENT_INT_CALL_CLEARED) {
        /* Recipient has picked up — int_call bit was cleared by S5   */
        LOG_INF("FSM %p: int_call cleared -> S1", (void *)inst);
        smf_set_state(SMF_CTX(inst), &fsm_states[S1]);
        return SMF_EVENT_HANDLED;
    }

    if (inst->events & EVENT_BTN_INACTIVE) {
        /* Caller hung up while waiting — clear int_call bit and      */
        /* notify recipient in S5 so it can return to S0             */
        int target_bit = (int)(inst->pulse_queue[1] - 1);
        atomic_clear_bit(&int_call, target_bit);
        long val = (long)atomic_get(&int_call);
        LOG_INF("FSM %p: hung up in S7, int_call=0x%03lx -> S0",
                (void *)inst, val);

        if (target_bit >= 0 && target_bit < FSM_MAX_INSTANCES &&
            fsm_registry[target_bit] != NULL) {
            k_event_post(&fsm_registry[target_bit]->event, EVENT_INT_CALL);
        }

        smf_set_state(SMF_CTX(inst), &fsm_states[S0]);
    }

    return SMF_EVENT_HANDLED;
}

static void s8_entry(void *o) { struct fsm_instance *inst = o; LOG_DBG("FSM %p -> S8", (void *)inst); }
static enum smf_state_result s8_run(void *o)
{
    struct fsm_instance *inst = o;
    if (inst->events & EVENT_BTN_INACTIVE) {
        smf_set_state(SMF_CTX(inst), &fsm_states[S0]);
    }
    return SMF_EVENT_HANDLED;
}

/* ------------------------------------------------------------------ */
/* Shared state table                                                  */
/* ------------------------------------------------------------------ */

static const struct smf_state fsm_states[] = {
    [S0] = SMF_CREATE_STATE(s0_entry, s0_run, NULL,     NULL, NULL),
    [S1] = SMF_CREATE_STATE(s1_entry, s1_run, NULL,     NULL, NULL),
    [S2] = SMF_CREATE_STATE(s2_entry, s2_run, s2_exit,  NULL, NULL),
    [S3] = SMF_CREATE_STATE(s3_entry, s3_run, s3_exit,  NULL, NULL),
    [S4] = SMF_CREATE_STATE(s4_entry, s4_run, NULL,     NULL, NULL),
    [S5] = SMF_CREATE_STATE(s5_entry, s5_run, NULL,     NULL, NULL),
    [S6] = SMF_CREATE_STATE(s6_entry, s6_run, NULL,     NULL, NULL),
    [S7] = SMF_CREATE_STATE(s7_entry, s7_run, NULL,     NULL, NULL),
    [S8] = SMF_CREATE_STATE(s8_entry, s8_run, NULL,     NULL, NULL),
};

/* ------------------------------------------------------------------ */
/* ISR — one function serves all instances via CONTAINER_OF            */
/* ------------------------------------------------------------------ */

static void button_pressed(const struct device *dev,
                            struct gpio_callback *cb,
                            uint32_t pins)
{
    struct fsm_instance *inst =
        CONTAINER_OF(cb, struct fsm_instance, button_cb);

    int level = gpio_pin_get_dt(&inst->button);

    if (level == 1) {
        k_event_post(&inst->event, EVENT_BTN_ACTIVE);
    } else {
        k_event_post(&inst->event, EVENT_BTN_INACTIVE);
    }
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

int fsm_init(struct fsm_instance *inst)
{
    int ret;

    /* Button */
    if (!gpio_is_ready_dt(&inst->button)) {
        LOG_ERR("FSM %p: button not ready", (void *)inst);
        return -ENODEV;
    }
    ret = gpio_pin_configure_dt(&inst->button, GPIO_INPUT);
    if (ret) return ret;

    ret = gpio_pin_interrupt_configure_dt(&inst->button,
                                          GPIO_INT_EDGE_BOTH);
    if (ret) return ret;

    gpio_init_callback(&inst->button_cb, button_pressed,
                       BIT(inst->button.pin));
    gpio_add_callback(inst->button.port, &inst->button_cb);

    /* pulse_timer */
    k_timer_init(&inst->pulse_timer, pulse_timer_expired, NULL);
    inst->pulse_expired = false;

    /* hang_up_timer */
    k_timer_init(&inst->hang_up_timer, hang_up_timer_expired, NULL);

    /* SMF */
    k_event_init(&inst->event);
    smf_set_initial(SMF_CTX(inst), &fsm_states[S0]);

    return 0;
}

int fsm_run(struct fsm_instance *inst)
{
    int ret;
    while (1) {
        inst->events = k_event_wait(&inst->event,
                                    EVENT_ALL,
                                    true, K_FOREVER);
        ret = smf_run_state(SMF_CTX(inst));
        if (ret) {
            LOG_ERR("FSM %p terminated (%d)", (void *)inst, ret);
            return ret;
        }
    }
}
