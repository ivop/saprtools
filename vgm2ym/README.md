# vgm2ym

Convert vgm/vgz files with AY8910 content to YM6!

Use https://github.com/digital-sound-antiques/kss2vgm to convert MSX/MSX2 .kss files to .vgm.

### Options

```
./vgm2ym -h
usage: vgm2ym [-o output] file.vgm
   -o output       write output to file [default: stdout]
   -r rate         specify framerate (i.e. 50, 59.94, ...)
   -d              show debug output to analyze bad conversions
                   try to reduce the number of bad writes by setting
                   the proper framerate or slightly below, or try -f
   -f              force new frame on double write
```
