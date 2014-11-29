/* =============================================================================
 *
 * bitmap.h
 *
 * =============================================================================
 *
 * Copyright (C) Stanford University, 2006.  All Rights Reserved.
 * Author: Chi Cao Minh
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


#ifndef BITMAP_H
#define BITMAP_H 1


#include "types.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct bitmap {
    long numBit;
    long numWord;
    ulong_t* bits;
} bitmap_t;


/* =============================================================================
 * bitmap_alloc
 * -- Returns NULL on failure
 * =============================================================================
 */
bitmap_t*
bitmap_alloc (long numBit);


/* =============================================================================
 * Pbitmap_alloc
 * -- Returns NULL on failure
 * =============================================================================
 */
bitmap_t*
Pbitmap_alloc (long numBit);


/* =============================================================================
 * bitmap_free
 * =============================================================================
 */
void
bitmap_free (bitmap_t* bitmapPtr);


/* =============================================================================
 * Pbitmap_free
 * =============================================================================
 */
void
Pbitmap_free (bitmap_t* bitmapPtr);


/* =============================================================================
 * bitmap_set
 * -- Sets ith bit to 1
 * -- Returns TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
bitmap_set (bitmap_t* bitmapPtr, long i);


/* =============================================================================
 * bitmap_clear
 * -- Clears ith bit to 0
 * -- Returns TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
bitmap_clear (bitmap_t* bitmapPtr, long i);


/* =============================================================================
 * bitmap_clearAll
 * -- Clears all bit to 0
 * =============================================================================
 */
void
bitmap_clearAll (bitmap_t* bitmapPtr);


/* =============================================================================
 * bitmap_isSet
 * -- Returns TRUE if ith bit is set, else FALSE
 * =============================================================================
 */
bool_t
bitmap_isSet (bitmap_t* bitmapPtr, long i);


/* =============================================================================
 * bitmap_findClear
 * -- Returns index of first clear bit
 * -- If start index is negative, will start from beginning
 * -- If all bits are set, returns -1
 * =============================================================================
 */
long
bitmap_findClear (bitmap_t* bitmapPtr, long startIndex);


/* =============================================================================
 * bitmap_findSet
 * -- Returns index of first set bit
 * -- If all bits are clear, returns -1
 * =============================================================================
 */
long
bitmap_findSet (bitmap_t* bitmapPtr, long startIndex);


/* =============================================================================
 * bitmap_getNumClear
 * =============================================================================
 */
long
bitmap_getNumClear (bitmap_t* bitmapPtr);


/* =============================================================================
 * bitmap_getNumSet
 * =============================================================================
 */
long
bitmap_getNumSet (bitmap_t* bitmapPtr);


/* =============================================================================
 * bitmap_copy
 * =============================================================================
 */
void
bitmap_copy (bitmap_t* dstPtr, bitmap_t* srcPtr);


/* =============================================================================
 * bitmap_toggleAll
 * =============================================================================
 */
void
bitmap_toggleAll (bitmap_t* bitmapPtr);


#define PBITMAP_ALLOC(n)                Pbitmap_alloc(n)
#define PBITMAP_FREE(b)                 Pbitmap_free(b)
#define PBITMAP_SET(b, i)               bitmap_set(b, i)
#define PBITMAP_CLEAR(b, i)             bitmap_clear(b, i)
#define PBITMAP_CLEARALL(b)             bitmap_clearAll(b)
#define PBITMAP_ISSET(b, i)             bitmap_isSet(b, i)
#define PBITMAP_FINDCLEAR(b, i)         bitmap_findClear(b, i)
#define PBITMAP_FINDSET(b, i)           bitmap_findSet(b, i)
#define PBITMAP_GETNUMCLEAR(b)          bitmap_getNumClear(b)
#define PBITMAP_GETNUMSET(b)            bitmap_getNumSet(b)
#define PBITMAP_COPY(b)                 bitmap_copy(b)
#define PBITMAP_TOGGLEALL(b)            bitmap_toggleAll(b)


#ifdef __cplusplus
}
#endif


#endif /* BITMAP_H */


/* =============================================================================
 *
 * End of bitmap.h
 *
 * =============================================================================
 */
