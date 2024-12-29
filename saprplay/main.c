#include <stdio.h>
#include <SDL.h>
#include <stdbool.h>
#include "mzpokey.h"

static struct mzpokey_context *mzp;
static char *sapr, *endsapr;
static bool play = true;

static void fill_audio(void *udata, Uint8 *stream, int len) {
    for (int i=0; i<=8; i++) {
        mzpokey_write_register(mzp, i, *sapr++, 0);
    }
    mzpokey_process(mzp, stream, len/2);
    if (sapr == endsapr) exit(0);
}

int main(int argc, char **argv) {
    FILE *fp;

    if (argc != 2) {
        printf("usage: saprplay filename.sapr\n");
        return 1;
    }

    // loading sapr file
    //
    if (!(fp = fopen(argv[1], "rb"))) {
        printf("error: unable to open %s\n", argv[1]);
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    sapr = malloc(size);
    if (!sapr) {
        printf("error: out of memory\n");
        return 1;
    }

    if (fread(sapr, size, 1, fp) != 1) {
        printf("error: error reading %s\n", argv[1]);
        return 1;
    }

    endsapr = sapr + size;

    sapr = strstr(sapr, "\r\n\r\n");
    if (!sapr) {
        printf("error: not a SAP-R file\n");
        return 1;
    }
    sapr += 4;

    if ((endsapr - sapr) % 9) {
        printf("error: SAP-R file does not contain a multiple of 9 bytes\n");
        return 1;
    }

    mzp = mzpokey_create(1773447, 44100, 1, 0);
    if (!mzp) {
        printf("error: unable to create mzpokey context\n");
        return 1;
    }
    mzpokey_write_register(mzp, SKCTL, 3, 0);

    SDL_AudioSpec wanted;

    wanted.freq = 44100;
    wanted.format = AUDIO_S16;
    wanted.channels = 1;
    wanted.samples = 882*2;               // one 50Hz frame @ 44100
    wanted.callback = fill_audio;
    wanted.userdata = NULL;

    if (SDL_OpenAudio(&wanted, NULL) < 0) {
        fprintf(stderr, "error: couldn't open audio: %s\n", SDL_GetError());
        return 1;
    }

    SDL_PauseAudio(0);

    while(play)
        SDL_Delay(100);
}
