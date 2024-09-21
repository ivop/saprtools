/*
 * @file    timedb68.c
 * @brief   music duration database
 * @author  http://sourceforge.net/users/benjihan
 *
 * Copyright (c) 1998-2016 Benjamin Gerard
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>

#define HBIT 32                 /* # of bit for hash     */
#define TBIT 6                  /* # of bit for track    */
#define WBIT 6                  /* # of bit for hardware */
#define FBIT (64-HBIT-TBIT-WBIT)        /* # of bit for frames   */
#define HFIX (32-HBIT)

#define TIMEDB_ENTRY(HASH,TRACK,FRAMES,FLAGS) \
  { 0x##HASH>>HFIX, TRACK-1, FLAGS, FRAMES }
#define E_EMPTY { 0,0,0,0 }

typedef struct {
    unsigned int hash:HBIT;     /* hash code              */
    unsigned int track:TBIT;    /* track number (0-based) */
    unsigned int flags:WBIT;    /* see enum               */
    unsigned int frames:FBIT;   /* length in frames       */
} dbentry_t;

enum {

    TDB_STE = 1 << 5,            /**< set if STE-DMA hardware is used. */

    TDB_PSG = 1 << 4,            /**< set if YM hardware is used. */

    TDB_TD = 1 << 3,             /**< set if MFP-timer D is used. */

    TDB_TC = 1 << 2,             /**< set if MFP-timer C is used. */

    TDB_TB = 1 << 1,             /**< set if MFP-timer B is used. */

    TDB_TA = 1 << 0,             /**< set if MFP-timer A is used. */
};

#define STE TDB_STE
#define YM  TDB_PSG
#define TA  TDB_TA
#define TB  TDB_TB
#define TC  TDB_TC
#define TD  TDB_TD
#define NA  0

static dbentry_t db[] = {
#include "timedb.inc"
    /* Add a bit of Supplemental empty space */
    E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY,
    E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY,
    E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY,
    E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY,
    E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY,
    E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY,
    E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY,
    E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY, E_EMPTY,
};

#define DB_MAX   (sizeof(db)/sizeof(*db))
#define DB_COUNT DB_MAX-64
#define DB_SORT  1

static dbentry_t db[DB_MAX];

static int dbcount = DB_COUNT;  /* entry count                 */
//static int dbmax = DB_MAX;      /* max entry                   */
static int dbsort = DB_SORT;    /* set if db is sorted         */
//static int dbchange = 0;        /* set if db has been modified */

static int cmp(const void *ea, const void *eb) {
    const dbentry_t *a = (const dbentry_t *)ea;
    const dbentry_t *b = (const dbentry_t *)eb;
    int v;

    v = a->hash - b->hash;
    if (!v)
        v = a->track - b->track;
    return v;
}

static dbentry_t *search_for(const dbentry_t * key) {
    if (!dbsort) {
        qsort(db, dbcount, sizeof(dbentry_t), cmp);
        dbsort = 1;
    }
    return (dbentry_t *) bsearch(key, db, dbcount, sizeof(dbentry_t), cmp);
}

int timedb_get(int hash, int track, unsigned int *frames, int *flags) {
    dbentry_t e, *s;

    e.hash = hash >> HFIX;
    e.track = track;
    s = search_for(&e);
    if (s) {
        if (frames)
            *frames = s->frames;
        if (flags)
            *flags = s->flags;
        return s - db;
    }
    return -1;
}
