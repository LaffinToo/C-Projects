/*
 * PoolManTest.c
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

 #include <stdio.h>

 #define POOL_SIZE (1024)
 #define POOL_KEYS (16)
 #include "poolman.h"

int main(int argc, char **argv)
{
   st_poolptrs *pp=PoolBase();
   int si[5],i;
   PoolInit(); // Yes You NEED to do this to Init or RESET

   si[0]=PoolAddStr("Hello World!");
   si[1]=PoolAddStr("Lucie was here");
   si[2]=PoolAddStr("Meh!");
   for(i=0;i<5;i++) printf("Wall %d %d: %s\n",i,si[i],PoolGetStr(si[i]));
   PoolDel(si[1]);
   for(i=0;i<5;i++) printf("Wall %d %d: %s\n",i,si[i],PoolGetStr(si[i]));
   si[1]=PoolAddStr("Damn");
   si[3]=PoolAddStr("Damn");
   si[4]=PoolAddStr("Damn");
   PoolSetStr(si[3],"Lucie was here");
   for(i=0;i<5;i++) printf("Wall %d %d: %s\n",i,si[i],PoolGetStr(si[i]));

   return 0;

}
