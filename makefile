CC        =  gcc
RELEASE   =  svn

LIBS = $$(sdl-config --libs)
CFLAGS = -Wall $$(sdl-config --cflags)

SRC = tennix.c game.c graphics.c input.c
OBJ = tennix.o game.o graphics.o input.o

DATAFILES = data/*.bmp

tennix: $(OBJ)
	$(CC) $(CFLAGS)   -o tennix $(OBJ) $(LIBS)
	test -x `which upx` -a -f tennix.exe && `which upx` tennix.exe

tennix.o: tennix.c tennix.h game.h graphics.h input.h
graphics.o: graphics.c graphics.h tennix.h input.h
game.o: game.c game.h graphics.h tennix.h

release-win32: tennix
	zip tennix-$(RELEASE).zip tennix.exe sdl.dll $(DATAFILES)

release-bin: tennix
	tar czvf tennix-$(RELEASE)-bin.tar.gz tennix $(DATAFILES)

clean:
	rm -f *.o tennix{,.exe}

distclean: clean
	rm -f tennix-$(RELEASE).zip

.PHONY: distclean clean release-bin release-win32

