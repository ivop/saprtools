

all:
	set -ex

	cd ay2ym && ./convertall.sh && cd ..

	cd sid2sapr && ./convertall.sh && cd ..

	cd vgm2sapr && ./convertall.sh && cd ..

	cd ym2sapr & ./convertall.sh && cd ..

clean:
	cd ay2ym && make clean && cd ..

	cd sid2sapr && make clean && cd ..

	cd vgm2sapr && make clean && cd ..

	cd ym2sapr && make clean && cd ..

