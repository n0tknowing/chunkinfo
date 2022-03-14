#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utils.h"

static uint8_t plt[256][3];
static uint8_t bit_depth, color_type;
enum COLOR_TYPE {
	GRAY = 0,
	RGB = 2,
	INDEXED = 3,
	GRAY_ALPHA = 4,
	RGB_ALPHA = 6
};

static int png_ok(FILE *f)
{
	const uint8_t sign[8] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
	uint8_t buf[8];
	if (fread(buf, 1, 8, f) != 8)
		return 0;
	if (!memcmp(sign, buf, 8))
		return 1;
	return 0;
}

static void decode_ihdr(const uint8_t *data, const uint32_t len)
{
	if (len != 13) {
		fprintf(stderr, "corrupted IHDR chunk\n");
		fprintf(stderr, "chunk length is expected 13, but found %u\n", len);
		return;
	}

	printf("\n");

	bit_depth = data[8], color_type = data[9];
	uint8_t wbuf[4] = { data[0], data[1], data[2], data[3] };
	uint8_t hbuf[4] = { data[4], data[5], data[6], data[7] };
	uint32_t w = 0, h = 0;

	memcpy(&w, wbuf, sizeof(uint32_t));
	memcpy(&h, hbuf, sizeof(uint32_t));

	printf("\tWidth       = %u\n", __builtin_bswap32(w));
	printf("\tHeight      = %u\n", __builtin_bswap32(h));
	printf("\tBit depth   = %u per channel\n", bit_depth);
	printf("\tColor type  = ");
	switch (color_type) {
	case GRAY:
		printf("Grayscale\n");
		printf("\tChannels    = 1 per pixel (%u)\n", bit_depth * 1);
		break;
	case RGB:
		printf("RGB\n");
		printf("\tChannels    = 3 per pixel (%u)\n", bit_depth * 3);
		break;
	case INDEXED:
		printf("Indexed color\n");
		printf("\tChannels    = 1 per pixel (%u)\n", bit_depth * 1);
		break;
	case GRAY_ALPHA:
		printf("Grayscale with Alpha channel\n");
		printf("\tChannels    = 2 per pixel (%u)\n", bit_depth * 2);
		break;
	case RGB_ALPHA:
		printf("RGB with Alpha channel\n");
		printf("\tChannels    = 4 per pixel (%u)\n", bit_depth * 4);
		break;
	default:
		printf("unknown\n");
		break;
	}

	const char *compression = data[10] == 0 ? "zlib deflate/inflate" : "unknown";
	const char *filter = data[11] == 0 ? "adaptive filtering" : "unknown";
	const char *interlace = data[12] == 7 ? "Adam7" : "no";
	printf("\tCompression = %u (%s)\n", data[10], compression);
	printf("\tFilter      = %u (%s)\n", data[11], filter);
	printf("\tInterlace   = %u (%s interlace)", data[12], interlace);
}

static void decode_plte(const uint8_t *data, const uint32_t len)
{
	if (len % 3 != 0) {
		fprintf(stderr, "corrupted PLTE chunk\n");
		fprintf(stderr, "chunk length is expected divisible by 3, but found %u\n", len);
		return;
	}

	printf("\n");

	uint32_t entries = len / 3;
	printf("\tEntries = %u\n", entries);

	// fill color
	for (uint32_t i = 0, col = 0; i < entries; i++, col += 3) {
		plt[i][0] = data[col];
		plt[i][1] = data[col+1];
		plt[i][2] = data[col+2];
	}

	// print color
	for (uint32_t i = 0, nl = 1; i < entries; i++, nl++) {
		printf("\t[%03u]", i);
		printf(" #%02x%02x%02x ", plt[i][0], plt[i][1], plt[i][2]);
		if ((nl % 6) == 0)
			printf("\n");
	}
}

static void decode_idat(const uint8_t *data, const uint32_t len)
{
	(void)data; (void)len;

	printf(".....");
/*	if (fflag && len > 0) {
		FILE *f = fopen("idat.zlib", "ab");
		if (!f)
			return;
		if (fwrite(data, sizeof(uint8_t), len, f) != len) {
			fclose(f);
			return;
		}
		fclose(f);
	} */
}

static void decode_time(const uint8_t *data, const uint32_t len)
{
	if (len != 7) {
		fprintf(stderr, "corrupted tIME chunk\n");
		fprintf(stderr, "chunk length is expected 7, but found %u\n", len);
		return;
	}

	uint8_t ybuf[4] = { 0x00, 0x00, data[0], data[1] };
	uint32_t year = 0;
	memcpy(&year, ybuf, sizeof(uint32_t));

	struct tm t = {
		.tm_sec = data[6],
		.tm_min = data[5],
		.tm_hour = data[4],
		.tm_mday = data[3],
		.tm_mon = data[2],
		.tm_year = __builtin_bswap32(year) - 1900
	};

	char buf[100] = {0};
	if (strftime(buf, 99, "%A, %d %b %Y - %I:%M %p", &t))
		printf("%s", buf);
}

static void decode_phys(const uint8_t *data, const uint32_t len)
{
	if (len != 9) {
		fprintf(stderr, "corrupted pHYs chunk\n");
		fprintf(stderr, "chunk length is expected 9, but found %u\n", len);
		return;
	}

	uint8_t xbuf[4] = { data[0], data[1], data[2], data[3] };
	uint8_t ybuf[4] = { data[4], data[5], data[6], data[7] };
	uint32_t x = 0, y = 0;

	memcpy(&x, xbuf, sizeof(uint32_t));
	memcpy(&y, ybuf, sizeof(uint32_t));
	x = __builtin_bswap32(x);
	y = __builtin_bswap32(y);

	uint32_t xdpi = x * 0.0254;
	uint32_t ydpi = y * 0.0254;
	const char *unit = !!data[8] ? " per meter" : "";

	printf("%u (%u DPI) x %u (%u DPI) pixels%s", x, xdpi , y, ydpi , unit);
}

static void decode_srgb(const uint8_t *data, const uint32_t len)
{
	if (len != 1) {
		fprintf(stderr, "corrupted sRGB chunk\n");
		fprintf(stderr, "chunk length is expected 1, but found %u\n", len);
		return;
	}

	const char *srgb_data[5] = {
		"Perceptual",
		"Relative colorimetric",
		"Saturation",
		"Absolute colorimetric",
		NULL
	};

	int intent = data[0];
	if (intent >= 0 && intent <= 3)
		printf("%s intent (%d)", srgb_data[intent], intent);
	else
		printf("Unknown intent (%d)", intent);
}

static void decode_gama(const uint8_t *data, const uint32_t len)
{
	if (len != 4) {
		fprintf(stderr, "corrupted gAMA chunk\n");
		fprintf(stderr, "chunk length is expected 4, but found %u\n", len);
		return;
	}

	uint32_t gama = 0;
	memcpy(&gama, data, sizeof(uint32_t));

	printf("%01.05f", (float)__builtin_bswap32(gama) / 100000);
}

static void decode_chrm(const uint8_t *data, const uint32_t len)
{
	if (len != 32) {
		fprintf(stderr, "corrupted cHRM chunk\n");
		fprintf(stderr, "chunk length is expected 32, but found %u\n", len);
		return;
	}

	uint8_t buf[8][4] = {
		{ data[0],  data[1],  data[2],  data[3]  }, // white point x
		{ data[4],  data[5],  data[6],  data[7]  }, // white point y
		{ data[8],  data[9],  data[10], data[11] }, // red x
		{ data[12], data[13], data[14], data[15] }, // red y
		{ data[16], data[17], data[18], data[19] }, // green x
		{ data[20], data[21], data[22], data[23] }, // green y
		{ data[24], data[25], data[26], data[27] }, // blue x
		{ data[28], data[29], data[30], data[31] }  // blue y
	};
	uint32_t res[8] = {0};

	for (uint8_t i = 0; i < 8; i++) {
		memcpy(&res[i], buf[i], sizeof(uint32_t));
		res[i] = __builtin_bswap32(res[i]);
	}

	printf("\n");
	printf("\tWhite point x  = %01.05f\n", (float)res[0] / 100000);
	printf("\tWhite point y  = %01.05f\n", (float)res[1] / 100000);
	printf("\tRed x          = %01.05f\n", (float)res[2] / 100000);
	printf("\tRed y          = %01.05f\n", (float)res[3] / 100000);
	printf("\tBlue x         = %01.05f\n", (float)res[4] / 100000);
	printf("\tBlue y         = %01.05f\n", (float)res[5] / 100000);
	printf("\tGreen x        = %01.05f\n", (float)res[6] / 100000);
	printf("\tGreen y        = %01.05f", (float)res[7] / 100000);
}

static void decode_iccp(const uint8_t *data, const uint32_t len)
{
	printf("\n");

	uint32_t l = len;
	printf("\tProfile name = ");
	while (*data) {
		putchar(*data);
		data++; l--;
	}
	putchar('\n');

	printf("\tCompression method = %u (zlib deflate/inflate)\n", *data);
	data += 2; l -= 2;

	printf("\tProfile (compressed) = ");
	if (l > 1)
		printf(".....");
}

static void decode_text(const uint8_t *data, const uint32_t len)
{
	printf("\n");

	uint32_t l = len;
	printf("\tKeyword = ");
	while (*data) {
		putchar(*data);
		data++; l--;
	}
	putchar('\n');

	printf("\tText    = ");
	while (l > 0) {
		if (isprint(*data))
			putchar(*data);
		data++; l--;
	}
}

static void decode_ztxt(const uint8_t *data, const uint32_t len)
{
	printf("\n");
	printf("\tKeyword = ");
	uint32_t l = len;
	while (*data) {
		putchar(*data);
		data++; l--;
	}
	putchar('\n');

	printf("\tCompression method = %u (zlib deflate/inflate)\n", *data);
	data += 2; l -= 2;

	printf("\tText (compressed) = ");
	if (l > 1)
		printf(".....");
}

static void decode_bkgd(const uint8_t *data, const uint32_t len)
{
	switch (color_type) {
	case GRAY: case GRAY_ALPHA:
		if (len != 2) {
			fprintf(stderr, "bKGD: invalid length!\n");
			return;
		}

		if (bit_depth < 16) {
			printf("%u", data[1]);
		} else {
			uint8_t col[2] = { data[0], data[1] };
			uint16_t val = 0;
			memcpy(&val, col, sizeof(uint16_t));
			printf("%u", __builtin_bswap16(val));
		}
		break;
	case RGB: case RGB_ALPHA:
		if (len != 6) {
			fprintf(stderr, "bKGD: invalid length!\n");
			return;
		}

		if (bit_depth < 16) {
			printf("%02x %02x %02x", data[1], data[3], data[5]);
		} else {
			printf("%02x%02x ", data[0], data[1]);
			printf("%02x%02x ", data[2], data[3]);
			printf("%02x%02x", data[4], data[5]);
		}
		break;
	case INDEXED: {
		if (len != 1) {
			fprintf(stderr, "bKGD: invalid length!\n");
			return;
		}

		uint32_t i = data[0];
		printf("%u ", i);
		printf("(#%02x%02x%02x)", plt[i][0], plt[i][1], plt[i][2]);
		break;
	}
	default:
		fprintf(stderr, "bKGD: invalid color type");
		break;
	}
}

static void decode_sbit(const uint8_t *data, const uint32_t len)
{
	switch (color_type) {
	case GRAY:
		if (len != 1) {
			fprintf(stderr, "sBIT: invalid length\n");
			return;
		}

		printf("%u", data[0]);
		break;
	case GRAY_ALPHA:
		if (len != 2) {
			fprintf(stderr, "sBIT: invalid length\n");
			return;
		}

		printf("%u %u", data[0], data[1]);
		break;
	case INDEXED: case RGB:
		if (len != 3) {
			fprintf(stderr, "sBIT: invalid length\n");
			return;
		}

		printf("%u %u %u", data[0], data[1], data[2]);
		break;
	case RGB_ALPHA:
		if (len != 4) {
			fprintf(stderr, "sBIT: invalid length\n");
			return;
		}

		printf("%u %u %u %u", data[0], data[1], data[2], data[3]);
		break;
	default:
		fprintf(stderr, "sBIT: invalid color type");
		break;
	}
}

static void decode_trns(const uint8_t *data, const uint32_t len)
{
	switch (color_type) {
	case GRAY: {
		if (len != 2) {
			fprintf(stderr, "tRNS: invalid length\n");
			return;
		}

		uint8_t trans[2] = { data[0], data[1] };
		uint16_t val = 0;
		memcpy(&val, trans, sizeof(uint16_t));
		val = __builtin_bswap16(val);
		printf("%u", val);
		break;
	}
	case RGB:
		if (len != 6) {
			fprintf(stderr, "tRNS: invalid length\n");
			return;
		}

		if (bit_depth < 16) {
			printf("%02x %02x %02x", data[1], data[3], data[5]);
		} else {
			printf("%02x%02x ", data[0], data[1]);
			printf("%02x%02x ", data[2], data[3]);
			printf("%02x%02x", data[4], data[5]);
		}
		break;
	case INDEXED:
		for (uint32_t i = 0, nl = 1; i < len; i++, i++) {
			printf("[%03u] %02x ", i, data[i]);
			if ((nl % 6) == 0)
				printf("\n");
		}
		break;
	default:
		fprintf(stderr, "tRNS: invalid color type");
		break;
	}
}

static void decode_chunk_data(const uint8_t *data, const char *type, const uint32_t len)
{
	if (!strcmp(type, "IHDR"))
		decode_ihdr(data, len);
	else if (!strcmp(type, "PLTE"))
		decode_plte(data, len);
	else if (!strcmp(type, "IDAT"))
		decode_idat(data, len);
	else if (!strcmp(type, "tIME"))
		decode_time(data, len);
	else if (!strcmp(type, "pHYs"))
		decode_phys(data, len);
	else if (!strcmp(type, "sRGB"))
		decode_srgb(data, len);
	else if (!strcmp(type, "gAMA"))
		decode_gama(data, len);
	else if (!strcmp(type, "cHRM"))
		decode_chrm(data, len);
	else if (!strcmp(type, "iCCP"))
		decode_iccp(data, len);
	else if (!strcmp(type, "tEXt"))
		decode_text(data, len);
	else if (!strcmp(type, "zTXt"))
		decode_ztxt(data, len);
	else if (!strcmp(type, "bKGD"))
		decode_bkgd(data, len);
	else if (!strcmp(type, "sBIT"))
		decode_sbit(data, len);
	else if (!strcmp(type, "tRNS"))
		decode_trns(data, len);
}

// __builtin_bswap32() is compiler extension
// tested on: gcc and clang
static void read_chunk(FILE *f)
{
	int not_iend = 1, i = 1;
	while (not_iend) {
		errno = 0;
		// read chunk size
		uint32_t size = fread_u32(f);
		if (errno) {
			fprintf(stderr, "error: %s\n", strerror(errno));
			return;
		}

		// read chunk type
		char type[5];
		if (fread(type, sizeof(char), 4, f) != 4) {
			fprintf(stderr, "failed to read chunk type\n");
			return;
		}

		type[4] = 0;
		printf("%d. Type = %s, Length = %u\n", i, type, size);
		if (!strcmp(type, "IEND"))
			not_iend = 0;

		uint32_t check = pd_crc32(0u, type, 4); // type
		// read chunk data
		printf("   Data = ");
		if (size > 0) {
			uint8_t *data = malloc(size);
			if (!data) {
				fprintf(stderr, "failed to allocate chunk data\n");
				return;
			}
			if (fread(data, sizeof(uint8_t), size, f) != size) {
				fprintf(stderr, "failed to read chunk data\n");
				free(data);
				return;
			}
			check = pd_crc32(check, data, size);  // data
			decode_chunk_data(data, type, size);
			putchar('\n');
			free(data);
		} else {
			printf("no data\n");
		}

		// read chunk crc
		uint32_t chunk_crc = fread_u32(f);
		if (errno) {
			fprintf(stderr, "error: %s\n", strerror(errno));
			return;
		}
		if (chunk_crc == check) {
			printf("   CRC  = %x\n", chunk_crc);
		} else {
			fprintf(stderr, "chunk %s has corrupted CRC\n", type);
			fprintf(stderr, "expected %x, got %x\n", check, chunk_crc);
			return;
		}

		if (not_iend)
			putchar('\n');
		i++;
	}
}

static int run(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s file.png\n", argv[0]);
		return 1;
	}

	errno = 0;
	FILE *f = fopen(argv[1], "rb");
	if (!f) {
		fprintf(stderr, "cannot open %s: %s\n", argv[1], strerror(errno));
		return !!errno;
	}

	if (png_ok(f)) {
		read_chunk(f);
	} else {
		errno = EINVAL;
		fprintf(stderr, "%s is not a valid PNG file\n", argv[1]);
	}

	fclose(f);
	return !!errno;
}

int main(int argc, char **argv)
{
	return run(argc, argv);
}
