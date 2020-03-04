#ifndef VFAT_COMMON_H
#define VFAT_COMMON_H

#include <stdint.h>

//typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

extern int32_t __vfat_errno;

/* FAT error codes */
#define EFATFULL -1

/* Other error codes */
#define EALREADYEXISTS -2

#endif /* VFAT_COMMON_H */
