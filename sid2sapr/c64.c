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
/* Some lines have survived the original ADSR code after my rewrite.
 * These are Copyright (C) 1999-2006 T. Hinrichs and R. Sinsch.
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
// CALCULATED "CONSTANTS"
//
static double adr_table[16];

// Proper rate counter periods
//
static const uint32_t periods[16] = {
    9, 32, 63, 95, 149, 220, 267, 313,
    392, 977, 1954, 3126, 3907, 11720, 19532, 31251
};

enum {
    ATTACK, DECAY, SUSTAIN, RELEASE
};

// --------------------------------------------------------------------------
// OUR "CHIPS"
//
struct sid sid;
static m6502_t cpu;
static uint64_t pins;
static uint8_t memory[65536];

// --------------------------------------------------------------------------
// INIT SID
//
void c64_sid_init(uint32_t mixfreq) {
    double cycles_per_sample = 985248.444 / mixfreq;
    for (int i = 0; i < 16; i++) {
        adr_table[i] = cycles_per_sample / periods[i] / 16.0;
    }

    memset(&sid, 0, sizeof(sid));
}

void c64_handle_adsr(uint32_t len) {
    uint8_t v;

    for (int scnt=0; scnt<len; scnt++) {            // sample counter

        for (v = 0; v < 3; v++) {       // handle ADSR for each voice
            struct sid_voice *p = &sid.v[v];

            switch (p->envphase) {
            case ATTACK:
                p->envval += p->attack;
                if (p->envval >= 15.0) {
                    p->envval = 15.0;
                    p->envphase = DECAY;
                }
                break;
            case DECAY:
                p->envval -= p->decay;
                if (p->envval <= p->sustain) {
                    p->envval = p->sustain;
                    p->envphase = SUSTAIN;
                }
                break;
            case SUSTAIN:
                if (p->envval != p->sustain) {
                    p->envphase = DECAY;
                }
                break;
            case RELEASE:
                p->envval -= p->release;
                if (p->envval < 0.1)
                    p->envval = 0.1;
                break; 
            }       // switch envphase

        }           // ADSR for each voice loop

    }               // sample counter loop
}

// --------------------------------------------------------------------------
// POKE SID REGISTERS
//
static void sid_write(int reg, unsigned char val) {
    int voice = 0;

    for (; reg >= 7 && reg < 21; voice++, reg-=7) ;

    struct sid_voice     *p = &sid.v[voice];
    struct sid_registers *r = &sid.r[voice];

    switch (reg) {
    case 0: r->freq = (r->freq & 0xff00) + val; break;
    case 1: r->freq = (r->freq & 0xff) + (val<<8); break;
    case 2: r->pulse = (r->pulse & 0xff00) + val; break;
    case 3: r->pulse = (r->pulse & 0xff) + ((val&0x0f)<<8); break;
    case 4:
        r->wave = val;
        if ((val&1) != p->gate) {
            if (val&1)
                p->envphase = ATTACK;
            else
                p->envphase = RELEASE;
        }
        p->gate    = val & 0x01;
        p->sync    = val & 0x02;
        p->ringmod = val & 0x04;
        p->test    = val & 0x08;
        p->tri     = val & 0x10;
        p->saw     = val & 0x20;
        p->pul     = val & 0x40;
        p->noise   = val & 0x80;
        break;
    case 5:
        r->ad = val;
        p->attack  = adr_table[r->ad >> 4];
        p->decay   = adr_table[r->ad & 0xf];
        break;
    case 6:
        r->sr = val;
        p->sustain = (r->sr & 0xf0) >> 4;
        p->release = adr_table[r->sr & 0xf];
        break;

    case 21:
        sid.ffreq = (sid.ffreq & 0x7f8) | (val & 7);
        break;
    case 22:
        sid.ffreq = (val << 3) | (sid.ffreq & 7);
        break;
    case 23:
        sid.res_filt = val;
        sid.v[0].filter = val & 1;
        sid.v[1].filter = val & 2;
        sid.v[2].filter = val & 4;
        break;
    case 24:
        sid.mode_vol = val;
        sid.lowpass  = val & 0x10;
        sid.bandpass = val & 0x20;
        sid.highpass = val & 0x40;
        sid.v[2].off = val & 0x80;
        break;
    }
    return;
}

/* ------------------------------------------------------------------------- */

static uint8_t memory_read(uint16_t addr) {
    if (addr == 0xdd0d)
        memory[addr] = 0;
    return memory[addr];
}

static void memory_write(uint16_t addr, uint8_t value) {
    memory[addr] = value;
    if ((addr & 0xfc00) == 0xd400) {
        sid_write(addr & 0x1f, value);
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
            M6502_SET_DATA(pins, memory_read(addr));
        } else {
            memory_write(addr, M6502_GET_DATA(pins));
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
                    char *copyright, uint8_t *ntsc) {
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

    if (version > 1) {
        int c = readBE16(f);
        *ntsc = c & 0b00001000;
    }

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

    fclose(f);

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

