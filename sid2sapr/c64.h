#ifndef C64_H
#define C64_H

#include <stdint.h>

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

struct sidosc {
    uint32_t pulse;
    uint8_t wave;
    uint32_t attack;
    uint32_t decay;
    uint32_t sustain;
    uint32_t release;
    signed int envval;
    uint8_t envphase;
};

extern struct s6581 sid;
extern struct sidosc osc[3];

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
