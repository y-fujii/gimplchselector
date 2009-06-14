.PHONY: all clean install uninstall

all: libcolor-selector-lch.so

clean:
	rm -f libcolor-selector-lch.so

install: libcolor-selector-lch.so
	sudo install libcolor-selector-lch.so $$(gimptool-2.0 --gimpplugindir)/modules/

uninstall:
	sudo rm $$(gimptool-2.0 --gimpplugindir)/modules/libcolor-selector-lch.so


libcolor-selector-lch.so: main.cpp selector-lch.hpp color-space.hpp
	$(CXX) \
		-ansi -pedantic -Wall -Wextra \
		-shared -fPIC -O3 \
		-o libcolor-selector-lch.so main.cpp \
		$$(gimptool-2.0 --cflags --libs)
