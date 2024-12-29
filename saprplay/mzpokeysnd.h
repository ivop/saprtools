#pragma once
#include <stdint.h>

extern int POKEYSND_volume;
extern uint8_t *POKEYSND_process_buffer;
extern unsigned int POKEYSND_process_buffer_length;
extern unsigned int POKEYSND_process_buffer_fill;
extern int POKEYSND_snd_flags;

extern void (*POKEYSND_Update_ptr)(uint16_t addr, uint8_t val, uint8_t chip, uint8_t gain);
extern void (*POKEYSND_Process_ptr)(void *sndbuffer, int sndn);
int MZPOKEYSND_Init(uint32_t freq17, int playback_freq, uint8_t num_pokeys,
                        int flags, int quality);

#define POKEY_OFFSET_AUDF1  0x00
#define POKEY_OFFSET_AUDC1  0x01
#define POKEY_OFFSET_AUDF2  0x02
#define POKEY_OFFSET_AUDC2  0x03
#define POKEY_OFFSET_AUDF3  0x04
#define POKEY_OFFSET_AUDC3  0x05
#define POKEY_OFFSET_AUDF4  0x06
#define POKEY_OFFSET_AUDC4  0x07
#define POKEY_OFFSET_AUDCTL 0x08
#define POKEY_OFFSET_STIMER 0x09
#define POKEY_OFFSET_SKRES  0x0a
#define POKEY_OFFSET_POTGO  0x0b
#define POKEY_OFFSET_SEROUT 0x0d
#define POKEY_OFFSET_IRQEN  0x0e
#define POKEY_OFFSET_SKCTL  0x0f

#define POKEYSND_BIT16  1

#ifdef __GNUC__
    #define UNUSED          __attribute__((unused))
    #define unreachable()   __builtin_unreachable()
#else
    #include <assert.h>
    #define UNUSED
    #define unreachable()   assert(false)
#endif
