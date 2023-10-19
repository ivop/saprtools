#include <stdio.h>
#include <SDL.h>
#include <stdbool.h>
#include "libatari800.h"

static char *argv_pal[]  = { "-config", "atari800-pal.cfg",  NULL };
static char *argv_ntsc[] = { "-config", "atari800-ntsc.cfg", NULL };

static char *sapr, *endsapr;
static char *bufpos;
static int bufleft;
static bool play = true;

void new_buf(void) {
    input_template_t input;
    emulator_state_t state;

    libatari800_get_current_state(&state);
    pokey_state_t *pokey = (pokey_state_t *)&state.state[state.tags.pokey];
    pokey->AUDF[0] = *sapr++;
    pokey->AUDC[0] = *sapr++;
    pokey->AUDF[1] = *sapr++;
    pokey->AUDC[1] = *sapr++;
    pokey->AUDF[2] = *sapr++;
    pokey->AUDC[2] = *sapr++;
    pokey->AUDF[3] = *sapr++;
    pokey->AUDC[3] = *sapr++;
    pokey->AUDCTL[0] = pokey->AUDCTL[1] = \
                       pokey->AUDCTL[2] = \
                       pokey->AUDCTL[3] = *sapr++;
    libatari800_restore_state(&state);

    libatari800_next_frame(&input);
    libatari800_clear_input_array(&input);

    bufleft = libatari800_get_sound_buffer_len();
    bufpos = libatari800_get_sound_buffer();

    if (sapr == endsapr)
        play = false;
}

void fill_audio(void *udata, Uint8 *stream, int len) {
    while (1) {
        if (bufleft > len) {
            memcpy(stream, bufpos, len);
            bufpos += len;
            bufleft -= len;
            return;
        }
        if (bufleft) {
            memcpy(stream, bufpos, bufleft);
            len -= bufleft;
            stream += bufleft;
        }
        new_buf();
    }
}

int main(int argc, char **argv) {
    emulator_state_t state;
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

    if (fread(sapr, 1, size, fp) != size) {
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

    // init libatari800
    //
    libatari800_init(-1, argv_pal);

    // set endless loop program
    //
    unsigned char *mem = libatari800_get_main_memory_ptr();
    mem[0x4000] = 0x4c; // jmp $4000
    mem[0x4001] = 0x00;
    mem[0x4002] = 0x40;

    libatari800_get_current_state(&state);
    ((pc_state_t *)&state.state[state.tags.pc])->PC = 0x4000;
    libatari800_restore_state(&state);

    // init Pokey
    //
    libatari800_get_current_state(&state);
    pokey_state_t *pokey = (pokey_state_t *)&state.state[state.tags.pokey];
    memset(pokey, 0, sizeof(pokey_state_t));
    pokey->SKCTL = 3;
    pokey->AUDF[0] = 0x47;
    pokey->AUDC[0] = 0xaf;
    libatari800_restore_state(&state);

    // get audio properties
    //
    int rate = libatari800_get_sound_frequency();
    int channels = libatari800_get_num_sound_channels();
    int sample_size = libatari800_get_sound_sample_size();

    if (sample_size != 2) {
        printf("error: libatari800 does not provide 16-bits samples");
        return 1;
    }

    // init audio
    //
    SDL_AudioSpec wanted;

    wanted.freq = rate;
    wanted.format = AUDIO_S16;
    wanted.channels = channels;
    wanted.samples = 1024;
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
