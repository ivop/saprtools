#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <unistd.h>
#include "sidengine.h"

//---------------------------------------SOUNDCARD-VARS--------------
//
#define DEVICE_NAME "/dev/dsp"

static const int BUF_SIZE = 882 * 2;   // Audio Buffer size in uint8_t
static int audio_fd;                   // Audio Device Handler
static pthread_t soundcard_thread;     // Audio thread
static int play_active = 0;            // Audio thread active
static uint16_t soundbuffer[882 * 16]; // The soundbuffer, 16*50Hz WORD Buffer

//---------------------------------------SID-VARS--------------------
//
static uint16_t init_addr, play_addr;
static uint8_t actual_subsong, max_subsong, play_speed;
static char song_name[32], song_author[32], song_copyright[32];

void stop_playing(void) {
    if (play_active) {
        play_active = 0;
        pthread_join(soundcard_thread, NULL);
        // Reset Audio device
        ioctl(audio_fd, SNDCTL_DSP_RESET, 0);
        close(audio_fd);
    }
}

int is_little_endian(void) {
    static const unsigned long a = 1;

    return *(const unsigned char *)&a;

}

void soundcard_init(void) {
    int format, format_backup, stereo, speed;

    // If playing thread is active kill it
    stop_playing();

    format_backup = format = is_little_endian()? AFMT_S16_LE : AFMT_S16_BE;
    stereo = 0;
    speed = 44100;

    if ((audio_fd = open(DEVICE_NAME, O_WRONLY)) == -1) {
        perror(DEVICE_NAME);
        return;
    }
    if (ioctl(audio_fd, SNDCTL_DSP_SETFMT, &format) == -1) {
        perror("SNDCTL_DSP_SETFMT");
        close(audio_fd);
        return;
    }
    if (format != format_backup) {
        perror("FORMAT NOT SUPPORTED!");
        close(audio_fd);
        return;
    }
    if (ioctl(audio_fd, SNDCTL_DSP_STEREO, &stereo) == -1) {
        perror("SNDCTL_DSP_STEREO");
        close(audio_fd);
        return;
    }
    if (stereo != 0) {
        perror("CHANNEL MODE NOT SUPPORTED!");
        close(audio_fd);
        return;
    }
    if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &speed) == -1) {
        perror("SNDCTL_DSP_SPEED");
        close(audio_fd);
        return;
    }
}

static void *play_thread_func(void *arg) {
    int j;

    while (play_active) {
        if (play_speed == 0)    // Single Speed (50Hz)
        {
            for (j = 0; j < 8; j++) {
                cpuJSR(play_addr, 0);
                synth_render(&soundbuffer[882 * j], 882);
            }
        }

        if (play_speed == 1)    // Double Speed (100Hz)
        {
            for (j = 0; j < 16; j++) {
                cpuJSR(play_addr, 0);
                synth_render(&soundbuffer[441 * j], 441);
            }
        }

        int ret = write(audio_fd, soundbuffer, BUF_SIZE * 8);

        if (ret < 0) {
            fprintf(stderr, "error writing to audio device!\n");
            return NULL;
        }
    }
    return NULL;
}
void start_playing(uint16_t nplay_addr, uint8_t nplay_speed) {
    play_addr = nplay_addr;
    play_speed = nplay_speed;

    play_active = -1;
    pthread_create(&soundcard_thread, NULL, play_thread_func, NULL);
}

void show_info(int argc) {
    printf("TinySID v0.94 linux version\n");
    printf("(c)Copyright 1999-2006 T. Hinrichs and R. Sinsch.\n");
    printf("All rights reserved.\n\n");
    if (argc < 2) {
        printf("Usage: tinysid [sid-file]\n");
        exit(0);
    }
}
int main(int argc, char *argv[]) {
    show_info(argc);
    c64Init();
    synth_init(44100);
    soundcard_init();
    printf("Loading %s...", argv[1]);
    if (c64SidLoad
        (argv[1], &init_addr, &play_addr, &actual_subsong, &max_subsong,
         &play_speed, song_name, song_author, song_copyright) == 0) {
        printf("failed!.\n\n");
        exit(0);
    } else
        printf("OK.\n");
    printf("TITLE    : %s\n", song_name);
    printf("AUTHOR   : %s\n", song_author);
    printf("COPYRIGHT: %s\n\n", song_copyright);

    cpuJSR(init_addr, actual_subsong);
    printf("Playing... Hit return quit.\n");
    start_playing(play_addr, play_speed);
    getchar();
    stop_playing();
    return (0);
}
