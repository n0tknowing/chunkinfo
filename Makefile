CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Wstrict-prototypes -Wpedantic -O2
SRC     = main.c
BIN     = chunkinfo
RM      = rm -rf
CTAGS   = ctags
IDAT    = -D_DECODE_IDAT
PS      = http://www.schaik.com/pngsuite/PngSuite-2017jul19.tgz

.default: no-idat

no-idat:
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)

idat:
	$(CC) $(CFLAGS) $(IDAT) $(SRC) -o $(BIN)

debug:
	$(CC) $(CFLAGS) $(IDAT) -g $(SRC) -o $(BIN)

debug-asan:
	$(CC) $(CFLAGS) $(IDAT) -g -fsanitize=address,undefined $(SRC) -o $(BIN)

clean:
	$(RM) $(BIN) tags test *.zlib

get-test:
	@echo "PngSuite"
	mkdir -p test
	curl -s $(PS) -o test/pngsuite.tgz
	tar xfz test/pngsuite.tgz -C test/
	@echo "OK"

tags:
	$(CTAGS) $(SRC)
