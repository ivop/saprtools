# saprtools
Tools for creating or manipulating Atari 8-bit SAP-R files.

### Creating SAP-R Files

#### Conversion from other platforms

| Platform | Soundchip | Filetype | Convert with...|
| --- | --- | --- | --- |
| Atari ST | YM2149 | .ym | Use ym2sapr |
| Atari ST | YM2149 | .sndh | Use Hatari[^1] to record .ym, then use ym2sapr |
| ZX Spectrum | AY8910 | .ay | Use aylet to convert to .ym, then use ym2sapr |
| Amstrad CPC | AY8910 | .ay | Use aylet to convert to .ym, then use ym2sapr |
| MSX/MSX2 | AY8910 | .vgm | Use vgm2ym to convert to .ym, then use ym2sapr |
| MSX/MSX2 | AY8910 | .kss | Use kss2vgm[^2], vgm2ym and finally ym2sapr |
| Commodore 64 | SID | .sid | Use sid2sapr |

### Credits

ym2sapr and vgm2ym are Copyright © 2023 by Ivo van Poorten. Licensed under the terms of the BSD-2 license. See LICENSE for details.

aylet is Copyright © 2001-2005 Russell Marks and Ian Collier. ym modifications are Copyright © 2023 by Ivo van Poorten. Licensed under the terms of the GPL-2. See COPYING for details.

lzss-sap is Copyright (c) 2020 Daniel Serpell. MIT license. See LICENSE for details.

[^1]:https://github.com/hatari  
[^2]:https://github.com/digital-sound-antiques/kss2vgm  
