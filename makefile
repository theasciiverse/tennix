CC        =  gcc

LIBS = $$(pkg-config --libs sdl)
CFLAGS = -Wall $$(pkg-config --cflags sdl)

SRC = tennix.c game.c graphics.c input.c
OBJ = tennix.o game.o graphics.o input.o

tennix: $(OBJ)
	$(CC) $(CFLAGS)   -o tennix $(OBJ) $(LIBS)

tennix.o: tennix.c tennix.h game.h graphics.h input.h
graphics.o: graphics.c graphics.h tennix.h input.h
game.o: game.c game.h graphics.h tennix.h

clean:
	rm -f *.o tennix

