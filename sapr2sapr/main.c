#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

static void usage(void) {
    fprintf(stderr, "usage: sapr2sapr [options] infile.sapr outfile.sapr\n\n"
"   -h                  display help\n"
"   -s reg=val[,..]     set channel to fixed value, comma separeted list\n"
"                           register = [0..8]\n"
"                           value    = [0..255] (decimal, octal, or hexadecimal)\n"
);
}

static bool parse_setvalue_args(char *args, int *setvalue) {
    char *p = strtok(args, ",\n");
    if (!p) {
invalid:
        fprintf(stderr, "invalid arguments to option '-s'\n");
        return false;
    }
    while (p) {
        char *q = strchr(p, '=');
        if (!q) goto invalid;
        long int ch = strtol(p, 0, 0);
        if (ch < 0 || ch > 8) {
            fprintf(stderr, "invalid channel number '%ld'\n", ch);
            goto invalid;
        }
        setvalue[ch] = strtol(q+1, 0, 0);
        if (setvalue[ch] < 0 || setvalue[ch] > 0xff) {
            fprintf(stderr, "value '%d' out of range [0..255]\n", setvalue[ch]);
            goto invalid;
        }
        p = strtok(NULL, ",\n");
    }
    return true;
}

int main(int argc, char **argv) {
    int option;
    int setvalue[9];
    bool do_setvalue = false;

    memset(setvalue, 0xff, sizeof(setvalue));

    while ((option = getopt(argc, argv, "hs:")) != -1) {
        switch (option) {
        case 's':
            if (!parse_setvalue_args(optarg, setvalue)) {
                return 1;
            }
            do_setvalue = true;
            break;
        case 'h':
        default:
            usage();
            return 0;
        }
    }

    if (optind+2 != argc) {
        fprintf(stderr, "wrong arguments\n");
        usage();
        return 1;
    }

    char *infile = argv[optind];
    char *outfile = argv[optind+1];

    printf("converting '%s' to '%s'\n", infile, outfile);

    int infd = open(infile, O_RDONLY);
    if (infd < 0) {
        perror(infile);
        return 1;
    }

    struct stat sb;
    if (fstat(infd, &sb) < 0) {
        perror(infile);
        return 1;
    }
    int file_size = sb.st_size;

    uint8_t *in = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, infd, 0);
    if (in == MAP_FAILED) {
        perror(infile);
        return 1;
    }

    int i = 0;
    while (1) {
        while (in[i] != 0x0a) i++;
        i++;
        if (in[i] == 0x0d && in[i+1] == 0x0a) { // data starts after empty line
            i += 2;
            break;
        }
    }

    int header_size = i;
    int data_size = file_size - header_size;
    int nframes = data_size / 9;

    printf("file size:    %8d\n", file_size);
    printf("header size:  %8d\n", header_size);
    printf("data size:    %8d\n", data_size);
    printf("pokey frames: %8d\n", nframes);
    if (data_size % 9) {
        fprintf(stderr, "error: data size is not a multiple of 9\n");
        return 1;
    }

    if (do_setvalue) {
        for (int i=0; i<9; i++) {
            if (setvalue[i] < 0) continue;
            printf("setting register '%d' to value '0x%02x'\n", i, setvalue[i]);
        }
    }

    int outfd = open(outfile, O_CREAT|O_WRONLY|O_TRUNC, 0666);
    if (outfd < 0) {
        perror(outfile);
        return 1;
    }

    if (write(outfd, in, header_size) < 0) {
        perror(outfile);
        return 1;
    }

    for (int i=0; i<nframes; i++) {
        uint8_t frame[9];
        memcpy(frame, in + header_size + i*9, 9);

        if (do_setvalue) {
            for (int i=0; i<9; i++) {
                if (setvalue[i] < 0) continue;
                frame[i] = setvalue[i];
            }
        }

        if (write(outfd, frame, 9) < 0) {
            perror(outfile);
            return 1;
        }
    }

    munmap(in, file_size);
    close(infd);
    close(outfd);
}
