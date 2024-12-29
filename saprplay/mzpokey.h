#pragma once
#include <stdint.h>

enum pokey_register {
    AUDF1, AUDC1, AUDF2, AUDC2, AUDF3, AUDC3, AUDF4, AUDC4,
    AUDCTL, STIMER, SKCTL = 15
};

void mzpokey_write_register(enum pokey_register reg, uint8_t val, uint8_t chip);
void mzpokey_process(void *sndbuffer, int sndn);
int mzpokey_init(uint32_t freq17, int playback_freq, uint8_t num_pokeys,
                    int quality);
