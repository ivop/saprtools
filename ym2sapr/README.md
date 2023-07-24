# YM2SAPR

Instructions:

```
# install https://github.com/dmsc/lzss-sap
make
./ym2sapr "test/testym5 - seven gates of jambala - level 11.ym"
make compress
echo -n "7 gates of Jambala L11 - Jochen Hippel" > asm/songname.txt
make player
atari800 player.xex
```

