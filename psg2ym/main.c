/* psg2ym - by Ivo van Poorten - Copyright © 2024
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
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

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

int main(int argc, char **argv) {
    FILE *f, *g;

    if (argc != 3) {
        fprintf(stderr, "usage: %s input.psg output.ym\n", argv[0]);
        return 1;
    }

    if (!(f = fopen(argv[1], "rb"))) {
        fprintf(stderr, "%s: unable to open '%s'\n", argv[0], argv[1]);
        return 1;
    }

    char magic[16];
    if (fread(magic, 16, 1, f) != 1) {
        fprintf(stderr, "%s: error reading '%s'\n", argv[0], argv[1]);
        return 1;
    }

    if (memcmp(magic, "PSG\x1a\0\0\0\0\0\0\0\0\0\0\0\0", 16)) {
        fprintf(stderr, "%s: not a PSG file (%s)\n", argv[0], argv[1]);
        return 1;
    }

    if (!(g = fopen(argv[2], "wb"))) {
        fprintf(stderr, "%s: unable to open '%s'\n", argv[0], argv[2]);
        return 1;
    }

    write_ym_header(g, 1773400, 50.0);
    int nframes = 0, b, c;
    uint8_t registers[16] = {0};

    while ((b = fgetc(f)) >= 0) {
        if (b == 0xff) {
            fwrite(registers, 16, 1, g);
            nframes++;
            continue;
        }
        if ((c = fgetc(f)) < 0) {
            printf("%s: warning: premature EOF\n", argv[0]);
            fwrite(registers, 16, 1, g);
            nframes++;
            break;
        }
        registers[b] = c;
    }

    fseek(g, 12, SEEK_SET);

    uint8_t dword[4];
    dword[0] = (nframes >> 24) & 0xff;
    dword[1] = (nframes >> 16) & 0xff;
    dword[2] = (nframes >>  8) & 0xff;
    dword[3] =  nframes        & 0xff;
    fwrite(dword, 1, 4, g);                 // rewrite number of frames

    fclose(f);
    fclose(g);
    return 0;
}
