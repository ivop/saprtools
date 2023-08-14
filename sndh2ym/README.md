# SNDH2YM

Convert .sndh files to .ym.

### Instructions

* Clone saprtools repo

```
cd saprtools/sndh2ym
make
./sndh2ym -o foo.ym foo.sndh
```

Note that this does not magically convert
digidrums or sid-voice or any other fancy effect. It _only_ dumps the YM
registers at a fixed interval. But that's good enough as ym2sapr does not
support anything else anyway, and it relieves us from the hassle of recording
with Hatari and cleaning up the .ym file afterwards.
