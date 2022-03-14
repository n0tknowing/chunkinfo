#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decoder.h"
#include "utils.h"

static int is_png(FILE *);
static int read_chunk(struct png_chunk *, FILE *);

static int chunk_add(struct png_chunk *, struct chunk *);
static int chunk_resize(struct png_chunk *, size_t);
static void chunk_clear(struct png_chunk *);

struct png_chunk *chunk_init(const char *pngf)
{
	if (!pngf)
		return NULL;

	struct png_chunk *this = malloc(sizeof(struct png_chunk));
	if (!this)
		return NULL;

	this->chunks = malloc(sizeof(struct chunk) * 3);
	if (!this->chunks) {
		free(this);
		return NULL;
	}
	this->capacity = 3;
	this->entry = 0;

	FILE *f = fopen(pngf, "rb");
	if (!f)
		goto clear;

	if (!is_png(f))
		goto close_fail;

	if (read_chunk(this, f) < 0)
		goto close_fail;

	fclose(f);
	return this;

close_fail:
	fclose(f);
clear:
	free(this->chunks);
	this->capacity = 0;
	this->entry = 0;
	free(this);

	return NULL;
}

void chunk_end(struct png_chunk *this)
{
	if (this) {
		if (this->chunks)
			chunk_clear(this);
		free(this);
	}
}

/* --- PRIVATE FUNCTIONS --- */

static int is_png(FILE *fp)
{
	const uint8_t sign[8] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
	uint8_t buf[8];
	if (fread(buf, 1, 8, fp) != 8)
		return 0;
	if (!memcmp(sign, buf, 8))
		return 1;
	return 0;
}

static int read_chunk(struct png_chunk *this, FILE *fp)
{
	int not_iend = 1;
	while (not_iend) {
		errno = 0;
		uint32_t length = fread_u32(fp);
		if (errno)
			return -1;

		char type[5];
		if (fread(type, sizeof(char), 4, fp) != 4)
			return -1;
		type[4] = 0;
		if (!strcmp(type, "IEND"))
			not_iend = 0;

		uint32_t check = pd_crc32(0, type, 4);
		void *data = NULL;
		if (length > 0) {
			data = malloc(length);
			if (!data)
				return -1;
			if (fread(data, sizeof(uint8_t), length, fp) != length) {
				free(data);
				return -1;
			}
			check = pd_crc32(check, data, length);
		}

		uint32_t crc = fread_u32(fp);
		if (errno) {
			if (data)
				free(data);
			return -1;
		}

		if (crc == check) {
			struct chunk *new = malloc(sizeof(struct chunk));
			if (!new) {
				if (data)
					free(data);
				return -1;
			}

			new->length = length;
			memcpy(new->type, type, 5);
			new->data = data;
			new->crc = crc;
			if (chunk_add(this, new) < 0) {
				free(new);
				if (data)
					free(data);
				return -1;
			}
		}
	}

	return 0;
}

static int chunk_add(struct png_chunk *this, struct chunk *c)
{
	if (this && c) {
		if (this->entry == this->capacity) {
			if (chunk_resize(this, this->capacity * 2) < 0)
				return -1;
			this->capacity *= 2;
		}
		this->chunks[this->entry] = c;
		this->entry++;
		return 0;
	}

	return -1;
}

static int chunk_resize(struct png_chunk *this, size_t new)
{
	if (this && new > 0) {
		this->chunks = realloc(this->chunks, new * sizeof(struct chunk));
		if (!this->chunks)
			return -1;
		this->capacity = new;
		return 0;
	}

	return -1;
}

static void chunk_clear(struct png_chunk *this)
{
	if (this) {
		for (size_t i = 0; i < this->entry; i++) {
			free(this->chunks[i]->data);
			free(this->chunks[i]);
			this->chunks[i] = NULL;
		}

		free(this->chunks);
	}
}
