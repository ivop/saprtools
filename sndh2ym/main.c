
/* sndh2ym - by Ivo van Poorten - Copyright (C) 2023
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "psgplay/psgplay.h"
#include "psgplay/sndh.h"

#include "system/unix/file.h"
#include "system/unix/sndh.h"

#include "atari/machine.h"
#include "atari/mmu.h"
#include "atari/psg.h"

#define STOP_INSTR  0x4e72

struct cb_vars {
    FILE *out;
    int counter;
    int stop;
    int interval;
};

/* ------------------------------------------------------------------------ */

/* Count machine cycles and capture PSG registers every interval */

static void cb(uint32_t pc, void *arg) {
    //uint16_t insn = probe_read_memory_16(pc);
    struct cb_vars *cb_vars = arg;
    FILE *out = cb_vars->out;
    union psg *psg = psg_device.state.internal;
    static uint64_t prev_cycle = 40000, cycle;

    cycle = machine_cycle();
    if (cycle - prev_cycle >= cb_vars->interval) {
        fwrite(psg, 16, 1, out);
        cb_vars->counter++;
        prev_cycle = cycle;
    }
}

/* ------------------------------------------------------------------------ */

static void write_ym_header(FILE *out, uint32_t clock, double rate) {
    fwrite("YM6!LeOnArD!", 1, 12, out);

    unsigned char dword[4];

    memset(dword,0,4);                  // write later when we know nframes
    fwrite(dword, 1, 4, out);

    fwrite(dword, 1, 4, out);           // attributes, not "interleaved"
    fwrite(dword, 1, 2, out);           // zero digidrums

    dword[0] = (clock >> 24) & 0xff;
    dword[1] = (clock >> 16) & 0xff;
    dword[2] = (clock >>  8) & 0xff;
    dword[3] =  clock        & 0xff;
    fwrite(dword, 1, 4, out);           // master clock

    memset(dword,0,4);
    dword[1] = round(rate);
    fwrite(dword, 1, 2, out);           // playrate

    memset(dword,0,4);
    fwrite(dword, 1, 4, out);           // loop frame
    fwrite(dword, 1, 2, out);           // skip

    fwrite("name\000", 1, 5, out);
    fwrite("author\000", 1, 7, out);
    fwrite("comment\000", 1, 8, out);
}

/* ------------------------------------------------------------------------ */

int timedb_get(int hash, int track, unsigned int *frames, int *flags);

static void calculate_hash(uint8_t *data, int len, unsigned int *hptr) {
    uint32_t h = *hptr;
    do {
        h += *data++;
        h += h << 10;
        h ^= h >> 6;
    } while (--len);
    *hptr = h;
}

/* ------------------------------------------------------------------------ */

static void usage() {
    fprintf(stderr, "usage: sndh2ym [options] filename.sndh\n\n"
    "   -o filename     write YM output to filename\n"
    "   -s seconds      dump this number of seconds [default: timedb or 60]\n"
    "   -t number       select track/subtune number [default: sndh or 1]\n"
    );
}

/* ------------------------------------------------------------------------ */

int main(int argc, char **argv) {
    char *outfile = "output.ym";
    int option;
    int subtune_count = 1;
    int subtune_default = 1;
    int subtune = 1;
    bool subtune_override = false;
    int seconds = 60;
    bool seconds_override = false;
    int timer_frequency = 50;
    int stop_position;

    while ((option = getopt(argc, argv, "ho:s:t:")) != -1) {
        switch (option) {
        case 'o':
            outfile = strdup(optarg);
            break;
        case 's':
            seconds = atoi(optarg);
            seconds_override = true;
            break;
        case 't':
            subtune = atoi(optarg);
            subtune_override = true;
            break;
        case 'h':
        default:
            usage();
            return 1;
        }
    }

    if (optind+1 != argc) {
        fprintf(stderr, "wrong arguments\n");
        usage();
        return 1;
    }

    const char *path = argv[optind];
    struct file f = sndh_read_file(path);

    if (!file_valid(f)) {
        fprintf(stderr, "%s: not a valid SNDH file\n", path);
        return 1;
    }

    sndh_for_each_tag (f.data, f.size)
        fprintf(stderr, "%s %s\n", sndh_tag_name, sndh_tag_value);

    struct psgplay *pp = psgplay_init(f.data, f.size, subtune, 44100);

    if (!pp) {
        fprintf(stderr, "%s: failed to play file\n", path);
        return 1;
    }

    struct sndh_timer timer;
    bool ret = sndh_tag_timer(&timer, f.data, f.size);

    if (!ret) {
        fprintf(stderr, "cannot determine timer frequency, assuming 50Hz\n");
        timer_frequency = 50;
    } else {
        timer_frequency = timer.frequency;
    }

    if (sndh_tag_subtune_count(&subtune_count, f.data, f.size)) {
        fprintf(stderr, "number of subtunes: %d\n", subtune_count);
        if (sndh_tag_default_subtune(&subtune_default, f.data, f.size)) {
            fprintf(stderr, "default subtune: %d\n", subtune_default);
            if (!subtune_override)
                subtune = subtune_default;
        }
        if (subtune_override) {
            if (subtune < 1 || subtune > subtune_count) {
                fprintf(stderr, "invalid subtune %d selected!\n", subtune);
                return 1;
            }
        }
    }

    fprintf(stderr, "dumping subtune %d\n", subtune);

    unsigned int hash = 0;
    calculate_hash(f.data, 32, &hash);
    calculate_hash(f.data, f.size, &hash);

    fprintf(stderr, "timedb hash: %08x\n", hash);

    unsigned int frames;
    int flags;
    if (timedb_get(hash, subtune-1, &frames, &flags) >= 0) {
        fprintf(stderr, "hash found!\n");
        if (!seconds_override) {
            fprintf(stderr, "setting time to dump\n");
            seconds = frames / timer_frequency;
        } else {
            fprintf(stderr, "using command line override of seconds\n");
        }
    } else {
        fprintf(stderr, "hash not found\n");
    }

    stop_position = timer_frequency * seconds;

    fprintf(stderr, "dumping %d seconds of data\n", seconds);

    fprintf(stderr, "writing output to %s\n", outfile);

    FILE *out = fopen(outfile, "wb");
    if (!out) {
        fprintf(stderr, "%s: cannot open for writing\n", outfile);
        return 1;
    }

    write_ym_header(out, 2000000, timer_frequency);

    struct cb_vars cb_vars = { out, 0, stop_position, 8000000/timer_frequency };

    psgplay_instruction_callback(pp, cb, &cb_vars);

    for (;;) {
        const ssize_t r = psgplay_read_stereo(pp, NULL, 4096);
        if (r <= 0) {
            fprintf(stderr, "unexpected end of audio data\n");
            break;
        }
        if (cb_vars.counter >= stop_position)
            break;
    }

    psgplay_free(pp);
    file_free(f);

    fseek(out, 12, SEEK_SET);

    uint8_t dword[4];
    dword[0] = (stop_position >> 24) & 0xff;
    dword[1] = (stop_position >> 16) & 0xff;
    dword[2] = (stop_position >>  8) & 0xff;
    dword[3] =  stop_position        & 0xff;
    fwrite(dword, 1, 4, out);           // number of frames

    fclose(out);

    fprintf(stderr, "finished!\n");

    return 0;
}
