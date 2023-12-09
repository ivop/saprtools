# VGM2SAPR

Convert SN76489, SEGA PSG, NCR8496, Game Boy DMG, and HuC6280 .vgm files to Atari XL/XE stereo SAP-R

### Instructions

* Clone saprtools repo

```
cd saprtools/vgm2sapr
make
./vgm2sapr sega/system-of-masters.vgz
make compress
echo -n "System Of Masters - Alex Mauer" > asm/songname.txt
make player60
atari800 player.xex
```

### Options

```
./vgm2sapr -h
usage: vgm2sapr [-options] file.vgm

   -r rate     specify framerate (i.e. 50, 59.94, ...)
   -d          show debug output to analyze bad conversions
               try to reduce the number of bad writes by setting
               the proper framerate or slightly below, or try -f
   -f          force new frame on double write
   -p volume   pokey maximum per channel volume [default: 15]
   -m div      mute high up to pokey divisor div [default: 0]
```
