/* ym2sapr - by Ivo van Poorten - Copyright (C) 2023
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include "lzh.h"

/* ------------------------------------------------------------------------ */

#define GETBE16(p) (((p)[0]<<8)+((p)[1]))
#define GETBE32(p) (((p)[0]<<24)+((p)[1]<<16)+((p)[2]<<8)+((p)[3]))
#define GETLE32(p) (((p)[3]<<24)+((p)[2]<<16)+((p)[1]<<8)+((p)[0]))

#define	ATARI_ST_CLOCK		2000000L
#define ATARI_XL_CLOCK      1773447L

uint8_t volumetab[16];

// fixed point envelope handling adapted from StSoundLib
// envPos and envStep are 5.27 fixed point values, automatic wrap-around

static const int  Env00xx[8]={ 1,0,0,0,0,0,0,0 };
static const int  Env01xx[8]={ 0,1,0,0,0,0,0,0 };
static const int  Env1000[8]={ 1,0,1,0,1,0,1,0 };
static const int  Env1001[8]={ 1,0,0,0,0,0,0,0 };
static const int  Env1010[8]={ 1,0,0,1,1,0,0,1 };
static const int  Env1011[8]={ 1,0,1,1,1,1,1,1 };
static const int  Env1100[8]={ 0,1,0,1,0,1,0,1 };
static const int  Env1101[8]={ 0,1,1,1,1,1,1,1 };
static const int  Env1110[8]={ 0,1,1,0,0,1,1,0 };
static const int  Env1111[8]={ 0,1,0,0,0,0,0,0 };
static const int *EnvWave[16] = { Env00xx,Env00xx,Env00xx,Env00xx,
                                  Env01xx,Env01xx,Env01xx,Env01xx,
                                  Env1000,Env1001,Env1010,Env1011,
                                  Env1100,Env1101,Env1110,Env1111};

// index as: envData[envShape][envPhase][envPos>>(32-5)]
// value of 0-15, index into volumetab

uint8_t envData[16][2][16*2];
int envShape, envPhase;
uint32_t envPos, envStep;

int samples_per_second = 44100;
int samples_per_frame = 44100/50;

bool use_envelopes = true;
bool fixed_envelopes = false;
bool use_envelope_frequency = false;
bool remap = false;
char *remapstring;
int remapindex;
unsigned int fediv = 1;       // factor to divide envelope frequency by
unsigned int fixedvol = 13;
uint32_t master_clock = 0;

#define DEFAULT_MAXPOKVOL 12
unsigned int maxpokvol = DEFAULT_MAXPOKVOL;

unsigned int mono = 0;
int basstype = 0;
char *bassstring;

enum {
    BASS_TRANSPOSE = 0,
    BASS_GRITTY,
    BASS_BUZZY,
    BASS_SOFTBASS,
    BASS_COUNT
};

const char *basstypes[BASS_COUNT] = {
    "transpose", "gritty", "buzzy", "softbass"
};

enum {
    REMAP_ABC = 0,
    REMAP_ACB,
    REMAP_BAC,
    REMAP_BCA,
    REMAP_CAB,
    REMAP_CBA,
    REMAP_COUNT
};

const char *valid_remaps[REMAP_COUNT] = {
    "ABC", "ACB", "BAC", "BCA", "CAB", "CBA"
};

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

static uint32_t envStepCompute(uint8_t rHigh, uint8_t rLow) {
    int per = (rHigh<<8)+rLow;
    if (per<3)
        return 0;

#if 1                                   // integer math only
    int64_t step = master_clock;
    step <<= (16+16-9);
    step /= (per * samples_per_second);
#else
    float step = master_clock;
    step /= ((float)per*512.0*(float)samples_per_second);
    step *= 65536.0*65536.0;
#endif

    return (uint32_t) step;
}

/* ------------------------------------------------------------------------ */

// Return pointer to next pEnv entry

static uint8_t *init_envData(uint8_t *pEnv, int a, uint8_t b)
{
    int i, d;

    d = b-a;
    a *= 15;
    for (i=0;i<16;i++) {
        *pEnv++ = (uint8_t)a;
        a += d;
    }
    return pEnv;
}

/* ------------------------------------------------------------------------ */

static void init_volumetab(int maxvol) {
    volumetab[0] = 0;
    for (int x=1; x<=15; x++) {
        double y = 4;
        double v = pow(x,y)/pow(15,y)*maxvol;
//        printf("%.2f, ", v);
        v = round(v);
        if (v==0.0) v=1.0;
        volumetab[x]=v;
    }
    // these are: 0.00, 0.00, 0.02, 0.06, do not choke 0.15 and 0.31
    volumetab[1] = volumetab[2] = volumetab[3] = volumetab[4] = 0;
}

/* ------------------------------------------------------------------------ */

/* Return pointer to start of register dump or NULL on error */

static uint8_t *ymdecode(uint8_t *buffer, off_t bufsize, off_t *nframes,
        int *framesize, bool *interleaved, bool *madmaxym2) {

    char ID[5];
    memcpy(ID, buffer, 4);
    ID[4] = 0;
    for (int i=0; i<4; i++) ID[i] = isprint(ID[i]) ? ID[i] : 'x';

    fprintf(stderr, "%s detected\n", ID);

    if (!strncmp(ID, "YM2!", 4)) {
        *nframes = (bufsize-4)/14;
        *framesize = 14;
        *interleaved = true;
        *madmaxym2 = true;
        if (!master_clock)
            master_clock = ATARI_ST_CLOCK;
        return buffer+4;
    }

    if (!strncmp(ID, "YM3!", 4)) {
        *nframes = (bufsize-4)/14;
        *framesize = 14;
        *interleaved = true;
        if (!master_clock)
            master_clock = ATARI_ST_CLOCK;
        return buffer+4;
    }

    if (!strncmp(ID, "YM3b", 4)) {
        *nframes = (bufsize-4)/14;
        *framesize = 14;
        *interleaved = true;
        /* note: EOF-4 contains loop info */
        if (!master_clock)
            master_clock = ATARI_ST_CLOCK;
        return buffer+4;
    }

    if (!strncmp(ID, "YM4!", 4)) {
        fprintf(stderr, "YM4! unsupported, use YM5 instead\n");
        return NULL;
    }

    if (!strncmp(ID, "YM5!", 4) ||
        !strncmp(ID, "YM6!", 4)) {
        if (strncmp((const char*)(buffer+4),"LeOnArD!",8)) {
            fprintf(stderr, "LeOnArD! marker not found\n");
            return NULL;
        }
        buffer += 12;
        *nframes = GETBE32(buffer);     buffer += 4;
        if (GETBE32(buffer) & 1)
            *interleaved = true;
        else
            *interleaved = false;
        /* attributes */                buffer += 4;
        if (GETBE16(buffer) != 0) {
            fprintf(stderr, "digidrums detected!\n");
            goto errout;
        }
        /* nbDrum */                    buffer += 2;
        if (!master_clock)
            master_clock = GETBE32(buffer);
        /* clock */                     buffer += 4;
        /* playrate */                  buffer += 2;
        /* loop frame */                buffer += 4;
        /* skip */                      buffer += GETBE16(buffer) + 2;
        char *name = strdup((char*)buffer);
        buffer += strlen(name)+1;
        char *author = strdup((char*)buffer);
        buffer += strlen(author)+1;
        char *comment = strdup((char*)buffer);
        buffer += strlen(comment)+1;
        fprintf(stderr, "name: %s\n", name);
        fprintf(stderr, "author: %s\n", author);
        fprintf(stderr, "comment: %s\n", comment);
        *framesize = 16;
        return buffer;
    }

errout:
    fprintf(stderr, "unsupported format\n");
    return NULL;
}

/* ------------------------------------------------------------------------ */

/* Returns pointer to decompressed output or NULL on error */

static uint8_t *unpack(uint8_t *inbuf, off_t insize, off_t *outsize) {
    lzhHeader_t *pHeader = (lzhHeader_t *) inbuf;

    if (pHeader->size == 0 || strncmp(pHeader->id, "-lh5-", 5)) {
        *outsize = insize;
        return inbuf;           // not compressed
    }

    *outsize = GETLE32(((uint8_t*)&pHeader->original));

    uint8_t *outbuf = (uint8_t *) malloc(*outsize);
    if (!outbuf) {
        fprintf(stderr, "out of memory\n");
        return NULL;
    }

    off_t packedSize = GETLE32(((uint8_t*)&pHeader->packed));
    inbuf  += pHeader->size + 2;    // skip header and crc16
    insize -= pHeader->size + 2;

    if (packedSize > insize)
        packedSize = insize;

    CLzhDepacker *pDepacker = new CLzhDepacker;
    bool ret = pDepacker->LzUnpack(inbuf, packedSize, outbuf, *outsize);
    delete pDepacker;

    if (!ret) {
        fprintf(stderr, "lh5 decompression error\n");
        return NULL;
    }

    return outbuf;
}

/* ------------------------------------------------------------------------ */

/* Return pointer to deinterleaved buffer or NULL on error */

static uint8_t *deinterleave(uint8_t *buffer, off_t nframes, int framesize) {
    uint8_t *newbuf = (uint8_t *) malloc(nframes*framesize);
    if (!newbuf) {
        fprintf(stderr, "out of memory\n");
        return NULL;
    }

    for (int c=0; c<framesize; c++) {
        for (int x=0; x<nframes; x++) {
            newbuf[x*framesize+c] = buffer[c*nframes+x];
        }
    }

    return newbuf;
}

/* ------------------------------------------------------------------------ */

static bool write_sapr_header(FILE *file) {
    if (fprintf(file, "SAP\r\nAUTHOR \"\"\r\nNAME \"\"\r\nDATE \"\"\r\nTYPE R\r\n\r\n") < 0) {
        fprintf(stderr, "error writing to file\n");
        return false;
    }
    return true;
}

/* ------------------------------------------------------------------------ */

static uint8_t find_closest_distc(const struct bass table[], double f) {
    double delta = 1000.0;  // big number
    int found = 0;

    for (int i=0; i<36; i++) {
        double newd = abs(table[i].realf - f);
        if (newd < delta) {
            delta = newd;
            found = i;
        }
    }
    return table[found].distc;
}

/* ------------------------------------------------------------------------ */

static void ym2pokey_8bit(uint8_t lsb, uint8_t msb, uint8_t volume,
                        bool tone, bool noise, uint8_t *pokey, uint8_t *ptr) {
    int TP;

    if (use_envelopes && use_envelope_frequency && volume &0x10) {
        TP = (ptr[11]<<8) + ptr[12];
        TP /= fediv;
    } else {
        msb &= 0x0f;
        TP = lsb + (msb<<8);
    }

    double f = (double) master_clock / (16*TP);

    double POKreal = (ATARI_XL_CLOCK / 28.0 / 2.0 / f) - 1;

    int POK = round(POKreal);

    int dist = 0xa0;

    if (POK < 0)
        POK = 0;

    if (POK > 255) {
        switch (basstype) {
        default:
        case BASS_TRANSPOSE:
            while (POK > 255) {
                POKreal /= 2.0;
                POK = round(POKreal);
            }
            break;
        case BASS_GRITTY:
            POK = find_closest_distc(gritty, f);
            dist = 0xc0;
            break;
        case BASS_BUZZY:
            POK = find_closest_distc(buzzy, f);
            dist = 0xc0;
            break;
        case BASS_SOFTBASS:
            POKreal /= 2.0;
            if (round(POKreal) <= 255) {
                POK = round(POKreal);
                dist = 0x30;                // one octave lower
            } else {
                POKreal /= 2.0;
                if (round(POKreal) <= 255) {
                    POK = round(POKreal);
                    dist = 0x70;            // two octaves lower
                } else {
                    POK = 255;              // very low is clipped
                    dist = 0x70;            // gigadist uses this?
                }
            }
            break;
        }
    }

    pokey[0] = POK;

    int v = volumetab[volume & 0x0f];

    if (volume & 0x10 && use_envelopes) {        // mode bit set
        if (fixed_envelopes)
            v = volumetab[fixedvol];
        else
            v = volumetab[envData[envShape][envPhase][envPos>>(32-5)]];
    }

    if (tone)
        pokey[1] = dist + v;

    if (noise)
        pokey[1] = 0x80 + v;
}

/* ------------------------------------------------------------------------ */

static void ym2pokey(uint8_t lsb, uint8_t msb, uint8_t volume,
                        bool tone, bool noise, uint8_t *pokey, uint8_t *ptr) {
    int TP;

    if (use_envelopes && use_envelope_frequency && volume &0x10) {
        TP = (ptr[11]<<8) + ptr[12];
        TP /= fediv;
    } else {
        msb &= 0x0f;
        TP = lsb + (msb<<8);
    }

    double f = (double) master_clock / (16*TP);

    int POK1 = (ATARI_XL_CLOCK / 2.0 / f) - 7;

    if (POK1 < 0)
        POK1 = 0;

    pokey[0] = POK1 & 0xff;
    pokey[2] = (POK1 >> 8) & 0xff;

    int v = volumetab[volume & 0x0f];

    if (volume & 0x10 && use_envelopes) {        // mode bit set
        if (fixed_envelopes)
            v = volumetab[fixedvol];
        else
            v = volumetab[envData[envShape][envPhase][envPos>>(32-5)]];
    }

    if (tone)
        pokey[3] = 0xa0 + v;
    if (noise)
        pokey[3] = 0x80 + v;
}

/* ------------------------------------------------------------------------ */

static void usage(void) {
    fprintf(stderr, "usage: ym2sapr [-dhm][-efcprb value] input.ym\n\n"
"   -h          display help\n"
"   -d          disable envelopes\n"
"   -e volume   envelopes as fixed YM volume\n"
"   -f divider  use envelope frequency / divider as note\n"
"   -c clock    override master clock [default:2000000 or read from file]\n"
"   -p volume   pokey maximum per channel volume [default: 12, softbass: 11]\n"
"   -r map      remap channels [default: abc]\n"
"   -m          eneable mono pokey mode [default: stereo pokey]\n"
"   -b type     mono bass type (transpose [default], gritty, buzzy or softbass)\n"
);
}

/* ------------------------------------------------------------------------ */

static void remap_channels(uint8_t *pokl, uint8_t *pokr) {
    uint8_t newl[8], newr[8];

    uint8_t *oldA = pokl, *oldB = pokl+4, *oldC = pokr, *oldN = pokr+4;
    uint8_t *newA = newl, *newB = newl+4, *newC = newr, *newN = newr+4;

    memcpy(newN, oldN, 4);      // noise always right

    switch (remapindex) {
    case REMAP_ABC:
        memcpy(newA, oldA, 4);
        memcpy(newB, oldB, 4);
        memcpy(newC, oldC, 4);
        break;
    case REMAP_ACB:
        memcpy(newA, oldA, 4);
        memcpy(newB, oldC, 4);
        memcpy(newC, oldB, 4);
        break;
    case REMAP_BAC:
        memcpy(newA, oldB, 4);
        memcpy(newB, oldA, 4);
        memcpy(newC, oldC, 4);
        break;
    case REMAP_BCA:
        memcpy(newA, oldB, 4);
        memcpy(newB, oldC, 4);
        memcpy(newC, oldA, 4);
        break;
    case REMAP_CAB:
        memcpy(newA, oldC, 4);
        memcpy(newB, oldA, 4);
        memcpy(newC, oldB, 4);
        break;
    case REMAP_CBA:
        memcpy(newA, oldC, 4);
        memcpy(newB, oldB, 4);
        memcpy(newC, oldA, 4);
        break;
    }

    memcpy(pokl, newl, 8);
    memcpy(pokr, newr, 8);
}

/* ------------------------------------------------------------------------ */

int main(int argc, char **argv) {
    int option, i;

    while ((option = getopt(argc, argv, "dhe:f:c:p:r:mb:")) != -1) {
        switch (option) {
        case 'd':
            use_envelopes = false;
            break;
        case 'e':
            fixed_envelopes = true;
            fixedvol = atoi(optarg) & 0x0f;
            break;
        case 'f':
            use_envelope_frequency = true;
            fediv = atoi(optarg);
            break;
        case 'c':
            master_clock = atoi(optarg);
            break;
        case 'p':
            maxpokvol = atoi(optarg);
            break;
        case 'r':
            remap = true;
            remapstring = strdup(optarg);
            for (char *c=remapstring; *c; c++)
                *c = toupper(*c);
            for (i=0; i<REMAP_COUNT; i++)
                if (!strcmp(remapstring, valid_remaps[i]))
                    break;
            if (i==REMAP_COUNT) {
                fprintf(stderr, "invalid remapping of channels\n");
                return 1;
            }
            remapindex = i;
            break;
        case 'm':
            mono = 1;
            break;
        case 'b':
            bassstring = strdup(optarg);
            for (char *c=bassstring; *c; c++)
                *c = tolower(*c);
            for (i=0; i<BASS_COUNT; i++)
                if (!strcmp(bassstring, basstypes[i]))
                    break;
            if (i==BASS_COUNT) {
                fprintf(stderr, "invalid bass type\n");
                return 1;
            }
            basstype = i;
            if (basstype == BASS_SOFTBASS && maxpokvol == DEFAULT_MAXPOKVOL)
                maxpokvol = 11;             // 12 sometimes distorts
            break;
        case 'h':
        default:
            usage();
            return 1;
            break;
        }
    }

    if (optind+1 != argc) {
        fprintf(stderr, "wrong arguments\n");
        usage();
        return 1;
    }

    FILE *input = fopen(argv[optind], "rb");
    if (!input) {
        fprintf(stderr, "cannot open %s\n", argv[optind]);
        return 1;
    }
    
    if (fseeko(input, 0, SEEK_END) < 0) {
        fprintf(stderr, "%s is not seekable\n", argv[optind]);
        return 1;
    }
    off_t filesize = ftello(input);
    fseeko(input, 0, SEEK_SET);

    uint8_t *inbuf = (uint8_t *) malloc(filesize);
    if (!inbuf) {
        fprintf(stderr, "unable to allocate memory\n");
        return 1;
    }

    if (fread(inbuf, 1, filesize, input) != filesize) {
        fprintf(stderr, "cannot read %s\n", argv[optind]);
        return 1;
    }
    fclose(input);

    off_t outsize;
    uint8_t *outbuf = unpack(inbuf, filesize, &outsize);

    if (!outbuf) {
        fprintf(stderr, "unable to decompress %s\n", argv[optind]);
        return 1;
    }

    off_t nframes = 0;
    int framesize = 0;
    bool interleaved = false;
    bool madmaxym2 = false;         // special envelope handling
    uint8_t *dumpbuf = ymdecode(outbuf, outsize, &nframes, &framesize,
                                            &interleaved, &madmaxym2);

    if (!dumpbuf || !nframes || !framesize) {
        fprintf(stderr, "unable to parse ym data\n");
        return 1;
    }

    fprintf(stderr, "frame size: %i\n", framesize);
    fprintf(stderr, "number of frames: %li\n", nframes);
    fprintf(stderr, "interleaved: %s\n", interleaved ? "true" : "false");

    if (interleaved) {
        dumpbuf = deinterleave(dumpbuf, nframes, framesize);
        if (!dumpbuf) {
            fprintf(stderr, "error during deinterleave\n");
            return 1;
        }
    }

#if 0       // debug print frames
    for (int c=0; c<nframes; c++) {
        for (int x = 0; x<framesize; x++) {
            printf("%02x ", ptr[x]);
        }
        printf("\n");
        ptr += framesize;
    }
#endif

/* YM2149 registers
 *
 * R0, R1   LSB,MSB     12-bit tone adjustment A (TP)
 * R2, R3   LSB,MSB     12-bit tone adjustment B (TP
 * R4, R5   LSB,MSB     12-bit tone adjustment C (TP)
 * R6       000xxxxx    5-bit noise adjustment (NP)
 * R7       ionnnttt    I/O, Noise, Tone per channel, low active
 * R8       000MVVVV    Mode (1=envelope), 4-bit Volume channel A
 * R9       000MVVVV    Mode (1=envelope), 4-bit Volume channel B
 * R10      000MVVVV    Mode (1=envelope), 4-bit Volume channel C
 * R11,R12  LSB,MSB     16-bit envelope frequency adjustment
 * R13      0000CAAH    CONT/ATT/ALT/HOLD envelope shape
 * R14,R15              not stored
 *
 * YM5!/YM6!    byte 14 and 15  extra data for sync-buzzer, sid voice, etc..
 *
 * f_master = 2000000 Hz
 *
 * f_T = f_master / 16TP
 * f_N = f_master / 16NP
 */

/* Pokey
 *
 * 0    AUDF1   LSB
 * 1    AUDC1   $00
 * 2    AUDF2   MSB
 * 3    AUDC2   tone: $av or noise: $8v, v = volume
 * 4    AUDF3   LSB
 * 5    AUDC3   $00
 * 6    AUDF4   MSB
 * 7    AUDC4   tone: $av or noise: $8v, v = volume
 * 8    AUDCTL  $40+$20+$10+$08=$78 (clock 1&3 at 1.77MHz, join 1+2 and 3+4)
 *
 * f_master = 1773447 Hz
 * f = f_master / 2 / (7+x)
 */

    uint8_t *ptr = dumpbuf;

    bool envelope_mode_used = false;
    bool tone_plus_noise = false;
    for (int c=0; c<nframes; c++) {
        if ((ptr[c*framesize+ 8] & 0x10) ||
            (ptr[c*framesize+ 9] & 0x10) ||
            (ptr[c*framesize+10] & 0x10) ) {
            envelope_mode_used = true;
        }
        int tpn = ptr[c*framesize+7];
        tpn ^= 0xff;
        tpn &= 0x3f;
        if (((tpn & 0b00100000) && (tpn & 0b00000100)) ||
            ((tpn & 0b00010000) && (tpn & 0b00000010)) ||
            ((tpn & 0b00001000) && (tpn & 0b00000001))) {
            tone_plus_noise = true;
            break;
        }
    }

    if (envelope_mode_used) {
        fprintf(stderr, "envelope mode bit(s) are used\n");
        if (use_envelopes) {
            fprintf(stderr, "warning: high frequency envelopes might sound odd\n");
            fprintf(stderr, "consider disabling them with -d\n");
            if (fixed_envelopes) {
                fprintf(stderr, "using fixed volume envelope, volume: %i\n",fixedvol);
            }
            if (use_envelope_frequency) {
                fprintf(stderr, "using envelope frequency as note, divided by %i\n", fediv);
            }
        } else {
            fprintf(stderr, "warning: emulating envelopes is disabled\n");
        }
    }

    if (tone_plus_noise) {
        fprintf(stderr, "has tone and noise enabled both at times\n");
    }

    fprintf(stderr, "master clock: %i Hz\n", master_clock);

    if (remap)
        fprintf(stderr, "channel mapping: %s\n", remapstring);

    if (mono) {
        fprintf(stderr, "mono pokey mode\n");
        fprintf(stderr, "bass type: %s\n", basstypes[basstype]);
    } else {
        fprintf(stderr, "stereo pokey mode\n");
    }

    FILE *left = fopen("left.sapr", "wb");
    if (!left) {
        fprintf(stderr, "unable to open 'left.sapr'\n");
        return 1;
    }
    if (!write_sapr_header(left))
        return 1;

    FILE *right = NULL;
    if (!mono) {
        right = fopen("right.sapr", "wb");
        if (!right) {
            fprintf(stderr, "unable to open 'right.sapr'\n");
            return 1;
        }
        if (!write_sapr_header(right))
            return 1;
    }


    if (mono)
        fprintf(stderr, "writing output to 'left.sapr'\n");
    else
        fprintf(stderr, "writing output to 'left.sapr' and 'right.sapr'\n");

    // init volumes and envelope data
    //
    fprintf(stderr, "maximum pokey volume set to %i\n", maxpokvol);
    init_volumetab(maxpokvol); // 13-15 sounds overdriven, 11 and 12 sometimes

    uint8_t *pEnv = &envData[0][0][0];

    for (int env=0; env<16; env++) {
        const int *pse = EnvWave[env];
        for (int phase=0; phase<4; phase++) {
            pEnv = init_envData(pEnv, pse[phase*2+0], pse[phase*2+1]);
        }
    }

    envPos = envPhase = envShape = envStep = 0;

    // loop through all frames and convert to pokey
    //
    uint8_t pokeyL[9], pokeyR[9];
    for (int c=0; c<nframes; c++) {
        memset(pokeyL, 0, 9);
        memset(pokeyR, 0, 9);
        if (!mono)
            pokeyL[8] = pokeyR[8] = 0x78;

        // handle envelope registers
        // if ptr[13] != 0xff handle YM2! and >=YM3! setting of
        // register 11,12,13 (envelope frqequency envStep and envPos/Shape)

        if (ptr[13] != 0xff && madmaxym2) {     // MadMax specific
            envStep = envStepCompute(0, ptr[11]);
            envPos = envPhase = 0;
            envShape = 10;
        } else {
            envStep = envStepCompute(ptr[12],ptr[11]);
            if (ptr[13] != 0xff) {
                envPos = envPhase = 0;
                envShape = ptr[13]&0x0f;
            }
        }

        // convert frequencies and volumes to Pokey

        int tpn = (ptr[7] ^ 0xff) & 0x3f;

        if (mono) {
            ym2pokey_8bit(ptr[0], ptr[1], ptr[ 8], tpn & 1, false, &pokeyL[0], ptr);
            ym2pokey_8bit(ptr[2], ptr[3], ptr[ 9], tpn & 2, false, &pokeyL[2], ptr);
            ym2pokey_8bit(ptr[4], ptr[5], ptr[10], tpn & 4, false, &pokeyL[6], ptr);
        } else {
            ym2pokey(ptr[0], ptr[1], ptr[ 8], tpn & 1, false, &pokeyL[0], ptr);
            ym2pokey(ptr[2], ptr[3], ptr[ 9], tpn & 2, false, &pokeyL[4], ptr);
            ym2pokey(ptr[4], ptr[5], ptr[10], tpn & 4, false, &pokeyR[0], ptr);
        }

        int noisevol = 0;
        if (tpn & 0x08)
            noisevol = ptr[8];
        if (tpn & 0x10)
            if (noisevol < ptr[9])
                noisevol = ptr[9];
        if (tpn & 0x20)
            if (noisevol < ptr[10])
                noisevol = ptr[10];

        if (mono) {
            ym2pokey_8bit(ptr[6]&0x1f, 0, noisevol , false, true, &pokeyL[4], ptr);
        } else {
            ym2pokey(ptr[6]&0x1f, 0, noisevol , false, true, &pokeyR[4], ptr);
        }

        // remap?

        if (remap && !mono)
            remap_channels(pokeyL, pokeyR);

        // write pokey frame to disk

        if (fwrite(pokeyL, 9, 1, left) < 1) {
            fprintf(stderr, "error writing to 'left.sapr'\n");
            return 1;
        }
        if (!mono) {
            if (fwrite(pokeyR, 9, 1, right) < 1) {
                fprintf(stderr, "error writing to 'right.sapr'\n");
                return 1;
            }
        }

        // do envelope frequency envPos += envStep; if overflow, envPhase=1
 
        for (int x=0; x<samples_per_frame; x++) {
            envPos += envStep;
            if (envPos < envStep)
                envPhase = 1;
        }

        // next frame

        ptr += framesize;
    }

    fclose(left);
    if (!mono)
        fclose(right);
    fprintf(stderr, "finished!\n");
}

/* ------------------------------------------------------------------------ */
