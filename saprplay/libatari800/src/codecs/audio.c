/*
 * audio.c - interface for audio codecs
 *
 * Copyright (C) 2021 Rob McMullen
 * Copyright (C) 1998-2021 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/* This file is only included in compilation when audio recording enabled,
   so no need to do any #ifdef AUDIO_RECORDING directives. */

#include <stdio.h>
#include <stdlib.h>
#include "cfg.h"
#include "util.h"
#include "log.h"
#include "sound.h"
#include "pokeysnd.h"
#include "file_export.h"

#include "codecs/audio.h"
#include "codecs/audio_pcm.h"
#ifdef AUDIO_CODEC_MP3
#include "codecs/audio_mp3.h"
#endif
#include "codecs/audio_adpcm.h"
#include "codecs/audio_mulaw.h"

/* Global pointer to current audio codec, or NULL if one has not been
   initialized. This pointer should not be used after a call to
   audio_codec->end(). */
AUDIO_CODEC_t *audio_codec = NULL;

/* Global pointer to the audio codec parameters and summary data. This pointer
   may still be referenced after a call to audio_codec->end(), as it only holds
   static data from the last use of the audio codec. */
AUDIO_OUT_t *audio_out = NULL;

/* Global variables for the audio buffer and size. This buffer is used to hold
   audio frames generated by the codec. */
int audio_buffer_size = 0;
UBYTE *audio_buffer = NULL;

static AUDIO_CODEC_t *requested_audio_codec = NULL;

static AUDIO_CODEC_t *known_audio_codecs[] = {
	&Audio_Codec_PCM,
#ifdef AUDIO_CODEC_MP3
	&Audio_Codec_MP3,
#endif
	&Audio_Codec_MULAW,
	&Audio_Codec_PCM_MULAW,
	&Audio_Codec_ADPCM,
	&Audio_Codec_ADPCM_IMA,
	&Audio_Codec_ADPCM_YAMAHA,
	&Audio_Codec_ADPCM_MS,
	NULL,
};

#ifdef AUDIO_CODEC_MP3
int audio_param_bitrate = 128;
int audio_param_samplerate = -1;
int audio_param_quality = 4;
#endif

static AUDIO_CODEC_t *match_audio_codec(char *id)
{
	AUDIO_CODEC_t **a = known_audio_codecs;
	AUDIO_CODEC_t *found = NULL;

	while (*a) {
		if (Util_stricmp(id, (*a)->codec_id) == 0) {
			found = *a;
			break;
		}
		a++;
	}
	return found;
}

static AUDIO_CODEC_t *get_best_audio_codec(void)
{
	return &Audio_Codec_PCM;
}

static char *audio_codec_args(char *buf)
{
	AUDIO_CODEC_t **a = known_audio_codecs;

	strcpy(buf, "\t-acodec auto");
	while (*a) {
		strcat(buf, "|");
		strcat(buf, (*a)->codec_id);
		a++;
	}
	return buf;
}


int CODECS_AUDIO_Initialise(int *argc, char *argv[])
{

	int i;
	int j;

	for (i = j = 1; i < *argc; i++) {
		int i_a = (i + 1 < *argc); /* is argument available? */
		int a_m = FALSE; /* error, argument missing! */
		int a_i = FALSE; /* error, argument invalid! */

		if (strcmp(argv[i], "-acodec") == 0) {
			if (i_a) {
				char *mode = argv[++i];
				if (strcmp(mode, "auto") == 0) {
					requested_audio_codec = NULL; /* want best available */
				}
				else {
					requested_audio_codec = match_audio_codec(mode);
					if (!requested_audio_codec) {
						a_i = TRUE;
					}
				}
			}
			else a_m = TRUE;
		}
#ifdef AUDIO_CODEC_MP3
		else if (strcmp(argv[i], "-ab") == 0) {
			if (i_a) {
				audio_param_bitrate = Util_sscandec(argv[++i]);
				if (audio_param_bitrate < 8 || audio_param_bitrate > 320) {
					Log_print("Invalid bitrate; must be between 8 and 320 kbps");
					return FALSE;
				}
			}
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-ar") == 0) {
			if (i_a) {
				audio_param_samplerate = Util_sscandec(argv[++i]);
				if (audio_param_samplerate < 8000 || audio_param_samplerate > 48000) {
					Log_print("Invalid output samplerate; must be between 8000 and 48000 Hz");
					return FALSE;
				}
			}
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-aq") == 0) {
			if (i_a) {
				audio_param_quality = Util_sscandec(argv[++i]);
				if (audio_param_quality < 0 || audio_param_quality > 9) {
					Log_print("Invalid audio quality; must be between 0 and 9");
					return FALSE;
				}
			}
			else a_m = TRUE;
		}
#endif /* AUDIO_CODEC_MP3 */
		else {
			if (strcmp(argv[i], "-help") == 0) {
				char buf[256];
				Log_print(audio_codec_args(buf));
				Log_print("\t                 Select audio codec (default: auto)");
#ifdef AUDIO_CODEC_MP3
				Log_print("\t-ab <num>        Set audio recording bitrate in kbps (8..320, default: 128)");
				Log_print("\t-ar <num>        Set audio recording sample rate in Hz (8000..48000, default: same as -dsprate)");
				Log_print("\t-aq <num>        Set audio recording quality (0..9, default 5)");
#endif
			}
			argv[j++] = argv[i];
		}

		if (a_m) {
			Log_print("Missing argument for '%s'", argv[i]);
			return FALSE;
		} else if (a_i) {
			Log_print("Invalid argument for '%s'", argv[--i]);
			return FALSE;
		}
	}
	*argc = j;

	return TRUE;
}

int CODECS_AUDIO_ReadConfig(char *string, char *ptr)
{
	if (strcmp(string, "AUDIO_CODEC") == 0) {
		if (Util_stricmp(ptr, "auto") == 0) {
			requested_audio_codec = NULL; /* want best available */
		}
		else {
			requested_audio_codec = match_audio_codec(ptr);
			if (!requested_audio_codec) {
				return FALSE;
			}
		}
	}
	else return FALSE;
	return TRUE;
}

void CODECS_AUDIO_WriteConfig(FILE *fp)
{
	if (!requested_audio_codec) {
		fprintf(fp, "AUDIO_CODEC=AUTO\n");
	}
	else {
		fprintf(fp, "AUDIO_CODEC=%s\n", requested_audio_codec->codec_id);
	}
}

int CODECS_AUDIO_CheckType(char *codec_id)
{
	AUDIO_CODEC_t *a;

	if (!requested_audio_codec) {
		a = get_best_audio_codec();
	}
	else {
		a = requested_audio_codec;
	}
	return strcmp(codec_id, a->codec_id) == 0;
}

int CODECS_AUDIO_Init(void)
{
	int sample_size;
	float fps;

	if (!audio_codec) {
		if (!requested_audio_codec) {
			audio_codec = get_best_audio_codec();
		}
		else {
			audio_codec = requested_audio_codec;
		}
	}

	sample_size = POKEYSND_snd_flags & POKEYSND_BIT16? 2 : 1;
	if (sample_size == 1 && !(audio_codec->codec_flags & AUDIO_CODEC_FLAG_SUPPORTS_8_BIT_SAMPLES)) {
		File_Export_SetErrorMessageArg("16 bit audio needed for %s", audio_codec->codec_id);
		return 0;
	}

	fps = Atari800_tv_mode == Atari800_TV_PAL ? Atari800_FPS_PAL : Atari800_FPS_NTSC;
	audio_buffer_size = audio_codec->init(POKEYSND_playback_freq, fps, sample_size, POKEYSND_num_pokeys);
	if (audio_buffer_size < 0) {
		File_Export_SetErrorMessageArg("Failed init of %s codec", audio_codec->codec_id);
		return 0;
	}
	audio_buffer = (UBYTE *)Util_malloc(audio_buffer_size);
	audio_out = audio_codec->audio_out();

	return 1;
}

void CODECS_AUDIO_End(void)
{
	if (audio_codec)
		audio_codec->end();
	if (audio_buffer) {
		free(audio_buffer);
		audio_buffer_size = 0;
		audio_buffer = NULL;
	}
	audio_codec = NULL;
}
