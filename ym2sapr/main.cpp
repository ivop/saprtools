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

/* ------------------------------------------------------------------------ */

static uint32_t envStepCompute(uint8_t rHigh, uint8_t rLow) {
    int per = (rHigh<<8)+rLow;
    if (per<3)
        return 0;

#if 1                                   // integer math only
    int64_t step = ATARI_ST_CLOCK;
    step <<= (16+16-9);
    step /= (per * samples_per_second);
#else
    float step = ATARI_ST_CLOCK;
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
        v = round(v);
        if (v==0.0) v=1.0;
//        printf("%.2f, ", v);
        volumetab[x]=v;
    }
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
        return buffer+4;
    }

    if (!strncmp(ID, "YM3!", 4)) {
        *nframes = (bufsize-4)/14;
        *framesize = 14;
        *interleaved = true;
        return buffer+4;
    }

    if (!strncmp(ID, "YM3b", 4)) {
        *nframes = (bufsize-4)/14;
        *framesize = 14;
        *interleaved = true;
        /* note: EOF-4 contains loop info */
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

static void ym2pokey(uint8_t lsb, uint8_t msb, uint8_t volume,
                        bool tone, bool noise, uint8_t *pokey, uint8_t *ptr) {
    int TP;

    if (use_envelopes && use_envelope_frequency && volume &0x10) {
        TP = (ptr[11]<<8) + ptr[12];
        TP /= 4;
    } else {
        msb &= 0x0f;
        TP = lsb + (msb<<8);
    }

    double f = (double) ATARI_ST_CLOCK / (16*TP);

    int POK1 = (ATARI_XL_CLOCK / 2.0 / f) - 7;

    if (POK1 < 0)
        POK1 = 0;

    pokey[0] = POK1 & 0xff;
    pokey[2] = (POK1 >> 8) & 0xff;

    int v = volumetab[volume & 0x0f];

    if (volume & 0x10 && use_envelopes) {        // mode bit set
        if (fixed_envelopes)
            v = volumetab[13];
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
    fprintf(stderr, "usage: ym2sapr [-h] input.ym\n\n");
    fprintf(stderr, "   -h  display help\n");
    fprintf(stderr, "   -d  disable envelopes\n");
    fprintf(stderr, "   -e  envelopes as fixed volume\n");
    fprintf(stderr, "   -f  use envelope frequency as note\n");
}

/* ------------------------------------------------------------------------ */

int main(int argc, char **argv) {
    int option;

    while ((option = getopt(argc, argv, "defh")) != -1) {
        switch (option) {
        case 'd':
            use_envelopes = false;
            break;
        case 'e':
            fixed_envelopes = true;
            break;
        case 'f':
            use_envelope_frequency = true;
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
                fprintf(stderr, "using fixed volume envelope\n");
            }
            if (use_envelope_frequency) {
                fprintf(stderr, "using envelope frequency as note\n");
            }
        } else {
            fprintf(stderr, "warning: emulating envelopes is disabled\n");
        }
    }

    if (tone_plus_noise) {
        fprintf(stderr, "has tone and noise enabled both at times\n");
    }

/* Map channel A and B to Pokey left, Channel C right pokey.
 * If tone_plus_noise, use a separate noise channel on the right Pokey.
 */

    FILE *left = fopen("left.sapr", "wb");
    if (!left) {
        fprintf(stderr, "unable to open 'left.sapr'\n");
        return 1;
    }
    FILE *right = fopen("right.sapr", "wb");
    if (!right) {
        fprintf(stderr, "unable to open 'right.sapr'\n");
        return 1;
    }
    if (!write_sapr_header(left))
        return 1;
    if (!write_sapr_header(right))
        return 1;

    fprintf(stderr, "writing output to 'left.sapr' and 'right.sapr'\n");

    // init volumes and envelope data
    //
    init_volumetab(12);     // 15 sounds overdriven

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

        ym2pokey(ptr[0], ptr[1], ptr[ 8], tpn & 1, false, &pokeyL[0], ptr);
        ym2pokey(ptr[2], ptr[3], ptr[ 9], tpn & 2, false, &pokeyL[4], ptr);
        ym2pokey(ptr[4], ptr[5], ptr[10], tpn & 4, false, &pokeyR[0], ptr);

        // do not maskout envelope mode bit, so it carries over to ym2pokey

        int noisevol = 0;
        if (tpn & 0x08)
            noisevol = ptr[8];
        if (tpn & 0x10)
            if (noisevol < ptr[9])
                noisevol = ptr[9];
        if (tpn & 0x20)
            if (noisevol < ptr[10])
                noisevol = ptr[10];
        ym2pokey(ptr[6], 0, noisevol , false, true, &pokeyR[4], ptr);

        // write pokey frame to disk

        if (fwrite(pokeyL, 9, 1, left) < 1) {
            fprintf(stderr, "error writing to 'left.sapr'\n");
            return 1;
        }
        if (fwrite(pokeyR, 9, 1, right) < 1) {
            fprintf(stderr, "error writing to 'right.sapr'\n");
            return 1;
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
    fclose(right);
}

/* ------------------------------------------------------------------------ */
