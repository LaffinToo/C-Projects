/*
 * poolman.h
 *
 * Copyright (c) 2021 Luis "Laffin" Espinoza <laffin<at>gmail.com>
 * All rights reserved.
 *
 * BSD 3-Clause License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
   static char Pool[POOL_SIZE+sizeof(st_poolinfo)+POOL_KEYS*sizeof(st_poolentry);
   static st_poolptrs pi,*pip=NULL;

   if(pip==NULL)  {
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
   if(((unsigned)(idx-1))<POOL_KEYS) {
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

void PoolCleanup(void)
{
   st_poolptrs *pp=PoolBase();
   st_poolentry pe[POOL_KEYS],*t1,*t2;
   int si,di,i,j,entries,midx,offset;

   // Collect all In Use Keys
   for(i=entries=0;i<POOL_KEYS;i++) {
      if(PoolIdxInUse(pp->info,i+1)) {
         pe[entries].offset=pp->entries[i].offset;
         pe[entries++].length=i;   /* Use length to store index */
      }
   }
   /* Sort by offsets */
   for(i=0;i<entries-1;i++) {
      midx=i;
      for(j=i+1;j<entries;j++) {
         if(pe[midx].offset>pe[j].offset) midx=j;
      if(midx!=i) {
         t1=&pe[i];
         t2=&pe[midx];
         t1->offset^=t2.offset;
         t2.offset^=t1->offset;
         t1->offset^=pe[midx].offset;
         t1->length=pe[midx].length
         t2->length=t1->length
         t1->length=pe[midx].length
      }
   }
   /* Compress Data */
   offset=pp->start;
   for(i=0;i<entries;i++) {
      t1=&pe[i];
      if(offset<t1->offset) {
         t2=&pp->entry[t1->length];
         for(j=0;j<=t2->length;j++) {
            *(pp->start+offset+j)=*(pp->start+t2->offset+j);
         }
         t2->offset=offset;
         offset+=t2->length;
      }
   }
}
int PoolSetStr(int idx,char *str)
{
   st_poolptrs *pp=PoolBase();

   char *start;
   int len,rc=0;

   if((((unsigned)(idx-1))<POOL_KEYS) && PoolIdxInUse(pp->info,idx)) {
      for(len=0;*(str+len);len++);
      pp->entry[idx-1].length=++len;
      pp->entry[idx-1].offset=pp->info->size;
      start=pp->start+pp->info->size;
      pp->info->size+=len;
      while(len-->0) *(start+len)=*(str+len);
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
         PoolCleanup();
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
