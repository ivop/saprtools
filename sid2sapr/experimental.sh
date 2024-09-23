#! /bin/sh

make

#if false; then

# Two channel sawtooth, has problems with vibrato because of high clock and
# fast changing of frequency. Some parts sound pretty good, others not so
# much...

./sid2sapr -p 12 -x 1 -e 1 -E 1.0 -b softbass sid/Syncopated.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
    "Commodore 64" "Syncopated" "Thomas E. Petersen (Laxity)" > songname.txt
make compress-softbass player50-softbass-sawtooth
mv player.xex experimental/exp-syncopated-saw.xex

./sid2sapr -p 15 -x 2 -e 2 -E 0.6 -b softbass sid/Metal_Warrior_4.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
    "Commodore 64" "Metal Warrior 4" "Cadaver" > songname.txt
make compress-softbass player50-softbass-sawtooth
mv player.xex experimental/exp-mw4-saw.xex

./sid2sapr -m both -p 10 -x 2 -e 2 -E 0.8 -b softbass sid/Zamzara.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
    "Commodore 64" "Zamzara" "Charles Deenen" > songname.txt
make compress-softbass player50-softbass-sawtooth
mv player.xex experimental/exp-zamzara-saw.xex

./sid2sapr -m ringmod -x 0 -e 1 -p 12 -E 0.8 -b softbass -t 1 sid/RoboCop.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
       "Commodore 64" "Robocop" "Jonathan Dunn" > songname.txt
make compress-softbass player50-softbass-sawtooth
mv player.xex experimental/exp-robocop-saw.xex

./sid2sapr -x 1 -p12 -E0.9 -F 1 -D -b softbass sid/R-Type.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
    "Commodore 64" "R-Type" "Chris Huelsbeck & Ramiro Vaca" > songname.txt
make compress-softbass player50-softbass
mv player.xex experimental/exp-rtype-hpf.xex

#fi

#if false; then

# Two channel "high-pass" filter, detune starts to sound out-of-tune when
# note frequency rises. Some parts sound really nice, other parts not so
# much... If only we could switch configurations at several places during
# the song ;-)

./sid2sapr -p10 -E0.9 -x 1 -F 3 -b softbass sid/Metal_Warrior_4.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
    "Commodore 64" "Metal Warrior 4" "Cadaver" > songname.txt
make compress-softbass player50-softbass
mv player.xex experimental/exp-mw4-hpf.xex

./sid2sapr -p10 -E0.9 -m both -x 2 -F 3 -D -b softbass sid/Zamzara.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
    "Commodore 64" "Zamzara" "Charles Deenen" > songname.txt
make compress-softbass player50-softbass
mv player.xex experimental/exp-zamzara-hpf.xex

./sid2sapr -m ringmod -x 0 -F 1 -D -t 1 -b softbass sid/RoboCop.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
       "Commodore 64" "Robocop" "Jonathan Dunn" > songname.txt
make compress-softbass player50-softbass
mv player.xex experimental/exp-robocop-hpf.xex

./sid2sapr -p11 -x 1 -F 1 -D -b softbass sid/Syncopated.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
    "Commodore 64" "Syncopated" "Thomas E. Petersen (Laxity)" > songname.txt
make compress-softbass player50-softbass
mv player.xex experimental/exp-syncopated-hpf.xex

./sid2sapr -p11 -x 0 -F 3 -D -b softbass sid/Freeze.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
    "Commodore 64" "Freeze" "Thomas E. Petersen (Laxity)" > songname.txt
make compress-softbass player50-softbass
mv player.xex experimental/exp-freeze-hpf.xex

#fi

#if false ; then

# Stereo HP filter, one full pokey for one channel. Now where talkin' ;)

# Most examples have the hpfilter channel transposed down one octave
# (which is sonically correct), but some very low parts sometimes sounded
# better when it was an octave higher. We really need an editor :D

./sid2sapr -m both -x 2 -s -F 3 -g 10.0 -G 0.9 -D sid/Zamzara.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
    "Commodore 64" "Zamzara" "Charles Deenen" > songname.txt
make compress-stereo player50-stereo
mv player.xex experimental/exp-zamzara-stereo-hpf.xex

./sid2sapr -x 2 -s -F 3 -g 10.0 -G 0.8 -D sid/Cybernoid.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
    "Commodore 64" "Cybernoid" "Jeroen Tel" > songname.txt
make compress-stereo player50-stereo
mv player.xex experimental/exp-cybernoid-stereo-hpf.xex

./sid2sapr -x 1 -m both -s -F 3 -g 10.0 -G 0.8 -D sid/Cybernoid_II.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
    "Commodore 64" "Cybernoid II" "Jeroen Tel" > songname.txt
make compress-stereo player50-stereo
mv player.xex experimental/exp-cybernoid2-stereo-hpf.xex

./sid2sapr -x 1 -s -F 1 -g 10.0 -D sid/Metal_Warrior_4.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
    "Commodore 64" "Metal Warrior 4" "Cadaver" > songname.txt
make compress-stereo player50-stereo
mv player.xex experimental/exp-mw4-stereo-hpf.xex

./sid2sapr -m ringmod -x 1 -s -F 1 -g 5.0 -D -t 1 sid/RoboCop.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
       "Commodore 64" "Robocop" "Jonathan Dunn" > songname.txt
make compress-stereo player50-stereo
mv player.xex experimental/exp-robocop-stereo-hpf.xex

#fi

# clear for further tests
rm -f songname.txt
