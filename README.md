# saprtools
Tools for creating or manipulating Atari 8-bit SAP-R files.

### Creating SAP-R Files

#### Conversion from other platforms

*Except for Hatari, all tools are in this repository.*

| Platform | Soundchip | Filetype | Convert with...|
| --- | --- | --- | --- |
| Atari ST | YM2149 | .ym | Use ym2sapr |
| Atari ST | YM2149 | .sndh | Use Hatari to record .ym, then use ym2sapr |
| ZX Spectrum | AY8910 | .ay | Use aylet to convert to .ym, then use ym2sapr |
| Amstrad CPC | AY8910 | .ay | Use aylet to convert to .ym, then use ym2sapr |
| MSX1/MSX2 | AY8910 | .vgm | Use vgmconv to convert to .ym, then use ym2sapr |
