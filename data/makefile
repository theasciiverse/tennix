
GRAPHICS=$(wildcard *.png)
SOUNDS=$(wildcard *.wav)

all: graphics_data.c sounds_data.c

data2csrc: data2csrc.o

graphics_data.c: data2csrc $(GRAPHICS)
	(for i in $(GRAPHICS); do base=$${i%%.png}; ./data2csrc $$i $${base/-/_}; done)>graphics_data.c

sounds_data.c: data2csrc $(SOUNDS)
	(for i in $(SOUNDS); do base=$${i%%.wav}; ./data2csrc $$i $${base/-/_}; done)>sounds_data.c

clean:
	rm -f data2csrc *.o graphics_data.c sounds_data.c

.PHONY: clean all
.DEFAULT: all

