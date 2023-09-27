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
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <zlib.h>

#define ATARI_CLOCK      1773447L

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

static double framerate;
static int debug;
static int force_new;

enum chiptype {
    CHIP_SN76489,
    CHIP_GAMEBOY_DMG
};

union sn76489 {
    struct {
        uint16_t freq1;
        uint16_t att1;
        uint16_t freq2;
        uint16_t att2;
        uint16_t freq3;
        uint16_t att3;
        uint16_t ctrl;
        uint16_t attn;
    } v;
    uint16_t r[8];
};

enum {
    REG_FREQ1,
    REG_ATT1,
    REG_FREQ2,
    REG_ATT2,
    REG_FREQ3,
    REG_ATT3,
    REG_CTRL,
    REG_ATTN
};

enum {
    PERIODIC_NOISE,
    WHITE_NOISE
};

enum {
    SHIFT_512,
    SHIFT_1024,
    SHIFT_2048,
    SHIFT_TONE3
};

union gameboy_dmg {
    struct {
        uint8_t square1[5];
        uint8_t square2[5];
        uint8_t wave[5];
        uint8_t noise[5];
        uint8_t control[3];
        uint8_t unused[9];
        uint8_t wavetable[16];
    } v;
    uint8_t r[48];
};

#define LARGEST_CHIP (sizeof(union gameboy_dmg))

static uint8_t voltab[16];  // note that 0 is the loudest, and 15 is silent

#define DEFAULT_MAXPOKVOL 15
static unsigned int maxpokvol = DEFAULT_MAXPOKVOL;

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

static void sn_to_pokey(union sn76489 *sn, uint8_t *pokey, int channel,
                                                    struct vgm_header *v) {
    uint16_t snf = sn->r[channel<<1];
    uint16_t snv = sn->r[(channel<<1)+1];
    uint16_t ctrl = sn->v.ctrl;
    uint16_t attn = sn->v.attn;

    if (snf == 0) snf++;            // avoid div by 0

    double f = v->sn76489_clock / (2 * snf * 16);

    int POK = round((ATARI_CLOCK / 2.0 / f) - 7);

    pokey[0] = POK & 0xff;
    pokey[2] = (POK >> 8) & 0xff;

    int volume = voltab[snv];

    pokey[3] = 0xa0 + volume;

    if (channel == 2) {
        double noisef;
        int dist = 0xa0;
        double extradiv = 1.0;

        switch (ctrl & 3) {
        case SHIFT_512:   noisef =  512; break;
        case SHIFT_1024:  noisef = 1024; break;
        case SHIFT_2048:  noisef = 2048; break;
        case SHIFT_TONE3:
            noisef = snf;
            break;
        }

        // note: periodic noise is played with generator A because we
        // do not really have a generator to play that any way else.
        // it should have a tone, so it has a tone.

        if (ctrl & 4) {
            dist = 0x80;
        } else {
            extradiv = v->sn76489_shift_width;
            if ((ctrl & 3) == SHIFT_TONE3)
                extradiv *= 8;
        }

        f = v->sn76489_clock / (2 * noisef) / extradiv;

        POK = round((ATARI_CLOCK / 2.0 / f) - 7);

        volume = voltab[sn->v.attn];

        pokey[4] = POK & 0xff;
        pokey[6] = (POK >> 8) & 0xff;
        pokey[7] = dist + volume;
    }
}

/* ------------------------------------------------------------------------ */

static void dmg_to_pokey(union gameboy_dmg *dmg, uint8_t *pokey, int channel,
                                                    struct vgm_header *v) {
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

static int write_sapr(gzFile file, struct vgm_header *v, enum chiptype chip) {
    uint8_t pokeyL[9], pokeyR[9];
    FILE *outleft, *outright;

    memset(pokeyL, 0, 9);
    memset(pokeyR, 0, 9);
    pokeyL[8] = pokeyR[8] = 0x78;

    outleft = fopen("left.sapr", "wb");
    if (!outleft) {
        fprintf(stderr, "unable to open left.sapr for writing\n");
        return -1;
    }

    outright = fopen("right.sapr", "wb");
    if (!outright) {
        fprintf(stderr, "unable to open right.sapr for writing\n");
        return -1;
    }

    fprintf(stderr, "writing SAP-R file to left.sapr and right.sapr\n");

    if (!write_sapr_header(outleft)) return -1;
    if (!write_sapr_header(outright)) return -1;

    uint32_t scnt = 0, nframes = 0;
    double fcnt = 0;
    double framelen = 44100 / framerate;

    int unknown = 0;
    int run = 1;
    int synced = 0;

    /* only one of them is used at the same time. multi chip files are NOT
     * supported, but we reuse the same timing/synchronization loop */

    union sn76489 sn;
    union gameboy_dmg dmg;

    uint8_t written[LARGEST_CHIP];

    memset(&sn, 0, sizeof(union sn76489));
    memset(&dmg, 0, sizeof(union gameboy_dmg));
    memset(written, 0, sizeof(written));

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
        case 0x31:      /* AY8910 */
        case 0x4f:      /* GG PSG */
            gzseek(file, 1, SEEK_CUR);
            break;      /* stereo mask ignored */

            /* SN76489 -- 0x50 dd */
        case 0x50: {
            uint8_t val = gzgetc(file);
            static uint8_t reg = 0;
            uint8_t dat;
            bool changed = false;

            if (val & 0x80) {
                reg = (val >> 4) & 0x07;

                if (sn.r[reg] & 0x0f != val & 0x0f)
                    changed = true;

                switch (reg) {
                case REG_FREQ1:
                case REG_FREQ2:
                case REG_FREQ3:
                    if (sn.r[reg] & 0x0f != val & 0x0f)
                        changed = true;
                    sn.r[reg] &= 0x03f0;
                    sn.r[reg] |= val & 0x0f;
                    break;
                case REG_ATT1:
                case REG_ATT2:
                case REG_ATT3:
                case REG_CTRL:
                case REG_ATTN:
                    sn.r[reg] = val & 0x0f;
                    break;
                }
            } else {
                if ((sn.r[reg] >> 4) != (val & 0x3f))
                    changed = true;

                sn.r[reg] &= 0x000f;
                sn.r[reg] |= (val & 0x3f) << 4;

                if (changed)
                    reg += 8;
            }

            if (changed) {
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
            }

            break;
            }

            /* GAMEBOY DMG -- 0xb3 aa dd */
        case 0xb3: {
            uint8_t aa = gzgetc(file);
            uint8_t dd = gzgetc(file);

            if (aa >= 0x30) {
                fprintf(stderr, "gameboy_dmg: aa=%02x out of range!\n", aa);
                break;
            }

            if (dmg.r[aa] != dd) {
                if (!written[aa]) {
                    written[aa]++;
                } else {
                    if (force_new) {
                        debug_fprintf(stderr, "*** new frame forced\n");
                        gzseek(file, -3, SEEK_CUR);
                        fcnt=framelen;              // force new frame
                        break;
                    }
                    written[aa]++;
                }
                dmg.r[aa] = dd;
            }
            break;
            }

            /* UNSUPPORTED -- xx aa dd */
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
        case 0xa0:
        case 0xb0:
        case 0xb1:
        case 0xb2:
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

            /* UNSUPPORTED -- xx + 3 bytes */
        case 0xc0:
        case 0xc1:
        case 0xc2:
        case 0xc3:
        case 0xc4:
        case 0xc5:
        case 0xc6:
        case 0xc7:
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
        case 0x80:  // YM2612 write + wait, we only wait
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
        case 0x86:
        case 0x87:
        case 0x88:
        case 0x89:
        case 0x8a:
        case 0x8b:
        case 0x8c:
        case 0x8d:
        case 0x8e:
        case 0x8f:
            wait = (c & 0xf);
            break;

            /* UNSUPPORTED -- xx + 4 bytes */
        case 0xe0:
        case 0xe1:
            gzseek(file, 4, SEEK_CUR);
            break;
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

            if (chip == CHIP_SN76489) {
                sn_to_pokey(&sn, &pokeyL[0], 0, v);
                sn_to_pokey(&sn, &pokeyL[4], 1, v);
                sn_to_pokey(&sn, &pokeyR[0], 2, v);
            } else if (chip == CHIP_GAMEBOY_DMG) {
                int newwave = 0;
                for (int x=0; x<16; x++)
                    newwave += written[0x20+x];
                if (newwave)
                    fprintf(stderr, "new wave written to wavetable\n");

                dmg_to_pokey(&dmg, &pokeyL[0], 0, v);
                dmg_to_pokey(&dmg, &pokeyL[4], 1, v);
                dmg_to_pokey(&dmg, &pokeyR[0], 2, v);
                dmg_to_pokey(&dmg, &pokeyR[4], 3, v);
            }

            fwrite(pokeyL, 1, 9, outleft);
            fwrite(pokeyR, 1, 9, outright);

            fcnt -= framelen;
            for (int i = 0; i<16; i++) {
                if (synced && written[i]>1)
                    debug_fprintf(stderr, "[%d] reg 0x%02x written to %d times\n", scnt, i, written[i]);
            }
            memset(written, 0, sizeof(written));
            synced = 1;
        }
    }

    nframes++;

    if (chip == CHIP_SN76489) {
        sn_to_pokey(&sn, &pokeyL[0], 0, v);
        sn_to_pokey(&sn, &pokeyL[4], 1, v);
        sn_to_pokey(&sn, &pokeyR[0], 2, v);
    } else if (chip == CHIP_GAMEBOY_DMG) {
        dmg_to_pokey(&dmg, &pokeyL[0], 0, v);
        dmg_to_pokey(&dmg, &pokeyL[4], 1, v);
        dmg_to_pokey(&dmg, &pokeyR[0], 2, v);
        dmg_to_pokey(&dmg, &pokeyR[4], 3, v);
    }

    fwrite(pokeyL, 1, 9, outleft);
    fwrite(pokeyR, 1, 9, outright);

    fclose(outleft);
    fclose(outright);

    return 0;
}

/* ------------------------------------------------------------------------ */

static void init_voltab_sn(int maxvol) {
    // fifteen steps of -2dB (-2/20 = -0.1)
    for (int i=0; i<16; i++) {
        voltab[i] = maxvol * pow(10.0, -0.1 * i);
    }
}

/* ------------------------------------------------------------------------ */

static void usage(void) {
    fprintf(stderr,
"usage: vgm2sapr [-options] file.vgm\n"
"\n"
"   -r rate     specify framerate (i.e. 50, 59.94, ...)\n"
"   -d          show debug output to analyze bad conversions\n"
"               try to reduce the number of bad writes by setting\n"
"               the proper framerate or slightly below, or try -f\n"
"   -f          force new frame on double write\n"
"   -p volume   pokey maximum per channel volume [default: 15]\n"
);

}

/* ------------------------------------------------------------------------ */

int main(int argc, char **argv) {
    int option;

    framerate = 0;

    while ((option = getopt(argc, argv, "dfhr:p:")) != -1) {
        switch (option) {
        case 'r':
            framerate = strtod(optarg, NULL);
            break;
        case 'd':
            debug = 1;
            break;
        case 'f':
            force_new = 1;
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
    fprintf(stderr, "maximum pokey volume: %d\n", maxpokvol);

    if (v->sn76489_clock) {
        init_voltab_sn(maxpokvol);

        fprintf(stderr, "detected SN76489\n");
        fprintf(stderr, "clock: %d Hz\n", v->sn76489_clock);
        if (v->version < 0x0110)
            v->sn76489_shift_width = 16;
        fprintf(stderr, "shift register width: %d bits\n", v->sn76489_shift_width);
        write_sapr(file, v, CHIP_SN76489);
    } else if (v->version >= 0x0161 && v->gameboy_dmg_clock) {
        fprintf(stderr, "detected GameBoy DMG\n");
        fprintf(stderr, "clock: %d Hz\n", v->gameboy_dmg_clock);
        write_sapr(file, v, CHIP_GAMEBOY_DMG);
    } else {
        fprintf(stderr, "no supported chip detected\n");
        return 1;
    }

    fprintf(stderr, "finished!\n");

    return 0;
}
