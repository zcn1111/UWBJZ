
#include <stdlib.h>
#include <string.h>
#include "vFile.h"
#include "Drv_W25Qxx_QSPI.h"

vFile *vfopen(uint32_t dp, uint32_t size)
{
    vFile *fp = NULL;
    
    fp = malloc(sizeof(vFile));
    if(fp != NULL)
    {
        fp->curptr = dp;
        fp->offset = 0;
        fp->size = size;
    }
    
    return(fp);
}

int vfread(vFile *fp, uint8_t *buff, int len)
{
    if(fp != NULL)
    {
        if((fp->offset + len) > fp->size)
        {
            len = fp->size - fp->offset;
        }
        //memcpy(buff, fp->curptr + fp->offset, len);
				W25qxx_ReadBytes(buff, fp->curptr + fp->offset, len);	
        fp->offset += len;
        
        return(len);
    }
    
    return(0);
}

//unsigned char b_buf[256];
//int vfread(vFile *fp, uint32_t buff, int len)
//{
//	int readlen = len;
//    if(fp != NULL)
//    {
//        if((fp->offset + len) > fp->size)
//        {
//            len = fp->size - fp->offset;
//						readlen = len;
//        }
//				while(readlen)
//				{
//					memset(b_buf, 0, 256);
//					if(readlen >= 256)
//					{
//						W25qxx_ReadBytes(b_buf, fp->curptr + fp->offset, 256);	
//						W25q_Write_N(b_buf, buff, 256);
//						fp->offset += 256;
//						readlen-=256;
//					}
//					else
//					{
//						W25qxx_ReadBytes(b_buf, fp->curptr + fp->offset, readlen);	
//						W25q_Write_N(b_buf, buff, readlen);
//						fp->offset += readlen;
//						readlen-=readlen;
//					}
//				}
//        return(len);
//    }
//    
//    return(0);
//}

//uint32_t *vfgetpos(vFile *fp, uint32_t *position)
//{
//    if(fp != NULL)
//    {
//        *position = fp->offset;
//        
//        return(fp->curptr + fp->offset);
//    }
//    
//    return(NULL);
//}

int vfsetpos(vFile *fp, uint32_t position)
{
    if(fp != NULL)
    {
        fp->offset = position;
    }
    
    return(fp->offset);
}

int vfclose(vFile *fp)
{
    if(fp != NULL)
    {
        free(fp);
    }
    
    return(0);
}

uint32_t vfgetlen(vFile *fp)
{
    return(fp->size);
}


//vFile *vfopen(const uint8_t *dp, uint32_t size)
//{
//    vFile *fp = NULL;
//    
//    fp = malloc(sizeof(vFile));
//    if(fp != NULL)
//    {
//        fp->curptr = (uint8_t*)dp;
//        fp->offset = 0;
//        fp->size = size;
//    }
//    
//    return(fp);
//}

//int vfread(vFile *fp, uint8_t *buff, int len)
//{
//    if(fp != NULL)
//    {
//        if((fp->offset + len) > fp->size)
//        {
//            len = fp->size - fp->offset;
//        }
//        memcpy(buff, fp->curptr + fp->offset, len);
//        fp->offset += len;
//        
//        return(len);
//    }
//    
//    return(0);
//}

//uint8_t *vfgetpos(vFile *fp, uint32_t *position)
//{
//    if(fp != NULL)
//    {
//        *position = fp->offset;
//        
//        return(fp->curptr + fp->offset);
//    }
//    
//    return(NULL);
//}

//int vfsetpos(vFile *fp, uint32_t position)
//{
//    if(fp != NULL)
//    {
//        fp->offset = position;
//    }
//    
//    return(fp->offset);
//}

//int vfclose(vFile *fp)
//{
//    if(fp != NULL)
//    {
//        free(fp);
//    }
//    
//    return(0);
//}

//uint32_t vfgetlen(vFile *fp)
//{
//    return(fp->size);
//}
