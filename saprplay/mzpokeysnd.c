/*
 * mzpokeysnd.c - POKEY sound chip emulation, v1.6
 *
 * Copyright (C) 2002-218 Michael Borisov, Krzystof Nikiel, Perry McFarlane,
 *                        Petr Stehlík, Piotr Fusik
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <math.h>
#include "mzpokeysnd.h"
#include "remez.h"

int POKEYSND_playback_freq = 44100;

#define Atari800_TV_PAL 312
#define Atari800_TV_NTSC 262
int Atari800_tv_mode = Atari800_TV_PAL;

#define Atari800_FPS_PAL 49.8607597
#define Atari800_FPS_NTSC 59.9227434

static int POKEYSND_volume = 0x100;

static const double pokeymix[61] = { /* Nonlinear POKEY mixing */
    0.000000, 5.169146, 10.157015, 15.166247,
    20.073793, 24.927443, 29.728237, 34.495266,
    39.181262, 43.839780, 48.429508, 52.932530,
    57.327319, 61.586304, 65.673220, 69.547672,
    73.207846, 76.594474, 79.739231, 82.631161,
    85.300361, 87.750638, 90.020656, 92.108334,
    94.051256, 95.848478, 97.521287, 99.080719,
    100.540674, 101.902750, 103.185339, 104.375596,
    105.491149, 106.523735, 107.473511, 108.361458,
    109.185669, 109.962251, 110.685574, 111.367150,
    112.008476, 112.612760, 113.185603, 113.722735,
    114.227904, 114.712206, 115.171007, 115.605730,
    116.024396, 116.416097, 116.803169, 117.155108,
    117.532921, 117.835494, 118.196180, 118.502785,
    118.825177, 119.138170, 119.421378, 119.734493,
    120.000000
};

#define SND_FILTER_SIZE  2048
#define NPOKEYS 2

/* M_PI was not defined in MSVC headers */
#ifndef M_PI
#    define M_PI 3.141592653589793
#endif

static int num_cur_pokeys = 0;

/* Filter */
static int pokey_frq;           /* Hz - for easier resampling */
static int filter_size;
static double filter_data[SND_FILTER_SIZE];
static int audible_frq;

static const int pokey_frq_ideal = 1789790;     /* Hz - True */

/* Poly tables */
static int poly4tbl[15];
static int poly5tbl[31];
static unsigned char poly17tbl[131071];
static int poly9tbl[511];

struct stPokeyState;

typedef int (*readout_t)(struct stPokeyState * ps);
typedef void (*event_t)(struct stPokeyState * ps, int p5v, int p4v, int p917v);

/* Change queue event value type */
typedef double qev_t;

static double ticks_per_sample;
static double samp_pos;

/* State variables for single Pokey Chip */
typedef struct stPokeyState {
    int curtick;
    /* Poly positions */
    int poly4pos;
    int poly5pos;
    int poly17pos;
    int poly9pos;

    /* Change queue */
    qev_t ovola;
    int qet[1322];              /* maximal length of filter */
    qev_t qev[1322];
    int qebeg;
    int qeend;

    /* Main divider (64khz/15khz) */
    int mdivk;                  /* 28 for 64khz, 114 for 15khz */

    /* Main switches */
    int selpoly9;
    int c0_hf;
    int c1_f0;
    int c2_hf;
    int c3_f2;

    /* SKCTL for two-tone mode */
    int skctl;

    /* Main output state */
    qev_t outvol_all;
    int forcero;                /* Force readout */

    /* channel 0 state */

    readout_t readout_0;
    event_t event_0;

    int c0divpos;
    int c0divstart;             /* AUDF0 recalculated */
    int c0divstart_p;           /* start value when c1_f0 */
    int c0diva;                 /* AUDF0 register */

    int c0t1;                   /* D - 5bit, Q goes to sw3 */
    int c0t2;                   /* D - out sw2, Q goes to sw4 and t3 */
    int c0t3;                   /* D - out t2, q goes to xor */

    int c0sw1;                  /* in1 - 4bit, in2 - 17bit, out goes to sw2 */
    int c0sw2;                  /* in1 - /Q t2, in2 - out sw1, out goes to t2 */
    int c0sw3;                  /* in1 - +5, in2 - Q t1, out goes to C t2 */
    int c0sw4;                  /* hi-pass sw */
    int c0vo;                   /* volume only */

    int vol0;

    int outvol_0;

    /* channel 1 state */

    readout_t readout_1;
    event_t event_1;

    int c1divpos;
    int c1divstart;
    int c1diva;

    int c1t1;
    int c1t2;
    int c1t3;

    int c1sw1;
    int c1sw2;
    int c1sw3;
    int c1sw4;
    int c1vo;

    int vol1;

    int outvol_1;

    /* channel 2 state */

    readout_t readout_2;
    event_t event_2;

    int c2divpos;
    int c2divstart;
    int c2divstart_p;           /* start value when c1_f0 */
    int c2diva;

    int c2t1;
    int c2t2;

    int c2sw1;
    int c2sw2;
    int c2sw3;
    int c2vo;

    int vol2;

    int outvol_2;

    /* channel 3 state */

    readout_t readout_3;
    event_t event_3;

    int c3divpos;
    int c3divstart;
    int c3diva;

    int c3t1;
    int c3t2;

    int c3sw1;
    int c3sw2;
    int c3sw3;
    int c3vo;

    int vol3;

    int outvol_3;

    /* GTIA speaker */

    int speaker;

} PokeyState;

PokeyState pokey_states[NPOKEYS];

static struct {
    double s16;
    double s8;
} volume;

// READ OUTPUTS ************************************************************
//
static int readout0_vo(PokeyState *ps) { return ps->vol0; }
static int readout1_vo(PokeyState *ps) { return ps->vol1; }
static int readout2_vo(PokeyState *ps) { return ps->vol2; }
static int readout3_vo(PokeyState *ps) { return ps->vol3; }

static int readout0_hipass(PokeyState * ps) {
    return (ps->c0t2 ^ ps->c0t3) ? ps->vol0 : 0;
}
static int readout1_hipass(PokeyState * ps) {
    return (ps->c1t2 ^ ps->c1t3) ? ps->vol1 : 0;
}

static int readout0_normal(PokeyState * ps) {
    return (ps->c0t2) ? ps->vol0 : 0;
}
static int readout1_normal(PokeyState * ps) {
    return (ps->c1t2) ? ps->vol1 : 0;
}
static int readout2_normal(PokeyState * ps) {
    return (ps->c2t2) ? ps->vol2 : 0;
}
static int readout3_normal(PokeyState * ps) {
    return (ps->c3t2) ? ps->vol3 : 0;
}

// CHANNEL EVENTS **********************************************************
//
// PURE TONE EVENTS ********************************************************
//
static void event0_pure(PokeyState * ps, int p5v, int p4v, int p917v) {
    ps->c0t2 = !ps->c0t2;
    ps->c0t1 = p5v;
}
static void event1_pure(PokeyState * ps, int p5v, int p4v, int p917v) {
    ps->c1t2 = !ps->c1t2;
    ps->c1t1 = p5v;
}
static void event2_pure(PokeyState * ps, int p5v, int p4v, int p917v) {
    ps->c2t2 = !ps->c2t2;
    ps->c2t1 = p5v;
    ps->c0t3 = ps->c0t2; /* high-pass clock for channel 0 */
}
static void event3_pure(PokeyState * ps, int p5v, int p4v, int p917v) {
    ps->c3t2 = !ps->c3t2;
    ps->c3t1 = p5v;
    ps->c1t3 = ps->c1t2; /* high-pass clock for channel 1 */
}

// POLY4 EVENTS ************************************************************
//
static void event0_p4(PokeyState * ps, int p5v, int p4v, int p917v) {
    ps->c0t2 = p4v;
    ps->c0t1 = p5v;
}
static void event1_p4(PokeyState * ps, int p5v, int p4v, int p917v) {
    ps->c1t2 = p4v;
    ps->c1t1 = p5v;
}
static void event2_p4(PokeyState * ps, int p5v, int p4v, int p917v) {
    ps->c2t2 = p4v;
    ps->c2t1 = p5v;
    ps->c0t3 = ps->c0t2; /* high-pass clock for channel 0 */
}
static void event3_p4(PokeyState * ps, int p5v, int p4v, int p917v) {
    ps->c3t2 = p4v;
    ps->c3t1 = p5v;
    ps->c1t3 = ps->c1t2; /* high-pass clock for channel 1 */
}

// POLY5 EVENTS ************************************************************
//
static void event0_p5(PokeyState * ps, int p5v, int p4v, int p917v) {
    if (ps->c0t1) ps->c0t2 = !ps->c0t2;
    ps->c0t1 = p5v;
}
static void event1_p5(PokeyState * ps, int p5v, int p4v, int p917v) {
    if (ps->c1t1) ps->c1t2 = !ps->c1t2;
    ps->c1t1 = p5v;
}
static void event2_p5(PokeyState * ps, int p5v, int p4v, int p917v) {
    if (ps->c2t1) ps->c2t2 = !ps->c2t2;
    ps->c2t1 = p5v;
    ps->c0t3 = ps->c0t2; /* high-pass clock for channel 0 */
}
static void event3_p5(PokeyState * ps, int p5v, int p4v, int p917v) {
    if (ps->c3t1) ps->c3t2 = !ps->c3t2;
    ps->c3t1 = p5v;
    ps->c1t3 = ps->c1t2; /* high-pass clock for channel 1 */
}

// POLY9/POLY17 EVENTS *****************************************************
//
static void event0_p917(PokeyState * ps, int p5v, int p4v, int p917v) {
    ps->c0t2 = p917v;
    ps->c0t1 = p5v;
}
static void event1_p917(PokeyState * ps, int p5v, int p4v, int p917v) {
    ps->c1t2 = p917v;
    ps->c1t1 = p5v;
}
static void event2_p917(PokeyState * ps, int p5v, int p4v, int p917v) {
    ps->c2t2 = p917v;
    ps->c2t1 = p5v;
    ps->c0t3 = ps->c0t2; /* high-pass clock for channel 0 */
}
static void event3_p917(PokeyState * ps, int p5v, int p4v, int p917v) {
    ps->c3t2 = p917v;
    ps->c3t1 = p5v;
    ps->c1t3 = ps->c1t2; /* high-pass clock for channel 1 */
}

// POLY4/POLY5 EVENTS ******************************************************
//
static void event0_p4_p5(PokeyState * ps, int p5v, int p4v, int p917v) {
    if (ps->c0t1) ps->c0t2 = p4v;
    ps->c0t1 = p5v;
}
static void event1_p4_p5(PokeyState * ps, int p5v, int p4v, int p917v) {
    if (ps->c1t1) ps->c1t2 = p4v;
    ps->c1t1 = p5v;
}
static void event2_p4_p5(PokeyState * ps, int p5v, int p4v, int p917v) {
    if (ps->c2t1) ps->c2t2 = p4v;
    ps->c2t1 = p5v;
    ps->c0t3 = ps->c0t2; /* high-pass clock for channel 0 */
}
static void event3_p4_p5(PokeyState * ps, int p5v, int p4v, int p917v) {
    if (ps->c3t1) ps->c3t2 = p4v;
    ps->c3t1 = p5v;
    ps->c1t3 = ps->c1t2; /* high-pass clock for channel 1 */
}

// POLY9/POLY17/POLY5 EVENTS ***********************************************
//
static void event0_p917_p5(PokeyState * ps, int p5v, int p4v, int p917v) {
    if (ps->c0t1) ps->c0t2 = p917v;
    ps->c0t1 = p5v;
}
static void event1_p917_p5(PokeyState * ps, int p5v, int p4v, int p917v) {
    if (ps->c1t1) ps->c1t2 = p917v;
    ps->c1t1 = p5v;
}
static void event2_p917_p5(PokeyState * ps, int p5v, int p4v, int p917v) {
    if (ps->c2t1) ps->c2t2 = p917v;
    ps->c2t1 = p5v;
    ps->c0t3 = ps->c0t2; /* high-pass clock for channel 0 */
}
static void event3_p917_p5(PokeyState * ps, int p5v, int p4v, int p917v) {
    if (ps->c3t1) ps->c3t2 = p917v;
    ps->c3t1 = p5v;
    ps->c1t3 = ps->c1t2; /* high-pass clock for channel 1 */
}

// RESET POKEY *************************************************************
//
static void ResetPokeyState(PokeyState * ps) {
    /* Poly positions */
    ps->poly4pos = 0;
    ps->poly5pos = 0;
    ps->poly9pos = 0;
    ps->poly17pos = 0;

    /* Change queue */
    ps->ovola = 0;
    ps->qebeg = 0;
    ps->qeend = 0;

    /* Global Pokey controls */
    ps->mdivk = 28;

    ps->selpoly9 = 0;
    ps->c0_hf = 0;
    ps->c1_f0 = 0;
    ps->c2_hf = 0;
    ps->c3_f2 = 0;

    /* SKCTL for two-tone mode */
    ps->skctl = 0;

    ps->outvol_all = 0;
    ps->forcero = 0;

    /* Channel 0 state */
    ps->readout_0 = readout0_normal;
    ps->event_0 = event0_pure;

    ps->c0divpos = ps->c0divstart = ps->c0divstart_p = 1000;
    ps->c0diva = 255;

    ps->c0t1  = ps->c0t2  = ps->c0t3  = 0;
    ps->c0sw1 = ps->c0sw2 = ps->c0sw3 = ps->c0sw4 = 0;
    ps->c0vo  = 1;
    ps->vol0  = ps->outvol_0 = 0;

    /* Channel 1 state */
    ps->readout_1 = readout1_normal;
    ps->event_1 = event1_pure;

    ps->c1divpos = ps->c1divstart = 1000;
    ps->c1diva = 255;

    ps->c1t1  = ps->c1t2  = ps->c1t3  = 0;
    ps->c1sw1 = ps->c1sw2 = ps->c1sw3 = ps->c1sw4 = 0;
    ps->c1vo  = 1;
    ps->vol1  = ps->outvol_1 = 0;

    /* Channel 2 state */
    ps->readout_2 = readout2_normal;
    ps->event_2 = event2_pure;

    ps->c2divpos = ps->c2divstart = ps->c2divstart_p = 1000;
    ps->c2diva = 255;

    ps->c2t1  = ps->c2t2  = 0;
    ps->c2sw1 = ps->c2sw2 = ps->c2sw3 = 0;
    ps->c2vo  = 0;
    ps->vol2  = ps->outvol_2 = 0;

    /* Channel 3 state */
    ps->readout_3 = readout3_normal;
    ps->event_3 = event3_pure;

    ps->c3divpos = 1000;
    ps->c3divstart = 1000;
    ps->c3diva = 255;

    ps->c3t1  = ps->c3t2  = 0;
    ps->c3sw1 = ps->c3sw2 = ps->c3sw3 = 0;
    ps->c3vo  = 0;
    ps->vol3  = ps->outvol_3 = 0;

    /* GTIA speaker */
    ps->speaker = 0;
}

// EVENT QUEUE HANDLING ****************************************************
//
static double read_resam_all(PokeyState * ps) {
    int i = ps->qebeg;
    qev_t avol, bvol;
    double sum;

    if (ps->qebeg == ps->qeend) {
        return ps->ovola * filter_data[0];      /* if no events in the queue */
    }

    avol = ps->ovola;
    sum = 0;

    /* Separate two loop cases, for wrap-around and without */
    if (ps->qeend < ps->qebeg) {        /* With wrap */
        while (i < filter_size) {
            bvol = ps->qev[i];
            sum += (avol - bvol) * filter_data[ps->curtick - ps->qet[i]];
            avol = bvol;
            ++i;
        }
        i = 0;
    }

    while (i < ps->qeend) {             /* without wrap */
        bvol = ps->qev[i];
        sum += (avol - bvol) * filter_data[ps->curtick - ps->qet[i]];
        avol = bvol;
        ++i;
    }

    sum += avol * filter_data[0];
    return sum;
}

static void add_change(PokeyState * ps, qev_t a) {
    ps->qev[ps->qeend] = a;
    ps->qet[ps->qeend] = ps->curtick;   /*0; */
    ++ps->qeend;
    if (ps->qeend >= filter_size)
        ps->qeend = 0;
}

static void bump_qe_subticks(PokeyState * ps, int subticks) {
    /* Remove too old events from the queue while bumping */
    int i = ps->qebeg;

    /* we must avoid curtick overflow in a 32-bit int, will happen in 20 min */
    static const int tickoverflowlimit = 1000000000;

    ps->curtick += subticks;
    if (ps->curtick > tickoverflowlimit) {
        ps->curtick -= tickoverflowlimit / 2;
        for (i = 0; i < filter_size; i++) {
            if (ps->qet[i] > tickoverflowlimit / 2) {
                ps->qet[i] -= tickoverflowlimit / 2;
            }
        }
    }

    if (ps->qeend < ps->qebeg) {        /* Loop with wrap */
        while (i < filter_size) {
            /*ps->qet[i] += subticks; */
            if (ps->curtick - ps->qet[i] >= filter_size - 1) {
                ps->ovola = ps->qev[i];
                ++ps->qebeg;
                if (ps->qebeg >= filter_size)
                    ps->qebeg = 0;
            } else {
                return;
            }
            ++i;
        }
        i = 0;
    }

    while (i < ps->qeend) {             /* loop without wrap */
        /*ps->qet[i] += subticks; */
        if (ps->curtick - ps->qet[i] >= filter_size - 1) {
            ps->ovola = ps->qev[i];
            ++ps->qebeg;
            if (ps->qebeg >= filter_size)
                ps->qebeg = 0;
        } else {
            return;
        }
        ++i;
    }
}

// PRE-CALCULATE POLY'S ****************************************************
//
static void build_poly4(void) {
    unsigned char c;
    unsigned char i;
    unsigned char poly4 = 1;

    for (i = 0; i < 15; i++) {
        poly4tbl[i] = ~poly4;
        c = ((poly4 >> 2) & 1) ^ ((poly4 >> 3) & 1);
        poly4 = ((poly4 << 1) & 15) + c;
    }
}

static void build_poly5(void) {
    unsigned char c;
    unsigned char i;
    unsigned char poly5 = 1;

    for (i = 0; i < 31; i++) {
        poly5tbl[i] = ~poly5;   /* Inversion! Attention! */
        c = ((poly5 >> 2) ^ (poly5 >> 4)) & 1;
        poly5 = ((poly5 << 1) & 31) + c;
    }
}

static void build_poly17(void) {
    unsigned int c;
    unsigned int i;
    unsigned int poly17 = 1;

    for (i = 0; i < 131071; i++) {
        poly17tbl[i] = (unsigned char)poly17;
        c = ((poly17 >> 11) ^ (poly17 >> 16)) & 1;
        poly17 = ((poly17 << 1) & 131071) + c;
    }
}

static void build_poly9(void) {
    unsigned int c;
    unsigned int i;
    unsigned int poly9 = 1;

    for (i = 0; i < 511; i++) {
        poly9tbl[i] = (unsigned char)poly9;
        c = ((poly9 >> 3) ^ (poly9 >> 8)) & 1;
        poly9 = ((poly9 << 1) & 511) + c;
    }
}

// ADVANCEMENT AND SAMPLE GENERATION ***************************************
//
static void advance_polies(PokeyState * ps, int tacts) {
    ps->poly4pos  = (tacts + ps->poly4pos)  % 15;
    ps->poly5pos  = (tacts + ps->poly5pos)  % 31;
    ps->poly17pos = (tacts + ps->poly17pos) % 131071;
    ps->poly9pos  = (tacts + ps->poly9pos)  % 511;
}

static void advance_ticks(PokeyState * ps, int ticks) {
    int ta, tbe, tbe0, tbe1, tbe2, tbe3;
    int p5v, p4v, p917v;

    qev_t outvol_new;
    int need0 = 0;
    int need1 = 0;
    int need2 = 0;
    int need3 = 0;

    int need = 0;

    if (ticks <= 0) return;

    if (ps->forcero) {
        ps->forcero = 0;
        outvol_new =
            pokeymix[ps->outvol_0 + ps->outvol_1 + ps->outvol_2 +
                     ps->outvol_3 + ps->speaker];
        if (outvol_new != ps->outvol_all) {
            ps->outvol_all = outvol_new;
            add_change(ps, outvol_new);
        }
    }

    while (ticks > 0) {
        tbe0 = ps->c0divpos;
        tbe1 = ps->c1divpos;
        tbe2 = ps->c2divpos;
        tbe3 = ps->c3divpos;

        tbe = ticks + 1;

        if (tbe0 < tbe) tbe = tbe0;
        if (tbe1 < tbe) tbe = tbe1;
        if (tbe2 < tbe) tbe = tbe2;
        if (tbe3 < tbe) tbe = tbe3;

        if (tbe > ticks) ta = ticks;
        else {
            ta = tbe;
            need = 1;
        }

        ticks -= ta;

        ps->c0divpos -= ta;
        ps->c1divpos -= ta;
        ps->c2divpos -= ta;
        ps->c3divpos -= ta;

        advance_polies(ps, ta);
        bump_qe_subticks(ps, ta);

        if (need) {
            p5v = poly5tbl[ps->poly5pos] & 1;
            p4v = poly4tbl[ps->poly4pos] & 1;
            if (ps->selpoly9) p917v = poly9tbl[ps->poly9pos] & 1;
            else              p917v = poly17tbl[ps->poly17pos] & 1;

            if (ta == tbe0) {
                ps->event_0(ps, p5v, p4v, p917v);
                ps->c0divpos = ps->c0divstart;
                need0 = 1;
            }
            if (ta == tbe1) {
                ps->event_1(ps, p5v, p4v, p917v);
                ps->c1divpos = ps->c1divstart;
                if (ps->c1_f0) ps->c0divpos = ps->c0divstart_p;
                need1 = 1;
                /*two-tone filter */
                /*use if send break is on and two-tone mode is on */
                /*reset channel 1 if channel 2 changed */
                if ((ps->skctl & 0x88) == 0x88) {
                    ps->c0divpos = ps->c0divstart;
                    /* it doesn't change the output state */
                    /*need0 = 1; */
                }
            }
            if (ta == tbe2) {
                ps->event_2(ps, p5v, p4v, p917v);
                ps->c2divpos = ps->c2divstart;
                need2 = 1;
                if (ps->c0sw4) need0 = 1;
            }
            if (ta == tbe3) {
                ps->event_3(ps, p5v, p4v, p917v);
                ps->c3divpos = ps->c3divstart;
                if (ps->c3_f2) ps->c2divpos = ps->c2divstart_p;
                need3 = 1;
                if (ps->c1sw4) need1 = 1;
            }

            if (need0) ps->outvol_0 = ps->readout_0(ps);
            if (need1) ps->outvol_1 = ps->readout_1(ps);
            if (need2) ps->outvol_2 = ps->readout_2(ps);
            if (need3) ps->outvol_3 = ps->readout_3(ps);

            outvol_new =
                pokeymix[ps->outvol_0 + ps->outvol_1 + ps->outvol_2 +
                         ps->outvol_3 + ps->speaker];
            if (outvol_new != ps->outvol_all) {
                ps->outvol_all = outvol_new;
                add_change(ps, outvol_new);
            }
        }
    }
}

static double generate_sample(PokeyState * ps) {
    /*unsigned long ta = (subticks+pokey_frq)/POKEYSND_playback_freq;
       subticks = (subticks+pokey_frq)%POKEYSND_playback_freq; */

    advance_ticks(ps, pokey_frq / POKEYSND_playback_freq);
    return read_resam_all(ps);
}

// FILTER TABLE GENERATOR **************************************************
//
static int remez_filter_table(double resamp_rate, /* output_rate/input_rate */
                              double *cutoff, int quality) {
    int i;
    static const int orders[] = { 600, 800, 1000, 1200 };
    static const struct {
        int stop;               /* stopband ripple */
        double weight;          /* stopband weight */
        double twidth[sizeof(orders) / sizeof(orders[0])];
    }
    paramtab[] = {
        { 70, 90, { 4.9e-3, 3.45e-3, 2.65e-3, 2.2e-3} },
        { 55, 25, { 3.4e-3, 2.7e-3, 2.05e-3, 1.7e-3} },
        { 40, 6.0, { 2.6e-3, 1.8e-3, 1.5e-3, 1.2e-3} },
        { -1, 0, { 0, 0, 0, 0} }
    };
    static const double passtab[] = { 0.5, 0.6, 0.7 };
    int ripple = 0, order = 0;
    int size;
    double weights[2], desired[2], bands[4];
    static const int interlevel = 5;
    double step = 1.0 / interlevel;

    *cutoff = 0.95 * 0.5 * resamp_rate;

    if (quality >= (int)(sizeof(passtab) / sizeof(passtab[0])))
        quality = (int)(sizeof(passtab) / sizeof(passtab[0])) - 1;

    for (ripple = 0; paramtab[ripple].stop > 0; ripple++) {
        for (order = 0; order < (int)(sizeof(orders) / sizeof(orders[0]));
             order++) {
            if ((*cutoff - paramtab[ripple].twidth[order])
                > passtab[quality] * 0.5 * resamp_rate)
                /* transition width OK */
                goto found;
        }
    }

    /* not found -- use shortest transition */
    ripple--;
    order--;

  found:
    size = orders[order] + 1;

    if (size > SND_FILTER_SIZE) /* static table too short */
        return 0;

    desired[0] = 1;
    desired[1] = 0;

    weights[0] = 1;
    weights[1] = paramtab[ripple].weight;

    bands[0] = 0;
    bands[2] = *cutoff;
    bands[1] = bands[2] - paramtab[ripple].twidth[order];
    bands[3] = 0.5;

    bands[1] *= (double)interlevel;
    bands[2] *= (double)interlevel;
    REMEZ_CreateFilter(filter_data, (size / interlevel) + 1, 2, bands, desired,
                       weights, REMEZ_BANDPASS);
    for (i = size - interlevel; i >= 0; i -= interlevel) {
        int s;
        double h1 = filter_data[i / interlevel];
        double h2 = filter_data[i / interlevel + 1];

        for (s = 0; s < interlevel; s++) {
            double d = (double)s * step;

            filter_data[i + s] = (h1 * (1.0 - d) + h2 * d) * step;
        }
    }

    /* compute reversed cumulative sum table */
    for (i = size - 2; i >= 0; i--)
        filter_data[i] += filter_data[i + 1];

    return size;
}

// UPDATE READOUT AND EVENT CALLBACKS **************************************
//
static void Update_readout_0(PokeyState * ps) {
         if (ps->c0vo)  ps->readout_0 = readout0_vo;
    else if (ps->c0sw4) ps->readout_0 = readout0_hipass;
    else                ps->readout_0 = readout0_normal;
}

static void Update_readout_1(PokeyState * ps) {
         if (ps->c1vo)  ps->readout_1 = readout1_vo;
    else if (ps->c1sw4) ps->readout_1 = readout1_hipass;
    else                ps->readout_1 = readout1_normal;
}

static void Update_readout_2(PokeyState * ps) {
    if (ps->c2vo) ps->readout_2 = readout2_vo;
    else          ps->readout_2 = readout2_normal;
}

static void Update_readout_3(PokeyState * ps) {
    if (ps->c3vo) ps->readout_3 = readout3_vo;
    else          ps->readout_3 = readout3_normal;
}

static void Update_event0(PokeyState * ps) {
    if (ps->c0sw3) {
        if (ps->c0sw2)     ps->event_0 = event0_pure;
        else {
            if (ps->c0sw1) ps->event_0 = event0_p4;
            else           ps->event_0 = event0_p917;
        }
    } else {
        if (ps->c0sw2)     ps->event_0 = event0_p5;
        else {
            if (ps->c0sw1) ps->event_0 = event0_p4_p5;
            else           ps->event_0 = event0_p917_p5;
        }
    }
}

static void Update_event1(PokeyState * ps) {
    if (ps->c1sw3) {
        if (ps->c1sw2)     ps->event_1 = event1_pure;
        else {
            if (ps->c1sw1) ps->event_1 = event1_p4;
            else           ps->event_1 = event1_p917;
        }
    } else {
        if (ps->c1sw2)     ps->event_1 = event1_p5;
        else {
            if (ps->c1sw1) ps->event_1 = event1_p4_p5;
            else           ps->event_1 = event1_p917_p5;
        }
    }
}

static void Update_event2(PokeyState * ps) {
    if (ps->c2sw3) {
        if (ps->c2sw2)     ps->event_2 = event2_pure;
        else {
            if (ps->c2sw1) ps->event_2 = event2_p4;
            else           ps->event_2 = event2_p917;
        }
    } else {
        if (ps->c2sw2)     ps->event_2 = event2_p5;
        else {
            if (ps->c2sw1) ps->event_2 = event2_p4_p5;
            else           ps->event_2 = event2_p917_p5;
        }
    }
}

static void Update_event3(PokeyState * ps) {
    if (ps->c3sw3) {
        if (ps->c3sw2)     ps->event_3 = event3_pure;
        else {
            if (ps->c3sw1) ps->event_3 = event3_p4;
            else           ps->event_3 = event3_p917;
        }
    } else {
        if (ps->c3sw2)     ps->event_3 = event3_p5;
        else {
            if (ps->c3sw1) ps->event_3 = event3_p4_p5;
            else           ps->event_3 = event3_p917_p5;
        }
    }
}

static void Update_c0divstart(PokeyState * ps) {
    if (ps->c1_f0) {
        if (ps->c0_hf) {
            ps->c0divstart = 256;
            ps->c0divstart_p = ps->c0diva + 7;
        } else {
            ps->c0divstart = 256 * ps->mdivk;
            ps->c0divstart_p = (ps->c0diva + 1) * ps->mdivk;
        }
    } else {
        if (ps->c0_hf) ps->c0divstart =  ps->c0diva + 4;
        else           ps->c0divstart = (ps->c0diva + 1) * ps->mdivk;
    }
}

static void Update_c1divstart(PokeyState * ps) {
    if (ps->c1_f0) {
        if (ps->c0_hf)
            ps->c1divstart = ps->c0diva + 256 * ps->c1diva + 7;
        else
            ps->c1divstart = (ps->c0diva + 256 * ps->c1diva + 1) * ps->mdivk;
    } else
        ps->c1divstart = (ps->c1diva + 1) * ps->mdivk;
}

static void Update_c2divstart(PokeyState * ps) {
    if (ps->c3_f2) {
        if (ps->c2_hf) {
            ps->c2divstart = 256;
            ps->c2divstart_p = ps->c2diva + 7;
        } else {
            ps->c2divstart = 256 * ps->mdivk;
            ps->c2divstart_p = (ps->c2diva + 1) * ps->mdivk;
        }
    } else {
        if (ps->c2_hf) ps->c2divstart =  ps->c2diva + 4;
        else           ps->c2divstart = (ps->c2diva + 1) * ps->mdivk;
    }
}

static void Update_c3divstart(PokeyState * ps) {
    if (ps->c3_f2) {
        if (ps->c2_hf)
            ps->c3divstart = ps->c2diva + 256 * ps->c3diva + 7;
        else
            ps->c3divstart = (ps->c2diva + 256 * ps->c3diva + 1) * ps->mdivk;
    } else
        ps->c3divstart = (ps->c3diva + 1) * ps->mdivk;
}

static void Update_audctl(PokeyState * ps, unsigned char val) {
    int nc0_hf, nc2_hf, nc1_f0, nc3_f2, nc0sw4, nc1sw4, new_divk;
    int recalc0 = 0;
    int recalc1 = 0;
    int recalc2 = 0;
    int recalc3 = 0;

    unsigned int cnt0 = 0;
    unsigned int cnt1 = 0;
    unsigned int cnt2 = 0;
    unsigned int cnt3 = 0;

    nc0_hf = (val & 0x40) != 0;
    nc2_hf = (val & 0x20) != 0;
    nc1_f0 = (val & 0x10) != 0;
    nc3_f2 = (val & 0x08) != 0;
    nc0sw4 = (val & 0x04) != 0;
    nc1sw4 = (val & 0x02) != 0;

    if (val & 0x01) new_divk = 114;
    else            new_divk = 28;

    if (new_divk != ps->mdivk) recalc0 = recalc1 = recalc2 = recalc3 = 1;
    if (nc1_f0 != ps->c1_f0)   recalc0 = recalc1 = 1;
    if (nc3_f2 != ps->c3_f2)   recalc2 = recalc3 = 1;

    if (nc0_hf != ps->c0_hf) {
        recalc0 = 1;
        if (nc1_f0) recalc1 = 1;
    }
    if (nc2_hf != ps->c2_hf) {
        recalc2 = 1;
        if (nc3_f2) recalc3 = 1;
    }

    if (recalc0) {
        if (ps->c0_hf) cnt0 = ps->c0divpos;
        else           cnt0 = ps->c0divpos / ps->mdivk;
    }
    if (recalc1) {
        if (ps->c1_f0) {
            if (ps->c0_hf) cnt1 = ps->c1divpos / 256;
            else           cnt1 = ps->c1divpos / 256 / ps->mdivk;
        } else             cnt1 = ps->c1divpos / ps->mdivk;
    }
    if (recalc2) {
        if (ps->c2_hf) cnt2 = ps->c2divpos;
        else           cnt2 = ps->c2divpos / ps->mdivk;
    }
    if (recalc3) {
        if (ps->c3_f2) {
            if (ps->c2_hf) cnt3 = ps->c3divpos / 256;
            else           cnt3 = ps->c3divpos / 256 / ps->mdivk;
        }
    }

    if (recalc0) {
        if (nc0_hf) ps->c0divpos = cnt0;
        else        ps->c0divpos = cnt0 * new_divk;
    }
    if (recalc1) {
        if (nc1_f0) {
            if (nc0_hf) ps->c1divpos =  cnt1 * 256 + cnt0;
            else        ps->c1divpos = (cnt1 * 256 + cnt0) * new_divk;
        } else          ps->c1divpos =  cnt1 * new_divk;
    }

    if (recalc2) {
        if (nc2_hf) ps->c2divpos = cnt2;
        else        ps->c2divpos = cnt2 * new_divk;
    }
    if (recalc3) {
        if (nc3_f2) {
            if (nc2_hf) ps->c3divpos =  cnt3 * 256 + cnt2;
            else        ps->c3divpos = (cnt3 * 256 + cnt2) * new_divk;
        }
    }

    ps->c0_hf = nc0_hf;
    ps->c2_hf = nc2_hf;
    ps->c1_f0 = nc1_f0;
    ps->c3_f2 = nc3_f2;
    ps->c0sw4 = nc0sw4;
    ps->c1sw4 = nc1sw4;
    ps->mdivk = new_divk;
}

/* SKCTL for two-tone mode */
static void Update_skctl(PokeyState * ps, unsigned char val) {
    ps->skctl = val;
}

static void Update_c0stop(PokeyState * ps) {
    ps->outvol_0 = ps->readout_0(ps);
}
static void Update_c1stop(PokeyState * ps) {
    ps->outvol_1 = ps->readout_1(ps);
}
static void Update_c2stop(PokeyState * ps) {
    ps->outvol_2 = ps->readout_2(ps);
}
static void Update_c3stop(PokeyState * ps) {
    ps->outvol_3 = ps->readout_3(ps);
}

// GENERATE 16-BIT SAMPLED OUTPUT ******************************************
//
// See mzpokeysnd.txt for details
//
#define MAX_SAMPLE 120

void MZPOKEYSND_Process(void *sndbuffer, int sndn) {
    int i;
    int nsam = sndn;
    int16_t *buffer = (int16_t *) sndbuffer;

    if (num_cur_pokeys < 1)
        return;                 /* module was not initialized */

    /* if there are two pokeys, then the signal is stereo
       we assume even sndn */
    while (nsam >= (int)num_cur_pokeys) {
        buffer[0] = (int16_t) floor(generate_sample(pokey_states)
                                  * (65535.0 / 2 / MAX_SAMPLE / 4 * M_PI *
                                     0.95) + 0.5 + 0.5 * rand() / RAND_MAX -
                                  0.25);
        for (i = 1; i < num_cur_pokeys; i++) {
            buffer[i] = (int16_t) floor(generate_sample(pokey_states + i)
                                      * (65535.0 / 2 / MAX_SAMPLE / 4 * M_PI *
                                         0.95) + 0.5 +
                                      0.5 * rand() / RAND_MAX - 0.25);
        }
        buffer += num_cur_pokeys;
        nsam -= num_cur_pokeys;
    }
}

/*****************************************************************************/
/* Inputs:  addr - the address of the parameter to be changed                */
/*          val - the new value to be placed in the specified address        */
/*          chip - chip # for stereo                                         */
/*                                                                           */

void MZPOKEYSND_Update(uint16_t addr, uint8_t val, uint8_t chip) {
    PokeyState *ps = pokey_states + chip;

    switch (addr & 0x0f) {
    case POKEY_OFFSET_AUDF1:
        ps->c0diva = val;
        Update_c0divstart(ps);
        if (ps->c1_f0) {
            Update_c1divstart(ps);
            Update_c1stop(ps);
        }
        Update_c0stop(ps);
        ps->forcero = 1;
        break;
    case POKEY_OFFSET_AUDC1:
        ps->c0sw1 = (val & 0x40) != 0;
        ps->c0sw2 = (val & 0x20) != 0;
        ps->c0sw3 = (val & 0x80) != 0;
        ps->vol0 = (val & 0xF);
        ps->c0vo = (val & 0x10) != 0;
        Update_readout_0(ps);
        Update_event0(ps);
        Update_c0stop(ps);
        ps->forcero = 1;
        break;
    case POKEY_OFFSET_AUDF2:
        ps->c1diva = val;
        Update_c1divstart(ps);
        if (ps->c1_f0) {
            Update_c0divstart(ps);
            Update_c0stop(ps);
        }
        Update_c1stop(ps);
        ps->forcero = 1;
        break;
    case POKEY_OFFSET_AUDC2:
        ps->c1sw1 = (val & 0x40) != 0;
        ps->c1sw2 = (val & 0x20) != 0;
        ps->c1sw3 = (val & 0x80) != 0;
        ps->vol1 = (val & 0xF);
        ps->c1vo = (val & 0x10) != 0;
        Update_readout_1(ps);
        Update_event1(ps);
        Update_c1stop(ps);
        ps->forcero = 1;
        break;
    case POKEY_OFFSET_AUDF3:
        ps->c2diva = val;
        Update_c2divstart(ps);
        if (ps->c3_f2) {
            Update_c3divstart(ps);
            Update_c3stop(ps);
        }
        Update_c2stop(ps);
        ps->forcero = 1;
        break;
    case POKEY_OFFSET_AUDC3:
        ps->c2sw1 = (val & 0x40) != 0;
        ps->c2sw2 = (val & 0x20) != 0;
        ps->c2sw3 = (val & 0x80) != 0;
        ps->vol2 = (val & 0xF);
        ps->c2vo = (val & 0x10) != 0;
        Update_readout_2(ps);
        Update_event2(ps);
        Update_c2stop(ps);
        ps->forcero = 1;
        break;
    case POKEY_OFFSET_AUDF4:
        ps->c3diva = val;
        Update_c3divstart(ps);
        if (ps->c3_f2) {
            Update_c2divstart(ps);
            Update_c2stop(ps);
        }
        Update_c3stop(ps);
        ps->forcero = 1;
        break;
    case POKEY_OFFSET_AUDC4:
        ps->c3sw1 = (val & 0x40) != 0;
        ps->c3sw2 = (val & 0x20) != 0;
        ps->c3sw3 = (val & 0x80) != 0;
        ps->vol3 = val & 0xF;
        ps->c3vo = (val & 0x10) != 0;
        Update_readout_3(ps);
        Update_event3(ps);
        Update_c3stop(ps);
        ps->forcero = 1;
        break;
    case POKEY_OFFSET_AUDCTL:
        ps->selpoly9 = (val & 0x80) != 0;
        Update_audctl(ps, val);
        Update_readout_0(ps);
        Update_readout_1(ps);
        Update_readout_2(ps);
        Update_readout_3(ps);
        Update_c0divstart(ps);
        Update_c1divstart(ps);
        Update_c2divstart(ps);
        Update_c3divstart(ps);
        Update_c0stop(ps);
        Update_c1stop(ps);
        Update_c2stop(ps);
        Update_c3stop(ps);
        ps->forcero = 1;
        break;
    case POKEY_OFFSET_STIMER:
        if (ps->c1_f0)
            ps->c0divpos = ps->c0divstart_p;
        else
            ps->c0divpos = ps->c0divstart;
        ps->c1divpos = ps->c1divstart;
        if (ps->c3_f2)
            ps->c2divpos = ps->c2divstart_p;
        else
            ps->c2divpos = ps->c2divstart;

        ps->c3divpos = ps->c3divstart;
        /*Documentation is wrong about which voices are on after STIMER */
        /*It is 3&4 which are on, tested on a real atari */
        ps->c0t2 = 0;
        ps->c1t2 = 0;
        ps->c2t2 = 1;
        ps->c3t2 = 1;
        break;
    case POKEY_OFFSET_SKCTL:
        Update_skctl(ps, val);
        break;
    }
}

/*****************************************************************************/
/* Module:  MZPOKEYSND_Init()                                                */
/* Purpose: to handle the power-up initialization functions                  */
/*          these functions should only be executed on a cold-restart        */
/*                                                                           */
/* Authors: Michael Borisov, Krzystof Nikiel                                 */
/*                                                                           */
/* Inputs:  freq17 - the value for the '1.79MHz' Pokey audio clock           */
/*          playback_freq - the playback frequency in samples per second     */
/*          num_pokeys - specifies the number of pokey chips to be emulated  */
/*                                                                           */
/* Outputs: Adjusts local globals - no return value                          */
/*                                                                           */
/*****************************************************************************/

static void init_syncsound(void) {
    double samples_per_frame =
        (double)POKEYSND_playback_freq / (Atari800_tv_mode ==
                                          Atari800_TV_PAL ? Atari800_FPS_PAL :
                                          Atari800_FPS_NTSC);
    unsigned int ticks_per_frame = Atari800_tv_mode * 114;

    ticks_per_sample = (double)ticks_per_frame / samples_per_frame;
    samp_pos = 0.0;
}

int MZPOKEYSND_Init(uint32_t freq17, int playback_freq, uint8_t num_pokeys,
                    int quality) {
    double cutoff;

    pokey_frq =
            (int)(((double)pokey_frq_ideal / POKEYSND_playback_freq) + 0.5)
            * POKEYSND_playback_freq;
    filter_size =
                remez_filter_table((double)POKEYSND_playback_freq / pokey_frq,
                               &cutoff, quality);
    audible_frq = (int)(cutoff * pokey_frq);

    build_poly4();
    build_poly5();
    build_poly9();
    build_poly17();

    ResetPokeyState(pokey_states);
    ResetPokeyState(pokey_states + 1);
    num_cur_pokeys = num_pokeys;

    init_syncsound();
    volume.s8 = POKEYSND_volume * 0xff / 256.0;
    volume.s16 = POKEYSND_volume * 0xffff / 256.0;

    return 0; // OK
}
