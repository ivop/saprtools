all:
	+make -C lzss-sap
	+test -n "${MSYSTEM}" || make -C sndh2ym
	+make -C ay2ym
	+make -C vgm2ym
	+make -C sid2sapr
	+make -C vgm2sapr
	+make -C ym2sapr

SILENT=-s

convertall: all
	+make -C sndh2ym $(SILENT) $@
	+make -C ay2ym $(SILENT) $@
	+make -C vgm2ym $(SILENT) $@
	+make -C sid2sapr $(SILENT) $@
	+make -C vgm2sapr $(SILENT) $@
	+make -C ym2sapr $(SILENT) $@

test: convertall
	+md5sum -c md5sums.txt

clean:
	+make -C zlib clean
	+make -C lzss-sap clean
	+make -C sndh2ym clean
	+make -C ay2ym clean
	+make -C vgm2ym clean
	+make -C sid2sapr clean
	+make -C vgm2sapr clean
	+make -C ym2sapr clean

distclean:
	+make -C zlib distclean
	+make -C lzss-sap distclean
	+make -C sndh2ym distclean
	+make -C ay2ym distclean
	+make -C vgm2ym distclean
	+make -C sid2sapr distclean
	+make -C vgm2sapr distclean
	+make -C ym2sapr distclean
