#ifndef C64_H
#define C64_H

#include <stdint.h>
#include <stdbool.h>

struct sid_registers {
    uint16_t freq;
    uint16_t pulse;
    uint8_t wave;
    uint8_t ad;
    uint8_t sr;
};

struct sid_voice {
    bool gate;
    bool sync;
    bool ringmod;
    bool test;
    bool tri;
    bool saw;
    bool pul;
    bool noise;
    bool filter;
    bool off;

    double attack;
    double decay;
    double sustain;
    double release;
    double envval;
    uint8_t envphase;
};

struct sid {
    struct sid_registers r[3];
    struct sid_voice v[3];
    uint16_t ffreq;
    uint8_t res_filt;
    uint8_t mode_vol;

    bool lowpass;
    bool bandpass;
    bool highpass;
};

extern struct sid sid;

extern void c64_init(void);
extern int  c64_cpu_jsr(uint16_t npc, uint8_t na);
extern void c64_sid_init(uint32_t mixfrq);
extern void c64_handle_adsr (uint32_t len);

extern uint16_t c64_load_sid(char *filename,
                    uint16_t *initAddress, uint16_t *playAddress,
                    uint16_t *songs, uint16_t *startSong,
                    uint32_t *speed, char *name, char *author,
                    char *copyright);

#endif
