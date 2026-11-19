
#include "IAP_bsdiff.h"
#include "stdlib.h"
#include "string.h"
#include "limits.h"
#include "vFile.h"
#include "Drv_W25Qxx_QSPI.h"
#include "iap.h"

static __inline ui32 fast_read(void const *src, ui32 bytes)
{
#ifndef X86X64
    unsigned char *p = (unsigned char *)src;
    switch(bytes)
    {
    case 4:
        return(*p | *(p + 1) << 8 | *(p + 2) << 16 | *(p + 3) << 24);
    case 3:
        return(*p | *(p + 1) << 8 | *(p + 2) << 16);
    case 2:
        return(*p | *(p + 1) << 8);
    case 1:
        return(*p);
    }
    return 0;
#else
    if(bytes >= 1 && bytes <= 4)
        return *((ui32 *)src);
    else
        return 0;
#endif
}

size_t qlz_size_compressed(const char *source)
{
    ui32 n, r;
    n = (((*source) & 2) == 2) ? 4 : 1;
    r = fast_read(source + 1, n);
    r = r & (0xffffffff >> ((4 - n) * 8));
    return r;
}

size_t qlz_size_decompressed(const char *source)
{
    ui32 n, r;
    n = (((*source) & 2) == 2) ? 4 : 1;
    r = fast_read(source + 1 + n, n);
    r = r & (0xffffffff >> ((4 - n) * 8));
    return r;
}
static void reset_table_decompress(qlz_state_decompress *state)
{
    int i;
    (void)state;
    (void)i;
#if QLZ_COMPRESSION_LEVEL == 2
    for(i = 0; i < QLZ_HASH_VALUES; i++)
    {
        state->hash_counter[i] = 0;
    }
#endif
}


static __inline void memcpy_up(unsigned char *dst, const unsigned char *src, ui32 n)
{
    // Caution if modifying memcpy_up! Overlap of dst and src must be special handled.
#ifndef X86X64
    unsigned char *end = dst + n;
    while(dst < end)
    {
        *dst = *src;
        dst++;
        src++;
    }
#else
    ui32 f = 0;
    do
    {
        *(ui32 *)(dst + f) = *(ui32 *)(src + f);
        f += MINOFFSET + 1;
    }
    while(f < n);
#endif
}

size_t qlz_size_header(const char *source)
{
    size_t n = 2 * ((((*source) & 2) == 2) ? 4 : 1) + 1;
    return n;
}
size_t qlz_decompress_core(const unsigned char *source, unsigned char *destination, size_t size, qlz_state_decompress *state, const unsigned char *history)
{
    const unsigned char *src = source + qlz_size_header((const char *)source);
    unsigned char *dst = destination;
    const unsigned char *last_destination_byte = destination + size - 1;
    ui32 cword_val = 1;
    const unsigned char *last_matchstart = last_destination_byte - UNCONDITIONAL_MATCHLEN - UNCOMPRESSED_END;
    unsigned char *last_hashed = destination - 1;
    const unsigned char *last_source_byte = source + qlz_size_compressed((const char *)source) - 1;
    static const ui32 bitlut[16] = {4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};

    (void) last_source_byte;
    (void) last_hashed;
    (void) state;
    (void) history;

    for(;;)
    {
        ui32 fetch;

        if(cword_val == 1)
        {
#ifdef QLZ_MEMORY_SAFE
            if(src + CWORD_LEN - 1 > last_source_byte)
                return 0;
#endif
            cword_val = fast_read(src, CWORD_LEN);
            src += CWORD_LEN;
        }

#ifdef QLZ_MEMORY_SAFE
        if(src + 4 - 1 > last_source_byte)
            return 0;
#endif

        fetch = fast_read(src, 4);

        if((cword_val & 1) == 1)
        {
            ui32 matchlen;
            const unsigned char *offset2;

#if QLZ_COMPRESSION_LEVEL == 1
            ui32 hash;
            cword_val = cword_val >> 1;
            hash = (fetch >> 4) & 0xfff;
            offset2 = (const unsigned char *)(size_t)state->hash[hash].offset;

            if((fetch & 0xf) != 0)
            {
                matchlen = (fetch & 0xf) + 2;
                src += 2;
            }
            else
            {
                matchlen = *(src + 2);
                src += 3;
            }

#elif QLZ_COMPRESSION_LEVEL == 2
            ui32 hash;
            unsigned char c;
            cword_val = cword_val >> 1;
            hash = (fetch >> 5) & 0x7ff;
            c = (unsigned char)(fetch & 0x3);
            offset2 = state->hash[hash].offset[c];

            if((fetch & (28)) != 0)
            {
                matchlen = ((fetch >> 2) & 0x7) + 2;
                src += 2;
            }
            else
            {
                matchlen = *(src + 2);
                src += 3;
            }

#elif QLZ_COMPRESSION_LEVEL == 3
            ui32 offset;
            cword_val = cword_val >> 1;
            if((fetch & 3) == 0)
            {
                offset = (fetch & 0xff) >> 2;
                matchlen = 3;
                src++;
            }
            else if((fetch & 2) == 0)
            {
                offset = (fetch & 0xffff) >> 2;
                matchlen = 3;
                src += 2;
            }
            else if((fetch & 1) == 0)
            {
                offset = (fetch & 0xffff) >> 6;
                matchlen = ((fetch >> 2) & 15) + 3;
                src += 2;
            }
            else if((fetch & 127) != 3)
            {
                offset = (fetch >> 7) & 0x1ffff;
                matchlen = ((fetch >> 2) & 0x1f) + 2;
                src += 3;
            }
            else
            {
                offset = (fetch >> 15);
                matchlen = ((fetch >> 7) & 255) + 3;
                src += 4;
            }

            offset2 = dst - offset;
#endif

#ifdef QLZ_MEMORY_SAFE
            if(offset2 < history || offset2 > dst - MINOFFSET - 1)
                return 0;

            if(matchlen > (ui32)(last_destination_byte - dst - UNCOMPRESSED_END + 1))
                return 0;
#endif

            memcpy_up(dst, offset2, matchlen);
            dst += matchlen;

#if QLZ_COMPRESSION_LEVEL <= 2
            update_hash_upto(state, &last_hashed, dst - matchlen);
            last_hashed = dst - 1;
#endif
        }
        else
        {
            if(dst < last_matchstart)
            {
                unsigned int n = bitlut[cword_val & 0xf];
#ifdef X86X64
                *(ui32 *)dst = *(ui32 *)src;
#else
                memcpy_up(dst, src, 4);
#endif
                cword_val = cword_val >> n;
                dst += n;
                src += n;
#if QLZ_COMPRESSION_LEVEL <= 2
                update_hash_upto(state, &last_hashed, dst - 3);
#endif
            }
            else
            {
                while(dst <= last_destination_byte)
                {
                    if(cword_val == 1)
                    {
                        src += CWORD_LEN;
                        cword_val = 1U << 31;
                    }
#ifdef QLZ_MEMORY_SAFE
                    if(src >= last_source_byte + 1)
                        return 0;
#endif
                    *dst = *src;
                    dst++;
                    src++;
                    cword_val = cword_val >> 1;
                }

#if QLZ_COMPRESSION_LEVEL <= 2
                update_hash_upto(state, &last_hashed, last_destination_byte - 3); // todo, use constant
#endif
                return size;
            }

        }
    }
}
size_t qlz_decompress(const char *source, void *destination, qlz_state_decompress *state)
{
    size_t dsiz = qlz_size_decompressed(source);

#if QLZ_STREAMING_BUFFER > 0
    if(state->stream_counter + qlz_size_decompressed(source) - 1 >= QLZ_STREAMING_BUFFER)
#endif
    {
        if((*source & 1) == 1)
        {
            reset_table_decompress(state);
            dsiz = qlz_decompress_core((const unsigned char *)source, (unsigned char *)destination, dsiz, state, (const unsigned char *)destination);
        }
        else
        {
            memcpy(destination, source + qlz_size_header(source), dsiz);
        }
        state->stream_counter = 0;
        reset_table_decompress(state);
    }
#if QLZ_STREAMING_BUFFER > 0
    else
    {
        unsigned char *dst = state->stream_buffer + state->stream_counter;
        if((*source & 1) == 1)
        {
            dsiz = qlz_decompress_core((const unsigned char *)source, dst, dsiz, state, (const unsigned char *)state->stream_buffer);
        }
        else
        {
            memcpy(dst, source + qlz_size_header(source), dsiz);
            reset_table_decompress(state);
        }
        memcpy(destination, dst, dsiz);
        state->stream_counter += dsiz;
    }
#endif
    return dsiz;
}

int64_t offtin(uint8_t *buf)
{
    int64_t y;

    y=buf[7]&0x7F;
    y=y*256;y+=buf[6];
    y=y*256;y+=buf[5];
    y=y*256;y+=buf[4];
    y=y*256;y+=buf[3];
    y=y*256;y+=buf[2];
    y=y*256;y+=buf[1];
    y=y*256;y+=buf[0];

    if(buf[7]&0x80) y=-y;

    return y;
}

vFile *patch_read_fp;
int bz2_read(const struct bspatch_stream *stream, void *buffer, int length)
{

		int n;
//		uint8_t *dp = (uint8_t*)buffer;
		vFile* pf;

		pf = patch_read_fp;

		n = vfread(pf, buffer,length);// 读完后函数内部已经将偏移量增加，无需再操作偏移量

    if (n != length)
        return -1;

    return 0;
}

vFile *patch_write_fp;
static int new_data_fp;
int bz2_write(const struct bspatch_stream* stream, void* buffer, int length)
{
	uint32_t file_addr;
	file_addr = *((uint32_t*)stream->opaque_write);

	W25q_Write_N(buffer, file_addr + new_data_fp, length);
	new_data_fp += length;
	return 0;
}

void FLASH_ReadByte(uint32_t addr , uint8_t *p , uint16_t Byte_Num)
{
	while(Byte_Num--)
	{
		*(p++)=*((uint8_t*)addr++);
	}
}
#define WRITE_BLOCK_SIZE    1024
uint8_t oldbuf[WRITE_BLOCK_SIZE];
uint8_t newbuf[WRITE_BLOCK_SIZE];
int64_t oldpos,newpos;
//						FLASH 旧BIN，								，			存到外部FLASH的新BIN文件，             解压后的BIN
int bspatch(uint32_t old, int64_t oldsize, uint32_t neww, int64_t newsize, struct bspatch_stream *stream)
{
     uint8_t buf[8];
    
    int64_t ctrl[3];
    int64_t i;
    int64_t len;
    
    int64_t size=0;
    oldpos=0;newpos=0;
    while(newpos<newsize) 
	{
        // Read control data
        for(i=0;i<=2;i++) 
		{
            if (stream->read(stream, buf, 8))
                return -1;
            ctrl[i]=offtin(buf);
        }

        // Sanity-check
        if (ctrl[0]<0 || ctrl[0]>INT_MAX ||
                ctrl[1]<0 || ctrl[1]>INT_MAX ||
                newpos+ctrl[0]>newsize)
            return -1;


        while(ctrl[0] > 0)
        {
            if(ctrl[0] > WRITE_BLOCK_SIZE)
				len = WRITE_BLOCK_SIZE;
            else 
				len = ctrl[0];
            if (stream->read(stream, newbuf, len))return -1;
					
					// 内存有限，一次只对比一部分。这里将内部FLASH的数据读出
						memcpy(oldbuf,  (uint8_t*)(old+oldpos), len);

            for (i = 0; i < len; i++)
            {
                if ((oldpos + i >= 0) && (oldpos + i < oldsize))
                {
                    newbuf[i] += oldbuf[i];
                }
            }
            stream->write(stream, newbuf, len);
            ctrl[0] -= len;
            oldpos += len;
            newpos += len;
        }

        // Sanity-check 
        if (newpos + ctrl[1] > newsize)
            return -1;

        // Read extra string 
        while(ctrl[1] > 0)
        {
            if(ctrl[1] > WRITE_BLOCK_SIZE)
				len = WRITE_BLOCK_SIZE;
            else 
				len = ctrl[1];
            if (stream->read(stream, newbuf, len))
				return -1;
            stream->write(stream, newbuf, len);
            ctrl[1] -= len;
            newpos += len;
        }

        // Adjust pointers 
        oldpos += ctrl[2];
    }

    return 0;
}

uint8_t diff_dcmprs_buffer[DCOMPRESS_BUFFER_SIZE] = { 0 };
uint8_t diff_buffer[DCOMPRESS_BUFFER_SIZE + BUFFER_PADDING] = { 0 };
uint8_t diff_w25q[DCOMPRESS_BUFFER_SIZE + BUFFER_PADDING] = { 0 };
int decompress_file(uint32_t srcFilePAN, int srclen, uint32_t destFilePAN, int *destlen)
{
    // Start to decompress file
    qlz_state_decompress *state_decompress = 0;
    //uint8_t *dcmprs_buffer = 0, *buffer = 0;
    uint8_t buffer_hdr[BLOCK_HEADER_SIZE] = { 0 };
    size_t dcmprs_size = 0, block_size = 0, total_dcmprs_size = 0;
    size_t file_size = 0, i = 0;
    int ret = 0;
	uint32_t store_p, compress_p;
	unsigned char *src_p;
	//unsigned char *dest_p;
	
	compress_p = destFilePAN;
	store_p = srcFilePAN;
	src_p = diff_w25q;


    file_size = srclen;
    if (file_size <= BLOCK_HEADER_SIZE)
    {
        ret = -1;
        goto _dcmprs_exit;
    }

		// 这里要 4K+
    state_decompress = (qlz_state_decompress *) malloc(sizeof(qlz_state_decompress));
    if (!state_decompress)
    {
        ret = -1;
        goto _dcmprs_exit;
    }
    memset(state_decompress, 0x00, sizeof(qlz_state_decompress));

    for (i = 0; i < file_size; i += BLOCK_HEADER_SIZE + block_size)
    {
      // Get the decompress block size from the block header. 
			
		W25qxx_ReadBytes(src_p, store_p, BLOCK_HEADER_SIZE);
		memcpy(buffer_hdr, src_p, BLOCK_HEADER_SIZE);
		store_p += BLOCK_HEADER_SIZE;// W25Q 读指针后移
		//src_p += BLOCK_HEADER_SIZE;
			
        block_size = buffer_hdr[0] * (1 << 24) + buffer_hdr[1] * (1 << 16) + buffer_hdr[2] * (1 << 8) + buffer_hdr[3];

        memset(diff_buffer, 0x00, COMPRESS_BUFFER_SIZE + BUFFER_PADDING);
        memset(diff_dcmprs_buffer, 0x00, DCOMPRESS_BUFFER_SIZE);

		W25qxx_ReadBytes(src_p, store_p, block_size);			
		memcpy(diff_buffer, src_p, block_size);
		store_p+= block_size;// W25Q 读指针后移
			//src_p += block_size;

        dcmprs_size = qlz_decompress((const char *) diff_buffer, diff_dcmprs_buffer, state_decompress);
			
		W25q_Write_N(diff_dcmprs_buffer, compress_p, dcmprs_size);//  数据写在 备份区域 FLASH_IAP_START2
		compress_p += dcmprs_size;// 写指针后移


        total_dcmprs_size += dcmprs_size;
    }
	*destlen = total_dcmprs_size;// 解压后的文件大小

_dcmprs_exit:

	if(state_decompress)
	{
		free(state_decompress);
	}

    if(ret == -1)
        return 1;
    return 0;
}


int64_t oldsize, newsize;

	
//														FLASH 旧BIN，								，			解压后的BIN，							，					存到外部FLASH的新BIN文件
int recover_bspatch_file(uint32_t oldFilePAN, int old_len, uint32_t diffFilePAN, int dif_len, uint32_t bspatchFilePAN, int64_t *bspatch_len)
{
    uint8_t header[24];
    
    //uint8_t *old, *neww;
    struct bspatch_stream stream;

	W25qxx_ReadBytes(header, diffFilePAN, 24);	
	//memcpy(header, diffFilePAN, 24);
	diffFilePAN+=24;// !!!!!!!  前 24 个字节是软件加上去的必要的BIN文件信息参数

		// 解压后前面会有这段字符串作为标识
    if (memcmp(header, "ENDSLEY/BSDIFF43", 16) != 0)
        return 1;
    // Read lengths from header 
    newsize=offtin(header+16);
    if(newsize<0)
        return 2;
		*bspatch_len = newsize;// 固定标识后，是 8 字节新文件最终的大小
		
    //创建patc文件
//    if((neww=(uint8_t *)malloc(newsize+1))==NULL)
//        return 3;

	patch_read_fp = vfopen(diffFilePAN, dif_len);
    stream.read = bz2_read;
    //stream.opaque = oldFilePAN;

	new_data_fp = 0;
	patch_write_fp = vfopen(bspatchFilePAN, newsize);
	stream.write = bz2_write;
    stream.opaque_write = &bspatchFilePAN;
		
//    if (bspatch(old, oldsize, neww, newsize, &stream))
//        return 4;
		
	if (bspatch(oldFilePAN, old_len, bspatchFilePAN, newsize, &stream))
        return 4;
		
    // Free the memory we used 
//    free(old);
    //free(neww);


    return 0;
}



