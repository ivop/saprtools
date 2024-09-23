/* sid2sapr - by Ivo van Poorten - Copyright (C) 2023
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
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "c64.h"
#include "md5.h"
#include "songlengths.h"
#include "saw.h"

#define C64_PAL_CLOCK        985248L
#define C64_NTSC_CLOCK      1022727L

#define ATARI_CLOCK     1773447L

/* ------------------------------------------------------------------------ */

static int c64_clock = C64_PAL_CLOCK;
static int nsamples = 44100 / 50;
static float rate = 50;

static uint16_t initAddress, playAddress, songs, startSong;
static uint32_t speed;
static char name[33], author[33], copyright[33];
static uint8_t ntsc;

#define DEFAULT_MAXPOKVOL 10
static unsigned int maxpokvol = DEFAULT_MAXPOKVOL;
#define DEFAULT_STEREO_MAXPOKVOL 12

static int basstype = 0;
static char *bassstring;

enum {
    BASS_TRANSPOSE = 0,
    BASS_GRITTY,
    BASS_BUZZY,
    BASS_SOFTBASS,
    BASS_COUNT
};

static const char *basstypes[BASS_COUNT] = {
    "transpose", "gritty", "buzzy", "softbass"
};

static double pulse_volume = 0.0;
static bool damp = false;
static int mute = 0;
static char *mutestring;
static int mute_detect[4];

enum {
    MUTE_NONE,
    MUTE_RINGMOD,
    MUTE_SYNC,
    MUTE_EITHER,
    MUTE_BOTH,
    MUTE_ALL,
    MUTE_COUNT
};

static const char *mutetypes[MUTE_COUNT] = {
    "none", "ringmod", "sync", "either", "both", "all"
};

static const char *mute_detect_combinations[4] = {
    "none", "sync", "ringmod", "ringmod+sync"
};

static bool bassfix = false;

static uint8_t voltab[16];

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

static bool write_sapr_header(FILE *file) {
    if (fprintf(file, "SAP\r\nAUTHOR \"\"\r\nNAME \"\"\r\nDATE \"\"\r\nTYPE R\r\n\r\n") < 0) {
        fprintf(stderr, "error writing to file\n");
        return false;
    }
    return true;
}

/* ------------------------------------------------------------------------ */

static bool save_pokey(uint8_t *pokey, FILE *outf) {
    if (fwrite(pokey, 9, 1, outf) < 1) {
        fprintf(stderr, "error writing to file\n");
        return false;
    }
    return true;
}

/* ------------------------------------------------------------------------ */

static void init_voltab(int maxvol) {
    voltab[0] = 0;
    for (int i=1; i<16; i++) {
        voltab[i] = round(i*maxvol/15.0);
        if (!voltab[i]) voltab[i]++;
    }
}

/* ------------------------------------------------------------------------ */

static uint8_t find_closest_distc(const struct bass table[], double f) {
    double delta = 1000.0;  // big number
    int found = 0;

    for (int i=0; i<36; i++) {
        double newd = fabs(table[i].realf - f);
        if (newd < delta) {
            delta = newd;
            found = i;
        }
    }
    return table[found].distc;
}

/* ------------------------------------------------------------------------ */

static void sid2pokey2(int voice, uint8_t *pokey) {
    struct sid_voice     *p = &sid.v[voice];
    struct sid_registers *r = &sid.r[voice];

    uint8_t wave = r->wave;

    double constant = pow(256.0, 3.0) / c64_clock;

    double f = r->freq / constant;

    double POKreal = (ATARI_CLOCK / 2.0 / f) - 7;

    int POK = round(POKreal);

    int dist = 0xa0;

    double volume = p->envval;

    if (pulse_volume > 0.0) {
        if (p->pul) {
            int pulse = r->pulse;
            if (pulse > 0x800)
                pulse = 0x800 - (pulse - 0x800);
            double delta = volume - (volume * ((double)pulse/0x800));
            volume = volume - pulse_volume * delta;
        }
    }

    int v = voltab[(int)round(volume)];

    if (POK < 0)
        POK = 0;

    if (p->test || p->off) v = 0; 
 
    if (p->ringmod && damp) v /= 2;

    if (p->noise) {
        POK = round(POKreal / 16.0); // divide by 16 (like I did w/ sid2gumby)
        if (v) {
            v >>= 1;
            if (!v) v = 1;
        }
        dist = 0x80;
    }

    switch (mute) {
    case MUTE_RINGMOD:
        if (p->ringmod) v = 0;
        break;
    case MUTE_SYNC:
        if (p->sync) v = 0;
        break;
    case MUTE_EITHER:
        if ((p->sync && !p->ringmod) || (p->ringmod && !p->sync)) v = 0;
        break;
    case MUTE_BOTH:
        if (p->ringmod && p->sync) v = 0;
        break;
    case MUTE_ALL:
        if (p->ringmod || p->sync) v = 0;
    case MUTE_NONE:
    default:
        if (wave & 6)
            mute_detect[(wave>>1)&3]++;
        break;
    }

    pokey[0] = POK & 0xff;
    pokey[2] = (POK >> 8) & 0xff;
    pokey[3] = dist + v;
}

/* ------------------------------------------------------------------------ */

static int find_closest_sawtooth(double f) {
    double delta = 100000.0;
    int idx = 0;
    for (int i=0; i<sawtabsize; i++) {
        if (fabs(sawtab[i].freq - f) < delta) {
            delta = fabs(sawtab[i].freq - f);
            idx = i;
        }
    }
    return idx;
}

/* ------------------------------------------------------------------------ */

static void sid2pokey(int voice, uint8_t *pokey, bool sawtooth) {
    struct sid_voice     *p = &sid.v[voice];
    struct sid_registers *r = &sid.r[voice];

    uint8_t wave = r->wave;

    double constant = pow(256.0, 3.0) / c64_clock;

    double f = r->freq / constant;

    double POKreal = (ATARI_CLOCK / 28.0 / 2.0 / f) - 1;

    int POK = round(POKreal);

    int dist = 0xa0;

    double volume = p->envval;

    if (pulse_volume > 0.0) {
        if (p->pul) {
            int pulse = r->pulse;
            if (pulse > 0x800)
                pulse = 0x800 - (pulse - 0x800);
            double delta = volume - (volume * ((double)pulse/0x800));
            volume = volume - pulse_volume * delta;
        }
    }

    int v = voltab[(int)round(volume)];

    if (POK < 0)
        POK = 0;

    if (POK > 255 && !p->noise) {
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
            if (bassfix)
                POK -= 1;
            break;
        }
    }

    if (p->test || p->off) v = 0; 
 
    if (p->ringmod && damp) v /= 2;

    if (p->noise) {
        POK = round(POKreal / 16.0); // divide by 16 (like I did w/ sid2gumby)
        if (POK > 255) POK = 255;
        if (v) {
            v >>= 1;
            if (!v) v = 1;
        }
        dist = 0x80;
    }

    int POK2 = 0;
    if (sawtooth) {
        if (v)
            v = voltab[(int)p->envval];      // ignore -w
        if (f < 20000.0) {
            int index = find_closest_sawtooth(f);
            POK  = sawtab[index].div1;
            POK2 = sawtab[index].div3;
        } else {
            POK = POK2 = 0;
            v = 0;
        }
//        if (p->noise) v=0;
    }

    switch (mute) {
    case MUTE_RINGMOD:
        if (p->ringmod) v = 0;
        break;
    case MUTE_SYNC:
        if (p->sync) v = 0;
        break;
    case MUTE_EITHER:
        if ((p->sync && !p->ringmod) || (p->ringmod && !p->sync)) v = 0;
        break;
    case MUTE_BOTH:
        if (p->ringmod && p->sync) v = 0;
        break;
    case MUTE_ALL:
        if (p->ringmod || p->sync) v = 0;
    case MUTE_NONE:
    default:
        if (wave & 6)
            mute_detect[(wave>>1)&3]++;
        break;
    }

    if (sawtooth) {
        pokey[0] = POK;
        pokey[1] = 0xa0 + v;
        if (p->noise)
            pokey[1] = 0x80 + v;
        pokey[4] = POK2;
        pokey[5] = 0;
    } else {
        pokey[0] = POK;
        pokey[1] = dist + v;
    }
}

/* ------------------------------------------------------------------------ */

static void adjust_channels(uint8_t *pokey, int c1, int c2) {
    if ((pokey[c1+1] & 0xa0) == 0xa0 && (pokey[c2+1] & 0xa0) == 0xa0)
        if (pokey[c1] && pokey[c2] && pokey[c1] == pokey[c2])
            pokey[c1]++;
}
static void adjust_for_cancellation(uint8_t *pokey) {
    adjust_channels(pokey, 0, 2);
    adjust_channels(pokey, 0, 6);
    adjust_channels(pokey, 2, 6);
}

/* ------------------------------------------------------------------------ */

static uint16_t detune_16bits(uint16_t div, double cents) {
    double f = ATARI_CLOCK / 2.0 / (div + 7);   // back to Hertz

    f *= pow(2, cents/1200.0);

    return round((ATARI_CLOCK / 2.0 / f) - 7);
}

/* ------------------------------------------------------------------------ */

static char *calculate_md5(char *filename) {
    unsigned char inBuf[1024];
    int inLen;
    unsigned char h[16];
    static char md5[33];

    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "unable to open %s\n", filename);
        return NULL;
    }

    MD5_CTX mdContext;
    MD5Init(&mdContext);

    while ((inLen = fread(inBuf, 1, sizeof(inBuf), f)) >= 1)
        MD5Update(&mdContext, inBuf, inLen);

    MD5Final(h, &mdContext);

    fclose(f);

    snprintf(md5, 33,
            "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            h[0], h[1], h[2], h[3], h[4], h[5], h[6], h[7],
            h[8], h[9], h[10], h[11], h[12], h[13], h[14], h[15]);

    return md5;
}

/* ------------------------------------------------------------------------ */

// returns seconds
//
static float find_songlength(const char *md5, int subtune) {
    int i, max = sizeof(songinfo)/sizeof(struct songinfo);
    for (i = 0; i<max; i++)
         if (!strcmp(songinfo[i].hash, md5)) break;

    if (i == max) goto errout;

    char *p = songinfo[i].lengths;

    subtune--;

    while (p && subtune--) {
        p = strchr(p, ' ');
        if (p) p++;
    }

    if (!p ||  *p == 0) goto errout;

    int minutes;
    float seconds;

    if (sscanf(p, "%d:%f", &minutes, &seconds) != 2) goto errout;

    seconds += minutes * 60;

    fprintf(stderr, "known song, subtune length: %.2f seconds\n", seconds);
    return seconds;

errout:
    fprintf(stderr, "unknown song\n");
    return 0;
}

/* ------------------------------------------------------------------------ */

static void usage(void) {
    fprintf(stderr, "usage: sid2sapr [options] sid-file\n\n"
"   -h          display help\n"
"   -b type     mono bass type (transpose [default], gritty, buzzy or softbass)\n"
"   -o file     output sap-r data to file [default: output.sapr]\n"
"   -p volume   pokey maximum per channel volume [default: 10, softbass: 9]\n"
"   -n num      override the number of frames to process\n"
"                                     [default: from hvsc database or 3000]\n"
"   -a          adjust for note cancellation (mono only)\n"
"   -t num      select track/subtune [default: from file or 1]\n"
"   -f          enable softbass off-by-one bassfix [default: off]\n"
"   -m which    mute ringmod and/or sync [default: none]\n"
"                   ringmod\n"
"                   sync\n"
"                   either (either one, but not both at the same time)\n"
"                   both (only both at the same time)\n"
"                   all (every combination)\n"
"   -d          damp ringmod to half volume\n"
"   -w value    let PWM influence the volume by factor [0.0-1.0]\n"
"   -s          enable stereo pokey mode [default: mono pokey]\n"
"   -x voice    extend one mono channel (0-2) to 16-bits [default: none]\n"
"               also selects HP filtered voice in stereo mode, see below\n"
"\n"
"               [HIGHLY EXPERIMENTAL]\n"
"   -e type     extend one mono channel to sawtooth\n"
"               must be used in combination with -x\n"
"               vibrato and glissando distorts!\n"
"               use make player-softbass-sawtooth for faster stores\n"
"               type: 1 - fully merged table\n"
"                     2 - single table (distance of 1 only)\n"
"                     3 - 12-TET table (selected from type 1)\n"
"   -E factor   change non-sawtooth volume [0.0-1.0, default: 1.0]\n"
"               sometimes use -p to raise overall volume first\n"
"               also works for hpfiler (-F) option\n"
"\n"
"   -F type     extend one channel to use HP filter\n"
"               must be used in combination with -x\n"
"               type: 1 - detuned channel +1, muted\n"
"               type: 2 - detuned channel -1, muted\n"
"               type: 3 - type 1, volume 50%%\n"
"               type: 4 - type 2, volume 50%%\n"
"               in stereo mode, one channel is extended to 32-bits\n"
"   -g cents    stereo HP filter detune amount in cents\n"
"   -G value    adjust HP filter volume [0.0-1.0]\n"
"   -D          transpose HP filter channel 1 octave down\n"
);
}

/* ------------------------------------------------------------------------ */

int main(int argc, char *argv[]) {
    char *outfile = "output.sapr";
    char *outfile2 = "right.sapr";
    FILE *outf = NULL;
    FILE *outf2 = NULL;
    int nframes = 3000;
    bool nframes_override = false;
    bool adjust = false;
    int subtune = 1;
    bool subtune_override = false;
    bool stereo = false;
    int xorder[3] = { 0, 1, 2 };
    bool xflag = false;
    int sawtooth = 0;
    double nonsawf = 1.0;
    int hpfilter = 0;
    double detune_cents = 0.0;
    double hpf_volume = 1.0;
    bool hpf_down = false;

    int option, i;

    while ((option = getopt(argc, argv, "hb:o:p:n:at:fm:dw:sx:e:E:F:g:G:D")) != -1) {
        switch (option) {
        case 'a':
            adjust = true;
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
                maxpokvol = 9;
            break;
        case 'f':
            bassfix = true;
            break;
        case 'd':
            damp = true;
            break;
        case 'o':
            outfile = strdup(optarg);
            break;
        case 'n':
            nframes = atoi(optarg);
            if (nframes <= 0) {
                fprintf(stderr, "invalid number of frames (%d)\n", nframes);
                return 1;
            }
            nframes_override = true;
            break;
        case 'p':
            maxpokvol = atoi(optarg);
            if (maxpokvol < 0 || maxpokvol > 15) {
                fprintf(stderr, "invalid maximum pokey volume\n");
                return 1;
            }
            break;
        case 't':
            subtune = atoi(optarg);
            subtune_override = true;
            break;
        case 'm':
            mutestring = strdup(optarg);
            for (char *c=mutestring; *c; c++)
                *c = tolower(*c);
            for (i=0; i<MUTE_COUNT; i++)
                if (!strcmp(mutestring, mutetypes[i]))
                    break;
            if (i==MUTE_COUNT) {
                fprintf(stderr, "invalid mute type\n");
                return 1;
            }
            mute = i;
            break;
        case 'w':
            pulse_volume = strtod(optarg, NULL);
            if (pulse_volume < 0.0 || pulse_volume > 1.0) {
                fprintf(stderr, "invalid PWM volume value\n");
                return 1;
            }
            break;
        case 's':
            stereo = true;
            outfile = "left.sapr";
            if (maxpokvol != DEFAULT_MAXPOKVOL)
                maxpokvol = DEFAULT_STEREO_MAXPOKVOL;
            break;
        case 'x':
            switch (atoi(optarg)) {
            case 0:
                xorder[0] = 2; xorder[2] = 0; break;
            case 1:
                xorder[1] = 2; xorder[2] = 1; break;
            case 2:
            default:
                break;
            }
            xflag = true;
            break;
        case 'e':
            sawtooth = atoi(optarg);
            if (sawtooth < 1 || sawtooth > 3) {
                fprintf(stderr, "invalid sawtooth type\n");
                return 1;
            };
            if (sawtooth == 2) {
                sawtab = sawtab_single;
                sawtabsize = sawtab_single_size;
            } else if (sawtooth == 3) {
                sawtab = sawtab_full_12tet;
                sawtabsize = sawtab_full_12tet_size;
            }
            break;
        case 'E':
            nonsawf = strtod(optarg, NULL);
            if (nonsawf < 0.0 || nonsawf > 1.0) {
                fprintf(stderr, "invalid non-sawtooth volume factor\n");
                return 1;
            }
            break;
        case 'F':
            hpfilter = atoi(optarg);
            if (hpfilter < 1 || hpfilter > 4) {
                fprintf(stderr, "invalid hpfilter type\n");
                return 1;
            }
            break;
        case 'g':
            detune_cents = strtod(optarg, NULL);
            break;
        case 'G':
            hpf_volume = strtod(optarg, NULL);
            if (hpf_volume < 0.0 || hpf_volume > 1.0) {
                fprintf(stderr, "invalid hpfilter volume adjustment\n");
                return 1;
            }
            break;
        case 'D':
            hpf_down = true;
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

    c64_init();
    c64_sid_init(44100);

    if (!c64_load_sid(argv[optind], &initAddress, &playAddress, &songs,
                &startSong, &speed, name, author, copyright, &ntsc)) {
        fprintf(stderr, "Failed to load %s\n", argv[optind]);
        return 1;
    }

    fprintf(stderr, "Title     : %s\nAuthor    : %s\n"
                    "Copyright : %s\n", name, author, copyright);

    fprintf(stderr, "number of subtunes: %d\n", songs);

    if (!subtune_override) {
        subtune = startSong;
    }

    if (subtune < 1 || subtune > songs) {
        fprintf(stderr, "selected subtune %d is invalid\n", subtune);
        return 1;
    }

    fprintf(stderr, "converting subtune %d\n", subtune);

    c64_cpu_jsr(initAddress, subtune-1);

    if (ntsc) {
        c64_clock = C64_NTSC_CLOCK;
        rate = 60;
    }

    fprintf(stderr, "tuning: %s\n", ntsc ? "NTSC" : "PAL");

    bool CIA = speed & (1 << (subtune-1));

    if (CIA) {
        uint16_t cia_timer = memory_read(0xdc04) | (memory_read(0xdc05) << 8);
        if (!cia_timer) {
            fprintf(stderr, "CIA timer not initialized, default to %.2f Hz\n", rate);
        } else {
            int scanlines = 312, cycles = 63, fps = 50;
            if (ntsc) {
                scanlines = 263;
                cycles = 65;
                fps = 60;
            }

            fprintf(stderr, "CIA timer set by init: %04x (%d)\n", cia_timer, cia_timer);

            rate = (float) scanlines * cycles * fps / (cia_timer + 1);

            fprintf(stderr, "often used faulty calculation ");
            fprintf(stderr, "(%d*%d*%d/(%d+1)) = %.2f\n", scanlines, cycles, fps, cia_timer, rate);

            rate = (float) c64_clock / (cia_timer+1);
            fprintf(stderr, "proper calculation (%d/(%d+1)) = %.2f\n",
                    c64_clock, cia_timer, rate);
        }
    }

    nsamples = 44100 / rate;

    fprintf(stderr, "song speed: %s %.2f Hz\n", CIA ? "CIA" : "VBI", rate);

    if (!nframes_override) {
        char *md5 = calculate_md5(argv[optind]);
        if (!md5) return 1;
        fprintf(stderr, "md5: %s\n", md5);

        float f = find_songlength(md5, subtune);
        int n = f * rate;
        if (n) nframes = n;
    }

    if (stereo)
        fprintf(stderr, "stereo pokey enabled\n");
    else
        fprintf(stderr, "mono pokey enabled\n");

    fprintf(stderr, "write output to %s\n", outfile);

    outf = fopen(outfile, "wb");
    if (!outf) {
        fprintf(stderr, "unable to open %s for writing\n", outfile);
        return 1;
    }
    if (!write_sapr_header(outf))
        return 1;

    if (stereo) {
        fprintf(stderr, "write output to %s\n", outfile2);
        outf2 = fopen(outfile2, "wb");
        if (!outf2) {
            fprintf(stderr, "unable to open %s for writing\n", outfile2);
            return 1;
        }
        if (!write_sapr_header(outf2))
            return 1;
    }

    fprintf(stderr, "maximum pokey volume: %d\n", maxpokvol);

    init_voltab(maxpokvol);

    if (!stereo) {
        fprintf(stderr, "bass type: %s\n", basstypes[basstype]);
        fprintf(stderr, "bassfix: %s\n", bassfix ? "enabled" : "disabled");
    }
    fprintf(stderr, "note cancellation adjustment: %s\n", adjust ?
                                               "enabled" : "disabled");
    fprintf(stderr, "mute: %s\n", mutetypes[mute]);
    fprintf(stderr, "damp ringmod: %s\n", damp ? "enabled" : "disabled");
    fprintf(stderr, "PWM controls volume by %d%%\n",
                                            (int)round(pulse_volume*100));
    if (xflag) {
        fprintf(stderr, "experimental: ");
        if (!stereo) {
            if (sawtooth) {
                fprintf(stderr, "extend 8-bit channel %d to double channel sawtooth\n", xorder[2]);
            } else if (hpfilter) {
                fprintf(stderr, "extend 8-bit channel %d to two channel hp filter\n", xorder[2]);
            } else {
                fprintf(stderr, "extend 8-bit channel %d to 16-bit\n", xorder[2]);
            }
        } else {
            if (sawtooth) {
                fprintf(stderr, "there is no sawtooth in stereo mode\n");
                return 1;
            } else if (hpfilter) {
                fprintf(stderr, "stereo: extend 16-bit channel %d to four channel hp filter\n", xorder[2]);
            } else {
                fprintf(stderr, "stereo: -x ignored\n");
            }
        }
    }

    fprintf(stderr, "dumping %d frames\n", nframes);

    int counter = 0;

    while (counter != nframes) {
        uint8_t pokey[9], pokey2[9];
        memset(pokey, 0, 9);

        if (stereo) {
            memset(pokey2, 0, 9);
            pokey[8] = pokey2[8] = 0x78; // join 1+2, join 3+4, high clock
        }

        c64_cpu_jsr(playAddress, 0);
        c64_handle_adsr(nsamples/2);

        if (stereo) {
            if (xflag && hpfilter) {
                sid2pokey2(xorder[0], &pokey[0]);
                sid2pokey2(xorder[1], &pokey[4]);
                sid2pokey2(xorder[2], &pokey2[0]);
            } else {
                sid2pokey2(0, &pokey[0]);
                sid2pokey2(1, &pokey[4]);
                sid2pokey2(2, &pokey2[0]);
            }
            if (hpfilter) {
                pokey2[8] = 0x7a; // join 1+2, join 3+4, high clock, filter 2+4
                if (hpf_down) { // XXX recalculate from SID frequency
                    uint16_t div = (pokey2[2] << 8) | pokey2[0];
                    div <<= 1;
                    pokey2[2] = div >> 8;
                    pokey2[0] = div & 0xff;
                }
                pokey2[4] = pokey2[0];
                pokey2[6] = pokey2[2];
                if (hpf_volume < 1.0) {
                    uint8_t v = pokey2[3] & 0x0f;
                    v = round((double) v * hpf_volume);
                    pokey2[3] = (pokey2[3] & 0xf0) | v;
                }
                if (hpfilter >= 3) {
                    uint8_t v = (pokey2[3] & 0x0f) >> 1;
                    pokey2[7] = (pokey2[3] & 0xf0) | v; // vol 50%
                } else {
                    pokey2[7] = pokey2[3] & 0xf0; // muted
                }
                if ((hpfilter-1) & 1 ) {    // type 1 and 3
                    uint16_t div = (pokey2[6] << 8) | pokey2[4];
                    uint16_t newdiv = detune_16bits(div, detune_cents);
                    if (newdiv == div) newdiv++;
                    pokey2[4] = newdiv & 0x00ff;
                    pokey2[6] = newdiv >> 8;
                } else {                    // type 2 and 4
                    uint16_t div = (pokey2[6] << 8) | pokey2[4];
                    uint16_t newdiv = detune_16bits(div, -detune_cents);
                    if (newdiv == div) newdiv--;
                    pokey2[4] = newdiv & 0x00ff;
                    pokey2[6] = newdiv >> 8;
                }
            }
        } else {
            if (xflag) {
                if (!sawtooth && !hpfilter) {
                    pokey[8] = 0x28;    // join 3+4, 3 @ high clock
                    sid2pokey(xorder[0], &pokey[0], false);
                    sid2pokey(xorder[1], &pokey[2], false);
                    sid2pokey2(xorder[2], &pokey[4]);
                } else if (sawtooth) {
                    pokey[8] = 0x64;    // filter 1+3, both @ high clock
                    sid2pokey(xorder[0], &pokey[2], false);
                    sid2pokey(xorder[1], &pokey[6], false);
                    sid2pokey(xorder[2], &pokey[0], true);  // <<-- saw!
adjust_nonsawf:
                    // adjust non-sawtooth volume
                    if (pokey[3] & 0x0f) {
                       int v = pokey[3]&0x0f;
                       v = round(v * nonsawf);
                       pokey[3] &= 0xf0;
                       pokey[3] |= v;
                    }
                    if (pokey[7] & 0x0f) {
                       int v = pokey[7]&0x0f;
                       v = round(v * nonsawf);
                       pokey[7] &= 0xf0;
                       pokey[7] |= v;
                    }

                } else if (hpfilter) {
                    pokey[8] = 0x04;    // filter 1+3, normal clock
                    sid2pokey(xorder[0], &pokey[2], false);
                    sid2pokey(xorder[1], &pokey[6], false);
                    sid2pokey(xorder[2], &pokey[0], false);
                    pokey[4] = pokey[0];
                    if (hpfilter >= 3) {
                        uint8_t v = (pokey[1] & 0x0f) >> 1;
                        pokey[5] = (pokey[1] & 0xf0) | v; // vol 50%
                    } else {
                        pokey[5] = pokey[1] & 0xf0; // muted
                    }
                    if ((hpfilter-1) & 1 ) {    // type 1 and 3
                        pokey[4]++;
                    } else {                    // type 2 and 4
                        pokey[4]--;
                    }
                    goto adjust_nonsawf;        // crude hack for now
                }
            } else {
                sid2pokey(0, &pokey[0], false);
                sid2pokey(1, &pokey[2], false);
                sid2pokey(2, &pokey[6], false);
            }
        }

        c64_handle_adsr(nsamples/2);

        if (!stereo && adjust) adjust_for_cancellation(pokey);
        if (!save_pokey(pokey, outf)) return 1;
        if (stereo)
            if (!save_pokey(pokey2, outf2)) return 1;

        counter++;
    }

    if (!mute) {
        for (int i=1; i<4; i++) {
          if (mute_detect[i])
            fprintf(stderr, "usage of %s detected but not muted (%d times)\n",
                                mute_detect_combinations[i], mute_detect[i]);
        }
    }

    fprintf(stderr, "finished!\n");
    return 0;
}

/* ------------------------------------------------------------------------ */

