# YM2SAPR

### Instructions

* Install https://github.com/dmsc/lzss-sap
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

