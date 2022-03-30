/*
 * chunkinfo - show information of PNG chunks
 *
 * chunkinfo has been placed in the public domain.
 * for details, see https://github.com/n0tknowing/chunkinfo
 */

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_CHUNK 256

enum COLOR_TYPE {
	GRAY = 0,
	RGB = 2,
	INDEXED = 3,
	GRAY_ALPHA = 4,
	RGB_ALPHA = 6
};

static char *pngf;
static uint8_t plt[256][3];
static uint8_t bit_depth, color_type;
static uint32_t plt_entry, bit_depth_max;
static const char *cstr[] = {
	[GRAY] = "Grayscale",
	/* empty */
	[RGB] = "RGB",
	[INDEXED] = "Indexed-color",
	[GRAY_ALPHA] = "Grayscale with Alpha channel",
	/* empty */
	[RGB_ALPHA] = "RGB with Alpha channel"
};

static uint32_t pd_crc32(uint32_t crc, const void *buf, size_t len)
{
	static const uint32_t crc32_table[] = {
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
		0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
		0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
		0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
		0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
		0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
		0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
		0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
		0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
		0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
		0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
		0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
		0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
		0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
		0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
		0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
		0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
		0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
		0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
		0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
		0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
		0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
		0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
		0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
		0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
		0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
		0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
		0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
		0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
		0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
		0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
		0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
		0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
		0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
		0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
		0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
		0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
		0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
		0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
		0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
		0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
		0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
		0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
		0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
		0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
		0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
		0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
		0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
		0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
		0x2d02ef8d
	};

	const unsigned char *p = buf;
	crc ^= 0xffffffff;
	for (size_t n = 0; n < len; n++)
		crc = crc32_table[(crc ^ p[n]) & 0xff] ^ (crc >> 8);

	return crc ^ 0xffffffff;
}

static uint32_t fread_u32(FILE *f)
{
	uint32_t ret = 0;
	if (fread(&ret, 4, 1, f) != 1)
		errno = ferror(f) ? EIO : EINVAL;

	return __builtin_bswap32(ret);
}

#define valid_keyword(c) ((c >= 0x20 && c <= 0x7e))

static char *get_name_or_keyword(const uint8_t *data, uint32_t *len)
{
	if (data && len) {
		size_t i = 0;
		char *ret = calloc(80, 1);
		if (!ret)
			return NULL;

		for (; i < 79; i++) {
			if (data[i] && valid_keyword(data[i]))
				ret[i] = data[i];
			else
				break;
		}

		*len = i;
		return ret;
	}

	return NULL;
}

static void die(const char *msg, ...)
{
	va_list ap;
	int save = errno;

	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	if (save)
		fprintf(stderr, " (%s)\n", strerror(save));
	else
		fputc('\n', stderr);
	va_end(ap);

	errno = save;
	exit(1);
}

static void out(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	fputc('\t', stdout);
	vfprintf(stdout, msg, ap);
	fputc('\n', stdout);
	va_end(ap);
}

static int png_ok(FILE *f)
{
	uint8_t buf[8];

	if (fread(buf, 1, 8, f) != 8)
		return 0;
	if (!memcmp(buf, "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a", 8))
		return 1;

	return 0;
}

#define valid_bdepth_gray(d) \
	((d == 1) || (d == 2) || (d == 4) || (d == 8) || (d == 16))
#define valid_bdepth_indexed(d) \
	((d == 1) || (d == 2) || (d == 4) || (d == 8))
#define valid_bdepth_rgb(d) ((d == 8) || (d == 16))

static void check_bit_depth_and_color_type(void)
{
	switch (color_type) {
	case GRAY:
		if (!valid_bdepth_gray(bit_depth))
			die("IHDR: invalid bit depth for grayscale");
		break;
	case RGB: case RGB_ALPHA: case GRAY_ALPHA:
		if (!valid_bdepth_rgb(bit_depth))
			die("IHDR: invalid bit depth for %s", cstr[color_type]);
		break;
	case INDEXED:
		if (!valid_bdepth_indexed(bit_depth))
			die("IHDR: invalid bit depth for indexed-color");
		break;
	default:
		die("IHDR: invalid color type");
		break;
	}
}

/**
 * IHDR
 *
 * offset   type    length   value
 * -------------------------------
 *   0      uint32    4      width (min=1,max=2^31)
 *   4      uint32    4      height (min=1,max=2^31)
 *   8      uint8     1      bit depth (1,2,4,8,16)
 *   9      uint8     1      color type (0,2,3,4,6)
 *  10      uint8     1      compression method (0)
 *  11      uint8     1      filter method (0)
 *  12      uint8     1      interlace method (0,1)
 */
static void decode_ihdr(const uint8_t *data, const uint32_t len)
{
	if (len != 13)
		die("IHDR: invalid chunk length");

	bit_depth = data[8], color_type = data[9];
	check_bit_depth_and_color_type();

	bit_depth_max = 1 << bit_depth;

	uint32_t w, h;
	uint8_t chan_bytes, comp, filter, interlace;
	uint8_t chan[] = {
		[GRAY] = 1,
		[RGB] = 3,
		[INDEXED] = 1,
		[GRAY_ALPHA] = 2,
		[RGB_ALPHA] = 4
	};

	comp = !!data[10];
	if (comp)
		die("IHDR: invalid compression method");

	filter = !!data[11];
	if (filter)
		die("IHDR: invalid filter method");

	interlace = !!data[12];
	if (interlace > 1)
		die("IHDR: invalid interlace method");

	w = h = 0;
	chan_bytes = chan[color_type] * bit_depth;

	memcpy(&w, data, 4);
	w = __builtin_bswap32(w);
	if (w > INT_MAX - 1)
		die("IHDR: width is too large");

	memcpy(&h, data + 4, 4);
	h = __builtin_bswap32(h);
	if (h > INT_MAX - 1)
		die("IHDR: height is too large");

	out("Width = %u", w);
	out("Height = %u", h);
	out("Bit depth = %u per channel", bit_depth);
	out("Color type = %s", cstr[color_type]);
	out("Channels = %u per pixel (%u bytes)", chan[color_type], chan_bytes);
	out("Compression = zlib deflate/inflate)");
	out("Filter = adaptive filtering");
	out("Interlace = %s interlace", interlace ? "Adam7" : "no");
}

/**
 * PLTE
 *
 * n = chunk length
 * palette entry = n / 3
 *
 * offset   type    length   value
 * -------------------------------
 *   0      uint8     1      red color
 *   1      uint8     1      green color
 *   2      uint8     1      blue color
 *   3      uint8     1      red color
 *   4      uint8     1      green color
 *   5      uint8     1      blue color
 *  ...
 *   n      uint8     1      blue color
 */
static void decode_plte(const uint8_t *data, const uint32_t len)
{
	if (len % 3 != 0)
		die("PLTE: invalid chunk length");

	uint32_t i, col;

	plt_entry = len / 3;
	if (plt_entry > bit_depth_max)
		die("PLTE: palette entries too large");

	out("Entries = %u", plt_entry);

	for (i = 0, col = 0; i < plt_entry; i++, col += 3) {
		plt[i][0] = data[col];      /* red */
		plt[i][1] = data[col + 1];  /* green */
		plt[i][2] = data[col + 2];  /* blue */
	}

	for (i = 0; i < plt_entry; i++) {
		printf("\t[%03u]", i);
		printf(" #%02x%02x%02x ", plt[i][0], plt[i][1], plt[i][2]);
		if ((i + 1) % 3 == 0)
			putchar('\n');
	}

	if (i % 3 != 0)
		putchar('\n');
}

static void decode_idat(const uint8_t *data, const uint32_t len)
{
#if _DECODE_IDAT
	FILE *f;

	f = fopen("idat.z", "ab");
	if (!f)
		die("IDAT: failed to open idat.z");

	if (fwrite(data, 1, len, f) != len) {
		fclose(f);
		die("IDAT: failed to write idat.z");
	}

	fclose(f);

	out("See idat.z");
#else
	(void)data; (void)len;

	out("Image data");
#endif
}

/**
 * tIME
 *
 * offset   type    length   value
 * -------------------------------
 *   0      uint16    2      year (example: 20,20)
 *   2      uint8     1      month (min=1,max=12)
 *   3      uint8     1      day (min=1,max=31)
 *   4      uint8     1      hour (min=0,max=23)
 *   5      uint8     1      minute (min=0,max=59)
 *   6      uint8     1      second (min=0,max=60)
 */
static void decode_time(const uint8_t *data, const uint32_t len)
{
	if (len != 7)
		die("tIME: invalid chunk length");

	uint16_t year;
	struct tm t;
	char buf[100] = {0};

	year = 0;
	memcpy(&year, data, 2);
	year = __builtin_bswap16(year) - 1900;

	t.tm_sec = data[6] <= 60 ? data[6] : 0;
	t.tm_min = data[5] <= 59 ? data[5] : 0;
	t.tm_hour = data[4] <= 23 ? data[4] : 0;
	t.tm_mday = (data[3] >= 1 && data[3] <= 31) ? data[3] : 1;
	t.tm_mon = (data[2] >= 1 && data[2] <= 12) ? data[2] : 1;
	t.tm_year = year;

	if (strftime(buf, 99, "%d %b %Y - %H:%M", &t))
		out("Last modification = %s", buf);
}

/**
 * pHYs
 *
 * offset   type    length   value
 * -------------------------------
 *   0      uint32    4      x axis
 *   4      uint32    4      y axis
 *   8      uint8     1      unit (0=pixel,1=meter)
 */
static void decode_phys(const uint8_t *data, const uint32_t len)
{
	if (len != 9)
		die("pHYs: invalid chunk length");

	char *unit;
	uint32_t x, y, dpi;

	x = y = 0;
	unit = !!data[8] ? " per meter" : "";

	memcpy(&x, data, 4);
	memcpy(&y, data + 4, 4);
	x = __builtin_bswap32(x);
	y = __builtin_bswap32(y);

	dpi = x * 0.0254;

	out("%u x %u pixels%s (approx. %u DPI)", x, y, unit, dpi);
}

/**
 * sRGB
 *
 * offset   type    length   value
 * -------------------------------
 *   0      uint8     1      rendering intent (0,1,2,3)
 */
static void decode_srgb(const uint8_t *data, const uint32_t len)
{
	if (len != 1)
		die("sRGB: invalid chunk length");

	if (data[0] > 3)
		die("sRGB: value out of range");

	char *srgb_data[5] = {
		"Perceptual",
		"Relative colorimetric",
		"Saturation",
		"Absolute colorimetric",
		NULL
	};

	out("%s intent", srgb_data[data[0]]);
}

/**
 * gAMA
 *
 * offset   type    length   value
 * -------------------------------
 *   0      uint32    4      gamma value
 */
static void decode_gama(const uint8_t *data, const uint32_t len)
{
	if (len != 4)
		die("gAMA: invalid chunk length");

	uint32_t gama = 0;
	memcpy(&gama, data, 4);
	gama = __builtin_bswap32(gama);
	if (gama == 0)
		die("gAMA: invalid gamma value");

	out("Gamma = %01.05f", (float)gama / 100000);
}

/**
 * cHRM
 *
 * offset   type    length   value
 * -------------------------------
 *   0      uint32    4      white point x
 *   4      uint32    4      white point y
 *   8      uint32    4      red x
 *   12     uint32    4      red y
 *   16     uint32    4      green x
 *   20     uint32    4      green y
 *   24     uint32    4      blue x
 *   28     uint32    4      blue y
 */
static void decode_chrm(const uint8_t *data, const uint32_t len)
{
	if (len != 32)
		die("cHRM: invalid chunk length");

	int offset, i;
	uint32_t res[8] = {0};
	uint32_t wx, wy, rx, ry, gx, gy, bx, by;

	offset = 0;

	for (i = 0; i < 8; i++) {
		memcpy(&res[i], data + offset, 4);
		res[i] = __builtin_bswap32(res[i]);
		offset += 4;
	}

	wx = res[0], wy = res[1];
	if (wx > 80000 || wy > 80000 || (wx + wy) > 100000)
		die("cHRM: invalid white point");

	rx = res[2], ry = res[3];
	if (rx > 80000 || ry > 80000 || (rx + ry) > 100000)
		die("cHRM: invalid red point");

	gx = res[4], gy = res[5];
	if (gx > 80000 || gx > 80000 || (gx + gy) > 100000)
		die("cHRM: invalid green point");

	bx = res[6], by = res[7];
	if (bx > 80000 || by > 80000 || (bx + by) > 100000)
		die("cHRM: invalid blue point");

	out("White point x = %01.05f", (float)wx / 100000);
	out("White point y = %01.05f", (float)wy / 100000);
	out("Red x = %01.05f", (float)rx / 100000);
	out("Red y = %01.05f", (float)ry / 100000);
	out("Blue x = %01.05f", (float)gx / 100000);
	out("Blue y = %01.05f", (float)gy / 100000);
	out("Green x = %01.05f", (float)bx / 100000);
	out("Green y = %01.05f", (float)by / 100000);
}

/**
 * iCCP
 *
 * offset   type    length   value
 * -------------------------------
 *   0      char    1 - 79   profile name (printable ascii)
 *   n      uint8     1      null separator (\0)
 *  n+1     uint8     1      compression method (0)
 *  n+2     uint8     m      compressed ICC profile
 */
static void decode_iccp(const uint8_t *data, const uint32_t len)
{
	uint32_t i, l;
	char *profile_name;

	i = 0;
	l = len;

	profile_name = get_name_or_keyword(data, &i);
	if (!profile_name)
		die("iCCP: failed to get profile name");

	out("Profile name = %s", profile_name);
	free(profile_name);
	data += i; l -= i;

	data++; l--;
	out("Compression method = %u (zlib deflate/inflate)", *data);

	data++; l--;
	out("Profile (compressed) = ......");
}

/**
 * tEXt (may appear more than one)
 *
 * offset   type    length   value
 * -------------------------------
 *   0      char    1 - 79   keyword (printable ascii)
 *   n      uint8     1      null separator (\0)
 *  n+1     char      m      text (printable ascii)
 */
static void decode_text(const uint8_t *data, const uint32_t len)
{

	uint32_t i, l;
	char *keyword;

	i = 0;
	l = len;

	keyword = get_name_or_keyword(data, &i);
	if (!keyword)
		die("tEXt: failed to get keyword");

	out("Keyword = %s", keyword);
	free(keyword);
	data += i; l -= i;

	data++; l--;

	printf("\tText = ");
	while (l > 0) {
		if (valid_keyword(*data))
			putchar(*data);
		data++; l--;
	}

	putchar('\n');
}

/**
 * iTXt (may appear more than one)
 *
 * offset   type    length   value
 * -------------------------------
 *   0      char    1 - 79   keyword (printable ascii)
 *   n      uint8     1      null separator (\0)
 *  n+1     uint8     1      compression flag (0=uncompress,1=compress)
 *  n+2     uint8     1      compression method
 *  n+3     char    0 - m    language tag (printable ascii)
 *   m      uint8     1      null separator (\0)
 *  m+1     utf8    0 - o    translated keyword (utf8)
 *   o      uint8     1      null separator (\0)
 *  o+1     utf8    0 - p    text (utf8 or compressed utf8)
 */
static void decode_itxt(const uint8_t *data, const uint32_t len)
{
	uint32_t i, l;
	uint8_t comp_flag;
	char *keyword, *comp;

	i = 0;
	l = len, i = 0;

	keyword = get_name_or_keyword(data, &i);
	if (!keyword)
		die("iTXt: failed to get keyword");

	out("Keyword = %s", keyword);
	free(keyword);
	data += i; l -= i;

	data++; l--;

	comp_flag = !!data[0];
	comp = comp_flag ? "compressed" : "uncompressed";
	out("Compression flag = %u (%s)", comp_flag, comp);

	data++; l--;
	out("Compression method = %u (zlib deflate/inflate)", *data);

	data++; l--;
	printf("\tLanguage tag = ");
	while (*data) {
		if (valid_keyword(*data))
			putchar(*data);
		data++; l--;
	}
	putchar('\n');

	data++; l--;
	out("Translated keyword (UTF-8) = .....");
	out("Text (UTF-8) = .....");
}

/**
 * zTXt (may appear more than one)
 *
 * offset   type    length   value
 * -------------------------------
 *   0      char    1 - 79   keyword (printable ascii)
 *   n      uint8     1      null separator (\0)
 *  n+1     uint8     1      compression method
 *  n+2     uint8     m      compressed text (ascii)
 */
static void decode_ztxt(const uint8_t *data, const uint32_t len)
{
	uint32_t i, l;
	char *keyword;

	i = 0;
	l = len;

	keyword = get_name_or_keyword(data, &i);
	if (!keyword)
		die("zTXt: failed to get keyword");

	out("Keyword = %s", keyword);
	free(keyword);
	data += i; l -= i;

	data++; l--;
	out("Compression method = %u (zlib deflate/inflate)", data[0]);

	data++; l--;
	out("Text (compressed) = .....");
}

/**
 * bKGD
 *
 * if indexed color
 * offset   type    length   value
 * -------------------------------
 *   0      uint8     1      palette index
 *
 * if grayscale or grayscale+alpha
 * offset   type    length   value
 * -------------------------------
 *   0      uint16    2      gray level
 *
 * if rgb or rgb+alpha
 * offset   type    length   value
 * -------------------------------
 *   0      uint16    2      red color
 *   2      uint16    2      green color
 *   4      uint16    2      blue color
 */
static void decode_bkgd(const uint8_t *data, const uint32_t len)
{
	if (len != 1 && len != 2 && len != 6)
		die("bKGD: invalid chunk length");

	uint8_t idx;
	uint32_t max;
	uint16_t gray, r, g, b;

	max = bit_depth_max - 1;

	/* die early rather than wait until all value are set */
	switch (color_type) {
	case GRAY: case GRAY_ALPHA:
		memcpy(&gray, data, 2);
		gray = __builtin_bswap16(gray);
		if (gray > max)
			die("bKGD: value out of range");

		out("Gray level = %u", gray);
		break;
	case RGB: case RGB_ALPHA:
		memcpy(&r, data, 2);
		r = __builtin_bswap16(r);
		if (r > max)
			die("bKGD: value out of range");

		memcpy(&g, data + 2, 2);
		g = __builtin_bswap16(g);
		if (g > max)
			die("bKGD: value out of range");

		memcpy(&b, data + 4, 2);
		b = __builtin_bswap16(b);
		if (b > max)
			die("bKGD: value out of range");

		if (bit_depth < 16)
			out("#%02x%02x%02x", r, g, b);
		else
			out("#%04x%04x%04x", r, g, b);
		break;
	case INDEXED:
		idx = data[0];
		if (idx > plt_entry)
			die("bKGD: palette index out of range");

		r = plt[idx][0];
		g = plt[idx][1];
		b = plt[idx][2];

		out("[%03u] #%02x%02x%02x", idx, r, g, b);
		break;
	default:
		die("bKGD: invalid color type");
		break;
	}
}

/**
 * sBIT
 *
 * if grayscale
 * offset   type    length   value
 * -------------------------------
 *   0      uint8     1      significant bits for gray
 *
 * if grayscale+alpha
 * offset   type    length   value
 * -------------------------------
 *   0      uint8     1      significant bits for gray
 *   1      uint8     1      significant bits for alpha
 *
 * if indexed or rgb
 * offset   type    length   value
 * -------------------------------
 *   0      uint8     1      significant bits for red
 *   1      uint8     1      significant bits for green
 *   2      uint8     1      significant bits for blue
 *
 * if rgb+alpha
 * offset   type    length   value
 * -------------------------------
 *   0      uint8     1      significant bits for red
 *   1      uint8     1      significant bits for green
 *   2      uint8     1      significant bits for blue
 *   3      uint8     1      significant bits for alpha
 */
static void decode_sbit(const uint8_t *data, const uint32_t len)
{
	if (len < 1 && len > 4)
		die("sBIT: invalid chunk length");

	uint32_t i;
	uint8_t max;
	uint8_t bit[4];
	uint8_t gray, r, g, b, alpha;

	max = (color_type == INDEXED) ? 8 : bit_depth;

	for (i = 0; i < len; i++) {
		bit[i] = data[i];
		if (bit[i] > max)
			die("sBIT: value out of range");
	}

	if (color_type == GRAY && i == 1) {
		gray = bit[0];
		out("gray(%u)", gray);
	} else if (color_type == GRAY_ALPHA && i == 2) {
		gray = bit[0];
		alpha = bit[1];
		out("gray(%u), alpha(%u)", gray, alpha);
	} else if ((color_type == INDEXED || color_type == RGB) && i == 3) {
		r = bit[0];
		g = bit[1];
		b = bit[2];
		out("red(%u), green(%u), blue(%u)", r, g, b);
	} else if (color_type == RGB_ALPHA && i == 4) {
		r = bit[0];
		g = bit[1];
		b = bit[2];
		alpha = bit[3];
		out("red(%u), green(%u), blue(%u), alpha(%u)", r, g, b, alpha);
	} else {
		die("sBIT: invalid color type");
	}
}

/**
 * tRNS
 *
 * if grayscale
 * offset   type    length   value
 * -------------------------------
 *   0      uint16    2      transparency level for gray
 *
 * if indexed
 * offset   type    length   value
 * -------------------------------
 *   0      uint8     1      transparency level for palette index 0
 *   1      uint8     1      transparency level for palette index 1
 *  ...
 *   n      uint8     1      transparency level for palette index n
 *
 * if rgb
 * offset   type    length   value
 * -------------------------------
 *   0      uint16    2      transparency level for red
 *   2      uint16    2      transparency level for green
 *   4      uint16    2      transparency level for blue
 */
static void decode_trns(const uint8_t *data, const uint32_t len)
{
	if (color_type != INDEXED && (len != 2 && len != 6))
		die("tRNS: invalid chunk length");

	uint32_t i, max;
	uint16_t gray, r, g, b;

	max = bit_depth_max - 1;

	switch (color_type) {
	case GRAY:
		memcpy(&gray, data, 2);
		gray = __builtin_bswap16(gray);
		if (gray > max)
			die("tRNS: value out of range");

		out("gray(%u)", gray);
		break;
	case RGB:
		memcpy(&r, data, 2);
		r = __builtin_bswap16(r);
		if (r > max)
			die("tRNS: value out of range");

		memcpy(&g, data + 2, 2);
		g = __builtin_bswap16(g);
		if (g > max)
			die("tRNS: value out of range");

		memcpy(&b, data + 4, 2);
		b = __builtin_bswap16(b);
		if (b > max)
			die("tRNS: value out of range");

		if (bit_depth < 16)
			out("red(%02x), green(%02x), blue(%02x)", r, g, b);
		else
			out("red(%04x), green(%04x), blue(%04x)", r, g, b);
		break;
	case INDEXED:
		for (i = 0; i < len; i++) {
			printf("\t[%03u] %02x", i, data[i]);
			if ((i + 1) % 3 == 0)
				putchar('\n');
		}

		if (i % 3 != 0)
			putchar('\n');
		break;
	default:
		die("tRNS: invalid color type");
		break;
	}
}

/**
 * sPLT (may appear more than one)
 *
 * offset   type    length   value
 * -------------------------------
 *   0      char    1 - 79   palette name (printable ascii)
 *   n      uint8     1      null separator (\0)
 *  n+1     uint8     1      sample depth (8,16)
 *  n+2    uint8/16  1/2     red color *
 *  n+3    uint8/16  1/2     green color *
 *  n+4    uint8/16  1/2     blue color *
 *  n+5    uint8/16  1/2     alpha *
 *  n+6    uint16     2      frequency
 *
 *  * = if sample depth is 8, then type is uint8 and each length is 1
 *      otherwise, it's 16, then type is uint16 and each length is 2
 */
static void decode_splt(const uint8_t *data, const uint32_t len)
{
	/* at least 1 character palette name, null, and sample depth */
	if (len < 3)
		die("sPLT: invalid chunk length");

	char *palette_name;
	uint8_t sample_depth;
	uint32_t col, entry, i, l;

	i = 0;
	l = len;
	col = 0;

	palette_name = get_name_or_keyword(data, &i);
	if (!palette_name)
		die("sPLT: failed to get palette name");

	out("Palette name = %s", palette_name);
	free(palette_name);
	data += i;
	l -= i;

	data++; l--;
	sample_depth = data[0];
	if (sample_depth != 8 && sample_depth != 16)
		die("sPLT: invalid sample depth");

	out("Sample depth = %u", sample_depth);

	data++; l--;
	if ((l % 6) != 0 && (l % 10) != 0)
		die("sPLT: invalid palette length");

	entry = sample_depth == 8 ? l / 6 : l / 10;
	out("Entries = %u", entry);

	if (sample_depth == 8) {
		for (i = 0; i < entry; i++, col += 6) {
			printf("\t[%03u] ", i);
			/* red */
			printf("#%02x", data[col]);
			/* green */
			printf("%02x", data[col+1]);
			/* blue */
			printf("%02x", data[col+2]);
			/* alpha */
			printf("%02x", data[col+3]);
			/* frequency */
			printf(" (%02x%02x)", data[col+4], data[col+5]);
			if ((i + 1) % 3 == 0)
				putchar('\n');
		}
	} else if (sample_depth == 16) {
		for (i = 0; i < entry; i++, col += 10) {
			printf("\t[%03u] ", i);
			/* red */
			printf("#%02x%02x", data[col], data[col+1]);
			/* green */
			printf("%02x%02x", data[col+2], data[col+3]);
			/* blue */
			printf("%02x%02x", data[col+4], data[col+5]);
			/* alpha */
			printf("%02x%02x", data[col+6], data[col+7]);
			/* frequency */
			printf(" (%02x%02x)", data[col+8], data[col+9]);
			if ((i + 1) % 3 == 0)
				putchar('\n');
		}
	}

	if (i % 3 != 0)
		putchar('\n');
}

/**
 * hIST
 *
 * entry = chunk length / 2
 *
 * offset   type    length   value
 * -------------------------------
 *   0      uint16    2      frequency of used for palette index 0
 *   2      uint16    2      frequency of used for palette index 1
 *   4      uint16    2      frequency of used for palette index 2
 *  ...
 *   n      uint16    2      frequency of used for palette index n
 */
static void decode_hist(const uint8_t *data, const uint32_t len)
{
	if (len % 2 != 0)
		die("hIST: invalid chunk length");

	if (plt_entry == 0)
		die("hIST: cannot find PLTE chunk");

	int off;
	uint16_t h;
	uint32_t i, entry;

	h = 0;
	off = 0;
	entry = len / 2;

	out("Entries = %u", entry);

	for (i = 0; i < entry; i++) {
		memcpy(&h, data + off, 2);
		h = __builtin_bswap16(h);
		off += 2;
		printf("\t[%03u] %u  ", i, h);
		if ((i + 1) % 3 == 0)
			putchar('\n');
	}

	if (i % 3 != 0)
		putchar('\n');
}

/**
 * oFFs
 *
 * offset   type    length   value
 * -------------------------------
 *   0      int32     4      x position
 *   4      int32     4      y position
 *   8      uint8     1      unit (0=pixel,1=micrometer)
 */
static void decode_ext_offs(const uint8_t *data, const uint32_t len)
{
	if (len != 9)
		die("oFFs: invalid chunk length");

	char *unit;
	uint32_t x, y;

	memcpy(&x, data, 4);
	x = __builtin_bswap32(x);

	memcpy(&y, data + 4, 4);
	y = __builtin_bswap32(y);

	unit = !!data[8] ? "micrometres" : "pixels";

	out("Image position = %d x %d %s", (int32_t)x, (int32_t)y, unit);
}

/**
 * sCAL
 *
 * offset   type    length   value
 * -------------------------------
 *   0      uint8     1      unit (1=meter,2=radian)
 *   1      char    1 - n    pixel width (ascii floating-point)
 *   n      uint8     1      null separator (\0)
 *  n+1     char    1 - m    pixel height (ascii floating-point)
 */
static void decode_ext_scal(const uint8_t *data, const uint32_t len)
{
	if (data[0] < 1 && data[0] > 2)
		die("sCAL: invalid unit specifier");

	char *unit;
	uint32_t l;

	unit = data[0] == 1 ? "meters" : "radians";
	l = len;

	data++; l--; /* next */
	printf("\tPhysical scale = ");
	while (*data) {
		if (valid_keyword(*data))
			putchar(*data);
		data++; l--;
	}

	data++; l--; /* next */
	printf(" x ");
	while (l > 0) {
		if (valid_keyword(*data))
			putchar(*data);
		data++; l--;
	}
	printf(" (%s)\n", unit);
}

/**
 * pCAL
 *
 * offset   type    length   value
 * -------------------------------
 *   0      char    1 - 79   calibration name (printable ascii)
 *   n      uint8     1      null separator (\0)
 *  n+1     int32     4      original zero (x0)
 *  n+5     int32     4      original max (x1)
 *  n+9     uint8     1      equation type (0,1,2,3)
 *  n+10    uint8     1      number of parameters
 *  n+11    char    0 - m    unit name (printable ascii)
 *   m      uint8     1      null separator (\0)
 *  m+1     char    1 - p    parameter 0 (p0) (floating-point ascii)
 *   p      uint8     1      null separator (\0)
 *  p+1     char    1 - o    parameter 1 (p1) (floating-point ascii)
 */
static void decode_ext_pcal(const uint8_t *data, const uint32_t len)
{
	uint8_t eq_type, params;
	char *eq_str, *name;
	uint32_t i, l, x0, x1;
	char *eq_arr[5] = {
		"linear", "exponential",
		"exponential arbitrary base", "hyperbolic sinusoidal",
		NULL
	};

	l = len;
	i = x0 = x1 = 0;

	/* get calibration name */
	name = get_name_or_keyword(data, &i);
	if (!name)
		die("pCAL: failed to get calibration name");

	out("Calibration name = %s", name);
	free(name);

	name = NULL;
	data += i; l -= i;
	i = 0;

	data++; l--; /* next */

	/* get x0 and x1 */
	memcpy(&x0, data, 4);
	x0 = __builtin_bswap32(x0);
	data += 4; l-= 4;

	memcpy(&x1, data, 4);
	x1 = __builtin_bswap32(x1);
	data += 4; l-= 4;

	out("Linear conversion = %d x %d", (int32_t)x0, (int32_t)x1);

	/* get equation type */
	eq_type = data[0];
	if (eq_type > 3)
		die("pCAL: invalid equation type");

	eq_str = eq_arr[eq_type];
	out("Equation type = %u (%s)", eq_type, eq_str);

	data++; l--; /* next */

	/* get total of parameters */
	params = data[0];
	out("Parameters = %u", params);

	data++; l--; /* next */

	/* get unit name */
	name = get_name_or_keyword(data, &i);
	if (!name)
		die("pCAL: failed to get unit name");

	out("Unit name = %s", name);
	free(name);

	name = NULL;
	data += i; l -= i;

	data++; l--; /* next */

	/* print all values */
	printf("\tValues = ");
	for (i = 0; i < params; i++) {
		while (l > 0) {
			if (valid_keyword(*data))
				putchar(*data);
			else
				printf(", ");
			data++; l--;
		}
	}

	putchar('\n');
}

/**
 * gIFg
 *
 * offset   type    length   value
 * -------------------------------
 *   0      uint8     1      disposal method
 *   1      uint8     1      user input flag
 *   2      uint16    2      delay time
 */
static void decode_ext_gifg(const uint8_t *data, const uint32_t len)
{
	if (len != 4)
		die("gIFg: invalid chunk length");

	uint8_t dis, input;
	uint16_t delay_time;

	dis = data[0];
	input = data[1];
	delay_time = 0;

	memcpy(&delay_time, data + 2, 2);
	delay_time = __builtin_bswap16(delay_time);

	out("Disposal method = %u", dis);
	out("User input = %u", input);
	out("Delay time = %lf seconds", (double).01 * delay_time);
}

/**
 * sTER
 *
 * offset   type    length   value
 * -------------------------------
 *   0      uint8     1      layout mode (0=cross-fuse,1=diverging-fuse)
 */
static void decode_ext_ster(const uint8_t *data, const uint32_t len)
{
	if (len != 1)
		die("sTER: invalid chunk length");

	if (data[0] > 1)
		die("sTER: invalid layout");

	char *layout = !!data[0] ? "Diverging-fuse" : "Cross-fuse";

	out("Layout = %s layout", layout);
}

/**
 * gIFx
 *
 * offset   type    length   value
 * -------------------------------
 *   0      uint8     8      application identifier (printable ascii)
 *   8      uint8     3      authentication code
 *   11     ?????     n      application data
 */
static void decode_ext_gifx(const uint8_t *data, const uint32_t len)
{
	if (len < 11)
		die("gIFx: invalid chunk length");

	out("Application ID = %.*s", 8, (char *)data);
	out("Authentication code = %02x%02x%02x", data[8], data[9], data[10]);
	out("Application data = .....");
}

/**
 * acTL
 *
 * offset   type    length   value
 * -------------------------------
 *   0      uint32    4      number of frames
 *   4      uint32    4      number of looping (0=infinite looping)
 */
static void decode_apng_actl(const uint8_t *data, const uint32_t len)
{
	if (len != 8)
		die("acTL: invalid chunk length");

	uint32_t nframes, nplays;

	nframes = nplays = 0;
	memcpy(&nframes, data, 4);
	memcpy(&nplays, data + 4, 4);

	nframes = __builtin_bswap32(nframes);
	nplays = __builtin_bswap32(nplays);

	out("Number of frames = %u", nframes);
	out("Number of plays = %u %s", nplays, nplays ? "" : "(infinite)");
}

/**
 * fcTL
 *
 * offset   type    length   value
 * -------------------------------
 *   0      uint32    4      sequence of chunk animation, start from 0
 *   4      uint32    4      width frame
 *   8      uint32    4      height frame
 *   12     uint32    4      X position
 *   16     uint32    4      Y position
 *   20     uint16    2      frame delay fraction numerator
 *   23     uint16    2      frame delay fraction denominator
 *   24     uint8     1      frame disposal type (0,1,2)
 *   25     uint8     1      frame blend type (0,1)
 */
static void decode_apng_fctl(const uint8_t *data, const uint32_t len)
{
	if (len != 26)
		die("fcTL: invalid chunk length");

	uint8_t i;
	int offset;
	char *dispose, *blend;
	char *bl_str[3] = { "Source", "Over", NULL };
	char *dis_str[4] = { "None", "Background", "Previous", NULL };

	uint32_t buf1[4] = {0}; /* width, height, x_offset, y_offset */
	uint16_t buf2[2] = {0}; /* delay_nums, delay_den */
	uint8_t buf3[2] = {0}; /* dispose_op, blend_op */

	offset = 4; /* skip sequence_number for now */
	for (i = 0; i < 4; i++) {
		memcpy(&buf1[i], data + offset, 4);
		buf1[i] = __builtin_bswap32(buf1[i]);
		offset += 4;
	}

	for (i = 0; i < 2; i++) {
		memcpy(&buf2[i], data + offset, 2);
		buf2[i] = __builtin_bswap16(buf2[i]);
		offset += 2;
	}

	buf3[0] = data[offset];
	if (buf3[0] > 3)
		die("fcTL: invalid disposal method");
	dispose = dis_str[buf3[0]];

	buf3[1] = data[offset + 1];
	if (buf3[1] > 1)
		die("fcTL: invalid blend method");
	blend = bl_str[buf3[1]];

	out("Width = %u", buf1[0]);
	out("Height = %u", buf1[1]);
	out("X offset = %u", buf1[2]);
	out("Y offset = %u", buf1[3]);
	out("Delays = %u (denominator %u)", buf2[0], buf2[1]);
	out("Disposal = %u (%s)", buf3[0], dispose);
	out("Blend = %u (%s)", buf3[1], blend);
}

#define decode_if(what, decode_func) \
	do {\
		if (!strcmp(type, what)) { \
			decode_func(data, len); \
			return; \
		} \
	} while (0)

static void decode_chunk_data(const uint8_t *data,
			      const char *type,
			      const uint32_t len)
{
	/* Critical chunks */
	decode_if("IHDR", decode_ihdr);
	decode_if("PLTE", decode_plte);
	decode_if("IDAT", decode_idat);

	/* ancillary chunks */
	decode_if("tIME", decode_time);
	decode_if("pHYs", decode_phys);
	decode_if("sRGB", decode_srgb);
	decode_if("gAMA", decode_gama);
	decode_if("cHRM", decode_chrm);
	decode_if("iCCP", decode_iccp);
	decode_if("tEXt", decode_text);
	decode_if("iTXt", decode_itxt);
	decode_if("zTXt", decode_ztxt);
	decode_if("bKGD", decode_bkgd);
	decode_if("sBIT", decode_sbit);
	decode_if("tRNS", decode_trns);
	decode_if("sPLT", decode_splt);
	decode_if("hIST", decode_hist);

	/* official PNG extension chunks */
	decode_if("oFFs", decode_ext_offs);
	decode_if("sCAL", decode_ext_scal);
	decode_if("pCAL", decode_ext_pcal);
	decode_if("gIFx", decode_ext_gifx);
	decode_if("gIFg", decode_ext_gifg);
	decode_if("sTER", decode_ext_ster);

	/* APNG */
	decode_if("acTL", decode_apng_actl);
	decode_if("fcTL", decode_apng_fctl);

	/* no decoder yet... */
	out(".....");
}

static void read_chunk(FILE *f)
{
	int not_iend, i;

	i = 0;
	not_iend = 1;

	while (not_iend) {
		if (i == MAX_CHUNK)
			break;

		if (i > 0 && (feof(f) || ferror(f)))
			die("failed to read chunk");

		long offset;
		uint8_t *data;
		char type[5] = {0};
		uint32_t check, chunk_crc, size;

		errno = 0;
		data = NULL;

		/* read chunk length */
		size = fread_u32(f);
		if (errno)
			die("failed to get chunk length");

		if (size > INT_MAX - 1)
			die("chunk length out of range");

		/* get current chunk offset */
		offset = ftell(f);
		if (offset < 0)
			die("failed to get chunk offset");

		/* read chunk type */
		if (fread(type, 1, 4, f) != 4)
			die("failed to get chunk type");

		if (i == 0 && strcmp(type, "IHDR"))
			die("first chunk found is not IHDR");

		if (!strcmp(type, "IEND"))
			not_iend = 0;

		/* crc chunk type to check */
		check = pd_crc32(0u, type, 4);

		/* read chunk data */
		if (size > 0) {
			data = malloc(size);
			if (!data)
				die("failed to read chunk data");

			if (fread(data, 1, size, f) != size) {
				free(data);
				die("failed to read chunk data");
			}

			/* crc chunk data to check */
			check = pd_crc32(check, data, size);
		}

		/* read chunk crc */
		chunk_crc = fread_u32(f);
		if (errno)
			die("failed to get chunk crc");

		if (chunk_crc == check) {
			printf("[%s] length %u at offset 0x%08lx (%04x)\n",
					type, size, offset, chunk_crc);
			if (data) {
				decode_chunk_data(data, type, size);
				free(data);
			} else {
				out("No data");
			};
		} else {
			if (data)
				free(data);
			die("%s: corrupted crc", type);
		}

		i++;
		putchar('\n');
	}

	printf("All OK.\n");
	printf("Found %d chunks from %s\n", i, pngf);
}

int main(int argc, char **argv)
{
	if (argc < 2)
		die("usage: %s file.png", argv[0]);

	FILE *f;

	pngf = argv[1];
	errno = 0;

	f = fopen(pngf, "rb");
	if (!f)
		die("%s: failed to open file", pngf);

	if (png_ok(f)) {
		read_chunk(f);
	} else {
		fclose(f);
		die("%s: not a valid PNG file", pngf);
	}

	fclose(f);
	return errno;
}
