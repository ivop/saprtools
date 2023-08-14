/* sndh2ym Copyright (C) 2023 by Ivo van Poorten
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "psgplay/psgplay.h"
#include "psgplay/sndh.h"

#include "system/unix/file.h"
#include "system/unix/sndh.h"

#include "atari/machine.h"
#include "atari/mmu.h"
#include "atari/psg.h"

#define STOP_INSTR  0x4e72

static void cb(uint32_t pc, void *arg) {
    uint16_t insn = probe_read_memory_16(pc);

    if (insn == STOP_INSTR) {
        // psg_device.state.internal is not set to anything (should be
        // a pointer to psg_state which is a struct containing union psg
        // and reg_select), so we read the registers one by one
        fprintf(stderr, "psg: ");
        for (int i=0; i<16; i++) {
            psg_device.wr_u8(&psg_device, 0, i);
            fprintf(stderr, "%02x ", psg_device.rd_u8(&psg_device, 0));
        }
        fprintf(stderr, "\n");
    }
}

int main(int argc, char **argv) {
	if (argc <= 1) {
		fprintf(stderr, "usage: sndh2ym <sndh-file>...\n");
		return 1;
	}

	for (int arg = 1; arg < argc; arg++) {
		const char *path = argv[arg];
		struct file f = sndh_read_file(path);

		if (!file_valid(f)) {
            fprintf(stderr, "%s: Not a valid SNDH file\n", path);
            return 1;
        }

        struct sndh_timer timer;
        bool ret = sndh_tag_timer(&timer, f.data, f.size);

        if (ret) {
            fprintf(stderr, "timer freq: %d Hz\n", timer.frequency);
        }

        int subtune = 4;

		struct psgplay *pp = psgplay_init(f.data, f.size, subtune, 44100);
		if (!pp) {
			fprintf(stderr, "%s: Failed to play file\n", path);
            return 1;
        }

        psgplay_instruction_callback(pp, cb, 0);

		for (;;) {
			struct psgplay_stereo buffer[4096];
			const ssize_t r = psgplay_read_stereo(pp, buffer, 4096);

			if (r <= 0)
				break;

			const ssize_t s = sizeof(struct psgplay_stereo[r]);
			const ssize_t w = xwrite(STDOUT_FILENO, buffer, s);

			if (w != s)
				break;
		}

		psgplay_free(pp);

		file_free(f);
	}

	return 0;
}
