# VGM2SAPR

Convert SN76489 .vgm files to Atari XL/XE stereo SAP-R

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
