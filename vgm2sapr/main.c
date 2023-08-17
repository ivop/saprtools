/* vgm2sapr - by Ivo van Poorten - Copyright (C) 2023
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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
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

    /* v1.01 additions */
    uint32_t rate;

    /* v1.10 additions */
    uint16_t sn76489_feedback;
    uint8_t  sn76489_shift_width;

    /* v1.51 additions */
    uint8_t  sn76489_flags;

    /* v1.10 additions */
    uint32_t ym2612_clock;

    uint32_t ym2151_clock;              /* $30 */

    /* v1.50 additions */
    uint32_t vgm_data_offset;

    /* v1.51 additions */
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

    /* v1.60 additions */
    uint8_t  volume_modifier;
    uint8_t  reserved;
    uint8_t  loop_base;

    /* v1.51 additions */
    uint8_t  loop_modifier;

    /* v1.61 additions */
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

    /* v1.71 additions */
    uint32_t scsp_clock;

    /* v1.70 additions */
    uint32_t extra_header_offset;

    /* v1.71 additions */
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

double framerate;
int debug;
int force_new;

#define debug_fprintf(stream, ...) \
    if (debug) fprintf(stream, __VA_ARGS__);

/* ------------------------------------------------------------------------ */

static inline uint16_t readLE16(uint8_t *ptr) {
    return (ptr[1]<<8) + ptr[0];
}

static inline uint32_t readLE32(uint8_t *ptr) {
    return (ptr[3]<<24) + (ptr[2]<<16) + (ptr[1]<<8) + ptr[0];
}

/* ------------------------------------------------------------------------ */

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

static int write_sapr(gzFile file, struct vgm_header *v, char *output) {
    FILE *outfile;

    if (output) {
        outfile = fopen(output, "wb");
        if (!outfile) {
            fprintf(stderr, "unable to open %s for writing\n", output);
            return -1;
        }
    }

    fprintf(stderr, "writing SAP-R file to %s\n", output);

    // write header

    uint32_t scnt = 0, nframes = 0;
    double fcnt = 0;
    double framelen = 44100 / framerate;

    int unknown = 0;
    int run = 1;
    int synced = 0;

    uint8_t registers[8];
    uint8_t written[8];

    memset(registers, 0, 8);
    memset(written, 0, 8);

    while (run) {
        int wait = 0;

        int c = gzgetc(file);

        switch (c) {
        case -1:
            fprintf(stderr, "premature end of file\n");
            return 1;
        case 0x67:
            gzgetc(file);
            int tt = gzgetc(file);
            uint32_t ss;
            gzfread((uint8_t *)&ss, 1, 4, file);
            ss = readLE32((uint8_t *)&ss);
            fprintf(stderr, "skipping data block type 0x%02x, size %08x\n", tt, ss);
            gzseek(file, ss, SEEK_CUR);
            break;
        case 0x31:
            break;      /* AY8910 stereo mask ignored */
        case 0x50:
            if (!unknown) {
                fprintf(stderr, "skipping unknown chipset\n");
                unknown = 1;
            }
            break;
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
        case 0x58:
        case 0x59:
        case 0x5a:
        case 0x5b:
        case 0x5c:
        case 0x5d:
        case 0x5e:
        case 0x5f:
        case 0xb0:
        case 0xb1:
        case 0xb2:
        case 0xb3:
        case 0xb4:
        case 0xb5:
        case 0xb6:
        case 0xb7:
        case 0xb8:
        case 0xb9:
        case 0xba:
        case 0xbb:
        case 0xbc:
        case 0xbd:
        case 0xbe:
        case 0xbf:
            if (!unknown) {
                fprintf(stderr, "skipping unknown chipset\n");
                unknown = 1;
            }
            gzseek(file, 2, SEEK_CUR);
            break;
        case 0xd0:
        case 0xd1:
        case 0xd2:
        case 0xd3:
        case 0xd4:
        case 0xd5:
        case 0xd6:
            if (!unknown) {
                fprintf(stderr, "skipping unknown chipset\n");
                unknown = 1;
            }
            gzseek(file, 3, SEEK_CUR);
            break;
        case 0x61:
            wait = gzgetc(file);
            wait += gzgetc(file) << 8;
            break;
        case 0x62:
            wait = 735;
            break;
        case 0x63:
            wait = 882;
            break;
        case 0x66:
            fprintf(stderr, "end of sound data\n");
            run = 0;
            break;
        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0x77:
        case 0x78:
        case 0x79:
        case 0x7a:
        case 0x7b:
        case 0x7c:
        case 0x7d:
        case 0x7e:
        case 0x7f:
            wait = (c & 0xf) + 1;
            break;
        case 0xa0: {
#if 0
            uint8_t reg = gzgetc(file) & 0x0f;
            uint8_t dat = gzgetc(file);

            if (registers[reg] != dat) {
                if (!written[reg]) {
                    written[reg]++;
                } else {
                    if (force_new) {
                        debug_fprintf(stderr, "*** new frame forced\n");
                        gzseek(file, -3, SEEK_CUR);
                        fcnt=framelen;              // force new frame
                        break;
                    }
                    written[reg]++;
                }
                registers[reg] = dat;
            }
            break;
#endif
            }
        default:
            fprintf(stderr, "unhandled data command %02x\n", c);
            return 0;
        }

        scnt += wait;

        if (!synced && wait > 400) {
            fcnt = framelen;
        } else {
            fcnt += wait;
        }

        while (fcnt >= framelen) {
            nframes++;
//            fwrite(registers, 1, 16, outfile);       // write to file

            fcnt -= framelen;
            for (int i = 0; i<16; i++) {
                if (synced && written[i]>1)
                    debug_fprintf(stderr, "[%d] reg 0x%02x written to %d times\n", scnt, i, written[i]);
            }
            memset(written, 0, 16);
            synced = 1;
        }
    }

    nframes++;
//    fwrite(registers, 1, 16, outfile);           // flush last registers

    fseek(outfile, 12, SEEK_SET);

    uint8_t dword[4];
    dword[0] = (nframes >> 24) & 0xff;
    dword[1] = (nframes >> 16) & 0xff;
    dword[2] = (nframes >>  8) & 0xff;
    dword[3] =  nframes        & 0xff;
    fwrite(dword, 1, 4, outfile);           // number of frames

    fclose(outfile);

    return 0;
}

/* ------------------------------------------------------------------------ */

static void usage(void) {
    fprintf(stderr,
"usage: vgm2sapr [-o output] file.vgm\n"
"\n"
"   -o  output      write output to file [default: stdout]\n"
"\n"
"   -r rate         specify framerate (i.e. 50, 59.94, ...)\n"
"\n"
"   -d              show debug output to analyze bad conversions\n"
"                   try to reduce the number of bad writes by setting\n"
"                   the proper framerate or slightly below, or try -f\n"
"\n"
"   -f              force new frame on double write\n"
);

}

/* ------------------------------------------------------------------------ */

int main(int argc, char **argv) {
    char *output = NULL;
    int option;

    framerate = 0;

    while ((option = getopt(argc, argv, "dfho:r:")) != -1) {
        switch (option) {
        case 'o':
            output = strdup(optarg);
            break;
        case 'r':
            framerate = strtod(optarg, NULL);
            break;
        case 'd':
            debug = 1;
            break;
        case 'f':
            force_new = 1;
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

    if (!output) {
        fprintf(stderr, "please specify an output file (-o file)\n");
        return 1;
    }

    gzFile file;
    file = gzopen(argv[optind], "r");
    if (!file) {
        fprintf(stderr, "unable to open %s\n", argv[optind]);
        return 1;
    }

    read_header(file);

    struct vgm_header *v = &vgm_header;

    if (strncmp(v->ident, "Vgm ", 4)) {
        fprintf(stderr, "Vgm header not found\n");
        return 1;
    }

    fprintf(stderr, "vgm version: %1x.%2x\n", v->version>>8, v->version &0xff);

    uint32_t data_offset = 0x40;
    if (v->version >= 0x150)
        data_offset = 0x34 + v->vgm_data_offset;

    fprintf(stderr, "data offset: 0x%02x\n", data_offset);

    gzseek(file, data_offset, SEEK_SET);

    if (framerate == 0.0) {
        if ((v->version >= 0x101 && !v->rate) || (v->version < 0x101)) {
            fprintf(stderr, "file does not specify framerate. use -r option\n");
            return 1;
        } 
        framerate = v->rate;
    }

    fprintf(stderr, "framerate: %.2f Hz\n", framerate);

    fprintf(stderr, "total number of samples: %d\n", v->total_nsamples);
    fprintf(stderr, "samples per frame: %.2f\n", 44100.0/framerate);
    fprintf(stderr, "song length: %.2f seconds\n", v->total_nsamples/44100.0);

#if 0
    if (v->ay8910_clock) {
        fprintf(stderr, "detected AY8910\n");
        write_ym6(file, v, output);
    } else {
        fprintf(stderr, "no supported chip detected\n");
        return 1;
    }
#endif

    fprintf(stderr, "finished!\n");

    return 0;
}
