/*
 * memcmp.c
 * 
 * Copyright (c) 2021 Luis "Laffin" Espinoza <laffintoo@gmail.com>
 * All rights reserved.
 * 
 * Licensed under BSD-3-Clause  - A copy of the license can be found in file
 * LICENSE included with the source package or you can view the full text at
 * https://spdx.org/licenses/BSD-3-Clause
 */


#include "strings.h"

int tb_memcmp(void *str1,void *str2,size_t size)
{
   int ret=0;
   while(size>=sizeof(uint64_t) && !(ret=(*((uint64_t *)str1)-*((uint64_t *)str2)))) {size-=sizeof(uint64_t); str1+=sizeof(uint64_t); str2+=sizeof(uint64_t);}
   if(!ret && size>=sizeof(uint32_t) && !(ret=(*((uint32_t *)str1)-*((uint32_t *)str2)))) {size-=sizeof(uint32_t); str1+=sizeof(uint32_t); str2+=sizeof(uint32_t);}
   if(!ret && size>=sizeof(uint16_t) && !(ret=(*((uint16_t *)str1)-*((uint16_t *)str2)))) {size-=sizeof(uint16_t); str1+=sizeof(uint16_t); str2+=sizeof(uint16_t);}
   if(!ret && size>=sizeof(uint8_t) && !(ret=(*((uint8_t *)str1)-(*(uint8_t *)str2))));

   return ret;
}
