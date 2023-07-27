/* aylet 0.4, a .AY music file player.
 * Copyright (C) 2001-2004 Russell Marks and Ian Collier. See main.c for licence.
 *
 * drv-oss.c - OSS sound code, partly due to Matan Ziv-Av.
 */

#ifdef DRIVER_OSS

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <string.h>
#include "sound.h"

#include "driver.h"

extern int ymframecount;
extern int fadetime;
extern int stopafter;

/* using (4) 256 byte frags for 8kHz, scale up for higher */
#define BASE_SOUND_FRAG_PWR	8


/* returns non-zero on success, and adjusts freq/stereo args to reflect
 * what we've actually got.
 */
int driver_init(int *freqptr,int *stereoptr)
{
int frag,tmp;

if(play_to_stdout)
  {
  soundfd=1;
  return(1);
  }

if (ym_to_stdout) {     // minimal intrusion, write samples to /dev/null
    soundfd=open("/dev/null", O_WRONLY);
    if (soundfd < 0)
        return 0;
    write(1, "YM6!LeOnArD!", 12);

    int nframes = stopafter*50+200+fadetime*50;
    unsigned char dword[4];
    dword[0] = (nframes >> 24) & 0xff;
    dword[1] = (nframes >> 16) & 0xff;
    dword[2] = (nframes >>  8) & 0xff;
    dword[3] =  nframes        & 0xff;
    write(1, dword, 4);

    memset(dword,0,4);
    write(1, dword, 4);         // attributes, not "interleaved"
    write(1, dword, 2);         // zero digidrums
 
    dword[0] = (1773400 >> 24) & 0xff;
    dword[1] = (1773400 >> 16) & 0xff;
    dword[2] = (1773400 >>  8) & 0xff;
    dword[3] =  1773400        & 0xff;
    write(1, dword, 4);         // master clock
 
    memset(dword,0,4);
    dword[1] = 50;
    write(1, dword, 2);         // playrate

    memset(dword,0,4);
    write(1, dword, 4);         // loop frame
    write(1, dword, 2);         // skip

    write(1, "name\000", 5);
    write(1, "author\000", 7);
    write(1, "comment\000", 8);

    return 1;
}

if((soundfd=open("/dev/dsp",O_WRONLY))<0)
  return(0);

tmp=AFMT_S16_NE;
if(!sixteenbit || ioctl(soundfd,SNDCTL_DSP_SETFMT,&tmp)==-1)
  {
  /* try 8-bit - may be a 8-bit-only device */
  tmp=AFMT_U8;
  if((ioctl(soundfd,SNDCTL_DSP_SETFMT,&tmp))==-1)
    {
    close(soundfd);
    return(0);
    }

  sixteenbit=0;
  }

tmp=(*stereoptr)?1:0;
if(ioctl(soundfd,SNDCTL_DSP_STEREO,&tmp)<0)
  {
  /* if it failed, make sure the opposite is ok */
  tmp=(*stereoptr)?0:1;
  if(ioctl(soundfd,SNDCTL_DSP_STEREO,&tmp)<0)
    {
    close(soundfd);
    return(0);
    }
  *stereoptr=tmp;
  }

frag=(0x40000|BASE_SOUND_FRAG_PWR);

if(ioctl(soundfd,SNDCTL_DSP_SPEED,freqptr)<0)
  {
  close(soundfd);
  return(0);
  }

if(*freqptr>8250) frag++;
if(*freqptr>16500) frag++;
if(*freqptr>33000) frag++;
if(*stereoptr) frag++;
if(sixteenbit) frag++;

if(ioctl(soundfd,SNDCTL_DSP_SETFRAGMENT,&frag)<0)
  {
  close(soundfd);
  return(0);
  }

return(1);	/* success */
}

void driver_end(void)
{
if (ym_to_stdout) {
    fprintf(stderr, "framecount: %i\n", ymframecount);
}
if(soundfd>1)
  close(soundfd);
}


void driver_frame(signed short *data,int len)
{
static unsigned char buf8[4096];
unsigned char *data8=(unsigned char *)data;
int ret=0,ofs=0;

len<<=1;	/* now in bytes */

if(!sixteenbit)
  {
  signed short *src;
  unsigned char *dst;
  int f;

  src=data; dst=buf8;
  len>>=1;
  for(f=0;f<len;f++)
    *dst++=128+(int)((*src++)/256);
  
  data8=buf8;
  }

while(len)
  {
  ret=write(soundfd,data8+ofs,len);
  if(ret>0)
    ofs+=ret,len-=ret;
  }
}

#endif	/* DRIVER_OSS */
