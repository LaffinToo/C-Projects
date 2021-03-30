/*
 * baseconv.c
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

/**
 * @brief Converts integer to ascii
 * @param sequence   - NULL for up to base 16 or User provided
 * @param base       - Number Base
 * @param num        - Number for conversion
 * @param out_buff   - Pointer to buffer, or NULL if dynamic allocated
 * @returns          - Pointer to buffer
 *
 *
 */
char *bcitoa(char *sequence,int base,int num,char *out_buff) {
   char *dflt="0123456789ABCDEF";
   char *str,*vals;
   int max=1,len=1,idx=0;

   vals=(sequence==NULL)?dflt:sequence;
   while((max*base)<num) {max*=base;len++;}
   if(out_buff) {
      str=out_buff;
   } else {
      str=(char *)malloc(1,len+1);
      str[len]=0;
   }
   while(len--) {
      str[idx++]=vals[(num/max)];
      num-=(num/max)*max;
      max/=base;
   }
   return str;
}

/**
 * @brief Converts an ascii string to an interger
 * @param sequence   - Default provides up to base 16 Conversion, User may provide his own
 * @param base       - Base to convert from
 * @param str        - Ascii string of number
 * @returns          - integer value of conversion or 0 on error
 *
 *
 */
int bcatoi(char *sequence,int base,char *str) {
   char *dflt="0123456789ABCDEF";
   char *val;
   int num=0,idx;

   val=(sequence==NULL)?dflt:sequence;
   while(*str) {
      for(idx=0;idx<base && *str!=val[idx];idx++);
      if(idx==base) {num=0;break;}
      num=(num*base)+idx;
      str++;
   }
   return num;
}

