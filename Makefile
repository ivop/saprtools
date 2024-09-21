all:
	+make -C lzss-sap
	+make -C sndh2ym
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

clean:
	+make -C lzss-sap clean
	+make -C sndh2ym clean
	+make -C ay2ym clean
	+make -C vgm2ym clean
	+make -C sid2sapr clean
	+make -C vgm2sapr clean
	+make -C ym2sapr clean
