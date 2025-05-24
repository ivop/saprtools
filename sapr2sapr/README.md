This is used by the experimental ```sid2sapr``` conversions to generate
a stereo conversion with two channels on the left (one High Pass Filter)
and one channel on the right that uses the full pokey for a single voice.

```
usage: sapr2sapr [options] infile.sapr outfile.sapr

   -h                  display help
   -s reg=val[,..]     set channel to fixed value, comma separeted list
                           register = [0..8]
                           value    = [0..255] (decimal, octal, or hexadecimal)
```
