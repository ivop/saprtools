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
mv player.xex experimental/exp-syncopated.xex

./sid2sapr -p 15 -x 2 -e 2 -E 0.6 -b softbass sid/Metal_Warrior_4.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
    "Commodore 64" "Metal Warrior 4" "Cadaver" > songname.txt
make compress-softbass player50-softbass-sawtooth
mv player.xex experimental/exp-mw4.xex

./sid2sapr -m both -p 10 -x 2 -e 2 -E 0.8 -b softbass sid/Zamzara.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
    "Commodore 64" "Zamzara" "Charles Deenen" > songname.txt
make compress-softbass player50-softbass-sawtooth
mv player.xex experimental/exp-zamzara.xex

./sid2sapr -m ringmod -x 0 -e 1 -p 12 -E 0.8 -b softbass -t 1 sid/RoboCop.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
       "Commodore 64" "Robocop" "Jonathan Dunn" > songname.txt
make compress-softbass player50-softbass-sawtooth
mv player.xex experimental/exp-robocop.xex

# fi

# Two channel "high-pass" filter, detune starts to sound out-of-tune when
# note frequency rises. Some parts sound really nice, other parts not so
# much... If only we could switch configurations at several places during
# the song ;-)

./sid2sapr -p10 -E0.9 -x 1 -F 1 -b softbass sid/Metal_Warrior_4.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
    "Commodore 64" "Metal Warrior 4" "Cadaver" > songname.txt
make compress-softbass player50-softbass
mv player.xex experimental/exp-mw4-hpf.xex

./sid2sapr -m both -x 2 -F 3 -b softbass sid/Zamzara.sid
printf "Source: %-32sTitle : %-32sAuthor: %s" \
    "Commodore 64" "Zamzara" "Charles Deenen" > songname.txt
make compress-softbass player50-softbass-sawtooth
mv player.xex experimental/exp-zamzara-hpf.xex

# clear for further tests
rm -f songname.txt
