/* =============================================================================
 *
 * element.h
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


#ifndef ELEMENT_H
#define ELEMENT_H 1


#include "coordinate.h"
#include "list.h"
#include "pair.h"
#include "tm.h"
#include "types.h"


typedef pair_t         edge_t;
typedef struct element element_t;


/* =============================================================================
 * element_compare
 * =============================================================================
 */
long
element_compare (element_t* aElementPtr, element_t* bElementPtr);


/* =============================================================================
 * element_listCompare
 *
 * For use in list_t
 * =============================================================================
 */
long
element_listCompare (const void* aPtr, const void* bPtr);


/* =============================================================================
 * element_mapCompare
 *
 * For use in MAP_T
 * =============================================================================
 */
long
element_mapCompare (const pair_t* aPtr, const pair_t* bPtr);


/* =============================================================================
 * element_alloc
 *
 * Contains a copy of input arg 'coordinates'
 * =============================================================================
 */
element_t*
element_alloc (coordinate_t* coordinates, long numCoordinate);


/* =============================================================================
 * Pelement_alloc
 *
 * Contains a copy of input arg 'coordinates'
 * =============================================================================
 */
element_t*
Pelement_alloc (coordinate_t* coordinates, long numCoordinate);


/* =============================================================================
 * TMelement_alloc
 *
 * Contains a copy of input arg 'coordinates'
 * =============================================================================
 */
element_t*
TMelement_alloc (TM_ARGDECL  coordinate_t* coordinates, long numCoordinate);


/* =============================================================================
 * element_free
 * =============================================================================
 */
void
element_free (element_t* elementPtr);


/* =============================================================================
 * Pelement_free
 * =============================================================================
 */
void
Pelement_free (element_t* elementPtr);


/* =============================================================================
 * TMelement_free
 * =============================================================================
 */
void
TMelement_free (TM_ARGDECL  element_t* elementPtr);


/* =============================================================================
 * element_getNumEdge
 * =============================================================================
 */
long
element_getNumEdge (element_t* elementPtr);


/* =============================================================================
 * element_getEdge
 *
 * Returned edgePtr is sorted; i.e., coordinate_compare(first, second) < 0
 * =============================================================================
 */
edge_t*
element_getEdge (element_t* elementPtr, long i);


/* ============================================================================
 * element_listCompareEdge
 *
 * For use in list_t
 * ============================================================================
 */
long
element_listCompareEdge (const void* aPtr, const void* bPtr);


/* =============================================================================
 * element_mapCompareEdge
 *
 * For use in MAP_T
 * =============================================================================
 */
long
element_mapCompareEdge (const pair_t* aPtr, const pair_t* bPtr);


/* =============================================================================
 * element_heapCompare
 *
 * For use in heap_t
 * =============================================================================
 */
long
element_heapCompare (const void* aPtr, const void* bPtr);


/* =============================================================================
 * element_isInCircumCircle
 * =============================================================================
 */
bool_t
element_isInCircumCircle (element_t* elementPtr, coordinate_t* coordinatePtr);


/* =============================================================================
 * element_clearEncroached
 * =============================================================================
 */
void
element_clearEncroached (element_t* elementPtr);


/* =============================================================================
 * element_getEncroachedPtr
 * =============================================================================
 */
edge_t*
element_getEncroachedPtr (element_t* elementPtr);


/* =============================================================================
 * element_isSkinny
 * =============================================================================
 */
bool_t
element_isSkinny (element_t* elementPtr);


/* =============================================================================
 * element_isBad
 * -- Does it need to be refined?
 * =============================================================================
 */
bool_t
element_isBad (element_t* elementPtr);


/* =============================================================================
 * element_isReferenced
 * =============================================================================
 */
bool_t
element_isReferenced (element_t* elementPtr);


/* =============================================================================
 * TMelement_isReferenced
 * =============================================================================
 */
TM_CALLABLE
bool_t
TMelement_isReferenced (TM_ARGDECL  element_t* elementPtr);


/* =============================================================================
 * element_setIsReferenced
 * =============================================================================
 */
void
element_setIsReferenced (element_t* elementPtr, bool_t status);


/* =============================================================================
 * TMelement_setIsReferenced
 * =============================================================================
 */
TM_CALLABLE
void
TMelement_setIsReferenced (TM_ARGDECL  element_t* elementPtr, bool_t status);


/* =============================================================================
 * element_isGarbage
 * -- Can we deallocate?
 * =============================================================================
 */
bool_t
element_isGarbage (element_t* elementPtr);


/* =============================================================================
 * TMelement_isGarbage
 * -- Can we deallocate?
 * =============================================================================
 */
TM_CALLABLE
bool_t
TMelement_isGarbage (TM_ARGDECL  element_t* elementPtr);


/* =============================================================================
 * element_setIsGarbage
 * =============================================================================
 */
void
element_setIsGarbage (element_t* elementPtr, bool_t status);


/* =============================================================================
 * TMelement_setIsGarbage
 * =============================================================================
 */
TM_CALLABLE
void
TMelement_setIsGarbage (TM_ARGDECL  element_t* elementPtr, bool_t status);


/* =============================================================================
 * element_addNeighbor
 * =============================================================================
 */
void
element_addNeighbor (element_t* elementPtr, element_t* neighborPtr);


/* =============================================================================
 * TMelement_addNeighbor
 * =============================================================================
 */
void
TMelement_addNeighbor (TM_ARGDECL  element_t* elementPtr, element_t* neighborPtr);


/* =============================================================================
 * element_getNeighborListPtr
 * =============================================================================
 */
list_t*
element_getNeighborListPtr (element_t* elementPtr);


/* =============================================================================
 * element_getCommonEdge
 * -- Returns pointer to aElementPtr's shared edge
 * =============================================================================
 */
edge_t*
element_getCommonEdge (element_t* aElementPtr, element_t* bElementPtr);


/* =============================================================================
 * element_getNewPoint
 * -- Either the element is encroached or is skinny, so get the new point to add
 * =============================================================================
 */
coordinate_t
element_getNewPoint (element_t* elementPtr);


/* =============================================================================
 * element_checkAngles
 *
 * Return FALSE if minimum angle constraint not met
 * =============================================================================
 */
bool_t
element_checkAngles (element_t* elementPtr);


/* =============================================================================
 * element_print
 * =============================================================================
 */
void
element_print (element_t* elementPtr);


/* =============================================================================
 * element_printEdge
 * =============================================================================
 */
void
element_printEdge (edge_t* edgePtr);


/* =============================================================================
 * element_printAngles
 * =============================================================================
 */
void
element_printAngles (element_t* elementPtr);


#define PELEMENT_ALLOC(c, n)            Pelement_alloc(c, n)
#define PELEMENT_FREE(e)                /*Pelement_free(e)*/


#define TMELEMENT_ALLOC(c, n)           TMelement_alloc(TM_ARG  c, n)
#define TMELEMENT_FREE(e)               /*TMelement_free(TM_ARG  e)*/
#define TMELEMENT_ISREFERENCED(e)       TMelement_isReferenced(TM_ARG  e)
#define TMELEMENT_SETISREFERENCED(e, s) TMelement_setIsReferenced(TM_ARG  e, s)
#define TMELEMENT_ISGARBAGE(e)          TMelement_isGarbage(TM_ARG  e)
#define TMELEMENT_SETISGARBAGE(e, s)    TMelement_setIsGarbage(TM_ARG  e, s)
#define TMELEMENT_ADDNEIGHBOR(e, n)     TMelement_addNeighbor(TM_ARG  e, n)
#define TMELEMENT_GETNEIGHBORLIST(e)    TMelement_getNeighborListPtr(TM_ARG  e)


#endif /* ELEMENT_H */


/* =============================================================================
 *
 * End of element.h
 *
 * =============================================================================
 */
