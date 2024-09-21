#pragma once

struct ay_track_tag {
    unsigned char *namestr,*data;
    unsigned char *data_stacketc,*data_memblocks;
    int fadestart,fadelen;
};

struct aydata_tag {
    unsigned char *filedata;
    int filelen;
    struct ay_track_tag *tracks;

    int filever,playerver;
    unsigned char *authorstr,*miscstr;
    int num_tracks;
    int first_track;
};

#define FRAME_STATES_48     (3500000/50)
#define FRAME_STATES_128    (3546900/50)
#define FRAME_STATES_CPC    (4000000/50)

extern unsigned char mem[];
extern unsigned long tstates, tsmax;

extern int do_interrupt(void);
extern unsigned int in(int h,int l);
extern unsigned int out(int h,int l,int a);
