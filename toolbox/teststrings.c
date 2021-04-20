/*
 * teststrings.c
 *
 * Copyright 2021 Luis "Laffin" Espinoza <laffintoo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */


#include <stdio.h>
#include <stdlib.h>


/** \brief Define _USE_TOOLBOX to use toolbox functions for memcpy,memset,memcmp,strlen
 */
// #define _USE_TOOLBOX

#include "strings.h"

// Performance Test, will iterate LOOPS times
#define DEBUG

// Used for string array
#define STRCNT (20)

#ifdef DEBUG
#define LOOPS 1
#else
#define LOOPS  (1000000)
#endif // DEBUG


int main(int argc, char **argv)
{
    char *str[STRCNT]= {"    Hello World    "};
    int i,x=LOOPS;

    while(x-->0)
    {
        if(!x) printf("Original : %p '%s'\n",str[0],str[0]);
        str[1]=tb_strdup(str[0]);
        if(!x) printf("Duplicate: %p '%s'\n",str[1],str[1]);
        str[2]=tb_strndup(str[1],9);
        if(!x) printf("Partial D: %p '%s'\n",str[2],str[2]);
        str[3]=tb_strltrim(str[1]);
        if(!x) printf("Left Trim: %p '%s'\n",str[3],str[3]);
        str[4]=tb_strrtrim(str[1]);
        if(!x) printf("Right Trm: %p '%s'\n",str[4],str[4]);
        str[5]=tb_strlrtrim(str[1]);
        if(!x) printf("L/R Trim : %p '%s'\n",str[5],str[5]);
        str[6]=tb_strsub(str[1],7,5);
        if(!x) printf("Substr   : %p '%s'\n",str[6],str[6]);
        str[7]=tb_strinsert(str[1],"there ",10);
        if(!x) printf("Insert   : %p '%s'\n",str[7],str[7]);
        str[8]=tb_strdup(str[1]);
        tb_strsplit(&str[8],&str[9],10);
        if(!x) {
           printf("StrSplit :\n        1: %p '%s'\n",str[8],str[8]);
           printf("        2: %p '%s'\n",str[9],str[9]);
        }
        str[10]=tb_strfind(str[7],"there");
        if(!x) printf("Find     : %p '%s'\n",str[10],str[10]);
        str[11]=tb_strdup(str[7]);
        tb_strreplace(&str[11],"World","Phyllo");
        if(!x) printf("Replace  : %p '%s'\n",str[11],str[11]);
        str[12]=tb_strdup("Phyllo");
        str[13]=tb_strdup("pHyLlo");
        if(!x) printf("StrCmp   : %p '%s' %1.1s '%s'\n",str[12],str[12],"<=>"+(tb_strcmp(str[12],str[13])+1),str[13]);
        if(!x) printf("StrICmp  : %p '%s' %1.1s '%s'\n",str[12],str[12],"<=>"+(tb_stricmp(str[12],str[13])+1),str[13]);
        if(!x) printf("StrCmp   : %p '%s' %1.1s '%s'\n",str[13],str[13],"<=>"+(tb_strcmp(str[13],str[12])+1),str[12]);
        char *strings[]={"One","Two","Three","Four","Five",NULL};
        str[14]=strdup("Zero");
        tb_strcatx(str[14],5,strings[0],strings[1],strings[2],strings[3],strings[4]);
        if(!x) printf("StrCatX  : %p '%s'\n",str[14],str[14]);
        str[15]=strdup("Zero");
        tb_strcatdx(str[15],',',5,strings[0],strings[1],strings[2],strings[3],strings[4]);
        if(!x) printf("StrCatDX : %p '%s'\n",str[15],str[15]);
        str[16]=strdup("Zero");
        str[16]=tb_strcatax(str[16],5,strings);
        if(!x) printf("StrCatAX : %p '%s'\n",str[16],str[16]);
        str[17]=strdup("Zero");
        str[17]=tb_strcatdax(str[17],' ',5,strings);
        if(!x) printf("StrCatDAX: %p '%s'\n",str[17],str[17]);
        
        
#ifndef DEBUG
        if(!(x%100)) printf("%d    \r",x);
        for(i=1; i<STRCNT; i++)
        {
            if(str[i] && i!=10)
            {
                free(str[i]);
                str[i]=NULL;
            }
        }
#endif

    }

    return 0;

}
