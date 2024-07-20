#ifndef COMMON_H_
#define COMMON_H_

#define read16_be(arr, offset) arr[offset] << 8 | arr[offset+1]
#define read24_be(arr, offset) arr[offset] << 16 | arr[offset+1] << 8 | arr[offset+2]
#define read32_be(arr, offset) arr[offset] << 24 | arr[offset+1] << 16 | arr[offset+2] << 8 | arr[offset+3]
#define wrap256(x) x == 0 ? 256 : x

#define SHM_KEY 0x4F56494E 

#endif
