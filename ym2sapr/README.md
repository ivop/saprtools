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

Note that the mono versions with buzzy/gritty bass are hit-or-miss. If the song utilizes portamento or vibrato in the bass there will be parts where the bass sounds growling. The softbass versions is almost always better, at the expense of a little bit more CPU time. Stereo sounds closest to a real AY/YM. Nevertheless, Burnin'Rubber, for example, sounds pretty good in just mono with distortion C bass. As if it were made for a single Pokey all along :)  
