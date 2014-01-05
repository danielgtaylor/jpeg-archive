LIBJPEG=/usr/local/opt/jpeg-turbo/lib/libjpeg.a
LIBIQA=src/iqa/build/release/libiqa.a

OPTS=-O2

all: jpeg-recompress jpeg-compare jpeg-hash

$(LIBIQA):
	cd src/iqa; RELEASE=1 make

jpeg-recompress: jpeg-recompress.c src/util.o src/commander.o $(LIBIQA)
	gcc $(OPTS) -o $@ $^ $(LIBJPEG)

jpeg-compare: jpeg-compare.c src/util.o src/hash.o src/commander.o $(LIBIQA)
	gcc $(OPTS) -o $@ $^ $(LIBJPEG)

jpeg-hash: jpeg-hash.c src/util.o src/hash.o src/commander.o
	gcc $(OPTS) -o $@ $^ $(LIBJPEG)

%.o: %.c %.h
	gcc -c -o $@ $< $(OPTS)

install:
	cp jpeg-recompress /usr/local/bin/
	cp jpeg-compare /usr/local/bin/
	cp jpeg-hash /usr/local/bin/

clean:
	rm -rf jpeg-recompress jpeg-compare jpeg-hash src/*.o src/iqa/build
