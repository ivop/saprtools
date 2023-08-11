#ifndef SIDENGINE_H
#define SIDENGINE_H

#include <stdint.h>

// SID register definition

struct s6581 {
    struct sidvoice {
        uint16_t freq;
        uint16_t pulse;
        uint8_t wave;
        uint8_t ad;
        uint8_t sr;
    } v[3];
    uint8_t ffreqlo;
    uint8_t ffreqhi;
    uint8_t res_ftv;
    uint8_t ftp_vol;
};

// internal oscillator def

struct sidosc {
    uint32_t freq;
    uint32_t pulse;
    uint8_t wave;
    uint8_t filter;
    uint32_t attack;
    uint32_t decay;
    uint32_t sustain;
    uint32_t release;
    uint32_t counter;           // Variablen
    signed int envval;
    uint8_t envphase;
    uint32_t noisepos;
    uint32_t noiseval;
    uint8_t noiseout;
};

// internal filter def

struct sidflt {
    int freq;
    uint8_t l_ena;
    uint8_t b_ena;
    uint8_t h_ena;
    uint8_t v3ena;
    int vol;
    int rez;
    int h;
    int b;
    int l;
};

// expose sid internals for sid2sapr
//
extern struct s6581 sid;
extern struct sidosc osc[3];

extern void synth_render (uint16_t *buffer, uint32_t len);
extern void c64Init(void);
extern uint16_t c64SidLoad(char *filename, uint16_t *init_addr, uint16_t *play_addr, uint8_t *sub_song_start, uint8_t *max_sub_songs, uint8_t *speed, char *name, char *author, char *copyright);
extern int cpuJSR(uint16_t npc, uint8_t na);
extern void synth_init(uint32_t mixfrq);

#endif
