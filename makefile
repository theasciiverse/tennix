CC        =  gcc
RELEASE   =  svn

TARGET ?= default

LIBS =
CFLAGS = -Wall

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

DATAFILES = data/*.bmp data/*.png data/*.wav
ifeq ($(TARGET),cocoa)
  DATAFILES += data/Tennix.icns
endif

tennix: $(OBJ)
	$(CC) $(CFLAGS)   -o tennix $(OBJ) $(LIBS)
	test -f tennix.exe && upx tennix.exe || true 

tennix.o: tennix.c tennix.h game.h graphics.h input.h sound.h
graphics.o: graphics.c graphics.h tennix.h input.h
game.o: game.c game.h graphics.h tennix.h sound.h
sound.o: sound.c sound.h tennix.h

release-osx: tennix
	mkdir -p $(OSXAPP)/Contents/{MacOS,/Resources}
	cp -rpv tennix $(OSXAPP)/Contents/MacOS/Tennix
	cp -rpv $(DATAFILES) $(OSXAPP)/Contents/Resources/
	cp -rpv osxapp.plist $(OSXAPP)/Contents/Info.plist
	echo 'APPL????' >$(OSXAPP)/Contents/PkgInfo
	zip -r tennix-$(RELEASE).zip $(OSXAPP)

release-win32: tennix
	zip tennix-$(RELEASE).zip tennix.exe $(WIN32LIBS) $(DATAFILES)

release-bin: tennix
	tar czvf tennix-$(RELEASE)-bin.tar.gz tennix $(DATAFILES)

clean:
	rm -f *.o tennix{,.exe}
	rm -rf $(OSXAPP)

distclean: clean
	rm -f tennix-$(RELEASE).zip tennix-$(RELEASE)-bin.tar.gz

.PHONY: distclean clean release-bin release-win32 release-osx

