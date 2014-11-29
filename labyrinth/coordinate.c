/* =============================================================================
 *
 * coordinate.c
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


#include <math.h>
#include <stdlib.h>
#include "coordinate.h"
#include "pair.h"
#include "tm.h"
#include "types.h"


/* =============================================================================
 * coordinate_alloc
 * =============================================================================
 */
coordinate_t*
coordinate_alloc (long x, long y, long z)
{
    coordinate_t* coordinatePtr;

    coordinatePtr = (coordinate_t*)malloc(sizeof(coordinate_t));
    if (coordinatePtr) {
        coordinatePtr->x = x;
        coordinatePtr->y = y;
        coordinatePtr->z = z;
    }

    return coordinatePtr;
}


/* =============================================================================
 * coordinate_free
 * =============================================================================
 */
void
coordinate_free (coordinate_t* coordinatePtr)
{
    free(coordinatePtr);
}


/* =============================================================================
 * coordinate_isEqual
 * =============================================================================
 */
bool_t
coordinate_isEqual (coordinate_t* aPtr, coordinate_t* bPtr)
{
    if ((aPtr->x == bPtr->x) &&
        (aPtr->y == bPtr->y) &&
        (aPtr->z == bPtr->z))
    {
        return TRUE;
    }

    return FALSE;
}


/* =============================================================================
 * getPairDistance
 * =============================================================================
 */
static double
getPairDistance (pair_t* pairPtr)
{
    coordinate_t* aPtr = (coordinate_t*)pairPtr->firstPtr;
    coordinate_t* bPtr = (coordinate_t*)pairPtr->secondPtr;
    long dx = aPtr->x - bPtr->x;
    long dy = aPtr->y - bPtr->y;
    long dz = aPtr->z - bPtr->z;
    long dx2 = dx * dx;
    long dy2 = dy * dy;
    long dz2 = dz * dz;
    return sqrt((double)(dx2 + dy2 + dz2));
}


/* =============================================================================
 * coordinate_comparePair
 * -- For sorting in list of source/destination pairs
 * -- Route longer paths first so they are more likely to succeed
 * =============================================================================
 */
long
coordinate_comparePair (const void* aPtr, const void* bPtr)
{
    double aDistance = getPairDistance((pair_t*)aPtr);
    double bDistance = getPairDistance((pair_t*)bPtr);

    if (aDistance < bDistance) {
        return 1;
    } else if (aDistance > bDistance) {
        return -1;
    }

    return 0;
}


/* =============================================================================
 * coordinate_areAdjacent
 * =============================================================================
 */
bool_t
coordinate_areAdjacent (coordinate_t* aPtr, coordinate_t* bPtr)
{
    long dx = aPtr->x - bPtr->x;
    long dy = aPtr->y - bPtr->y;
    long dz = aPtr->z - bPtr->z;
    long dx2 = dx * dx;
    long dy2 = dy * dy;
    long dz2 = dz * dz;

    return (((dx2 + dy2 + dz2) == 1) ? TRUE : FALSE);
}


/* =============================================================================
 *
 * End of coordinate.c
 *
 * =============================================================================
 */
