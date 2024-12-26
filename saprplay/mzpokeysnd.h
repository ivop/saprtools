#pragma once
#include <stdint.h>

typedef int8_t   SBYTE;
typedef uint8_t  UBYTE;
typedef int16_t  SWORD;
typedef uint16_t UWORD;
typedef int32_t  SLONG;
#ifndef ULONG
typedef uint32_t ULONG;
#endif

extern int POKEYSND_volume;
extern UBYTE *POKEYSND_process_buffer;
extern unsigned int POKEYSND_process_buffer_length;
extern unsigned int POKEYSND_process_buffer_fill;
extern int POKEYSND_snd_flags;

extern void (*POKEYSND_Update_ptr)(UWORD addr, UBYTE val, UBYTE chip, UBYTE gain);
extern void (*POKEYSND_Process_ptr)(void *sndbuffer, int sndn);
int MZPOKEYSND_Init(ULONG freq17, int playback_freq, UBYTE num_pokeys,
                        int flags, int quality);

#define POKEY_OFFSET_AUDF1 0x00
#define POKEY_OFFSET_AUDC1 0x01
#define POKEY_OFFSET_AUDF2 0x02
#define POKEY_OFFSET_AUDC2 0x03
#define POKEY_OFFSET_AUDF3 0x04
#define POKEY_OFFSET_AUDC3 0x05
#define POKEY_OFFSET_AUDF4 0x06
#define POKEY_OFFSET_AUDC4 0x07
#define POKEY_OFFSET_AUDCTL 0x08
#define POKEY_OFFSET_STIMER 0x09
#define POKEY_OFFSET_SKRES 0x0a
#define POKEY_OFFSET_POTGO 0x0b
#define POKEY_OFFSET_SEROUT 0x0d
#define POKEY_OFFSET_IRQEN 0x0e
#define POKEY_OFFSET_SKCTL 0x0f

#define POKEY_OFFSET_POT0 0x00
#define POKEY_OFFSET_POT1 0x01
#define POKEY_OFFSET_POT2 0x02
#define POKEY_OFFSET_POT3 0x03
#define POKEY_OFFSET_POT4 0x04
#define POKEY_OFFSET_POT5 0x05
#define POKEY_OFFSET_POT6 0x06
#define POKEY_OFFSET_POT7 0x07
#define POKEY_OFFSET_ALLPOT 0x08
#define POKEY_OFFSET_KBCODE 0x09
#define POKEY_OFFSET_RANDOM 0x0a
#define POKEY_OFFSET_SERIN 0x0d
#define POKEY_OFFSET_IRQST 0x0e
#define POKEY_OFFSET_SKSTAT 0x0f

#define POKEY_OFFSET_POKEY2 0x10

#define POKEYSND_BIT16  1

#ifdef __GNUC__
    #define UNUSED          __attribute__((unused))
    #define unreachable()   __builtin_unreachable()
#else
    #include <assert.h>
    #define UNUSED
    #define unreachable()   assert(false)
#endif