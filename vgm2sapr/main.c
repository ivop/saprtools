/* vgm2sapr - by Ivo van Poorten - Copyright (C) 2023
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/* SN76489:
 *      https://map.grauw.nl/resources/sound/texas_instruments_sn76489an.pdf
 */

/* DMG docs
 *      https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware
 *      https://nightshade256.github.io/2021/03/27/gb-sound-emulation.html
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <zlib.h>

#define ATARI_CLOCK      1773447L

struct vgm_header {
    char ident[4];                      /* $00 */
    uint32_t eof_offset;
    uint32_t version;
    uint32_t sn76489_clock;

    uint32_t ym2413_clock;              /* $10 */
    uint32_t gd3_offset;
    uint32_t total_nsamples;
    uint32_t loop_offset;

    uint32_t loop_nsamples;             /* $20 */

    /* v1.01 additions */
    uint32_t rate;

    /* v1.10 additions */
    uint16_t sn76489_feedback;
    uint8_t  sn76489_shift_width;

    /* v1.51 additions */
    uint8_t  sn76489_flags;

    /* v1.10 additions */
    uint32_t ym2612_clock;

    uint32_t ym2151_clock;              /* $30 */

    /* v1.50 additions */
    uint32_t vgm_data_offset;

    /* v1.51 additions */
    uint32_t sega_pcm_clock;
    uint32_t spcm_interface;

    uint32_t rf5c68_clock;              /* $40 */
    uint32_t ym2203_clock;
    uint32_t ym2608_clock;
    uint32_t ym2610b_clock;

    uint32_t ym3812_clock;              /* $50 */
    uint32_t ym3526_clock;
    uint32_t y8950_clock;
    uint32_t ymf262_clock;

    uint32_t ymf278b_clock;             /* $60 */
    uint32_t ymf271_clock;
    uint32_t ymz280b_clock;
    uint32_t rf5c164_clock;

    uint32_t pwm_clock;                 /* $70 */
    uint32_t ay8910_clock;
    uint8_t  ay8910_chip_type;
    uint8_t  ay8910_flags;
    uint8_t  ay8910_ym2203_flags;
    uint8_t  ay8910_ym2608_flags;

    /* v1.60 additions */
    uint8_t  volume_modifier;
    uint8_t  reserved;
    uint8_t  loop_base;

    /* v1.51 additions */
    uint8_t  loop_modifier;

    /* v1.61 additions */
    uint32_t gameboy_dmg_clock;         /* $80 */
    uint32_t nes_apu_clock;
    uint32_t multipcm_clock;
    uint32_t upd7759_clock;

    uint32_t okim6258_clock;            /* $90 */
    uint8_t  okim6258_flags;
    uint8_t  k054539_flags;
    uint8_t  c140_chip_type;
    uint8_t  reserved2;
    uint32_t okim6295_clock;
    uint32_t k051649_k052539_clock;

    uint32_t k054539_clock;             /* $a0 */
    uint32_t huc6280_clock;
    uint32_t c140_clock;
    uint32_t k053260_clock;

    uint32_t pokey_clock;               /* $b0 */
    uint32_t qsound_clock;

    /* v1.71 additions */
    uint32_t scsp_clock;

    /* v1.70 additions */
    uint32_t extra_header_offset;

    /* v1.71 additions */
    uint32_t wonderswan_clock;          /* $c0 */
    uint32_t vsu_clock;
    uint32_t saa1099_clock;
    uint32_t es5503_clock;

    uint32_t es5505_es5506_clock;       /* $d0 */
    uint8_t  es5503_nchannels;
    uint8_t  es5505_es5506_nchannels;
    uint8_t  c352_clock_divider;
    uint8_t  reserved3;
    uint32_t x1_010_clock;
    uint32_t c352_clock;

    uint32_t ga20_clock;                /* $e0 */
    uint32_t reserved4;
} vgm_header;

static double framerate;
static int debug;
static int force_new;

enum chiptype {
    CHIP_SN76489,
    CHIP_GAMEBOY_DMG,
    CHIP_HUC6280
};

/* SN76489 */

union sn76489 {
    struct {
        uint16_t freq1;
        uint16_t att1;
        uint16_t freq2;
        uint16_t att2;
        uint16_t freq3;
        uint16_t att3;
        uint16_t ctrl;
        uint16_t attn;
    } v;
    uint16_t r[8];
};

enum {
    REG_FREQ1,
    REG_ATT1,
    REG_FREQ2,
    REG_ATT2,
    REG_FREQ3,
    REG_ATT3,
    REG_CTRL,
    REG_ATTN
};

enum {
    PERIODIC_NOISE,
    WHITE_NOISE
};

enum {
    SHIFT_512,
    SHIFT_1024,
    SHIFT_2048,
    SHIFT_TONE3
};

/* GAMEBOY DMG */

struct gameboy_dmg {
    struct dmg_square {
        /* sweeps only for square1 */
        bool     sweep_enable;
        int      sweep_period;
        uint8_t  sweep_period_load;
        bool     sweep_negate;
        uint8_t  sweep_shift;
        uint16_t sweep_shadow;

        /* square1 and square2 */
        uint8_t duty;

        struct dmg_length {
            bool    enable;
            int     counter;
            uint8_t load;
        } length;

        struct dmg_volume {
            uint8_t value;
            uint8_t load;

            int     envelope_period;
            uint8_t envelope_period_load;
            bool    envelope_add_mode;
            bool    envelope_running;
        } volume;

        int      frequency;
        uint16_t frequency_load;

        bool    trigger;
        bool    enabled;
    } square1;

    struct dmg_square square2;

    struct dmg_wave {
        bool dac_power;

        struct dmg_length length;

        uint8_t volume_code;

        int      frequency;
        uint16_t frequency_load;

        bool    trigger;
        bool    enabled;
    } wave;

    struct dmg_noise {
        struct dmg_length length;
        struct dmg_volume volume;

        int     frequency;
        uint8_t clock_shift;
        bool    width_mode;
        uint8_t divisor_code;

        bool    trigger;
        bool    enabled;
    } noise;

    struct dmg_ctrl {
        bool    vin_left_enable;
        uint8_t left_volume;
        bool    vin_right_enable;
        uint8_t right_volume;
        bool    left_enables[4];
        bool    right_enables[4];
        bool    power_control;
    } ctrl;
};

static const unsigned int dmg_divisors[8] = { 8, 16, 32, 48, 64, 80, 96, 112 };

struct huc6280 {
    uint8_t channel_select;
    uint8_t left_main_amplitude_level;
    uint8_t right_main_amplitude_level;
    struct huc_channel {
        uint16_t frequency;
        bool on;
        bool dda;
        uint8_t amplitude_level;
        uint8_t left_amplitude_level;
        uint8_t right_amplitude_level;
        uint8_t wave_data;
        bool noise_enable;                  // only channel 5-6
        uint8_t noise_frequency;            // idem
    } channels[6];
    uint8_t lfo_frq;
    bool lf_trg;
    uint8_t lf_ctl;
};

#define SIZE_REGS_SN76489   16
#define SIZE_REGS_DMG       48
#define SIZE_REGS_HUC       (10*6)

#define LARGEST_CHIP (60)

static uint8_t voltab[16];

#define DEFAULT_MAXPOKVOL 15
static unsigned int maxpokvol = DEFAULT_MAXPOKVOL;

#define debug_fprintf(stream, ...) \
    if (debug) fprintf(stream, __VA_ARGS__);

static int mute_high = 0;
static bool single_pokey;
static float huc_master_volume = 1.0;

/* ------------------------------------------------------------------------ */

struct bass {
    double tet12f;          // 12-TET frequency, A=440Hz
    uint8_t distc;          // closest dist C divider
    double realf;           // real frequency of distc divider
};

static const struct bass gritty[36] = {
    /* C-0 */ { 16.35159783, 0xff, 16.49411272 },
    /* C#0 */ { 17.32391444, 0xf3, 17.30529859 },
    /* D-0 */ { 18.35404799, 0xe4, 18.43883344 },
    /* D#0 */ { 19.44543648, 0xd8, 19.45849243 },
    /* E-0 */ { 20.60172231, 0xcd, 20.49753814 },
    /* F-0 */ { 21.82676446, 0xc0, 21.87820133 },
    /* F#0 */ { 23.12465142, 0xb5, 23.20051020 },
    /* G-0 */ { 24.49971475, 0xab, 24.54937708 },
    /* G#0 */ { 25.95654360, 0xa2, 25.90486415 },
    /* A-0 */ { 27.50000000, 0x99, 27.41878479 },
    /* A#0 */ { 29.13523509, 0x91, 28.92118395 },
    /* B-0 */ { 30.86770633, 0x88, 30.82111575 },
    /* C-1 */ { 32.70319566, 0x7f, 32.98822545 },
    /* C#1 */ { 34.64782887, 0x79, 34.61059719 },
    /* D-1 */ { 36.70809599, 0x73, 36.40080049 },
    /* D#1 */ { 38.89087297, 0x6c, 38.73846658 },
    /* E-1 */ { 41.20344461, 0x66, 40.99507628 },
    /* F-1 */ { 43.65352893, 0x60, 43.53085420 },
    /* F#1 */ { 46.24930284, 0x5a, 46.40102041 },
    /* G-1 */ { 48.99942950, 0x55, 49.09875415 },
    /* G#1 */ { 51.91308720, 0x51, 51.49381533 },
    /* A-1 */ { 55.00000000, 0x4c, 54.83756957 },
    /* A#1 */ { 58.27047019, 0x48, 57.84236791 },
    /* B-1 */ { 61.73541266, 0x43, 62.09548319 },
    /* C-2 */ { 65.40639133, 0x3f, 65.97645089 },
    /* C#2 */ { 69.29565774, 0x3c, 69.22119438 },
    /* D-2 */ { 73.41619198, 0x39, 72.80160099 },
    /* D#2 */ { 77.78174593, 0x34, 79.66967655 },
    /* E-2 */ { 82.40688923, 0x33, 81.20178571 },
    /* F-2 */ { 87.30705786, 0x30, 86.17332362 },
    /* F#2 */ { 92.49860568, 0x2d, 91.79332298 },
    /* G-2 */ { 97.99885900, 0x2a, 98.19750831 },
    /* G#2 */ { 103.82617439, 0x28, 102.98763066 },
    /* A-2 */ { 110.00000000, 0x25, 111.11823308 },
    /* A#2 */ { 116.54094038, 0x24, 114.12142857 },
    /* B-2 */ { 123.47082531, 0x21, 124.19096639 },
};

static const struct bass buzzy[36] = {
    /* C-0 */ { 16.35159783, 0xff, 16.49411272 }, /* very low is still gritty */
    /* C#0 */ { 17.32391444, 0xf3, 17.30529859 },
    /* D-0 */ { 18.35404799, 0xe4, 18.43883344 },
    /* D#0 */ { 19.44543648, 0xd8, 19.45849243 },
    /* E-0 */ { 20.60172231, 0xcd, 20.49753814 },
    /* F-0 */ { 21.82676446, 0xc0, 21.87820133 },
    /* F#0 */ { 23.12465142, 0xb5, 23.20051020 },
    /* G-0 */ { 24.49971475, 0xab, 24.54937708 },
    /* G#0 */ { 25.95654360, 0xa2, 25.90486415 },
    /* A-0 */ { 27.50000000, 0x99, 27.41878479 },
    /* A#0 */ { 29.13523509, 0x91, 28.92118395 },
    /* B-0 */ { 30.86770633, 0x88, 30.82111575 },
    /* C-1 */ { 32.70319566, 0x7f, 32.98822545 },
    /* C#1 */ { 34.64782887, 0x79, 34.61059719 },
    /* D-1 */ { 36.70809599, 0x73, 36.40080049 },
    /* D#1 */ { 38.89087297, 0x6c, 38.73846658 },
    /* E-1 */ { 41.20344461, 0x66, 40.99507628 },
    /* F-1 */ { 43.65352893, 0x60, 43.53085420 },
    /* F#1 */ { 46.24930284, 0x5a, 46.40102041 },
    /* G-1 */ { 48.99942950, 0x55, 49.09875415 },
    /* G#1 */ { 51.91308720, 0xf2, 52.12954145 }, /* buzzy from here */
    /* A-1 */ { 55.00000000, 0xe6, 54.83756957 },
    /* A#1 */ { 58.27047019, 0xd7, 58.64573413 },
    /* B-1 */ { 61.73541266, 0xcb, 62.09548319 },
    /* C-2 */ { 65.40639133, 0xbf, 65.97645089 },
    /* C#2 */ { 69.29565774, 0xb6, 69.22119438 },
    /* D-2 */ { 73.41619198, 0xad, 72.80160099 },
    /* D#2 */ { 77.78174593, 0xa1, 78.19431217 },
    /* E-2 */ { 82.40688923, 0x98, 82.79397759 },
    /* F-2 */ { 87.30705786, 0x8f, 87.96860119 },
    /* F#2 */ { 92.49860568, 0x89, 91.79332298 },
    /* G-2 */ { 97.99885900, 0x80, 98.19750831 },
    /* G#2 */ { 103.82617439, 0x7a, 102.98763066 },
    /* A-2 */ { 110.00000000, 0x71, 111.11823308 },
    /* A#2 */ { 116.54094038, 0x6b, 117.29146825 },
    /* B-2 */ { 123.47082531, 0x65, 124.19096639 },
};

/* ------------------------------------------------------------------------ */

static uint8_t find_closest_distc(const struct bass table[], double f) {
    double delta = 1000.0;  // big number
    int found = 0;

    for (int i=0; i<36; i++) {
        double newd = fabs(table[i].realf - f);
        if (newd < delta) {
            delta = newd;
            found = i;
        }
    }
    return table[found].distc;
}

/* ------------------------------------------------------------------------ */

static inline uint16_t readLE16(uint8_t *ptr) {
    return (ptr[1]<<8) + ptr[0];
}

static inline uint32_t readLE32(uint8_t *ptr) {
    return (ptr[3]<<24) + (ptr[2]<<16) + (ptr[1]<<8) + ptr[0];
}

/* ------------------------------------------------------------------------ */

static void read_header(gzFile file) {
    gzread(file, &vgm_header, sizeof(struct vgm_header));
    for (uint8_t pos = 4; pos < sizeof(struct vgm_header); pos += 4) {
        uint8_t *ptr = (uint8_t *) &vgm_header;
        switch (pos) {
        case 0x28:      /* 16-bit LE */
            *(uint16_t *)ptr = readLE16(ptr);
            break;
        case 0x78:      /* blocks of four 8-bit values, leave alone */
        case 0x7c:
        case 0x94:
        case 0xd4:
            break;
        default:        /* 32-bit LE */
            *(uint32_t *)ptr = readLE32(ptr);
            break;
        }
    }
}

/* ------------------------------------------------------------------------ */

static void sn_to_single_pokey(union sn76489 *sn, uint8_t *pokey, int channel,
                                                    struct vgm_header *v) {
    uint16_t snf = sn->r[channel<<1];
    uint16_t snv = sn->r[(channel<<1)+1];
    uint16_t ctrl = sn->v.ctrl;
    uint16_t attn = sn->v.attn;
    uint8_t dist = 0xa0;

    if (snf == 0) snf++;            // avoid div by 0

    double f = v->sn76489_clock / (2.0 * snf * 16);

    int POK = round((ATARI_CLOCK / 28.0 / 2.0 / f) - 1);

    if (POK > 255) {
        POK = find_closest_distc(buzzy, f);
        dist = 0xc0;
    }

    pokey[0] = POK;

    int volume = voltab[snv];

    if (POK < mute_high)
        volume = 0;

    pokey[1] = dist + volume;

    if (channel == 2) {
        double noisef;
        double extradiv = 1.0;
        dist = 0xa0;

        switch (ctrl & 3) {
        case SHIFT_512:   noisef =  512; break;
        case SHIFT_1024:  noisef = 1024; break;
        case SHIFT_2048:  noisef = 2048; break;
        case SHIFT_TONE3:
            noisef = snf;
            break;
        }

        // note: periodic noise is played with generator A because we
        // do not really have a generator to play that any way else.
        // it should have a tone, so it has a tone.

        if (ctrl & 4) {
            dist = 0x80;
        } else {
            extradiv = v->sn76489_shift_width;
            if ((ctrl & 3) == SHIFT_TONE3)
                extradiv *= 8;
        }

        f = v->sn76489_clock / (2 * noisef) / extradiv;

        POK = round((ATARI_CLOCK / 28.0 / 2.0 / f) - 1);

        volume = voltab[sn->v.attn];

        if (dist == 0xa0 && POK < mute_high)
            volume = 0;

        if (dist == 0x80 && POK > 255) POK = 255;

        if (dist == 0xa0 && POK > 255) {
            POK = find_closest_distc(buzzy, f);
            dist = 0xc0;
        }

        pokey[2] = POK;
        pokey[3] = dist + volume;
    }
}

/* ------------------------------------------------------------------------ */

static void sn_to_pokey(union sn76489 *sn, uint8_t *pokey, int channel,
                                                    struct vgm_header *v) {
    uint16_t snf = sn->r[channel<<1];
    uint16_t snv = sn->r[(channel<<1)+1];
    uint16_t ctrl = sn->v.ctrl;
    uint16_t attn = sn->v.attn;

    if (snf == 0) snf++;            // avoid div by 0

    double f = v->sn76489_clock / (2.0 * snf * 16);

    int POK = round((ATARI_CLOCK / 2.0 / f) - 7);

    pokey[0] = POK & 0xff;
    pokey[2] = (POK >> 8) & 0xff;

    int volume = voltab[snv];

    if (POK < mute_high)
        volume = 0;

    pokey[3] = 0xa0 + volume;

    if (channel == 2) {
        double noisef;
        int dist = 0xa0;
        double extradiv = 1.0;

        switch (ctrl & 3) {
        case SHIFT_512:   noisef =  512; break;
        case SHIFT_1024:  noisef = 1024; break;
        case SHIFT_2048:  noisef = 2048; break;
        case SHIFT_TONE3:
            noisef = snf;
            break;
        }

        // note: periodic noise is played with generator A because we
        // do not really have a generator to play that any way else.
        // it should have a tone, so it has a tone.

        if (ctrl & 4) {
            dist = 0x80;
        } else {
            extradiv = v->sn76489_shift_width;
            if ((ctrl & 3) == SHIFT_TONE3)
                extradiv *= 8;
        }

        f = v->sn76489_clock / (2 * noisef) / extradiv;

        POK = round((ATARI_CLOCK / 2.0 / f) - 7);

        volume = voltab[sn->v.attn];

        if (dist == 0xa0 && POK < mute_high)
            volume = 0;

        pokey[4] = POK & 0xff;
        pokey[6] = (POK >> 8) & 0xff;
        pokey[7] = dist + volume;
    }
}

/* ------------------------------------------------------------------------ */

static void dmg_to_single_pokey(struct gameboy_dmg *dmg, uint8_t *pokey,
                                          int channel, struct vgm_header *v) {
    struct dmg_square *s = channel ? &dmg->square2 : &dmg->square1;
    struct dmg_wave   *w = &dmg->wave;
    struct dmg_noise  *n = &dmg->noise;
    struct dmg_ctrl   *c = &dmg->ctrl;

    int volcodes[4] = { 0, 12, 6, 3 };

    double f = 1.0;
    int vol = 0;
    int dist = 0xa0;

    switch (channel) {
    case 0:     /* square1 */
    case 1:     /* square2 */
        if (s->frequency)
            f = v->gameboy_dmg_clock / (s->frequency * 8.0);
        else
            f = v->gameboy_dmg_clock;
        if (s->enabled)
            vol = s->volume.value;
        break;
    case 2:     /* wave */
        if (w->frequency)
            f = v->gameboy_dmg_clock / (w->frequency * 8.0 * 4.0);
        else
            f = v->gameboy_dmg_clock;
        if (w->enabled)
            vol = volcodes[w->volume_code];
        break;
    case 3:     /* noise */
        if (n->frequency)
            f = v->gameboy_dmg_clock / (n->frequency * (n->width_mode?3.5:7.5));
        else
            f = v->gameboy_dmg_clock;
        if (n->enabled)
            vol = n->volume.value;
        dist = 0x80;
        break;
    default:    /* make the compiler happy */
        break;
    }

    switch (c->left_enables[channel] + c->right_enables[channel]) {
    case 0: vol = 0; break;
    case 1: vol *= 0.7; break;
    case 2: break;
    }


    if (f < 1.0) f = 1.0;       /* avoid divide by zero */

    int POK = round((ATARI_CLOCK / 28.0 / 2.0 / f) - 1);

    if (dist == 0xa0 && POK < mute_high)
        vol = 0;

    if (POK > 255) {
        POK = find_closest_distc(buzzy, f);
        dist = 0xc0;
    }

    pokey[0] = POK;
    pokey[1] = dist + voltab[vol];
}

/* ------------------------------------------------------------------------ */

static void dmg_to_pokey(struct gameboy_dmg *dmg, uint8_t *pokey, int channel,
                                                    struct vgm_header *v) {
    struct dmg_square *s = channel ? &dmg->square2 : &dmg->square1;
    struct dmg_wave   *w = &dmg->wave;
    struct dmg_noise  *n = &dmg->noise;
    struct dmg_ctrl   *c = &dmg->ctrl;

    int volcodes[4] = { 0, 12, 6, 3 };

    double f = 1.0;
    int vol = 0;
    int dist = 0xa0;

    switch (channel) {
    case 0:     /* square1 */
    case 1:     /* square2 */
        if (s->frequency)
            f = v->gameboy_dmg_clock / (s->frequency * 8.0);
        else
            f = v->gameboy_dmg_clock;
        if (s->enabled)
            vol = s->volume.value;
        break;
    case 2:     /* wave */
        if (w->frequency)
            f = v->gameboy_dmg_clock / (w->frequency * 8.0 * 4.0);
        else
            f = v->gameboy_dmg_clock;
        if (w->enabled)
            vol = volcodes[w->volume_code];
        break;
    case 3:     /* noise */
        if (n->frequency)
            f = v->gameboy_dmg_clock / (n->frequency * (n->width_mode?3.5:7.5));
        else
            f = v->gameboy_dmg_clock;
        if (n->enabled)
            vol = n->volume.value;
        dist = 0x80;
        break;
    default:    /* make the compiler happy */
        break;
    }

    switch (c->left_enables[channel] + c->right_enables[channel]) {
    case 0: vol = 0; break;
    case 1: vol *= 0.7; break;
    case 2: break;
    }


    if (f < 1.0) f = 1.0;       /* avoid divide by zero */

    int POK = round((ATARI_CLOCK / 2.0 / f) - 7);

    if (dist == 0xa0 && POK < mute_high)
        vol = 0;

    pokey[0] = POK & 0xff;
    pokey[2] = (POK >> 8) & 0xff;

    pokey[3] = dist + voltab[vol];
}

/* ------------------------------------------------------------------------ */

static void huc_to_pokey(struct huc6280 *huc, uint8_t *pokey, int channel,
                                                    struct vgm_header *v) {
    struct huc_channel *c = &huc->channels[channel];
    double f, fmaster;
    int dist = 0xa0;

    fmaster = v->huc6280_clock * 2.0;

    if (c->noise_enable) {
        dist = 0x80;
        if (c->noise_frequency)
            f = fmaster / (2.0 * 32.0 * 0.5 * c->noise_frequency);
        else
            f = 1.0;
    } else {
        f = fmaster / (2.0 * 32.0 * c->frequency);
    }

    if (f < 1.0) f = 1.0;

    double POKreal = (ATARI_CLOCK / 28.0 / 2.0 / f) - 1;

    int POK = round(POKreal);

    if (POK > 255) {
        POK = find_closest_distc(buzzy, f);
        dist = 0xc0;
    }

    double amplitude = c->amplitude_level / 2;
    amplitude *= (c->left_amplitude_level + c->right_amplitude_level) / 30.0;

    int volume = voltab[(int)round(amplitude)];

    if (c->noise_enable)
        volume *= 0.7;

    if (c->dda || !c->on)
        volume = 0;

    if (dist == 0xa0 && POK < mute_high)
        volume = 0;

    volume *= huc_master_volume;
    if (volume > 15) volume = 15;

    pokey[0] = POK;
    pokey[1] = dist + volume;
}

/* ------------------------------------------------------------------------ */

static uint16_t dmg_sweep_calculation(struct dmg_square *s) {
    uint16_t newf = 0;
    newf = s->sweep_shadow >> s->sweep_shift;
    newf = s->sweep_negate ? s->sweep_shadow - newf : s->sweep_shadow + newf;
    if (newf > 2047) s->enabled = false;
    return newf;
}

static void dmg_length_clock(struct dmg_length *length, bool *enabled) {
    if (length->enable) {
        if (length->counter > 0) {
            length->counter--;
            if (!length->counter)
                *enabled = false;
        } else {
            *enabled = false;
        }
    }
}

static void dmg_sweep_clock(struct dmg_square *s) {
    if (--s->sweep_period <= 0) {
        s->sweep_period = s->sweep_period_load;
        if (!s->sweep_period)
            s->sweep_period = 8;
        if (s->sweep_enable && s->sweep_period_load > 0) {
            uint16_t newf = dmg_sweep_calculation(s);
            if (newf <= 2047 && s->sweep_shift > 0) {
                s->sweep_shadow = newf;
                s->frequency_load = newf;
                dmg_sweep_calculation(s);   // not needed?
            }
            dmg_sweep_calculation(s);
        }
    }
}

static void dmg_envelope_cock(struct dmg_volume *v) {
    if (--v->envelope_period <= 0) {
        v->envelope_period = v->envelope_period_load;
        if (!v->envelope_period)
            v->envelope_period = 8;
        if (v->envelope_running && v->envelope_period_load > 0) {
            if (v->envelope_add_mode && v->value < 15)
                v->value++;
            else if (!v->envelope_add_mode && v->value > 0)
                v->value--;
        }
        if (v->value == 0 || v->value == 15)
            v->envelope_running = false;
    }
}

/* we run the frame sequencery at 60Hz instead of 64Hz to ease emulation.
 * The difference is hopefully small enough to be mostly unnoticable.
 * Clocks are now 60Hz, 120Hz, and 240Hz instead of 64Hz, 128Hz, and 256Hz */

static void dmg_run_frame_sequencer(struct gameboy_dmg *dmg) {
    for (int i=0; i<8; i++) {
        switch (i) {
        case 0:
        case 4:
            /* clock all length counters */
            dmg_length_clock(&dmg->square1.length, &dmg->square1.enabled);
            dmg_length_clock(&dmg->square2.length, &dmg->square2.enabled);
            dmg_length_clock(&dmg->wave.length,    &dmg->wave.enabled);
            dmg_length_clock(&dmg->noise.length,   &dmg->noise.enabled);
            break;
        case 2:
        case 6:
            /* clock all length counters */
            dmg_length_clock(&dmg->square1.length, &dmg->square1.enabled);
            dmg_length_clock(&dmg->square2.length, &dmg->square2.enabled);
            dmg_length_clock(&dmg->wave.length,    &dmg->wave.enabled);
            dmg_length_clock(&dmg->noise.length,   &dmg->noise.enabled);
            /* clock sweep counter */
            dmg_sweep_clock(&dmg->square1);
            break;
        case 7:
            /* clock all envelope counters */
            dmg_envelope_cock(&dmg->square1.volume);
            dmg_envelope_cock(&dmg->square2.volume);
            dmg_envelope_cock(&dmg->noise.volume);
            break;
        case 1:
        case 3:
        case 5:
        default:
            /* nothing */
            break;
        }
    }
}

static void write_dmg_register(struct gameboy_dmg *dmg, uint8_t a, uint8_t d) {
    struct dmg_square *s = &dmg->square1;
    struct dmg_wave   *w = &dmg->wave;
    struct dmg_noise  *n = &dmg->noise;
    struct dmg_ctrl   *c = &dmg->ctrl;

    if (a >= 0x30) return;

    if (a >= 5 && a <= 9) s = &dmg->square2;

    switch(a) {
        /* squares */
    case 0x00:
    case 0x05:
        s->sweep_shift  = d & 7;
        s->sweep_negate = d & 8;
        s->sweep_period_load = (d>>4) & 7;
        break;
    case 0x01:
    case 0x06:
        s->length.load = d & 0x3f;
        s->length.counter = 64 - s->length.load;
        s->duty = (d>>6) & 3;
        break;
    case 0x02:
    case 0x07:
        s->volume.value = s->volume.load = (d>>4) & 15;
        s->volume.envelope_add_mode = d & 8;
        s->volume.envelope_period = s->volume.envelope_period_load = d & 7;
        break;
    case 0x03:
    case 0x08:
        s->frequency_load = (s->frequency_load & 0x700) | d;
        s->frequency = (2048 - s->frequency_load) * 4;
        break;
    case 0x04:
    case 0x09:
        s->frequency_load = (s->frequency_load & 0x0ff) | ((d & 7) << 8);
        s->frequency = (2048 - s->frequency_load) * 4;
        s->length.enable = d & 0x40;
        s->trigger = d & 0x80;
        if (s->trigger) {
            s->enabled = true;
            if (!s->length.counter)
                s->length.counter = 64 - s->length.load;
            s->frequency = (2048 - s->frequency_load) * 4;
            s->volume.envelope_running = true;
            s->volume.envelope_period = s->volume.envelope_period_load;
            s->volume.value = s->volume.load;
            s->sweep_shadow = s->frequency_load;
            s->sweep_period = s->sweep_period_load ? s->sweep_period_load : 8;
                /* XXX sweep_period>0 always true */
            s->sweep_enable = s->sweep_period > 0 || s->sweep_shift > 0;
            if (s->sweep_shift > 0)
                dmg_sweep_calculation(s); // discard newf, only check overflow
        }
        break;
        /* wave */
    case 0x0a:
        w->dac_power = d & 0x80;
        break;
    case 0x0b:
        w->length.load = d;
        w->length.counter = 256 - d;
        break;
    case 0x0c:
        w->volume_code = (d>>5) & 3;
        break;
    case 0x0d:
        w->frequency_load = (w->frequency_load & 0x700) | d;
        w->frequency = (2048 - w->frequency_load) * 2;
        break;
    case 0x0e:
        w->frequency_load = (w->frequency_load & 0x0ff) | ((d & 7) << 8);
        w->frequency = (2048 - w->frequency_load) * 2;
        w->length.enable = d & 0x40;
        w->trigger = d & 0x80;
        if (w->trigger) {
            w->enabled = true;
            if (!w->length.counter)
                w->length.counter = 256 - w->length.load;
            w->frequency = (2048 - w->frequency_load) * 2;
        }
        break;
        /* noise */
    case 0x0f:
        /* unused */
        break;
    case 0x10:
        n->length.load = d & 0x3f;
        n->length.counter = 64 - n->length.load;
        break;
    case 0x11:
        n->volume.load = (d>>4) & 15;
        n->volume.envelope_add_mode = d & 8;
        n->volume.envelope_period = n->volume.envelope_period_load = d & 7;
        n->volume.value = n->volume.load;
        break;
    case 0x12:
        n->divisor_code = d & 7;
        n->width_mode = d & 8;
        n->clock_shift = (d>>4) & 15;
        break;
    case 0x13:
        n->length.enable = d & 0x40;
        n->trigger = d & 0x80;
        if (n->trigger) {
            n->enabled = true;
            if (!n->length.counter)
                n->length.counter = 64 - n->length.load;
            n->frequency = dmg_divisors[n->divisor_code] << n->clock_shift;
            n->volume.envelope_period = n->volume.envelope_period_load;
            n->volume.envelope_running = true;
            n->volume.value = n->volume.load;
        }
        break;
        /* control/status */
    case 0x14:
        c->right_volume = d & 7;
        c->vin_right_enable = d & 8;
        c->left_volume = (d>>4) & 7;
        c->vin_left_enable = d & 0x80;
        break;
    case 0x15:
        for (int i=0; i<4; i++) {
            c->right_enables[i] = (d>>i) & 1;
            c->left_enables[i] = (d>>(i+4)) & 1;
        }
        break;
    case 0x16:
        break;

        /* unused */
    case 0x17:
    case 0x18:
    case 0x19:
    case 0x1a:
    case 0x1b:
    case 0x1c:
    case 0x1d:
    case 0x1e:
    case 0x1f:
        break;

        /* wave table */
    default:
        break;
    }
}

/* ------------------------------------------------------------------------ */

static bool write_sapr_header(FILE *file) {
    if (fprintf(file, "SAP\r\nAUTHOR \"\"\r\nNAME \"\"\r\nDATE \"\"\r\nTYPE R\r\n\r\n") < 0) {
        fprintf(stderr, "ERROR: error writing to file\n");
        return false;
    }
    return true;
}

/* ------------------------------------------------------------------------ */

static int write_sapr(gzFile file, struct vgm_header *v, enum chiptype chip) {
    uint8_t pokeyL[9], pokeyR[9];
    FILE *outleft, *outright;

    memset(pokeyL, 0, 9);
    memset(pokeyR, 0, 9);
    if (!single_pokey && chip != CHIP_HUC6280)
        pokeyL[8] = pokeyR[8] = 0x78;

    outleft = fopen("left.sapr", "wb");
    if (!outleft) {
        fprintf(stderr, "ERROR: unable to open left.sapr for writing\n");
        return -1;
    }

    if (!single_pokey) {
        outright = fopen("right.sapr", "wb");
        if (!outright) {
            fprintf(stderr, "ERROR: unable to open right.sapr for writing\n");
            return -1;
        }

        fprintf(stdout, "writing SAP-R file to left.sapr and right.sapr\n");
    } else {
        fprintf(stdout, "writing SAP-R file to left.sapr \n");
    }

    if (!write_sapr_header(outleft)) return -1;
    if (!single_pokey && !write_sapr_header(outright)) return -1;

    uint32_t scnt = 0, nframes = 0;
    double fcnt = 0;
    double framelen = 44100 / framerate;

    int unknown = 0;
    int run = 1;
    int synced = 0;

    /* only one of them is used at the same time. multi chip files are NOT
     * supported, but we reuse the same timing/synchronization loop */

    union sn76489 sn;
    struct gameboy_dmg dmg;
    struct huc6280 huc;

    memset(&sn, 0, sizeof(union sn76489));
    memset(&dmg, 0, sizeof(struct gameboy_dmg));
    memset(&huc, 0, sizeof(struct huc6280));

    uint8_t written[LARGEST_CHIP];
    memset(written, 0, sizeof(written));

    while (run) {
        int wait = 0;

        int c = gzgetc(file);

        switch (c) {
        case -1:
            fprintf(stderr, "premature end of file\n");
            return 1;
        case 0x67:
            gzgetc(file);
            int tt = gzgetc(file);
            uint32_t ss;
            gzfread((uint8_t *)&ss, 1, 4, file);
            ss = readLE32((uint8_t *)&ss);
            fprintf(stdout, "skipping data block type 0x%02x, size %08x\n", tt, ss);
            gzseek(file, ss, SEEK_CUR);
            break;
        case 0x31:      /* AY8910 */
        case 0x4f:      /* GG PSG */
            gzseek(file, 1, SEEK_CUR);
            break;      /* stereo mask ignored */

            /* SN76489 -- 0x50 dd */
        case 0x50: {
            uint8_t val = gzgetc(file);
            static uint8_t reg = 0;
            uint8_t dat;
            bool changed = false;

            if (val & 0x80) {
                reg = (val >> 4) & 0x07;

                if ((sn.r[reg] & 0x0f) != (val & 0x0f))
                    changed = true;

                switch (reg) {
                case REG_FREQ1:
                case REG_FREQ2:
                case REG_FREQ3:
                    if ((sn.r[reg] & 0x0f) != (val & 0x0f))
                        changed = true;
                    sn.r[reg] &= 0x03f0;
                    sn.r[reg] |= val & 0x0f;
                    break;
                case REG_ATT1:
                case REG_ATT2:
                case REG_ATT3:
                case REG_CTRL:
                case REG_ATTN:
                    sn.r[reg] = val & 0x0f;
                    break;
                }
            } else {
                if ((sn.r[reg] >> 4) != (val & 0x3f))
                    changed = true;

                sn.r[reg] &= 0x000f;
                sn.r[reg] |= (val & 0x3f) << 4;

                if (changed)
                    reg += 8;
            }

            if (changed) {
                if (!written[reg]) {
                    written[reg]++;
                } else {
                    if (force_new) {
                        debug_fprintf(stdout, "*** new frame forced\n");
                        gzseek(file, -3, SEEK_CUR);
                        fcnt=framelen;              // force new frame
                        break;
                    }
                    written[reg]++;
                }
            }

            break;
            }

            /* GAMEBOY DMG -- 0xb3 aa dd */
        case 0xb3: {
            uint8_t aa = gzgetc(file);
            uint8_t dd = gzgetc(file);

            if (aa >= 0x30) {
                fprintf(stderr, "ERROR: gameboy_dmg: aa=%02x out of range!\n", aa);
                break;
            }

//DEBUG            fprintf(stderr, "aa=%02x dd=%02x\n", aa, dd);

            if (!written[aa]) {
                written[aa]++;
            } else {
                if (force_new) {
                    debug_fprintf(stdout, "*** new frame forced\n");
                    gzseek(file, -3, SEEK_CUR);
                    fcnt=framelen;              // force new frame
                    break;
                }
                written[aa]++;
            }
            write_dmg_register(&dmg, aa, dd);
            break;
            }
            /* HuC6280 */
        case 0xb9: {
            uint8_t aa = gzgetc(file);
            uint8_t dd = gzgetc(file);

            if (aa > 9) {
                fprintf(stderr, "ERROR: huc6280: aa=%02x out of range!\n", aa);
                break;
            }

            if (aa == 0) {
                huc.channel_select = dd;
                break;
            }

            int channel = huc.channel_select;
            struct huc_channel *c = &huc.channels[channel];

            switch (aa) {
            case 0:
                break;
            case 1:
                huc.left_main_amplitude_level  = dd >> 4;
                huc.right_main_amplitude_level = dd & 15;
                break;
            case 2:
                c->frequency = (c->frequency & 0xf00) | dd;
                break;
            case 3:
                c->frequency = (c->frequency & 0x0ff) | ((dd & 0x0f) << 8);
                break;
            case 4:
                c->on  = dd & 0x80;
                c->dda = dd & 0x40;
                c->amplitude_level = dd & 0x1f;
                break;
            case 5:
                c->left_amplitude_level  = dd >> 4;
                c->right_amplitude_level = dd & 15;
                break;
            case 6:
                /* we do not store wave data written here */
                break;
            case 7:
                c->noise_enable    = dd & 0x80;
                c->noise_frequency = dd & 0x1f;
                break;
            case 8:
                huc.lfo_frq = dd;
                break;
            case 9:
                huc.lf_trg = dd & 0x80;
                huc.lf_ctl = dd & 3;
                break;
            }

            int bb = channel * 10 + aa;
            if (!written[bb]) {
                if (aa != 7)   //XXX fix, 6?  // do not count wave data writes
                    written[bb]++;
            } else {
                if (force_new) {
                    debug_fprintf(stdout, "*** new frame forced\n");
                    gzseek(file, -3, SEEK_CUR);
                    fcnt=framelen;              // force new frame
                    break;
                }
                written[bb]++;
            }
            break;
            }
            /* UNSUPPORTED -- xx aa dd */
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
        case 0x58:
        case 0x59:
        case 0x5a:
        case 0x5b:
        case 0x5c:
        case 0x5d:
        case 0x5e:
        case 0x5f:
        case 0xa0:
        case 0xb0:
        case 0xb1:
        case 0xb2:
        case 0xb4:
        case 0xb5:
        case 0xb6:
        case 0xb7:
        case 0xb8:
        case 0xba:
        case 0xbb:
        case 0xbc:
        case 0xbd:
        case 0xbe:
        case 0xbf:
            if (!unknown) {
                fprintf(stderr, "ERROR: skipping unknown chipset\n");
                unknown = 1;
            }
            gzseek(file, 2, SEEK_CUR);
            break;

            /* UNSUPPORTED -- xx + 3 bytes */
        case 0xc0:
        case 0xc1:
        case 0xc2:
        case 0xc3:
        case 0xc4:
        case 0xc5:
        case 0xc6:
        case 0xc7:
        case 0xd0:
        case 0xd1:
        case 0xd2:
        case 0xd3:
        case 0xd4:
        case 0xd5:
        case 0xd6:
            if (!unknown) {
                fprintf(stderr, "ERROR: skipping unknown chipset\n");
                unknown = 1;
            }
            gzseek(file, 3, SEEK_CUR);
            break;

        case 0x61:
            wait = gzgetc(file);
            wait += gzgetc(file) << 8;
            break;
        case 0x62:
            wait = 735;
            break;
        case 0x63:
            wait = 882;
            break;
        case 0x66:
            fprintf(stdout, "end of sound data\n");
            run = 0;
            break;
        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0x77:
        case 0x78:
        case 0x79:
        case 0x7a:
        case 0x7b:
        case 0x7c:
        case 0x7d:
        case 0x7e:
        case 0x7f:
            wait = (c & 0xf) + 1;
            break;
        case 0x80:  // YM2612 write + wait, we only wait
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
        case 0x86:
        case 0x87:
        case 0x88:
        case 0x89:
        case 0x8a:
        case 0x8b:
        case 0x8c:
        case 0x8d:
        case 0x8e:
        case 0x8f:
            wait = (c & 0xf);
            break;

            /* UNSUPPORTED -- xx + 4 bytes */
        case 0xe0:
        case 0xe1:
            gzseek(file, 4, SEEK_CUR);
            break;
        default:
            fprintf(stderr, "ERROR: unhandled data command %02x\n", c);
            return 0;
        }

        scnt += wait;

        if (!synced && wait > 400) {
            fcnt = framelen;
        } else {
            fcnt += wait;
        }

        while (fcnt >= framelen) {
            nframes++;

            if (chip == CHIP_SN76489) {
                if (single_pokey) {
                    sn_to_single_pokey(&sn, &pokeyL[0], 0, v);
                    sn_to_single_pokey(&sn, &pokeyL[2], 1, v);
                    sn_to_single_pokey(&sn, &pokeyL[4], 2, v);
                } else {
                    sn_to_pokey(&sn, &pokeyL[0], 0, v);
                    sn_to_pokey(&sn, &pokeyL[4], 1, v);
                    sn_to_pokey(&sn, &pokeyR[0], 2, v);
                }
            } else if (chip == CHIP_GAMEBOY_DMG) {
                int newwave = 0;
                for (int x=0; x<16; x++)
                    newwave += written[0x20+x];
                if (newwave)
                    fprintf(stdout, "new wave written to wavetable\n");

                if (single_pokey) {
                    dmg_to_single_pokey(&dmg, &pokeyL[0], 0, v);
                    dmg_to_single_pokey(&dmg, &pokeyL[2], 1, v);
                    dmg_to_single_pokey(&dmg, &pokeyL[4], 2, v);
                    dmg_to_single_pokey(&dmg, &pokeyL[6], 3, v);
                } else {
                    dmg_to_pokey(&dmg, &pokeyL[0], 0, v);
                    dmg_to_pokey(&dmg, &pokeyL[4], 1, v);
                    dmg_to_pokey(&dmg, &pokeyR[0], 2, v);
                    dmg_to_pokey(&dmg, &pokeyR[4], 3, v);
                }

                dmg_run_frame_sequencer(&dmg);
            } else if (chip == CHIP_HUC6280) {
                huc_to_pokey(&huc, &pokeyL[0], 0, v);
                huc_to_pokey(&huc, &pokeyL[2], 1, v);
                huc_to_pokey(&huc, &pokeyL[4], 2, v);
                huc_to_pokey(&huc, &pokeyR[0], 3, v);
                huc_to_pokey(&huc, &pokeyR[2], 4, v);
                huc_to_pokey(&huc, &pokeyR[4], 5, v);
            }

            fwrite(pokeyL, 1, 9, outleft);
            if (!single_pokey)
                fwrite(pokeyR, 1, 9, outright);

            fcnt -= framelen;
            for (int i = 0; i<16; i++) {
                if (synced && written[i]>1)
                    debug_fprintf(stdout, "[%d] reg 0x%02x written to %d times\n", scnt, i, written[i]);
            }
            memset(written, 0, sizeof(written));
            synced = 1;
        }
    }

    nframes++;

    if (chip == CHIP_SN76489) {
        if (single_pokey) {
            sn_to_single_pokey(&sn, &pokeyL[0], 0, v);
            sn_to_single_pokey(&sn, &pokeyL[2], 1, v);
            sn_to_single_pokey(&sn, &pokeyL[4], 2, v);
        } else {
            sn_to_pokey(&sn, &pokeyL[0], 0, v);
            sn_to_pokey(&sn, &pokeyL[4], 1, v);
            sn_to_pokey(&sn, &pokeyR[0], 2, v);
        }
    } else if (chip == CHIP_GAMEBOY_DMG) {
        if (single_pokey) {
            dmg_to_single_pokey(&dmg, &pokeyL[0], 0, v);
            dmg_to_single_pokey(&dmg, &pokeyL[2], 1, v);
            dmg_to_single_pokey(&dmg, &pokeyL[4], 2, v);
            dmg_to_single_pokey(&dmg, &pokeyL[6], 3, v);
        } else {
            dmg_to_pokey(&dmg, &pokeyL[0], 0, v);
            dmg_to_pokey(&dmg, &pokeyL[4], 1, v);
            dmg_to_pokey(&dmg, &pokeyR[0], 2, v);
            dmg_to_pokey(&dmg, &pokeyR[4], 3, v);
        }
    } else if (chip == CHIP_HUC6280) {
        huc_to_pokey(&huc, &pokeyL[0], 0, v);
        huc_to_pokey(&huc, &pokeyL[2], 1, v);
        huc_to_pokey(&huc, &pokeyL[4], 2, v);
        huc_to_pokey(&huc, &pokeyR[0], 3, v);
        huc_to_pokey(&huc, &pokeyR[2], 4, v);
        huc_to_pokey(&huc, &pokeyR[4], 5, v);
    }

    fwrite(pokeyL, 1, 9, outleft);
    if (!single_pokey)
        fwrite(pokeyR, 1, 9, outright);

    fclose(outleft);
    if (!single_pokey)
        fclose(outright);

    return 0;
}

/* ------------------------------------------------------------------------ */

/* note that 0 is the loudest, and 15 is silent */

static void init_voltab_sn(int maxvol) {
    // fifteen steps of -2dB (-2/20 = -0.1)
    for (int i=0; i<16; i++) {
        voltab[i] = round(maxvol * pow(10.0, -0.1 * i));
    }
}

static void init_voltab_dmg(int maxvol) {
    for (int i=0; i<16; i++) {
        voltab[i] = round(i*maxvol/15.0);
    }
}

static void init_voltab_huc(int maxvol) {
    double level = maxvol;
    double step = 48.0 / 16.0;          // 48dB in sixteen steps
    for (int i=0; i<16; i++) {
        voltab[15-i] = round(level);
        level /= pow(10.0, step/20.0);
    }
}

/* ------------------------------------------------------------------------ */

static void usage(void) {
    fprintf(stderr,
"usage: vgm2sapr [-options] file.vgm\n"
"\n"
"   -r rate     specify framerate (i.e. 50, 59.94, ...)\n"
"   -d          show debug output to analyze bad conversions\n"
"               try to reduce the number of bad writes by setting\n"
"               the proper framerate or slightly below, or try -f\n"
"   -f          force new frame on double write\n"
"   -p volume   pokey maximum per channel volume [default: 15]\n"
"   -m div      mute high up to pokey divisor div [default: 0]\n"
"   -s          single Pokey output (SN76489 and DMG only) [default: off]\n"
"   -h vol      HuC6280 master volume (0.0-4.0) [default: 1.0]\n"
);

}

/* ------------------------------------------------------------------------ */

int main(int argc, char **argv) {
    int option;

    framerate = 0;

    while ((option = getopt(argc, argv, "dfr:p:m:sh:")) != -1) {
        switch (option) {
        case 'r':
            framerate = strtod(optarg, NULL);
            break;
        case 'd':
            debug = 1;
            break;
        case 'f':
            force_new = 1;
            break;
        case 'p':
            maxpokvol = atoi(optarg);
            if (maxpokvol < 0 || maxpokvol > 15) {
                fprintf(stderr, "ERROR: invalid maximum pokey volume\n");
                return 1;
            }
            break;
        case 'm':
            mute_high = atoi(optarg);
            if (mute_high < 0)
                mute_high = 0;
            if (mute_high > 255)
                mute_high = 255;
            break;
        case 's':
            single_pokey = true;
            break;
        case 'h':
            huc_master_volume = strtod(optarg, NULL);
            if (huc_master_volume < 0.0) huc_master_volume = 0.0;
            if (huc_master_volume > 4.0) huc_master_volume = 4.0;
            break;
        default:
            usage();
            return 1;
        }
    }

    if (optind+1 != argc) {
        fprintf(stderr, "ERROR: wrong arguments\n");
        usage();
        return 1;
    }

    gzFile file;
    file = gzopen(argv[optind], "r");
    if (!file) {
        fprintf(stderr, "ERROR: unable to open %s\n", argv[optind]);
        return 1;
    }

    read_header(file);

    struct vgm_header *v = &vgm_header;

    if (strncmp(v->ident, "Vgm ", 4)) {
        fprintf(stderr, "ERROR: Vgm header not found\n");
        return 1;
    }

    fprintf(stdout, "vgm version: %1x.%2x\n", v->version>>8, v->version &0xff);

    uint32_t data_offset = 0x40;
    if (v->version >= 0x150)
        data_offset = 0x34 + v->vgm_data_offset;

    fprintf(stdout, "data offset: 0x%02x\n", data_offset);

    gzseek(file, data_offset, SEEK_SET);

    if (framerate == 0.0) {
        if ((v->version >= 0x101 && !v->rate) || (v->version < 0x101)) {
            fprintf(stderr, "ERROR: file does not specify framerate. use -r option\n");
            return 1;
        } 
        framerate = v->rate;
    }

    fprintf(stdout, "framerate: %.2f Hz\n", framerate);

    fprintf(stdout, "total number of samples: %d\n", v->total_nsamples);
    fprintf(stdout, "samples per frame: %.2f\n", 44100.0/framerate);
    fprintf(stdout, "song length: %.2f seconds\n", v->total_nsamples/44100.0);
    fprintf(stdout, "maximum pokey volume: %d\n", maxpokvol);
    if (mute_high)
        fprintf(stdout, "mute clear pokey div up to: %d\n", mute_high);

    if (v->sn76489_clock) {
        init_voltab_sn(maxpokvol);

        fprintf(stdout, "detected SN76489\n");
        fprintf(stdout, "clock: %d Hz\n", v->sn76489_clock);
        if (v->version < 0x0110)
            v->sn76489_shift_width = 16;
        fprintf(stdout, "shift register width: %d bits\n", v->sn76489_shift_width);
        if (single_pokey) fprintf(stdout, "single pokey output enabled\n");
        write_sapr(file, v, CHIP_SN76489);
    } else if (v->version >= 0x0161 && v->gameboy_dmg_clock) {
        init_voltab_dmg(maxpokvol);

        fprintf(stdout, "detected GameBoy DMG\n");
        fprintf(stdout, "clock: %d Hz\n", v->gameboy_dmg_clock);
        if (single_pokey) fprintf(stdout, "single pokey output enabled\n");
        write_sapr(file, v, CHIP_GAMEBOY_DMG);
    } else if (v->version >= 0x0161 && v->huc6280_clock) {
        init_voltab_huc(maxpokvol);

        fprintf(stdout, "detected HuC6280\n");
        fprintf(stdout, "clock: %d Hz\n", v->huc6280_clock);
        fprintf(stdout, "huc6280 master volume: %f\n", huc_master_volume);
        if (single_pokey) goto unsupported;
        write_sapr(file, v, CHIP_HUC6280);
    } else {
        fprintf(stderr, "no supported chip detected\n");
        return 1;
    }

    fprintf(stdout, "finished!\n");

    return 0;

unsupported:
    fprintf(stdout, "single pokey not supported for selected soundchip\n");
    return 1;
}
