# Path to the command-line utility program
MCUFONT = ../encoder/mcufont

# Names of fonts to process
FONTS = Airnt32

# Characters to include in the fonts
CHARS = 0-255 0x2010-0x2015

all: $(FONTS:=.c) $(FONTS:=.dat) fonts.h

clean:
	rm -f $(FONTS:=.c) $(FONTS:=.dat)

fonts.h: $(FONTS:=.c)
	printf '$(foreach font,$(FONTS),\n#include "$(font).c")\n' > $@

%.c: %.dat $(MCUFONT)
	$(MCUFONT) rlefont_export $<

Airnt32.dat: Airnt.ttf
	$(MCUFONT) import_ttf $< 32
	$(MCUFONT) filter $@ $(CHARS)
	$(MCUFONT) rlefont_optimize $@ 50

%.dat: %.bdf
	$(MCUFONT) import_bdf $<
	$(MCUFONT) filter $@ $(CHARS)
	$(MCUFONT) rlefont_optimize $@ 50
