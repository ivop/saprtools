/*
 * libatari800/init.c - Atari800 as a library - initialisation routines
 *
 * Copyright (c) 2012 Tomasz Krasuski
 * Copyright (C) 2012 Atari800 development team (see DOC/CREDITS)
 * Copyright (c) 2016-2019 Rob McMullen
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

#include <stdlib.h>
#include <string.h>

#include "atari.h"
#include "cfg.h"
#include "util.h"
#include "libatari800/init.h"
#include "libatari800/cpu_crash.h"


int LIBATARI800_Initialise(void)
{
	libatari800_continue_on_brk = FALSE;
	return TRUE;
}

int LIBATARI800_ReadConfig(char *option, char *parameters)
{
	if (strcmp(option, "LIBATARI800_CONTINUE_ON_BRK") == 0)
		return (libatari800_continue_on_brk = Util_sscanbool(parameters)) != -1;
	else
		return FALSE;
	return TRUE;
}

void LIBATARI800_Exit(void)
{
}

/*
vim:ts=4:sw=4:
*/
