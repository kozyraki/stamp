/* =============================================================================
 *
 * grid.h
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


#ifndef GRID_H
#define GRID_H 1


#include "types.h"
#include "vector.h"


typedef struct grid {
    long width;
    long height;
    long depth;
    long* points;
    long* points_unaligned;
} grid_t;

enum {
    GRID_POINT_FULL  = -2L,
    GRID_POINT_EMPTY = -1L
};


/* =============================================================================
 * grid_alloc
 * =============================================================================
 */
grid_t*
grid_alloc (long width, long height, long depth);


/* =============================================================================
 * TMgrid_alloc
 * =============================================================================
 */
grid_t*
Pgrid_alloc (long width, long height, long depth);


/* =============================================================================
 * grid_free
 * =============================================================================
 */
void
grid_free (grid_t* gridPtr);


/* =============================================================================
 * Pgrid_free
 * =============================================================================
 */
void
Pgrid_free (grid_t* gridPtr);


/* =============================================================================
 * grid_copy
 * =============================================================================
 */
void
grid_copy (grid_t* dstGridPtr, grid_t* srcGridPtr);


/* =============================================================================
 * grid_isPointValid
 * =============================================================================
 */
bool_t
grid_isPointValid (grid_t* gridPtr, long x, long y, long z);


/* =============================================================================
 * grid_getPointRef
 * =============================================================================
 */
long*
grid_getPointRef (grid_t* gridPtr, long x, long y, long z);


/* =============================================================================
 * grid_getPointIndices
 * =============================================================================
 */
void
grid_getPointIndices (grid_t* gridPtr,
                      long* gridPointPtr, long* xPtr, long* yPtr, long* zPtr);


/* =============================================================================
 * grid_getPoint
 * =============================================================================
 */
long
grid_getPoint (grid_t* gridPtr, long x, long y, long z);


/* =============================================================================
 * grid_isPointEmpty
 * =============================================================================
 */
bool_t
grid_isPointEmpty (grid_t* gridPtr, long x, long y, long z);


/* =============================================================================
 * grid_isPointFull
 * =============================================================================
 */
bool_t
grid_isPointFull (grid_t* gridPtr, long x, long y, long z);


/* =============================================================================
 * grid_setPoint
 * =============================================================================
 */
void
grid_setPoint (grid_t* gridPtr, long x, long y, long z, long value);


/* =============================================================================
 * grid_addPath
 * =============================================================================
 */
void
grid_addPath (grid_t* gridPtr, vector_t* pointVectorPtr);


/* =============================================================================
 * TMgrid_addPath
 * =============================================================================
 */
TM_CALLABLE
void
TMgrid_addPath (TM_ARGDECL  grid_t* gridPtr, vector_t* pointVectorPtr);


/* =============================================================================
 * grid_print
 * =============================================================================
 */
void
grid_print (grid_t* gridPtr);


#define PGRID_ALLOC(x, y, z)            Pgrid_alloc(x, y, z)
#define PGRID_FREE(g)                   Pgrid_free(g)

#define TMGRID_ADDPATH(g, p)            TMgrid_addPath(TM_ARG  g, p)


#endif /* GRID_H */


/* =============================================================================
 *
 * End of grid.c
 *
 * =============================================================================
 */
