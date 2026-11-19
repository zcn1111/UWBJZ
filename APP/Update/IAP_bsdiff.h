
#ifndef __IAP_BSDIFF_H
#define __IAP_BSDIFF_H

//#include "type.h"
#include "stdint.h"
#include "string.h"

#define MIN(x,y) (((x)<(y)) ? (x) : (y))

#define QLZ_VERSION_MAJOR 1
#define QLZ_VERSION_MINOR 5
#define QLZ_VERSION_REVISION 0

#define QLZ_COMPRESSION_LEVEL 3
#define QLZ_STREAMING_BUFFER 0
#define QLZ_MEMORY_SAFE
#define QLZ_BUFFER_PADDING    400
typedef unsigned int ui32;
typedef unsigned short int ui16;

// Decrease QLZ_POINTERS for level 3 to increase compression speed. Do not touch any other values!
#if QLZ_COMPRESSION_LEVEL == 1
#define QLZ_POINTERS 1
#define QLZ_HASH_VALUES 4096
#elif QLZ_COMPRESSION_LEVEL == 2
#define QLZ_POINTERS 4
#define QLZ_HASH_VALUES 2048
#elif QLZ_COMPRESSION_LEVEL == 3
#define QLZ_POINTERS 16
#define QLZ_HASH_VALUES 4096
#endif

#define MINOFFSET 2
#define UNCONDITIONAL_MATCHLEN 6
#define UNCOMPRESSED_END 4
#define CWORD_LEN 4

#if QLZ_COMPRESSION_LEVEL == 1 && defined QLZ_PTR_64 && QLZ_STREAMING_BUFFER == 0
#define OFFSET_BASE source
#define CAST (ui32)(size_t)
#else
#define OFFSET_BASE 0
#define CAST
#endif

#define BLOCK_HEADER_SIZE              4

#define COMPRESS_BUFFER_SIZE           4096
#define DCOMPRESS_BUFFER_SIZE          4096

/* Buffer padding for destination buffer, least size + 400 bytes large because incompressible data may increase in size. */
#define BUFFER_PADDING                 QLZ_BUFFER_PADDING


		struct bsdiff_stream
    {
        void* opaque;

        void* (*malloc)(size_t size);
        void (*free)(void* ptr);
        int (*write)(struct bsdiff_stream* stream, const void* buffer, int size);
    };
    struct bsdiff_request
    {
        const uint8_t* old;
        int64_t oldsize;
        const uint8_t* neww;
        int64_t newsize;
        struct bsdiff_stream* stream;
        int64_t *I;
        uint8_t *buffer;
    };
    struct bspatch_stream
    {
        void* opaque_read;
        int (*read)(const struct bspatch_stream* stream, void* buffer, int length);

        void* opaque_write;
        int (*write)(const struct bspatch_stream* stream, void* buffer, int length);
    };
    // hash entry
    typedef struct
    {
    #if QLZ_COMPRESSION_LEVEL == 1
        ui32 cache;
    #if defined QLZ_PTR_64 && QLZ_STREAMING_BUFFER == 0
        unsigned int offset;
    #else
        const unsigned char *offset;
    #endif
    #else
        const unsigned char *offset[QLZ_POINTERS];
    #endif

    } qlz_hash_compress;
    typedef struct
    {
    #if QLZ_COMPRESSION_LEVEL == 1
        const unsigned char *offset;
    #else
        const unsigned char *offset[QLZ_POINTERS];
    #endif
    } qlz_hash_decompress;


    // states
    typedef struct
    {
    #if QLZ_STREAMING_BUFFER > 0
        unsigned char stream_buffer[QLZ_STREAMING_BUFFER];
    #endif
        size_t stream_counter;
        qlz_hash_compress hash[QLZ_HASH_VALUES];
        unsigned char hash_counter[QLZ_HASH_VALUES];
    } qlz_state_compress;


    #if QLZ_COMPRESSION_LEVEL == 1 || QLZ_COMPRESSION_LEVEL == 2
    typedef struct
    {
    #if QLZ_STREAMING_BUFFER > 0
        unsigned char stream_buffer[QLZ_STREAMING_BUFFER];
    #endif
        qlz_hash_decompress hash[QLZ_HASH_VALUES];
        unsigned char hash_counter[QLZ_HASH_VALUES];
        size_t stream_counter;
    } qlz_state_decompress;
    #elif QLZ_COMPRESSION_LEVEL == 3
    typedef struct
    {
    #if QLZ_STREAMING_BUFFER > 0
        unsigned char stream_buffer[QLZ_STREAMING_BUFFER];
    #endif
    #if QLZ_COMPRESSION_LEVEL <= 2
        qlz_hash_decompress hash[QLZ_HASH_VALUES];
    #endif
        size_t stream_counter;
    } qlz_state_decompress;
    #endif

	
int decompress_file(uint32_t srcFilePAN, int srclen, 
													uint32_t destFilePAN, int* destlen);
int recover_bspatch_file(uint32_t oldFilePAN, int old_len, 
													uint32_t diffFilePAN, int dif_len, 
													uint32_t bspatchFilePAN, int64_t *bspatch_len);

#endif
