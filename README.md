# saprtools
Tools for creating or manipulating Atari 8-bit SAP-R files.

### Creating SAP-R Files

#### Conversion from other platforms

| Platform | Soundchip | Filetype | Convert with...|
| --- | --- | --- | --- |
| Atari ST | YM2149 | .ym | ym2sapr |
| Atari ST | YM2149 | .sndh | sndh2ym, ym2sapr |
| ZX Spectrum | AY8910 | .ay | aylet -y, ym2sapr |
| Amstrad CPC | AY8910 | .ay | aylet -y, ym2sapr |
| MSX/MSX2 | AY8910 | .vgm | vgm2ym, ym2sapr |
| MSX/MSX2 | AY8910 | .kss | kss2vgm[^1], vgm2ym, ym2sapr |
| Commodore 64 | SID | .sid | sid2sapr |
| BBC Micro | SN76489 | .vgm | vgm2sapr |
| SG-1000 | SN76489 | .vgm | vgm2sapr |
| Sega Master System | SEGA PSG | .vgm | vgm2sapr |
| Sega Game Gear | SEGA PSG | .vgm | vgm2sapr |

### Credits

ym2sapr, vgm2ym, sid2sapr, sndh2ym, and vgm2sapr are Copyright © 2023 by Ivo van Poorten. Licensed under the terms of the BSD-2 license. See LICENSE for details.

aylet is Copyright © 2001-2005 by Russell Marks and Ian Collier. ym modifications are Copyright © 2023 by Ivo van Poorten. Licensed under the terms of the GPL-2. See COPYING for details.

lzss-sap is Copyright © 2020 by Daniel Serpell. MIT license. See LICENSE for details.  

libpsgplay is Copyright © 2019 by Fredrik Noring, GPL-2 license. M68k emulator is Copyright © 1998-2001 by Karl Stenerud, MIT license. See license directory for details.  

[^1]:https://github.com/digital-sound-antiques/kss2vgm  
