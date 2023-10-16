CC = clang
CFLAGS = -c -g
LDFLAGS =


all: libmlpt.a mlpt

clean:
	rm --force mlpt main.o libmlpt.a mlpt.o

.PHONY: all clean

mlpt: main.o libmlpt.a
	$(CC) $(LDFLAGS) $< -L. -lmlpt -o mlpt

main.o: main.c config.h mlpt.h 
	$(CC) $(CFLAGS) $< 

libmlpt.a: mlpt.o
	ar rcs $(LDFLAGS) $@ $<

mlpt.o: mlpt.c config.h mlpt.h
	$(CC) $(CFLAGS) $<
