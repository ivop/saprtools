/* Bare minimum C64 - by Ivo van Poorten - Copyright (C) 2023
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
/* ADSR code Copyright (C) 1999-2006 T. Hinrichs and R. Sinsch.
 */
/* M6502 emulator Copyright (c) 2018 Andre Weissflog.
 * See m6502.h for zlib license
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "c64.h"

#define CHIPS_IMPL
#include "m6502.h"

// --------------------------------------------------------------------------
// FIXED POINT HELPERS  (16.16 signed)
//
static inline int fixed_from_float(float f) {
    return (int)(f * (1 << 16));
}

// --------------------------------------------------------------------------
// CONSTANTS
//
static const float attack_times[16] = {
    0.0022528606, 0.0080099577, 0.0157696042, 0.0237795619, 0.0372963655,
    0.0550684591, 0.0668330845, 0.0783473987, 0.0981219818, 0.244554021,
    0.489108042, 0.782472742, 0.977715461, 2.93364701, 4.88907793, 7.82272493
};

static const float decay_release_times[16] = {
    0.00891777693, 0.024594051, 0.0484185907, 0.0730116639, 0.114512475,
    0.169078356, 0.205199432, 0.240551975, 0.301266125, 0.750858245,
    1.50171551, 2.40243682, 3.00189298, 9.00721405, 15.010998, 24.0182111
};

// --------------------------------------------------------------------------
// CALCULATED "CONSTANTS"
//
static int attacks[16];
static int releases[16];

// --------------------------------------------------------------------------
// OUR "CHIPS"
//
struct s6581 sid;
struct sidosc osc[3];
static m6502_t cpu;
static uint64_t pins;
static uint8_t memory[65536];

// --------------------------------------------------------------------------
// INIT SID
//
void c64_sid_init(uint32_t mixfrq) {
    int i;

    for (i = 0; i < 16; i++) {
        attacks[i] = (int)(0x1000000 / (attack_times[i] * mixfrq));
        releases[i] = (int)(0x1000000 / (decay_release_times[i] * mixfrq));
    }
    memset(&sid, 0, sizeof(sid));
    memset(osc, 0, sizeof(osc));
}

void c64_handle_adsr(uint32_t len) {
    uint8_t v;

    for (v = 0; v < 3; v++) {
        osc[v].pulse = (sid.v[v].pulse & 0xfff) << 16;
        osc[v].attack = attacks[sid.v[v].ad >> 4];
        osc[v].decay = releases[sid.v[v].ad & 0xf];
        osc[v].sustain = sid.v[v].sr & 0xf0;
        osc[v].release = releases[sid.v[v].sr & 0xf];
        osc[v].wave = sid.v[v].wave;
    }

    for (int scnt=0; scnt<len; scnt++) {            // sample counter

        for (v = 0; v < 3; v++) {       // handle ADSR for each voice

            if (!(osc[v].wave & 0x01))  // not gate, release
                osc[v].envphase = 3;
            else if (osc[v].envphase == 3) {
                osc[v].envphase = 0;
            }
            switch (osc[v].envphase) {
            case 0:{           // Phase 0 : Attack
                osc[v].envval += osc[v].attack;
                if (osc[v].envval >= 0xFFFFFF) {
                    osc[v].envval = 0xFFFFFF;
                    osc[v].envphase = 1;
                }
                break;
            }
            case 1:{           // Phase 1 : Decay
                osc[v].envval -= osc[v].decay;
                if ((signed int)osc[v].envval <=
                    (signed int)(osc[v].sustain << 16)) {
                    osc[v].envval = osc[v].sustain << 16;
                    osc[v].envphase = 2;
                }
                break;
            }
            case 2:{           // Phase 2 : Sustain
                if ((signed int)osc[v].envval !=
                    (signed int)(osc[v].sustain << 16)) {
                    osc[v].envphase = 1;
                }
                break;
            }
            case 3:{           // Phase 3 : _release
                osc[v].envval -= osc[v].release;
                if (osc[v].envval < 0x40000)
                    osc[v].envval = 0x40000;
                break; 
            }
            }       // switch envphase

        }           // ADSR for each voice loop

    }               // sample counter loop
}

// --------------------------------------------------------------------------
// POKE SID REGISTERS
//
static void sidPoke(int reg, unsigned char val) {
    int voice = 0;

    if ((reg >= 7) && (reg <= 13)) {
        voice = 1;
        reg -= 7;
    }
    if ((reg >= 14) && (reg <= 20)) {
        voice = 2;
        reg -= 14;
    }

    switch (reg) {
    case 0: sid.v[voice].freq = (sid.v[voice].freq & 0xff00) + val; break;
    case 1: sid.v[voice].freq = (sid.v[voice].freq & 0xff) + (val<<8); break;
    case 2: sid.v[voice].pulse = (sid.v[voice].pulse & 0xff00) + val; break;
    case 3: sid.v[voice].pulse = (sid.v[voice].pulse & 0xff) + (val<<8); break;
    case 4: sid.v[voice].wave = val; break;
    case 5: sid.v[voice].ad   = val; break;
    case 6: sid.v[voice].sr   = val; break;

    case 21: sid.ffreqlo = val; break;
    case 22: sid.ffreqhi = val; break;
    case 23: sid.res_ftv = val; break;
    case 24: sid.ftp_vol = val; break;
    }
    return;
}

/* ------------------------------------------------------------------------- */

static uint8_t getmem(uint16_t addr) {
    if (addr == 0xdd0d)
        memory[addr] = 0;
    return memory[addr];
}

static void setmem(uint16_t addr, uint8_t value) {
    memory[addr] = value;
    if ((addr & 0xfc00) == 0xd400) {
        sidPoke(addr & 0x1f, value);
    }
}

int c64_cpu_jsr(uint16_t newpc, uint8_t newa) {
    int ccl=0;
    memory[0xff00] = 0x20;            // JSR
    memory[0xff01] = newpc & 0xff;    // LSB
    memory[0xff02] = newpc >> 8;      // MSB

    pins = M6502_SYNC;
    M6502_SET_ADDR(pins, 0xff00);
    M6502_SET_DATA(pins, memory[0xff00]);
    m6502_set_pc(&cpu, 0xff00);
    m6502_set_a(&cpu, newa);

    while (m6502_pc(&cpu) != 0xff03) {
        pins = m6502_tick(&cpu, pins);
        const uint16_t addr = M6502_GET_ADDR(pins);
        if (pins & M6502_RW) {
            M6502_SET_DATA(pins, getmem(addr));
        } else {
            setmem(addr, M6502_GET_DATA(pins));
        }
    }
    return ccl;
}

void c64_init(void) {
    memset(memory, 0, 65536);
    pins = m6502_init(&cpu, &(m6502_desc_t){ });
}

/* ------------------------------------------------------------------------- */

static inline uint16_t readLE16(FILE *f) {
    uint16_t t = fgetc(f);
    return t + (fgetc(f)<<8);
}

static inline uint16_t readBE16(FILE *f) {
    uint16_t t = fgetc(f) << 8;
    return t + fgetc(f);
}

static inline uint16_t readBE32(FILE *f) {
    uint32_t t = fgetc(f) << 24;
    t += fgetc(f) << 16;
    t += fgetc(f) << 8;
    return t + fgetc(f);
}

uint16_t c64_load_sid(char *filename,
                    uint16_t *initAddress, uint16_t *playAddress,
                    uint16_t *songs, uint16_t *startSong,
                    uint32_t *speed, char *name, char *author,
                    char *copyright) {
    char magic[4];
    uint16_t version, dataOffset, loadAddress;

    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "unable to open %s\n", filename);
        return 0;
    }

    if (fread(magic, 4, 1, f) < 1) return 0;

    if (strncmp(magic+1, "SID", 3)) {
        fprintf(stderr, "not a valid SID file\n");
        return 0;
    }

    version = readBE16(f);
    fprintf(stderr, "%s version %d\n", magic, version);

     dataOffset  = readBE16(f);
     loadAddress = readBE16(f);
    *initAddress = readBE16(f);
    *playAddress = readBE16(f);
    *songs       = readBE16(f);
    *startSong   = readBE16(f);
    *speed       = readBE32(f);

    if (fread(name, 32, 1, f) < 1) return 0;
    if (fread(author, 32, 1, f) < 1) return 0;
    if (fread(copyright, 32, 1, f) < 1) return 0;

    if (fseek(f, dataOffset, SEEK_SET) < 0) return 0;

    if (!loadAddress)
        loadAddress = readLE16(f);

    int c = fgetc(f);
    uint16_t p = loadAddress;

    while (c >= 0) {
        memory[p] = c;
        p++;
        c = fgetc(f);
    }

    if (!*playAddress) {
        fprintf(stderr, "play address not set, trying interrupt vector(s)\n");
        c64_cpu_jsr(*initAddress, 0);
        if ((memory[1] & 7) == 5)
            *playAddress = memory[0xfffe] + (memory[0xffff] << 8);
        else
            *playAddress = memory[0x0314] + (memory[0x0315] << 8);
        if (!*playAddress) {
            fprintf(stderr, "interrupt vector is also not set!\n");
            return 0;
        }
    }

    return loadAddress;
}

