
#
# Tennix! SDL Port
# Copyright (C) 2003, 2007 Thomas Perl <thp@perli.net>
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
# MA  02110-1301, USA.
#

CC        =  gcc
RELEASE   =  0.4.1

TARGET ?= default

PREFIX ?= usr/local/

LIBS =
CFLAGS += -Wall -DVERSION=\"$(RELEASE)\"

ifeq ($(DEBUG),1)
  CFLAGS += -DDEBUG
endif

ifeq ($(TARGET),cocoa)
  LIBS += SDLmain.m -framework SDL -framework Cocoa -framework SDL_image -framework SDL_mixer
  CFLAGS += -I/Library/Frameworks/SDL.framework/Headers -I/Library/Frameworks/SDL_image.framework/Headers -I/Library/Frameworks/SDL_mixer.framework/Headers -DMACOSX
else
  LIBS += $$(sdl-config --libs) -lSDL_mixer -lSDL_image
  CFLAGS += $$(sdl-config --cflags)
endif

SRC = tennix.c game.c graphics.c input.c sound.c
OBJ = tennix.o game.o graphics.o input.o sound.o

WIN32LIBS = *.dll
OSXAPP = Tennix.app

DATAFILES = README README.*
ifeq ($(TARGET),cocoa)
  DATAFILES += data/Tennix.icns
endif

tennix: $(OBJ)
	$(CC) $(CFLAGS)   -o tennix $(OBJ) $(LIBS)
	test -f tennix.exe && upx tennix.exe || true 

install: tennix
	install -d $(DESTDIR)/$(PREFIX)/bin $(DESTDIR)/$(PREFIX)/share/pixmaps $(DESTDIR)/$(PREFIX)/share/applications
	install -s tennix $(DESTDIR)/$(PREFIX)/bin/tennix
	install -m644 data/icon.png $(DESTDIR)/$(PREFIX)/share/pixmaps/tennix.png
	install -m644 tennix.desktop $(DESTDIR)/$(PREFIX)/share/applications/

tennix.o: tennix.c tennix.h game.h graphics.h input.h sound.h
graphics.o: graphics.c graphics.h tennix.h input.h data/graphics_data.c
game.o: game.c game.h graphics.h tennix.h sound.h
sound.o: sound.c sound.h tennix.h data/sounds_data.c

data/graphics_data.c: data/*.png
	make -C data graphics_data.c

data/sounds_data.c: data/*.wav
	make -C data sounds_data.c

release-osx: tennix
	mkdir -p $(OSXAPP)/Contents/{MacOS,/Resources}
	cp -rpv tennix $(OSXAPP)/Contents/MacOS/Tennix
	cp -rpv $(DATAFILES) $(OSXAPP)/Contents/Resources/
	cp -rpv osxapp.plist $(OSXAPP)/Contents/Info.plist
	echo 'APPL????' >$(OSXAPP)/Contents/PkgInfo
	zip -r tennix-$(RELEASE)-macosx.zip $(OSXAPP)

release-win32: tennix
	zip tennix-$(RELEASE)-win32.zip tennix.exe $(WIN32LIBS) $(DATAFILES)

release-bin: tennix
	tar czvf tennix-$(RELEASE)-bin.tar.gz tennix $(DATAFILES)

clean:
	rm -f *.o tennix{,.exe}
	rm -rf $(OSXAPP)
	make -C data clean

distclean: clean
	rm -f tennix-$(RELEASE).zip tennix-$(RELEASE)-bin.tar.gz

.PHONY: distclean clean release-bin release-win32 release-osx install

