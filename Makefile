CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Wstrict-prototypes -Wpedantic
SRC     = utils.c decoder.c
BIN     = pngdec
RM      = rm -f

.default: test

test:
	$(CC) $(CFLAGS) $(SRC) test.c -o $(BIN)

debug:
	$(CC) $(CFLAGS) -g $(SRC) test.c -o $(BIN)

debug-asan:
	$(CC) $(CFLAGS) -g -fsanitize=address,undefined $(SRC) test.c -o $(BIN)

clean:
	$(RM) $(BIN)
