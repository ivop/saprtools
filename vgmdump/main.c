/* vgmdump - by Ivo van Poorten - Copyright (C) 2023
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
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <zlib.h>

struct vgm_header {
    char ident[4];                      /* $00 */
    uint32_t eof_offset;
    uint32_t version;
    uint32_t sn76489_clock;

    uint32_t ym2413_clock;              /* $10 */
    uint32_t gd3_offset;
    uint32_t total_nsamples;
    uint32_t loop_offset;

    uint32_t loop_nsamples;             /* $20 */
    uint32_t rate;
    uint16_t sn76489_feedback;
    uint8_t  sn76489_shift_width;
    uint8_t  sn76489_flags;
    uint32_t ym2612_clock;

    uint32_t ym2151_clock;              /* $30 */
    uint32_t vgm_data_offset;
    uint32_t sega_pcm_clock;
    uint32_t spcm_interface;

    uint32_t rf5c68_clock;              /* $40 */
    uint32_t ym2203_clock;
    uint32_t ym2608_clock;
    uint32_t ym2610b_clock;

    uint32_t ym3812_clock;              /* $50 */
    uint32_t ym3526_clock;
    uint32_t y8950_clock;
    uint32_t ymf262_clock;

    uint32_t ymf278b_clock;             /* $60 */
    uint32_t ymf271_clock;
    uint32_t ymz280b_clock;
    uint32_t rf5c164_clock;

    uint32_t pwm_clock;                 /* $70 */
    uint32_t ay8910_clock;
    uint8_t  ay8910_chip_type;
    uint8_t  ay8910_flags;
    uint8_t  ay8910_ym2203_flags;
    uint8_t  ay8910_ym2608_flags;
    uint8_t  volume_modifier;
    uint8_t  reserved;
    uint8_t  loop_base;
    uint8_t  loop_modifier;

    uint32_t gameboy_dmg_clock;         /* $80 */
    uint32_t nes_apu_clock;
    uint32_t multipcm_clock;
    uint32_t upd7759_clock;

    uint32_t okim6258_clock;            /* $90 */
    uint8_t  okim6258_flags;
    uint8_t  k054539_flags;
    uint8_t  c140_chip_type;
    uint8_t  reserved2;
    uint32_t okim6295_clock;
    uint32_t k051649_k052539_clock;

    uint32_t k054539_clock;             /* $a0 */
    uint32_t huc6280_clock;
    uint32_t c140_clock;
    uint32_t k053260_clock;

    uint32_t pokey_clock;               /* $b0 */
    uint32_t qsound_clock;
    uint32_t scsp_clock;
    uint32_t extra_header_offset;

    uint32_t wonderswan_clock;          /* $c0 */
    uint32_t vsu_clock;
    uint32_t saa1099_clock;
    uint32_t es5503_clock;

    uint32_t es5505_es5506_clock;       /* $d0 */
    uint8_t  es5503_nchannels;
    uint8_t  es5505_es5506_nchannels;
    uint8_t  c352_clock_divider;
    uint8_t  reserved3;
    uint32_t x1_010_clock;
    uint32_t c352_clock;

    uint32_t ga20_clock;                /* $e0 */
    uint32_t reserved4;
} vgm_header;

/* ------------------------------------------------------------------------ */

static inline uint16_t readLE16(uint8_t *ptr) {
    return (ptr[1]<<8) + ptr[0];
}

static inline uint32_t readLE32(uint8_t *ptr) {
    return (ptr[3]<<24) + (ptr[2]<<16) + (ptr[1]<<8) + ptr[0];
}

/* ------------------------------------------------------------------------ */

static void usage(void) {
    fprintf(stderr, "usage: vgmconv file.vgm\n");
}

static void read_header(gzFile file) {
    gzread(file, &vgm_header, sizeof(struct vgm_header));
    for (uint8_t pos = 4; pos < sizeof(struct vgm_header); pos += 4) {
        uint8_t *ptr = (uint8_t *) &vgm_header;
        switch (pos) {
        case 0x28:      /* 16-bit LE */
            *(uint16_t *)ptr = readLE16(ptr);
            break;
        case 0x78:      /* blocks of four 8-bit values, leave alone */
        case 0x7c:
        case 0x94:
        case 0xd4:
            break;
        default:        /* 32-bit LE */
            *(uint32_t *)ptr = readLE32(ptr);
            break;
        }
    }
}

/* ------------------------------------------------------------------------ */

int main(int argc, char **argv) {
    int option;

    while ((option = getopt(argc, argv, "h")) != -1) {
        switch (option) {
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

    gzFile file;
    file = gzopen(argv[optind], "r");
    if (!file) {
        fprintf(stderr, "unable to open %s\n", argv[optind]);
        return 1;
    }

    read_header(file);

    if (strncmp(vgm_header.ident, "Vgm ", 4)) {
        fprintf(stderr, "Vgm header not found\n");
        return 1;
    }

}
