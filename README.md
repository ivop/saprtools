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
| Sega Pico | SEGA PSG | .vgm | vgm2sapr |
| Tandy 1000 | NCR8496 | .vgm | vgm2sapr |
| IBM PCjr | SN76489 | .vgm | vgm2sapr |
| Nintendo Game Boy | Game Boy DMG | .vgm | vgm2sapr |
| Nintendo Game Boy Color | Game Boy DMG | .vgm | vgm2sapr |
| Nintendo Game Boy | Game Boy DMG | .gbs | gbsplay[^2], vgm2sapr |
| Nintendo Game Boy Color | Game Boy DMG | .gbs | gbsplay[^2], vgm2sapr |

### Notes

* This is not an emulator. Your Pokey does not magically start sounding like a SID chip, although the YM2149/AY-3-8910 and SN76489/Sega PSG conversions sound pretty close. Anything emulation related is done in the converters. The end result sounds like Pokey.
* Not all example conversions are necessarily done with the best parameters. After new options were added I did not painstakingly adjust each and every conversion to benefit from it. For example the -x option to sid2sapr is not (yet) widely used.
* Some conversions are just there for completeness but sound horrible. For example, some Hippel conversions with dist C bass sound bad because Hippel used a lot of vibrato in his bass lines which is not possible with dist C's fixed bass frequencies. The softbass and stereo versions sound much better.
* When doing your own conversions keep in mind that the converters and the Pokey players have limitations. The main goal was low CPU overhead in the Pokey players, no high CPU usage for emulation loops. So basically everything that would need lots of CPU time (if possible at all) is not converted. That includes ultra high speed ADSR envelopes to simulate sawtooths, anything done with timer interrupts outside of the sound chip, SID voice on the Atari ST, digi drums, and more. Some can, with some options tweaking, be converted to plain dist A notes (e.g. sawtooth bass on YM/AY chip), others will just not work.

### Credits

ym2sapr, vgm2ym, sid2sapr, sndh2ym, and vgm2sapr are Copyright © 2023 by Ivo van Poorten. Licensed under the terms of the BSD-2 license. See LICENSE for details.

aylet is Copyright © 2001-2005 by Russell Marks and Ian Collier. ym modifications are Copyright © 2023 by Ivo van Poorten. Licensed under the terms of the GPL-2. See COPYING for details.

lzss-sap is Copyright © 2020 by Daniel Serpell. MIT license. See LICENSE for details.  

libpsgplay is Copyright © 2019 by Fredrik Noring, GPL-2 license. M68k emulator is Copyright © 1998-2001 by Karl Stenerud, MIT license. See license directory for details.  

[^1]:https://github.com/digital-sound-antiques/kss2vgm  
[^2]:https://github.com/mmitch/gbsplay
