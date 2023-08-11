# SID2SAPR

Convert Commodore C64 .ym files to Atari XL/XE SAP-R

### Instructions

* Clone saprtools repo

```
cd saprtools/sid2sapr
make
./sid2sapr -n 3850 -b softbass sid/freeze.sid
make compress-softbass
echo -n "Freeze - Laxity" > asm/songname.txt
make player-softbass
atari800 player.xex
```

