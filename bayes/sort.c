/* =============================================================================
 *
 * sort.c
 *
 * =============================================================================
 *
 * Quick sort
 *
 * Copyright (C) 2002 Michael Ringgaard. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF  SUCH DAMAGE
 *
 * =============================================================================
 *
 * Modifed October 2007 by Chi Cao Minh
 * -- Changed signature of comparison function
 *
 * =============================================================================
 *
 * For the license of bayes/sort.h and bayes/sort.c, please see the header
 * of the files.
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of kmeans, please see kmeans/LICENSE.kmeans
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of ssca2, please see ssca2/COPYRIGHT
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of lib/mt19937ar.c and lib/mt19937ar.h, please see the
 * header of the files.
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of lib/rbtree.h and lib/rbtree.c, please see
 * lib/LEGALNOTICE.rbtree and lib/LICENSE.rbtree
 * 
 * ------------------------------------------------------------------------
 * 
 * Unless otherwise noted, the following license applies to STAMP files:
 * 
 * Copyright (c) 2007, Stanford University
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 * 
 *     * Neither the name of Stanford University nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =============================================================================
 */

#include "sort.h"

#define CUTOFF 8


/* =============================================================================
 * swap
 * =============================================================================
 */
static void
swap (char* a, char* b, unsigned width)
{
    if (a != b) {
        while (width--) {
            char tmp = *a;
            *a++ = *b;
            *b++ = tmp;
        }
    }
}


/* =============================================================================
 * shortsort
 * =============================================================================
 */
static void
shortsort (char* lo,
           char* hi,
           unsigned width,
           int (*cmp)(const void* p1, const void* p2, long n, long offset),
           long n,
           long offset)
{
    while (hi > lo) {
        char* max = lo;
        char* p;
        for (p = (lo + width); p <= hi; p += width) {
            if (cmp(p, max, n, offset) > 0) {
                max = p;
            }
        }
        swap(max, hi, width);
        hi -= width;
    }
}


/* =============================================================================
 * sort
 * =============================================================================
 */
void
sort (void *base,
      unsigned num,
      unsigned width,
      int (*cmp)(const void* p1, const void* p2, long n, long offset),
      long n,
      long offset)
{
    if (num < 2 || width == 0) {
        return;
    }

    char* lostk[30];
    char* histk[30];

    int stkptr = 0;

    char* lo = (char*)base;
    char* hi = (char*)base + (width * (num - 1));

    unsigned size;

recurse:

    size = (hi - lo) / width + 1;

    if (size <= CUTOFF) {

        shortsort(lo, hi, width, cmp, n, offset);

    } else {

        char* mid = lo + (size / 2) * width;
        swap(mid, lo, width);

        char* loguy = lo;
        char* higuy = hi + width;

        for (;;) {
            do {
                loguy += width;
            } while (loguy <= hi && cmp(loguy, lo, n, offset) <= 0);
            do {
                higuy -= width;
            } while (higuy > lo && cmp(higuy, lo, n, offset) >= 0);
            if (higuy < loguy) {
                break;
            }
            swap(loguy, higuy, width);
        }

        swap(lo, higuy, width);

        if (higuy - 1 - lo >= hi - loguy) {
            if (lo + width < higuy) {
                lostk[stkptr] = lo;
                histk[stkptr] = higuy - width;
                ++stkptr;
            }

            if (loguy < hi) {
                lo = loguy;
                goto recurse;
            }
        } else {
            if (loguy < hi) {
                lostk[stkptr] = loguy;
                histk[stkptr] = hi;
                ++stkptr;
            }
            if (lo + width < higuy) {
                hi = higuy - width;
                goto recurse;
            }
        }

    }

    --stkptr;
    if (stkptr >= 0) {
        lo = lostk[stkptr];
        hi = histk[stkptr];
        goto recurse;
    }
}


/* =============================================================================
 *
 * End of sort.c
 *
 * =============================================================================
 */
