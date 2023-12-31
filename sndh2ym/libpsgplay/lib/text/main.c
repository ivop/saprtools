// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Fredrik Noring
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "psgplay/sndh.h"

#include "text/main.h"
#include "text/mode.h"

static void main_title(struct vt_buffer *vtb, int row, const char *title)
{
	for (int c = 0, i = 0; c < vtb->server.size.cols; c++)
		vt_putc_reverse(vtb, row, c, title[i] ? title[i++] : ' ');
}

static void main_track(struct vt_buffer *vtb, int track, bool playing)
{
	if (track)
		vt_printf(vtb, 3 + track, 1,
			playing ? vt_attr_reverse : vt_attr_normal,
			"%2d.", track);		/* FIXME: LED digits */
}

static void main_form(struct vt_buffer *vtb, const struct text_sndh *sndh)
{
	for (int i = 0; i < sndh->subtune_count; i++)
		main_track(vtb, i + 1, false);
}

static void main_data(struct vt_buffer *vtb, const struct text_sndh *sndh)
{
	int title_count = 0;
	int time_count = 0;
	int time_total = 0;

	sndh_for_each_tag (sndh->data, sndh->size)
		if (strcmp(sndh_tag_name, "COMM") == 0) {
			vt_printf(vtb, 1, 0, vt_attr_normal,
				"%s", sndh_tag_value);
		} else if (strcmp(sndh_tag_name, "!#SN") == 0) {
			title_count++;

			vt_printf(vtb, 3 + title_count, 5,
				vt_attr_normal, "%s", sndh_tag_value);
		}

	if (!title_count)
		for (int i = 0; i < sndh->subtune_count; i++)
			vt_printf(vtb, 4 + i, 5, vt_attr_normal,
				"%s", sndh->title);

	vt_printf(vtb, 2, 0, vt_attr_normal, "%s", sndh->title);

	sndh_for_each_tag (sndh->data, sndh->size)
		if (strcmp(sndh_tag_name, "TIME") == 0) {
			time_count++;

			char *end = NULL;
			const long v = strtol(sndh_tag_value, &end, 10);
			if (*end != '\0')
				continue;
			const int t = v;

			if (!t) {
				vt_printf(vtb, 3 + time_count, 34,
					vt_attr_normal, " --:--");

				time_total = -1;
			} else
				vt_printf(vtb, 3 + time_count, 34,
					vt_attr_normal, " %02d:%02d",
					t / 60, t % 60);

			if (time_total >= 0)
				time_total += t;
		}

	if (time_total > 0)
		vt_printf(vtb, 24, 35, vt_attr_reverse, "%02d:%02d",
			time_total / 60, time_total % 60);
	else if (time_total < 0)
		vt_printf(vtb, 24, 35, vt_attr_reverse, "--:--");
}

static void cursor_hide(struct vt_buffer *vtb, int cursor)
{
	if (cursor)
		vt_putc_normal(vtb, 3 + cursor, 0, ' ');
}

static void cursor_show(struct vt_buffer *vtb, int cursor)
{
	if (cursor)
		vt_putc_normal(vtb, 3 + cursor, 0, '>');
}

static void cursor_update(struct vt_buffer *vtb,
	struct text_state *view, const struct text_state *model)
{
	if (view->cursor == model->cursor)
		return;

	cursor_hide(vtb, view->cursor);
	view->cursor = model->cursor;
	cursor_show(vtb, view->cursor);
}

static void track_update(struct vt_buffer *vtb,
	struct text_state *view, const struct text_state *model)
{
	if (view->track == model->track &&
	    view->op == model->op)
		return;

	main_track(vtb, view->track, false);

	view->track = model->track;
	view->op = model->op;

	if (view->op == TRACK_PLAY ||
	    view->op == TRACK_PAUSE)
		main_track(vtb, view->track, true);
}

static void volume_update(struct vt_buffer *vtb,
	struct text_state *view, const struct text_state *model)
{
	if (view->mixer.volume == model->mixer.volume)
		return;

	view->mixer.volume = model->mixer.volume;

	if (view->mixer.volume)
		vt_printf(vtb, vtb->server.size.rows - 1, 0, vt_attr_reverse,
			"Volume %d dB  ", view->mixer.volume);
	else
		vt_printf(vtb, vtb->server.size.rows - 1, 0, vt_attr_reverse,
			"               ");
}

static u64 time_update(struct vt_buffer *vtb, struct text_state *view,
	const struct text_state *model, u64 timestamp)
{
	const int col = vtb->server.size.cols - 5;

	if (view->op != TRACK_PLAY &&
	    view->op != TRACK_PAUSE) {
		vt_printf(vtb, 0, col, vt_attr_reverse, "     ");

		return 0;
	}

	if (view->timestamp <= timestamp) {
		const u64 p = model->pause_offset + (model->pause_timestamp ?
			timestamp - model->pause_timestamp : 0);
		const int s = (timestamp - model->timestamp - p) / 1000;
		const int m = s / 60;

		if (m < 60)
			vt_printf(vtb, 0, col, vt_attr_reverse,
				"%02d:%02d", m, s % 60);
		else
			vt_printf(vtb, 0, col, vt_attr_reverse, "--:--");

		view->timestamp = model->timestamp + p + (s + 1) * 1000;
	}

	return view->timestamp;
}

static void main_init(struct vt_buffer *vtb, struct text_state *view,
	const struct text_state *model, const struct text_sndh *sndh,
	u64 timestamp)
{
	vt_clear(vtb);

	main_title(vtb, 0, "PSG play");
	main_title(vtb, vtb->server.size.rows - 1, "");

	main_form(vtb, sndh);
	main_data(vtb, sndh);
}

static u64 main_view(struct vt_buffer *vtb, struct text_state *view,
	const struct text_state *model, const struct text_sndh *sndh,
	u64 timestamp)
{
	if (view->mode != model->mode) {
		view->mode = model->mode;

		main_init(vtb, view, model, sndh, timestamp);
	}

	cursor_update(vtb, view, model);

	track_update(vtb, view, model);

	volume_update(vtb, view, model);

	if (model->redraw)
		vt_redraw(vtb);

	return time_update(vtb, view, model, timestamp);
}

static bool valid_track(int track, const struct text_sndh *sndh)
{
	return 1 <= track && track <= sndh->subtune_count;
}

static void main_ctrl(const unicode_t key, struct text_state *ctrl,
	const struct text_state *model, const struct text_sndh *sndh)
{
	ctrl->redraw = false;

	switch (key) {
	case 0:
		break;
	case '\r':
		ctrl->track = ctrl->cursor;
		ctrl->op = TRACK_RESTART;
		break;
	case '1' ... '9': {
		const int track = key - '0';

		if (valid_track(track, sndh)) {
			ctrl->track = ctrl->cursor = track;
			ctrl->op = TRACK_RESTART;
		}
		break;
	}
	case 's':
		ctrl->op = TRACK_STOP;
		break;
	case ' ':
	case 'p':
		if (ctrl->op == TRACK_PLAY)
			ctrl->op = TRACK_PAUSE;
		else if (ctrl->op == TRACK_PAUSE)
			ctrl->op = TRACK_PLAY;
		break;
	case '<':
		if (valid_track(ctrl->track - 1, sndh)) {
			ctrl->track--;
			ctrl->cursor = ctrl->track;
			ctrl->op = TRACK_RESTART;
		}
		break;
	case '>':
		if (valid_track(ctrl->track + 1, sndh)) {
			ctrl->track++;
			ctrl->cursor = ctrl->track;
			ctrl->op = TRACK_RESTART;
		}
		break;
	case '-':
		ctrl->mixer.volume = max(-80, ctrl->mixer.volume - 1);
		break;
	case '+':
		ctrl->mixer.volume = min(0, ctrl->mixer.volume + 1);
		break;
	case 'k':
	case U_ARROW_UP:
		if (valid_track(ctrl->cursor - 1, sndh))
			ctrl->cursor--;
		break;
	case 'j':
	case U_ARROW_DOWN:
		if (valid_track(ctrl->cursor + 1, sndh))
			ctrl->cursor++;
		break;
	case '\014':	/* ^L */
		ctrl->redraw = true;
		break;
	case 'q':
	case '\033':	/* Escape */
		ctrl->op = TRACK_STOP;
		ctrl->quit = true;
		break;
	}
}

const struct text_mode text_mode_main = {
	.view = main_view,
	.ctrl = main_ctrl,
};
