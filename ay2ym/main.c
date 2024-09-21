/*
 * ay2ym - Convert .ay to .ym file
 *
 * Copyright © 2024 Ivo van Poorten
 * Based on aylet 0.5, Copyright © 2001-2004 Russell Marks
 * Includes Z80 emulation, Copyright © 1994 Ian Collier
 * GPL-2 License, see LICENSE file for details
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "main.h"
#include "z80.h"

unsigned char mem[65536];

static unsigned int nframes, ymframecount;
struct aydata_tag aydata;

#define FRAME_STATES_48     (3500000/50)
#define FRAME_STATES_128    (3546900/50)
#define FRAME_STATES_CPC    (4000000/50)

static unsigned int ay_track, stopafter = 180, ay_current_reg;
static unsigned char ay_registers[16];
static int do_cpc;
static FILE *outf;
static char *ofname;
unsigned long tstates, tsmax = FRAME_STATES_128;

static void mem_init(int track) {
    static unsigned char intz[] = {
        0xf3,                   /* di */
        0xcd, 0, 0,             /* call init */
        0xed, 0x5e,             /* loop: im 2 */
        0xfb,                   /* ei */
        0x76,                   /* halt */
        0x18, 0xfa              /* jr loop */
    };
    static unsigned char intnz[] = {
        0xf3,                   /* di */
        0xcd, 0, 0,             /* call init */
        0xed, 0x56,             /* loop: im 1 */
        0xfb,                   /* ei */
        0x76,                   /* halt */
        0xcd, 0, 0,             /* call interrupt */
        0x18, 0xf7              /* jr loop */
    };
    int init, ay_1st_block, ourinit, interrupt;
    unsigned char *ptr;
    int addr, len, ofs;

#define GETWORD(x) (((*(x))<<8)|(*(x+1)))

    init = GETWORD(aydata.tracks[track].data_stacketc + 2);
    interrupt = GETWORD(aydata.tracks[track].data_stacketc + 4);
    ay_1st_block = GETWORD(aydata.tracks[track].data_memblocks);

    memset(mem + 0x0000, 0xc9, 0x0100);
    memset(mem + 0x0100, 0xff, 0x3f00);
    memset(mem + 0x4000, 0x00, 0xc000);
    mem[0x38] = 0xfb;           /* ei */

/* call first AY block if no init */
    ourinit = (init ? init : ay_1st_block);

    if (!interrupt)
        memcpy(mem, intz, sizeof(intz));
    else {
        memcpy(mem, intnz, sizeof(intnz));
        mem[9] = interrupt % 256;
        mem[10] = interrupt / 256;
    }

    mem[2] = ourinit % 256;
    mem[3] = ourinit / 256;

/* now put the memory blocks in place */
    ptr = aydata.tracks[track].data_memblocks;
    while ((addr = GETWORD(ptr)) != 0) {
        len = GETWORD(ptr + 2);
        ofs = GETWORD(ptr + 4);
        if (ofs >= 0x8000)
            ofs = -0x10000 + ofs;

        /* range check */
        if (ptr - 4 - aydata.filedata + ofs >= aydata.filelen ||
            ptr - 4 - aydata.filedata + ofs < 0) {
            ptr += 6;
            continue;
        }

        /* fix any broken length */
        if (ptr + 4 + ofs + len >= aydata.filedata + aydata.filelen)
            len = aydata.filedata + aydata.filelen - (ptr + 4 + ofs);
        if (addr + len > 0x10000)
            len = 0x10000 - addr;

        memcpy(mem + addr, ptr + 4 + ofs, len);
        ptr += 6;
    }
}

static int read_ay_file(char *filename) {
    FILE *in;
    unsigned char *data, *buf, *ptr, *ptr2;
    int data_alloc = 16384, buf_alloc = 16384, data_ofs = 0;
    int data_len;
    int ret, tmp, f;

    if ((buf = malloc(buf_alloc)) == NULL)
        return 0;

    if ((data = malloc(data_alloc)) == NULL) {
        free(buf);
        return 0;
    }

    if ((in = fopen(filename, "rb")) == NULL) {
        free(buf);
        free(data);
        return 0;
    }

    while ((ret = fread(buf, 1, buf_alloc, in)) > 0) {
        if (data_ofs + ret >= data_alloc) {
            unsigned char *oldptr = data;

            data_alloc += buf_alloc;
            if ((data = realloc(data, data_alloc)) == NULL) {
                fclose(in);
                free(oldptr);
                free(buf);
                return (0);
            }
        }

        memcpy(data + data_ofs, buf, ret);
        data_ofs += ret;
    }

    free(buf);

    if (ferror(in)) {
        fclose(in);
        free(data);
        return 0;
    }

    fclose(in);

    data_len = data_ofs;

    if (memcmp(data, "ZXAYEMUL", 8) != 0) {
        free(data);
        return 0;
    }

    aydata.tracks = NULL;

#define READWORD(x)	(x)=256*(*ptr++); (x)|=*ptr++
#define READWORDPTR(x)	READWORD(tmp); \
		if(tmp>=0x8000) tmp=-0x10000+tmp; \
		if(ptr-data-2+tmp>=data_len || ptr-data-2+tmp<0) \
		  { \
                  free(data); \
                  if(aydata.tracks) free(aydata.tracks); \
		  return 0; \
                  } \
		(x)=ptr-2+tmp
#define CHECK_ASCIIZ(x) \
		if(!memchr((x),0,data+data_len-(x))) \
		  { \
                  free(data); \
                  if(aydata.tracks) free(aydata.tracks); \
		  return 0; \
                  }

    ptr = data + 8;
    aydata.filever = *ptr++;
    aydata.playerver = *ptr++;
    ptr += 2;                   /* skip `custom player' stuff */
    READWORDPTR(aydata.authorstr);
    CHECK_ASCIIZ(aydata.authorstr);
    READWORDPTR(aydata.miscstr);
    CHECK_ASCIIZ(aydata.miscstr);
    aydata.num_tracks = 1 + *ptr++;
    aydata.first_track = *ptr++;

    READWORDPTR(ptr2);          /* skip to track info */
    ptr = ptr2;

    if ((aydata.tracks =
         malloc(aydata.num_tracks * sizeof(struct ay_track_tag))) == NULL) {
        free(data);
        return 0;
    }

    for (f = 0; f < aydata.num_tracks; f++) {
        READWORDPTR(aydata.tracks[f].namestr);
        CHECK_ASCIIZ(aydata.tracks[f].namestr);
        READWORDPTR(aydata.tracks[f].data);
    }

    for (f = 0; f < aydata.num_tracks; f++) {
        if (aydata.tracks[f].data - data + 10 > data_len - 4) {
            free(aydata.tracks);
            free(data);
            return 0;
        }

        ptr = aydata.tracks[f].data + 10;
        READWORDPTR(aydata.tracks[f].data_stacketc);
        READWORDPTR(aydata.tracks[f].data_memblocks);

        ptr = aydata.tracks[f].data + 4;
        READWORD(aydata.tracks[f].fadestart);
        READWORD(aydata.tracks[f].fadelen);
    }

    aydata.filedata = data;
    aydata.filelen = data_len;
    return 1;
}

// return 0 if we want to exit emulation
int do_interrupt(void) {
    ymframecount++;
    fwrite(ay_registers, 14, 1, outf);
    fputc(0, outf);
    fputc(0, outf);

    if (ymframecount == nframes) return 0;

    return 1;
}

unsigned int in(int h,int l) {
    return 0xff;
}

static void sound_ay_reset_cpc(void) {
    memset(ay_registers, 0, sizeof(ay_registers));
}

static void sound_beeper(int on) {
    static int does_beeper;
    if (!does_beeper) puts("warning: file uses beeper");
    does_beeper = 1;
}

static void sound_ay_write(int reg,int val,unsigned long tstates) {
    ay_registers[reg] = val;
}

unsigned int out(int h, int l, int a) {
    static int cpc_f4 = 0;

/* unlike a real speccy, it seems we should only emulate exact port
 * number matches, rather than using bitmasks.
 */
    if (do_cpc < 1)
        switch (l) {
        case 0xfd:
            switch (h) {
            case 0xff:
                do_cpc = -1;
              write_reg:
                ay_current_reg = (a & 15);
                break;
            case 0xbf:
                do_cpc = -1;
              write_dat:
                sound_ay_write(ay_current_reg, a, tstates);
                break;
            default:
                /* ok, since we do at least have low byte=FDh,
                 * do bitmask for top byte to allow for
                 * crappy .ay conversions. But don't disable
                 * CPC autodetect, just in case.
                 */
                if ((h & 0xc0) == 0xc0)
                    goto write_reg;
                if ((h & 0xc0) == 0x80)
                    goto write_dat;
            }
            break;

        case 0xfe:
            do_cpc = -1;
            sound_beeper(a & 0x10);
            break;
        }

    if (do_cpc > -1)
        switch (h) {
        case 0xf6:
            switch (a & 0xc0) {
            case 0x80:         /* write */
                sound_ay_write(ay_current_reg, cpc_f4, tstates);
                break;

            case 0xc0:         /* select */
                ay_current_reg = (cpc_f4 & 15);
                break;
            }
            break;

        case 0xf4:
            cpc_f4 = a;
            if (!do_cpc) {
                /* restart as a more CPC-ish emulation */
                do_cpc = 1;
                sound_ay_reset_cpc();
                tsmax = FRAME_STATES_CPC;
                if (tstates > tsmax)
                    tstates -= tsmax;
            }
            break;
        }

    return 0;                 /* additional t-states */
}

// ----------------------------------------------------------------------------
//
static void write_ym_header(void) {
    fputs("YM6!LeOnArD!", outf);

    unsigned char dword[4];
    dword[0] = (nframes >> 24) & 0xff;
    dword[1] = (nframes >> 16) & 0xff;
    dword[2] = (nframes >>  8) & 0xff;
    dword[3] =  nframes        & 0xff;
    fwrite(dword, 4, 1, outf);

    memset(dword,0,4);
    fwrite(dword, 4, 1, outf);  // attributes, not "interleaved"
    fwrite(dword, 2, 1, outf);  // zero digidrums

    int clock = do_cpc ? 1000000 : 1773400;
    dword[0] = (clock >> 24) & 0xff;
    dword[1] = (clock >> 16) & 0xff;
    dword[2] = (clock >>  8) & 0xff;
    dword[3] =  clock        & 0xff;
    fwrite(dword, 4, 1, outf); // master clock

    memset(dword,0,4);
    dword[1] = 50;
    fwrite(dword, 2, 1, outf); // playrate

    memset(dword,0,4);
    fwrite(dword, 4, 1, outf);  // loop frame
    fwrite(dword, 2, 1, outf);  // skip

    fwrite("name\000", 5, 1, outf);
    fwrite("author\000", 7, 1, outf);
    fwrite("comment\000", 8, 1, outf);
}

// ----------------------------------------------------------------------------
//
static void usage(void) {
    fprintf(stderr, "usage: ay2ym [options] filename.ay\n\n"
"   -h              display help\n"
"   -A num          stop conversion after num seconds (default: 180)\n"
"   -c              force CPC\n"
"   -t num          convert specified track number (default: 1)\n"
"   -o filename.ym  write output to filename.ym instead of output.ym\n"
);
}

// ----------------------------------------------------------------------------
//
int main(int argc, char **argv) {
    puts("ay2ym - Copyright © 2024 Ivo van Poorten");

    int option;

    while ((option = getopt(argc, argv, "A:cho:t:")) != -1) {
        switch (option) {
        case 'h':
            usage();
            return 0;
            break;
        case 'A':
            stopafter = atoi(optarg);
            break;
        case 'c':
            do_cpc = 1;
            break;
        case 'o':
            ofname = strdup(optarg);
            break;
        case 't':
            ay_track = atoi(optarg) - 1;
            break;
        }
    }

    if (optind+1 != argc) {
        fprintf(stderr, "wrong arguments\n");
        usage();
        return 1;
    }

    if (!read_ay_file(argv[optind])) {
        fprintf(stderr, "failed to load '%s'\n", argv[optind]);
        return 1;
    }

    printf("AY File Version = %d\n", aydata.filever);
    printf("AY Player Version = %d\n", aydata.playerver);
    printf("Author: %s\n", (char *) aydata.authorstr ?: "unknown");
    printf("Misc: %s\n", (char *) aydata.miscstr ?: "unknown");
    printf("Number of tracks: %d\n", aydata.num_tracks);
    printf("Playing track: %d\n", ay_track+1);

    if (ay_track >= aydata.num_tracks) {
        fprintf(stderr, "invalid track number\n");
        return 1;
    }

    printf("Stop after %d seconds.\n", stopafter);

    if (stopafter < 1 || stopafter > 10*60) {
        fprintf(stderr, "invalid stop time\n");
        return 1;
    }

    if (!ofname) ofname = "output.ym";

    outf = fopen(ofname, "wb");
    if (!outf) {
        fprintf(stderr, "unable to open '%s' for writing\n", ofname);
        return 1;
    }

    nframes = stopafter * 50;  // xxx: +200+fadetime*50

    write_ym_header();

    mem_init(ay_track);
    z80loop(aydata.tracks[ay_track].data,
            aydata.tracks[ay_track].data_stacketc);

    fclose(outf);
    puts("Done.");
}
