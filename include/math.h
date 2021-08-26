/*
 * Copyright 2019 Orace KPAKPO </ orace.kpakpo@yahoo.fr >
 *
 * This file is part of EOS.
 *
 * EOS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * EOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __MATH_H__
#define __MATH_H__
#include <types.h>

#define abs(a) (((a) < 0)?-(a):(a))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define sign(x) ((x < 0) ? -1 :((x > 0) ? 1 : 0))

static void __do_udiv64(u64_t __num, u64_t __den,
	u64_t* __quotient, u64_t* __remainder)
{
         u64_t n, d, x, answer;
        
        if(__den == 0)
          n = 0; /* intentionally do a division by zero */
        for(n = __num, d = __den, x = 1; n >= d && 
        	((d & 0x8000000000000000) == 0); x <<= 1, d <<= 1);

        for(answer = 0; x != 0; x >>= 1, d >>= 1)
        {
          if(n >= d)
          {
                  n -= d;
                  answer |= x;
          }
        }
        *__quotient = answer;
        *__remainder = n;
}

static u64_t __udivdi3(u64_t __num, u64_t __den)
{
  u64_t __quotient, __remainder;
  
  __do_udiv64(__num, __den, &__quotient, &__remainder);
  return __quotient;
}

static u64_t __umoddi3(u64_t __num, u64_t __den)
{
  u64_t __quotient, __remainder;
  
  __do_udiv64(__num, __den, &__quotient, &__remainder);
  return __remainder;
}

#endif //__MATH_H__