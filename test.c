#include <stdio.h>

#include "decoder.h"

int main(int argc, char **argv)
{
	if (argc < 2)
		return 1;

	struct png_chunk *chunk = chunk_init(argv[1]);
	if (!chunk)
		return 1;

	printf("file: %s\n", argv[1]);
	printf("list chunks: \n");
	for (size_t i = 0; i < chunk->entry; i++) {
		printf(" - %s", chunk->chunks[i]->type);
		printf(" (length: %u)", chunk->chunks[i]->length);
		printf(" (crc: %04x)\n", chunk->chunks[i]->crc);
	}

	chunk_end(chunk);
	return 0;
}
