PRJNAME = toastnotify
SRC = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, %.o, $(SRC))
CC_x64 := x86_64-w64-mingw32-gcc
STRIP_x64 := x86_64-w64-mingw32-strip

all: $(OBJS)

%.o: src/%.c
	mkdir -p build/
	$(CC_x64) $(CFLAGS) -I src/ -o $@ -c $<
	$(STRIP_x64) --strip-unneeded $@
	mv *.o build/$(PRJNAME).o

clean:
	rm -f build/$(PRJNAME).o build/$(PRJNAME).x64.o *.o