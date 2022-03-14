#ifndef utils_h
#define utils_h

uint32_t pd_crc32(uint32_t, const void *, size_t);
uint8_t fread_u8(FILE *);
uint16_t fread_u16(FILE *);
uint32_t fread_u32(FILE *);

#endif // utils_h
