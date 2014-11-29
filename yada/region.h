/* =============================================================================
 *
 * region.h
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


#ifndef REGION_H
#define REGION_H 1


#include "element.h"
#include "heap.h"
#include "mesh.h"
#include "tm.h"


typedef struct region  region_t;


/* =============================================================================
 * Pregion_alloc
 * =============================================================================
 */
region_t*
Pregion_alloc ();


/* =============================================================================
 * Pregion_free
 * =============================================================================
 */
void
Pregion_free (region_t* regionPtr);


/* =============================================================================
 * TMregion_refine
 *
 * Calculate refined triangles. The region is built by using a breadth-first
 * search starting from the element (elementPtr) containing the new point we
 * are adding. If expansion hits a boundary segment (encroachment) we build
 * a region around that element instead, to avoid a potential infinite loop.
 *
 * Returns net number of elements added to mesh.
 * =============================================================================
 */
long
TMregion_refine (TM_ARGDECL
                 region_t* regionPtr, element_t* elementPtr, mesh_t* meshPtr);


/* =============================================================================
 * Pregion_clearBad
 * =============================================================================
 */
void
Pregion_clearBad (region_t* regionPtr);


/* =============================================================================
 * TMregion_transferBad
 * =============================================================================
 */
void
TMregion_transferBad (TM_ARGDECL  region_t* regionPtr, heap_t* workHeapPtr);


#define PREGION_ALLOC()                 Pregion_alloc()
#define PREGION_FREE(r)                 Pregion_free(r)
#define PREGION_CLEARBAD(r)             Pregion_clearBad(r)
#define TMREGION_REFINE(r, e, m)        TMregion_refine(TM_ARG  r, e, m)
#define TMREGION_TRANSFERBAD(r, q)      TMregion_transferBad(TM_ARG  r, q)


#endif /* REGION_H */


/* =============================================================================
 *
 * End of region.h
 *
 * =============================================================================
 */
