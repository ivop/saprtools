# YM2SAPR

Convert Atari ST .ym files to Atari XL/XE stereo SAP-R

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

