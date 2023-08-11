#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "sidengine.h"

#define NSAMPLES 882                    // 44100/50 = 882
static uint16_t soundbuffer[NSAMPLES*2];

static uint16_t init_addr, play_addr;
static uint8_t actual_subsong, max_subsong, speed100Hz;
static char song_name[32], song_author[32], song_copyright[32];

void show_info(int argc) {
    fprintf(stderr, "usage: sid2sapr sid-file\n"
);
}
int main(int argc, char *argv[]) {
    if (argc < 2) {
        show_info(argc);
        return 1;
    }

    c64Init();
    synth_init(44100);

    if (c64SidLoad
        (argv[1], &init_addr, &play_addr, &actual_subsong, &max_subsong,
         &speed100Hz, song_name, song_author, song_copyright) == 0) {
        fprintf(stderr, "Failed to load %s\n", argv[1]);
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
