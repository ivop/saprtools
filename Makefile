all:
	cd ay2ym    && ./convertall.sh
	+make -C lzss-sap
	cd sid2sapr && ./convertall.sh
	cd vgm2sapr && ./convertall.sh
	cd ym2sapr  && ./convertall.sh

clean:
	+make -C ay2ym clean
	+make -C lzss-sap clean
	+make -C sid2sapr clean
	+make -C vgm2sapr clean
	+make -C ym2sapr clean
