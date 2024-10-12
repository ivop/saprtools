#! /bin/sh
./sid2sapr -x 2 -b buzzy -m ringmod Hawkeye.sid
./lzss -6 output.sapr output.lz16
./mads -d:SID2SAPR -o:player-mono.xex asm/playlzs16-mono.asm

./sid2sapr -x 2 -b softbass -m ringmod Hawkeye.sid
./lzss -6 output.sapr output.lz16
./mads -d:SID2SAPR -o:player-softbass.xex asm/playlzs16-softbass.asm

./sid2sapr -s -m ringmod Hawkeye.sid
./lzss -6 left.sapr left.lz16
./lzss -6 right.sapr right.lz16
./mads -d:SID2SAPR -o:player-stereo.xex asm/playlzs16-stereo.asm
