# saprtools

Tools for creating or manipulating Atari 8-bit SAP-R files.

### Contents

1. [Creating SAP-R Files](#creating-sap-r-files)
2. [Notes](#notes)
3. [Sample Conversions](#sample-conversions)
4. [Build Instructions](#build-instructions)
5. [Credits](#credits)

### Creating SAP-R Files

#### Conversion from other platforms

| | Platform | Soundchip | Filetype | Convert with...|
| --- | --- | --- | --- | --- |
| 1. | Atari ST | YM2149 | .ym | ym2sapr |
| | Atari ST | YM2149 | .sndh | sndh2ym, ym2sapr |
| 2. | ZX Spectrum | AY8910 | .ay | ay2ym, ym2sapr |
| 3. | Amstrad CPC | AY8910 | .ay | ay2ym, ym2sapr |
| 4. | MSX/MSX2 | AY8910 | .vgm | vgm2ym, ym2sapr |
| | MSX/MSX2 | AY8910 | .kss | kss2vgm[^1], vgm2ym, ym2sapr |
| 5. | Commodore 64 | SID | .sid | sid2sapr |
| 6. | BBC Micro | SN76489 | .vgm | vgm2sapr |
| 7. | SG-1000 | SN76489 | .vgm | vgm2sapr |
| 8. | Sega Master System | SEGA PSG | .vgm | vgm2sapr |
| 9. | Sega Game Gear | SEGA PSG | .vgm | vgm2sapr |
| 10. | Sega Pico | SEGA PSG | .vgm | vgm2sapr |
| 11. | Tandy 1000 | NCR8496 | .vgm | vgm2sapr |
| 12. | IBM PCjr | SN76489 | .vgm | vgm2sapr |
| 13. | Nintendo Game Boy | Game Boy DMG | .vgm | vgm2sapr |
| | Nintendo Game Boy | Game Boy DMG | .gbs | gbsplay[^2], vgm2sapr |
| 14. | Nintendo Game Boy Color | Game Boy DMG | .vgm | vgm2sapr |
| | Nintendo Game Boy Color | Game Boy DMG | .gbs | gbsplay[^2], vgm2sapr |
| 15. | TurboGrafx-16 / PC Engine | HuC6280 | .vgm | vgm2sapr |

### Notes

* This is not an emulator. Your Pokey does not magically start sounding like a SID chip, although the YM2149/AY-3-8910 and SN76489/Sega PSG conversions sound pretty close. Anything emulation related is done in the converters. The end result sounds like Pokey.
* Not all example conversions are necessarily done with the best parameters. After new options were added I did not painstakingly adjust each and every conversion to benefit from it. For example the -x option to sid2sapr is not (yet) widely used.
* Some conversions are just there for completeness but sound horrible. For example, some Hippel conversions with dist C bass sound bad because Hippel used a lot of vibrato in his bass lines which is not possible with dist C's fixed bass frequencies. The softbass and stereo versions sound much better.
* When doing your own conversions keep in mind that the converters and the Pokey players have limitations. The main goal was low CPU overhead in the Pokey players, no high CPU usage for emulation loops. So basically everything that would need lots of CPU time (if possible at all) is not converted. That includes ultra high speed ADSR envelopes to simulate sawtooths, anything done with timer interrupts outside of the sound chip, SID voice on the Atari ST, digi drums, and more. Some can, with some options tweaking, be converted to plain dist A notes (e.g. sawtooth bass on YM/AY chip), others will just not work.

### Sample Conversions

##### Stereo

Two Pokeys. Either four 16-bit channels, or eight 8-bit channels (six used by PC Engine conversions).

https://github.com/ivop/saprtools/tree/main/xex/ym2sapr/stereo  
https://github.com/ivop/saprtools/tree/main/xex/sid2sapr/stereo  
https://github.com/ivop/saprtools/tree/main/xex/vgm2sapr/stereo  

##### Mono with SOFTBASS

Single Pokey. Four 8-bit channels, with up to three channels with low-CPU IRQ-based SOFTBASS for two extra octaves of distortion A on the low end.
On emulation, you can turn on Altirra's sound monitor and see when channels switch to V (volume only) for the low basses.
Some SID conversions use two 8-bit channels and one 16-bit channel, but not all are adjusted to use that yet.

https://github.com/ivop/saprtools/tree/main/xex/ym2sapr/softbass  
https://github.com/ivop/saprtools/tree/main/xex/sid2sapr/softbass  

##### Mono with distortion C bass

Single Pokey with distortion C bass. These are hit or miss. No bass vibrato, so some sound bad. But it's the Pokey bass sound ;)
All were done with the "buzzy" type, which only becomes "gritty" when it goes lower than the range "buzzy" bass provides.

https://github.com/ivop/saprtools/tree/main/xex/ym2sapr/mono  
https://github.com/ivop/saprtools/tree/main/xex/sid2sapr/mono  

### Build Instructions

Run "make" in the main directory of the repository.
Alternatively enter the directory of the converter you want to build, and type "make".
Needed are a C and C++ compiler like CLang or GCC, and to assemble the Atari 8-bit players you need Mad Assembler[^3] (mads) available in your path.
All tools were tested on a recent Linux distribution, but should also work with WSL2 on Windows.
Native Windows with cygwin has been tested. Native windows with MSYS2 has not been tested yet, but some known portability issues were taken into consideration during development, like using "rb" with fopen() to open in binary mode.
Each directory contains its own README.md with sometimes more detailed instructions, a short usage example, and the help output listing all of its options.
The following lines need to be present in your "~/.gitconfig" to see binary differences for the involved files in git.
`[diff "hex"]`
`textconv = hexdump -v -C`
`binary = true`     

### Credits

ym2sapr, vgm2ym, sid2sapr, sndh2ym, and vgm2sapr are Copyright © 2023-2024 by Ivo van Poorten. Licensed under the terms of the BSD-2 license. See LICENSE for details.

ay2ym is Copyright © 2024 by Ivo van Poorten, Copyright © 2001-2005 by Russell Marks, Copyright © 1994 by Ian Collier. Licensed under the terms of the GPL-2. See LICESE for details.

lzss-sap is Copyright © 2020 by Daniel Serpell. MIT license. See LICENSE for details.  

libpsgplay is Copyright © 2019 by Fredrik Noring, GPL-2 license. M68k emulator is Copyright © 1998-2001 by Karl Stenerud, MIT license. See license directory for details.  

[^1]:https://github.com/digital-sound-antiques/kss2vgm  
[^2]:https://github.com/mmitch/gbsplay
[^3]:https://github.com/tebe6502/Mad-Assembler
