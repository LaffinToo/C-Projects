/*
 * memset.c
 * 
 * Copyright (c) 2021 Luis "Laffin" Espinoza <laffintoo@gmail.com>
 * All rights reserved.
 * 
 * Licensed under BSD-3-Clause  - A copy of the license can be found in file
 * LICENSE included with the source package or you can view the full text at
 * https://spdx.org/licenses/BSD-3-Clause
 */


#include "strings.h"

void *tb_memset(void *dst,int chr,size_t size)
{
#ifdef _USE_64BIT
    uint64_t chd;

    (((uint8_t *)&chd))[0]=((uint8_t *)&chd)[1]=((uint8_t *)&chd)[2]=((uint8_t *)&chd)[3]=((uint8_t *)&chd)[4]=((uint8_t *)&chd)[5]=((uint8_t *)&chd)[6]=((uint8_t *)&chd)[7]=(uint8_t)chr;

    while(sizeof(uint64_t)<=size)
    {
        *(uint64_t *)dst=chd;
        dst+=sizeof(uint64_t);
        size-=sizeof(uint64_t);
    }
    if(sizeof(uint32_t)<=size)
    {
        *(uint32_t *)dst=(uint32_t)chd;
        dst+=sizeof(uint32_t);
        size-=sizeof(uint32_t);
    }
#else
    uint32_t chd;

    (((uint8_t *)&chd))[0]=((uint8_t *)&chd)[1]=((uint8_t *)&chd)[2]=((uint8_t *)&chd)[3]=(uint8_t)chr;
    while(sizeof(uint32_t)<=size)
    {
        *(uint32_t *)dst=(uint32_t)chd;
        dst+=sizeof(uint32_t);
        size-=sizeof(uint32_t);
    }
#endif
    if(sizeof(uint16_t)<=size)
    {
        *(uint16_t *)dst=(uint16_t)chd;
        dst+=sizeof(uint16_t);
        size-=sizeof(uint16_t);
    }
    if(size)  *(uint8_t *)dst=(uint8_t)chd;

    return dst;
}
