/*
 * PoolMan.h
 *
 * Copyright 2021 Luis "Laffin" Espinoza <laffintoo at gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __POOLMAN_H
#define __POOLMAN_H

// Defaults - 24k for our Pool - 512 Keys
#ifndef POOL_SIZE
#define POOL_SIZE (1024*24)
#endif
#ifndef POOL_KEYS
#define POOL_KEYS (512)
#endif
#define POOL_IDX_SIZE (((POOL_KEYS/8)+((POOL_KEYS%8)>0)))

// Literal Entry
// Since 16bits allows for 64k, Dont need anything bigger than int16_t
typedef struct {
   short    offset,
            length;
} st_poolentry;

typedef struct {
   short    entries,
            indexmap[POOL_KEYS],
            size;                // Current Size of Pool (start+size = start of free space)
} st_poolinfo;

typedef struct {
   st_poolinfo    *info;   // Pointer to Header
   st_poolentry   *entry;  // Pointer to Entries (Array)
   char           *start;  // Start of Pool

} st_poolptrs;

st_poolptrs *PoolBase(void)
{
   // Hide our Pool from Everyone
   static char Pool[POOL_SIZE];
   static st_poolptrs pi,*pip=NULL;

   if(pip==NULL)
   {
      pi.info=(st_poolinfo *)&Pool;
      pi.entry=(st_poolentry *)&Pool+sizeof(st_poolinfo);
      pi.start=(char *)&Pool+sizeof(st_poolentry)*POOL_KEYS;
      pip=&pi;
   }

   return pip;
}

void PoolInit(void)
{
   int pidx;
   st_poolptrs *pp;

   pp=PoolBase();
   pp->info->size=pp->info->entries=0;
   for(pidx=0;pidx<POOL_IDX_SIZE;pidx++) {
         pp->info->indexmap[pidx]=0; // Clear our index map
         pp->entry[pidx].offset=pp->entry[pidx].length=0;
   }
}

#define PoolIdxBit(idx)       (1<<((idx-1)&7))
#define PoolIdxOff(idx)       (((idx-1)>>3)&0x1FFF)
#define PoolIdx(pi,idx)       (pi->indexmap[PoolIdxOff(idx)])
#define PoolIdxInUse(pi,idx)  ((PoolIdx(pi,idx)&PoolIdxBit(idx))>0)
#define xPoolIdxFree(pi,idx)   (PoolIdx(pi,idx)&=(~PoolIdxBit(idx)))
#define xPoolIdxSet(pi,idx)    (PoolIdx(pi,idx)|=(PoolIdxBit(idx)))

int PoolIdxFree(st_poolinfo *pi,int idx)
{
   int ret;
   if((ret=((((unsigned)(idx-1))<POOL_KEYS) && PoolIdxInUse(pi,idx))))
      PoolIdx(pi,idx)&=~PoolIdxBit(idx);
   return ret;
}

int PoolIdxSet(st_poolinfo *pi,int idx)
{
   int ret;
   if((ret=((((unsigned)(idx-1))<POOL_KEYS) && (!PoolIdxInUse(pi,idx))))) {
      PoolIdx(pi,idx)|=PoolIdxBit(idx);
   }
   return ret;
}

int PoolIdxNew(st_poolinfo *pi)
{
   int idx,bit;

   for(idx=1;(((unsigned)(idx-1))<POOL_KEYS) && PoolIdx(pi,idx)==0xFF;idx+=8);
   if(((unsigned)(idx-1))<POOL_IDX_SIZE) {
      for(bit=0;bit<8 && PoolIdxInUse(pi,idx+bit);bit++);
      idx+=bit;
   } else
      idx=0;
   return idx;
}

int PoolAddStr(char *str)
{
   st_poolptrs *pp=PoolBase();
   int len,idx=0,offset;
   for(len=0;*(str+len);++len);
   if((pp->info->entries<POOL_KEYS) && ((pp->info->size+len+1)<POOL_SIZE)) {
      idx=PoolIdxNew(pp->info)-1;
      pp->entry[idx].length=++len;
      offset=pp->entry[idx].offset=pp->info->size;
      pp->info->size+=len;
      pp->info->entries++;
      while(len-->0) *(pp->start+offset+len)=*(str+len);
      idx++;
      PoolIdxSet(pp->info,idx);
   }

   return idx;
}

int PoolSetStr(int idx,char *str)
{
   st_poolptrs *pp=PoolBase();
   char *start;
   int len,rc=0;

   if((((unsigned)(idx-1))<POOL_KEYS) && PoolIdxInUse(pp->info,idx)) {
      for(len=0;*(str+len);len++);
      pp->entry[idx-1].offset=pp->info->size;
      start=pp->start+pp->info->size;
      while(len-->0) *(start+len)=*(str+len);
      pp->info->size+=len;
      ++rc;
   }
   return rc;
}

void PoolDel(int idx)
{
   st_poolptrs *pp=PoolBase();

   if(((unsigned)(idx-1))<POOL_KEYS) {
      PoolIdxFree(pp->info,idx);
      pp->info->entries--;
      if(POOL_SIZE-(POOL_SIZE>>2)<pp->info->size) { // >>2 sets Garbage Collection to occur at 75% Pool Use
         // TODO: Add Garbage Removal Here
      }
   }

   return;
}

char *PoolGetStr(int idx)
{
   st_poolptrs *pp=PoolBase();
   char *str;

   str=((((unsigned)(idx-1))<=POOL_KEYS) && PoolIdxInUse(pp->info,idx))?pp->start+pp->entry[--idx].offset:NULL;
   return str;
}

#endif
