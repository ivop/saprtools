# SID2SAPR

Convert Commodore C64 .sid files to Atari XL/XE SAP-R

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

Also note that the buzzy/gritty bass versions are hit-or-miss. There's no vibrato or portamento possible with distortion C, so songs that utilize them will most of the time sound weird, with growling background noise. The softbass versions are almost always better, at the expense of a little bit more CPU time.  

### Options

```
./sid2sapr -h
usage: sid2sapr [options] sid-file

   -h          display help
   -b type     mono bass type (transpose [default], gritty, buzzy or softbass)
   -o file     output sap-r data to file [default: output.sapr]
   -p volume   pokey maximum per channel volume [default: 10, softbass: 9]
   -n num      override the number of frames to process
                                     [default: from hvsc database or 3000]
   -a          adjust for note cancellation (mono only)
   -t num      select track/subtune [default: from file or 1]
   -f          enable softbass off-by-one bassfix [default: off]
   -m which    mute ringmod and/or sync [default: none]
                   ringmod
                   sync
                   either (either one, but not both at the same time)
                   both (only both at the same time)
                   all (every combination)
   -d          damp ringmod to half volume
   -w value    let PWM influence the volume by factor [0.0-1.0]
   -s          enable stereo pokey mode [default: mono pokey]
   -x voice    extend one mono channel (0-2) to 16-bits [default: none]
               also selects HP filtered voice in stereo mode, see below

               [HIGHLY EXPERIMENTAL]
   -e type     extend one mono channel to sawtooth
               must be used in combination with -x
               vibrato and glissando distorts!
               use make player-softbass-sawtooth for faster stores
               type: 1 - fully merged table
                     2 - single table (distance of 1 only)
                     3 - 12-TET table (selected from type 1)
   -E factor   change non-sawtooth volume [0.0-1.0, default: 1.0]
               sometimes use -p to raise overall volume first
               also works for hpfiler (-F) option

   -F type     extend one channel to use HP filter
               must be used in combination with -x
               type: 1 - detuned channel +, muted
               type: 2 - detuned channel -, muted
               type: 3 - type 1, volume 50%
               type: 4 - type 2, volume 50%
               in stereo mode, one channel is extended to 32-bits
   -g cents    HP filter detune amount in cents
   -G value    adjust HP filter volume [0.0-1.0] (if type 3/4 is too loud)
   -D          transpose HP filter channel 1 octave down
