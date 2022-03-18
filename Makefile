CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Wstrict-prototypes -Wpedantic
SRC     = pngdec.c
BIN     = pngdec
RM      = rm -f

.default: build

build:
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)

debug:
	$(CC) $(CFLAGS) -g $(SRC) -o $(BIN)

debug-asan:
	$(CC) $(CFLAGS) -g -fsanitize=address,undefined $(SRC) -o $(BIN)

clean:
	$(RM) $(BIN)
