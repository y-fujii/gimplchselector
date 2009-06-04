.PHONY: all clean install

all: libcolorsel_lch.so

clean:
	rm -f libcolorsel_lch.so

install: libcolorsel_lch.so
	sudo install libcolorsel_lch.so $$(gimptool-2.0 --gimpplugindir)/modules/


libcolorsel_lch.so: color-selector-lch.c
	gcc \
		-shared -fPIC -Wall -Wextra -Wno-unused-parameter -O3 \
		-o libcolorsel_lch.so color-selector-lch.c \
		$$(gimptool-2.0 --cflags) $$(gimptool-2.0 --libs)
