/*
 * PoolManTest.c
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
