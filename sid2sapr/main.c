#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "sidengine.h"

#define C64_CLOCK        985248L
#define ATARI_CLOCK     1773447L

#define NSAMPLES 882                    // 44100/50 = 882

/* ------------------------------------------------------------------------ */

static uint16_t soundbuffer[NSAMPLES*2];

static uint16_t init_addr, play_addr;
static uint8_t actual_subsong, max_subsong, speed100Hz;
static char song_name[33], song_author[33], song_copyright[33];

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


/* ------------------------------------------------------------------------ */

void usage(void) {
    fprintf(stderr, "usage: sid2sapr [-b type] sid-file\n\n"
"   -b type     bass type (transpose [default], gritty, buzzy or softbass)\n");
}

/* ------------------------------------------------------------------------ */

int main(int argc, char *argv[]) {
    int option, i;

    while ((option = getopt(argc, argv, "dhe:f:c:p:r:mb:")) != -1) {
        switch (option) {
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

    c64Init();
    synth_init(44100);

    if (c64SidLoad
        (argv[optind], &init_addr, &play_addr, &actual_subsong, &max_subsong,
         &speed100Hz, song_name, song_author, song_copyright) == 0) {
        fprintf(stderr, "Failed to load %s\n", argv[optind]);
        return 1;
    }

    printf("Title     : %s\n", song_name);
    printf("Author    : %s\n", song_author);
    printf("Copyright : %s\n", song_copyright);
    printf("Speed     : %s\n", speed100Hz ? "100Hz" : "50Hz");

    cpuJSR(init_addr, actual_subsong);

    int counter = 0;

    while (counter != 500) {
        if (!speed100Hz) {
                cpuJSR(play_addr, 0);
                synth_render(soundbuffer, NSAMPLES);
                fprintf(stderr, "wave = $%02x, freq = %04x, vol = %03x\n", sid.v[1].wave, sid.v[1].freq, osc[1].envval>>20);
        } else {
                cpuJSR(play_addr, 0);
                synth_render(soundbuffer, NSAMPLES/2);
                cpuJSR(play_addr, 0);
                synth_render(soundbuffer + (NSAMPLES/2*2), NSAMPLES/2);
        }
        counter++;
    }

    return 1;
}

/* ------------------------------------------------------------------------ */

