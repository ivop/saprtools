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

#define C64_CLOCK        985248L
#define ATARI_CLOCK     1773447L

#define NSAMPLES 882                    // 44100/50 = 882

/* ------------------------------------------------------------------------ */

static uint16_t initAddress, playAddress, songs, startSong;
static uint32_t speed;
static char name[33], author[33], copyright[33];

#define DEFAULT_MAXPOKVOL 10
static unsigned int maxpokvol = DEFAULT_MAXPOKVOL;

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
        double newd = abs(table[i].realf - f);
        if (newd < delta) {
            delta = newd;
            found = i;
        }
    }
    return table[found].distc;
}

/* ------------------------------------------------------------------------ */

static void sid2pokey(int voice, uint8_t *pokey) {
    bool noise = false;
    uint8_t wave = sid.v[voice].wave;

    if (wave & 0x80) noise = true;

    double constant = pow(256.0, 3.0) / C64_CLOCK;

    double f = sid.v[voice].freq / constant;

    double POKreal = (ATARI_CLOCK / 28.0 / 2.0 / f) - 1;

    int POK = round(POKreal);

    int dist = 0xa0;

    int volume = (osc[voice].envval >> 20) & 0xf;
    int v = voltab[volume];

    switch (wave&0x70) {
    case 4:
    case 2:
    case 1:
        break;
    default:
        volume = 0;
        break;
    }
    if (wave & 0b110) volume = 0;

    if (POK < 0)
        POK = 0;

    if (POK > 255 && !noise) {
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

    if (wave & 0x08) v = 0;             // test bit
 
    pokey[0] = POK;
    pokey[1] = dist + v;

    if (noise) {
        POK = round(POKreal / 16.0); // divide by 16 (like I did w/ sid2gumby)
        if (POK > 255) POK = 255;
        if (v) {
            v >>= 1;
            if (!v) v = 1;
        }
        pokey[0] = POK;
        pokey[1] = 0x80 + v;
    }

    if (wave == 0x16 || wave == 0x17)   // cybernoid 2 hack
        pokey[1] = 0;
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

static void usage(void) {
    fprintf(stderr, "usage: sid2sapr [-b type] [-o file] sid-file\n\n"
"   -h          display help\n"
"   -b type     bass type (transpose [default], gritty, buzzy or softbass)\n"
"   -o file     output sap-r data to file [default: output.sapr]\n"
"   -p volume   pokey maximum per channel volume [default: 12, softbass: 10]\n"
"   -n num      number of frames to process [default: 3000] (60s at 50Hz)\n"
"   -a          do not adjust for note cancellation\n"
);
}

/* ------------------------------------------------------------------------ */

int main(int argc, char *argv[]) {
    char *outfile = "output.sapr";
    int nframes = 3000;
    bool no_adjust = false;

    int option, i;

    while ((option = getopt(argc, argv, "hb:o:p:n:a")) != -1) {
        switch (option) {
        case 'a':
            no_adjust = true;
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
        case 'o':
            outfile = strdup(optarg);
            break;
        case 'n':
            nframes = atoi(optarg);
            if (nframes <= 0) {
                fprintf(stderr, "invalid number of frames (%d)\n", nframes);
                return 1;
            }
            break;
        case 'p':
            maxpokvol = atoi(optarg);
            if (maxpokvol < 0 || maxpokvol > 15) {
                fprintf(stderr, "invalid maximum pokey volume\n");
                return 1;
            }
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
                &startSong, &speed, name, author, copyright)) {
        fprintf(stderr, "Failed to load %s\n", argv[optind]);
        return 1;
    }

    fprintf(stderr, "Title     : %s\nAuthor    : %s\n"
                    "Copyright : %s\n", name, author, copyright);

    c64_cpu_jsr(initAddress, startSong-1);

    fprintf(stderr, "write output to %s\n", outfile);

    FILE *outf = fopen(outfile, "wb");
    if (!outf) {
        fprintf(stderr, "unable to open %s for writing\n", outfile);
        return 1;
    }
    if (!write_sapr_header(outf))
        return 1;

    fprintf(stderr, "maximum pokey volume: %d\n", maxpokvol);

    init_voltab(maxpokvol);

    fprintf(stderr, "bass type: %s\n", basstypes[basstype]);

    int counter = 0;

    while (counter != nframes) {
        uint8_t pokey[9];
        memset(pokey, 0, 9);

        c64_cpu_jsr(playAddress, 0);
        c64_handle_adsr(NSAMPLES/2);

        sid2pokey(0, &pokey[0]);
        sid2pokey(1, &pokey[2]);
        sid2pokey(2, &pokey[6]);

        c64_handle_adsr(NSAMPLES/2);

        if (!no_adjust) adjust_for_cancellation(pokey);
        if (!save_pokey(pokey, outf)) return 1;

        counter++;
    }

    fprintf(stderr, "finished!\n");
    return 0;
}

/* ------------------------------------------------------------------------ */

