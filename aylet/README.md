# aylet

Resurrected ancient aylet 0.5.  
Added -y option to export .ym files in YM6! format with correct ZX spectrum
or CPC clock information. Output is "non-interleaved" (in the wrong way around sense of stsoundlib) and uncompressed.  
Conversion with ym2sapr runs succesfully and modern YM players play them
correctly. Old versions of STYmulator have a bug and are unable to play
"non-interleaved" YM files, because they overwrite the attribute dword.  
