# YM2SAPR

Convert Atari ST .ym files to Atari XL/XE mono or stereo SAP-R

### Instructions

* Clone saprtools repo

```
cd saprtools/ym2sapr
make
./ym2sapr "test/testym5 - seven gates of jambala - level 11.ym"
make compress
echo -n "7 gates of Jambala L11 - Jochen Hippel" > asm/songname.txt
make player
atari800 player.xex
```

Note that the mono versions with buzzy/gritty bass are hit-or-miss. If the song utilizes portamento or vibrato in the bass there will be parts where the bass sounds growling. The softbass versions are almost always better, at the expense of a little bit more CPU time. Stereo sounds closest to a real AY/YM. Nevertheless, Burnin'Rubber, for example, sounds pretty good in just mono with distortion C bass. As if it were made for a single Pokey all along :)  

### Options

```
./ym2sapr -h
usage: ym2sapr [-dhm][-efcprb value] input.ym

   -h          display help
   -d          disable envelopes
   -e volume   envelopes as fixed YM volume
   -f divider  use envelope frequency / divider as note
   -c clock    override master clock [default:2000000 or read from file]
   -p volume   pokey maximum per channel volume [default: 12, softbass: 11]
   -r map      remap channels [default: abc]
   -m          eneable mono pokey mode [default: stereo pokey]
   -b type     mono bass type (transpose [default], gritty, buzzy or softbass)
```
