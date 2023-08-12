# SID2SAPR

Convert Commodore C64 .ym files to Atari XL/XE SAP-R

### Instructions

* Clone saprtools repo

```
cd saprtools/sid2sapr
make
./sid2sapr -n 3850 -b softbass sid/Freeze.sid
make compress-softbass
echo -n "Freeze - Laxity" > asm/songname.txt
make player-softbass
atari800 player.xex
```

Note that this not magically make Pokey sound like SID. It plays everything with Pokey sound and noise generators with very little CPU usage. If you want a full blown softsynth with sawtooths, pulse, and triangle, take a look at [AtariSid](https://github.com/ivop/atarisid).

Also note that the buzzy/gritty bass versions are hit-or-miss. There's no vibrato or portamento possible with distortion C, so songs that utilize those will most of the time sound weird, with growling background noise. The softbass versions are almost always better, at the expense of a little bit more CPU time.  
