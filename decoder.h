#ifndef decoder_h
#define decoder_h

#include <stdint.h>

struct chunk {
	uint32_t length;
	char type[5];
	void *data;
	uint32_t crc;
};

// vector
struct png_chunk {
	struct chunk **chunks;
	size_t capacity;
	size_t entry;
};

struct png_chunk *chunk_init(const char *);
void chunk_end(struct png_chunk *);

/*
 * chunk_find();
 * chunk_decode();
 *
 */

#endif // decoder_h
