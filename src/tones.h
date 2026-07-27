//
// Created by Hans Gelke on 21.06.2026.
//

#ifndef SRC_TONES_H
#define SRC_TONES_H

#include <stdint.h>
#include <stdbool.h>
#include <zephyr/kernel.h>

/* ------------------------------------------------------------------ */
/* note_t — one segment of a ringing cadence                          */
/* ------------------------------------------------------------------ */
typedef struct {
    uint32_t duration;   /* segment duration in microseconds          */
    double   freq_1;     /* tone frequency 1 in Hz                    */
    double   freq_2;     /* tone frequency 2 in Hz                    */
    double   vol;        /* volume (0.0 – 1.0)                        */
    bool     tone_on;    /* true = tone generator active              */
    bool     ringer_on;  /* true = AC ringer active                   */
    bool     skip;       /* true = last segment, restart from index 0 */
} note_t;

/* ------------------------------------------------------------------ */
/* gb_ring — British ringing cadence                                  */
/*   400 ms on / 200 ms off / 400 ms on / 2000 ms off, then repeat   */
/* static: prevents multiple-definition errors when header is         */
/* included in more than one translation unit                         */
/* ------------------------------------------------------------------ */
#define CADENCE_MAX_SEGMENTS 8

static note_t gb_ring[CADENCE_MAX_SEGMENTS] = {
    /* duration   freq_1   freq_2   vol    tone_on  ringer_on  skip  */
    {  400000,    450.0,   400.0,   0.5,   true,    true,      false },
    {  400000,    450.0,   400.0,   0.5,   false,   false,     false },
    {  400000,    450.0,   400.0,   0.5,   true,    true,      false },
    { 2000000,    450.0,   400.0,   0.5,   false,   false,     true  },
    /* unused slots */
    { 0, 0.0, 0.0, 0.0, false, false, false },
    { 0, 0.0, 0.0, 0.0, false, false, false },
    { 0, 0.0, 0.0, 0.0, false, false, false },
    { 0, 0.0, 0.0, 0.0, false, false, false },
};

static note_t de_ring[CADENCE_MAX_SEGMENTS] = {
    /* duration   freq_1   freq_2   vol    tone_on  ringer_on  skip  */
    {  2000000,    450.0,   400.0,   0.5,   true,    true,      true },
    {  4000000,    450.0,   400.0,   0.5,   false,   false,     true },

};

/* ------------------------------------------------------------------ */
/* tone_instance_t — per-FSM tone task context                        */
/* Each FSM instance has its own tone_instance so each phone has an   */
/* independent ringing cadence.                                       */
/* ------------------------------------------------------------------ */
typedef struct {
    struct k_sem  tone_run;      /* semaphore controlling this task    */
    note_t       *melody;        /* pointer to the cadence array       */
    uint8_t       periph_addr;   /* SLIC I2C address for this line     */
} tone_instance_t;

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */
int  initCMX865(void);
void tone_task(void *inst, void *b, void *c);

#endif /* SRC_TONES_H */
