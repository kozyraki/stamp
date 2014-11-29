/* =============================================================================
 *
 * element.c
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


#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "coordinate.h"
#include "element.h"
#include "pair.h"
#include "tm.h"
#include "types.h"


struct element {
    coordinate_t coordinates[3];
    long numCoordinate;
    coordinate_t circumCenter;
    double circumRadius;
    double minAngle;
    edge_t edges[3];
    long numEdge;
    coordinate_t midpoints[3]; /* midpoint of each edge */
    double radii[3];           /* half of edge length */
    edge_t* encroachedEdgePtr; /* opposite obtuse angle */
    bool_t isSkinny;
    list_t* neighborListPtr;
    bool_t isGarbage;
    bool_t isReferenced;
};


#if defined(TEST_ELEMENT) || defined(TEST_MESH)
double global_angleConstraint = 20.0;
#else
extern double global_angleConstraint;
#endif


/* =============================================================================
 * minimizeCoordinates
 * -- put smallest coordinate in position 0
 * =============================================================================
 */
static void
minimizeCoordinates (element_t* elementPtr)
{
    long i;
    coordinate_t* coordinates = elementPtr->coordinates;
    long numCoordinate = elementPtr->numCoordinate;
    long minPosition = 0;

    for (i = 1; i < numCoordinate; i++) {
        if (coordinate_compare(&coordinates[i], &coordinates[minPosition]) < 0) {
            minPosition = i;
        }
    }

    while (minPosition != 0) {
        coordinate_t tmp = coordinates[0];
        long j;
        for (j = 0; j < (numCoordinate - 1); j++) {
            coordinates[j] = coordinates[j+1];
        }
        coordinates[numCoordinate-1] = tmp;
        minPosition--;
    }
}


/* =============================================================================
 * checkAngles
 * -- Sets isSkinny to TRUE if the angle constraint is not met
 * =============================================================================
 */
static void
checkAngles (element_t* elementPtr)
{
    long numCoordinate = elementPtr->numCoordinate;
    double angleConstraint = global_angleConstraint;
    double minAngle = 180.0;

    assert(numCoordinate == 2 || numCoordinate == 3);
    elementPtr->isReferenced = FALSE;
    elementPtr->isSkinny = FALSE;
    elementPtr->encroachedEdgePtr = NULL;

    if (numCoordinate == 3) {
        long i;
        coordinate_t* coordinates = elementPtr->coordinates;
        for (i = 0; i < 3; i++) {
            double angle = coordinate_angle(&coordinates[i],
                                            &coordinates[(i + 1) % 3],
                                            &coordinates[(i + 2) % 3]);
            assert(angle > 0.0);
            assert(angle < 180.0);
            if (angle > 90.0) {
                elementPtr->encroachedEdgePtr = &elementPtr->edges[(i + 1) % 3];
            }
            if (angle < angleConstraint) {
                elementPtr->isSkinny = TRUE;
            }
            if (angle < minAngle) {
                minAngle = angle;
            }
        }
        assert(minAngle < 180.0);
    }

    elementPtr->minAngle = minAngle;
}


/* =============================================================================
 * calculateCircumCenter
 *
 * Given three points A(ax,ay), B(bx,by), C(cx,cy), circumcenter R(rx,ry):
 *
 *              |                         |
 *              | by - ay   (||b - a||)^2 |
 *              |                         |
 *              | cy - ay   (||c - a||)^2 |
 *              |                         |
 *   rx = ax - -----------------------------
 *                   |                   |
 *                   | bx - ax   by - ay |
 *               2 * |                   |
 *                   | cx - ax   cy - ay |
 *                   |                   |
 *
 *              |                         |
 *              | bx - ax   (||b - a||)^2 |
 *              |                         |
 *              | cx - ax   (||c - a||)^2 |
 *              |                         |
 *   ry = ay + -----------------------------
 *                   |                   |
 *                   | bx - ax   by - ay |
 *               2 * |                   |
 *                   | cx - ax   cy - ay |
 *                   |                   |
 *
 * =============================================================================
 */
static void
calculateCircumCircle (element_t* elementPtr)
{
    long numCoordinate = elementPtr->numCoordinate;
    coordinate_t* coordinates = elementPtr->coordinates;
    coordinate_t* circumCenterPtr = &elementPtr->circumCenter;

    assert(numCoordinate == 2 || numCoordinate == 3);

    if (numCoordinate == 2) {
        circumCenterPtr->x = (coordinates[0].x + coordinates[1].x) / 2.0;
        circumCenterPtr->y = (coordinates[0].y + coordinates[1].y) / 2.0;
    } else {
        double ax = coordinates[0].x;
        double ay = coordinates[0].y;
        double bx = coordinates[1].x;
        double by = coordinates[1].y;
        double cx = coordinates[2].x;
        double cy = coordinates[2].y;
        double bxDelta = bx - ax;
        double byDelta = by - ay;
        double cxDelta = cx - ax;
        double cyDelta = cy - ay;
        double bDistance2 = (bxDelta * bxDelta) + (byDelta * byDelta);
        double cDistance2 = (cxDelta * cxDelta) + (cyDelta * cyDelta);
        double xNumerator = (byDelta * cDistance2) - (cyDelta * bDistance2);
        double yNumerator = (bxDelta * cDistance2) - (cxDelta * bDistance2);
        double denominator = 2 * ((bxDelta * cyDelta) - (cxDelta * byDelta));
        double rx = ax - (xNumerator / denominator);
        double ry = ay + (yNumerator / denominator);
        assert(fabs(denominator) > DBL_MIN); /* make sure not colinear */
        circumCenterPtr->x = rx;
        circumCenterPtr->y = ry;
    }

    elementPtr->circumRadius = coordinate_distance(circumCenterPtr,
                                                   &coordinates[0]);
}


/* =============================================================================
 * setEdge
 *
  * Note: Makes pairPtr sorted; i.e., coordinate_compare(first, second) < 0
 * =============================================================================
 */
static void
setEdge (element_t* elementPtr, long i)
{
    long numCoordinate = elementPtr->numCoordinate;
    coordinate_t* coordinates = elementPtr->coordinates;

    coordinate_t* firstPtr = &coordinates[i];
    coordinate_t* secondPtr = &coordinates[(i + 1) % numCoordinate];

    edge_t* edgePtr = &elementPtr->edges[i];

    long cmp = coordinate_compare(firstPtr, secondPtr);
    assert(cmp != 0);
    if (cmp < 0) {
        edgePtr->firstPtr  = (void*)firstPtr;
        edgePtr->secondPtr = (void*)secondPtr;
    } else {
        edgePtr->firstPtr  = (void*)secondPtr;
        edgePtr->secondPtr = (void*)firstPtr;
    }

    coordinate_t* midpointPtr = &elementPtr->midpoints[i];
    midpointPtr->x = (firstPtr->x + secondPtr->x) / 2.0;
    midpointPtr->y = (firstPtr->y + secondPtr->y) / 2.0;

    elementPtr->radii[i] = coordinate_distance(firstPtr, midpointPtr);
}


/* =============================================================================
 * initEdges
 * =============================================================================
 */
static void
initEdges (element_t* elementPtr, coordinate_t* coordinates, long numCoordinate)
{
    long numEdge = ((numCoordinate * (numCoordinate - 1)) / 2);

    elementPtr->numEdge = numEdge;

    long e;
    for (e = 0; e < numEdge; e++) {
        setEdge(elementPtr, e);
    }
}


/* =============================================================================
 * element_compare
 * =============================================================================
 */
long
element_compare (element_t* aElementPtr, element_t* bElementPtr)
{
    long aNumCoordinate = aElementPtr->numCoordinate;
    long bNumCoordinate = bElementPtr->numCoordinate;
    coordinate_t* aCoordinates = aElementPtr->coordinates;
    coordinate_t* bCoordinates = bElementPtr->coordinates;

    if (aNumCoordinate < bNumCoordinate) {
        return -1;
    } else if (aNumCoordinate > bNumCoordinate) {
        return 1;
    }

    long i;
    for (i = 0; i < aNumCoordinate; i++) {
        long compareCoordinate =
            coordinate_compare(&aCoordinates[i], &bCoordinates[i]);
        if (compareCoordinate != 0) {
            return compareCoordinate;
        }
    }

    return 0;
}


/* =============================================================================
 * element_listCompare
 *
 * For use in list_t
 * =============================================================================
 */
long
element_listCompare (const void* aPtr, const void* bPtr)
{
    element_t* aElementPtr = (element_t*)aPtr;
    element_t* bElementPtr = (element_t*)bPtr;

    return element_compare(aElementPtr, bElementPtr);
}


/* =============================================================================
 * element_mapCompare
 *
 * For use in MAP_T
 * =============================================================================
 */
long
element_mapCompare (const pair_t* aPtr, const pair_t* bPtr)
{
    element_t* aElementPtr = (element_t*)(aPtr->firstPtr);
    element_t* bElementPtr = (element_t*)(bPtr->firstPtr);

    return element_compare(aElementPtr, bElementPtr);
}


/* =============================================================================
 * element_alloc
 *
 * Contains a copy of input arg 'coordinates'
 * =============================================================================
 */
element_t*
element_alloc (coordinate_t* coordinates, long numCoordinate)
{
    element_t* elementPtr;

    elementPtr = (element_t*)malloc(sizeof(element_t));
    if (elementPtr) {
        long i;
        for (i = 0; i < numCoordinate; i++) {
            elementPtr->coordinates[i] = coordinates[i];
        }
        elementPtr->numCoordinate = numCoordinate;
        minimizeCoordinates(elementPtr);
        checkAngles(elementPtr);
        calculateCircumCircle(elementPtr);
        initEdges(elementPtr, coordinates, numCoordinate);
        elementPtr->neighborListPtr = list_alloc(element_listCompare);
        assert(elementPtr->neighborListPtr);
        elementPtr->isGarbage = FALSE;
        elementPtr->isReferenced = FALSE;
    }

    return elementPtr;
}


/* =============================================================================
 * Pelement_alloc
 *
 * Contains a copy of input arg 'coordinates'
 * =============================================================================
 */
element_t*
Pelement_alloc (coordinate_t* coordinates, long numCoordinate)
{
    element_t* elementPtr;

    elementPtr = (element_t*)P_MALLOC(sizeof(element_t));
    if (elementPtr) {
        long i;
        for (i = 0; i < numCoordinate; i++) {
            elementPtr->coordinates[i] = coordinates[i];
        }
        elementPtr->numCoordinate = numCoordinate;
        minimizeCoordinates(elementPtr);
        checkAngles(elementPtr);
        calculateCircumCircle(elementPtr);
        initEdges(elementPtr, coordinates, numCoordinate);
        elementPtr->neighborListPtr = PLIST_ALLOC(element_listCompare);
        assert(elementPtr->neighborListPtr);
        elementPtr->isGarbage = FALSE;
        elementPtr->isReferenced = FALSE;
    }

    return elementPtr;
}


/* =============================================================================
 * TMelement_alloc
 *
 * Contains a copy of input arg 'coordinates'
 * =============================================================================
 */
element_t*
TMelement_alloc (TM_ARGDECL  coordinate_t* coordinates, long numCoordinate)
{
    element_t* elementPtr;

    elementPtr = (element_t*)TM_MALLOC(sizeof(element_t));
    if (elementPtr) {
        long i;
        for (i = 0; i < numCoordinate; i++) {
            elementPtr->coordinates[i] = coordinates[i];
        }
        elementPtr->numCoordinate = numCoordinate;
        minimizeCoordinates(elementPtr);
        checkAngles(elementPtr);
        calculateCircumCircle(elementPtr);
        initEdges(elementPtr, coordinates, numCoordinate);
        elementPtr->neighborListPtr = TMLIST_ALLOC(element_listCompare);
        assert(elementPtr->neighborListPtr);
        elementPtr->isGarbage = FALSE;
        elementPtr->isReferenced = FALSE;
    }

    return elementPtr;
}


/* =============================================================================
 * element_free
 * =============================================================================
 */
void
element_free (element_t* elementPtr)
{
    list_free(elementPtr->neighborListPtr);
    free(elementPtr);
}


/* =============================================================================
 * Pelement_free
 * =============================================================================
 */
void
Pelement_free (element_t* elementPtr)
{
    PLIST_FREE(elementPtr->neighborListPtr);
    P_FREE(elementPtr);
}


/* =============================================================================
 * TMelement_free
 * =============================================================================
 */
void
TMelement_free (TM_ARGDECL  element_t* elementPtr)
{
    TMLIST_FREE(elementPtr->neighborListPtr);
    TM_FREE(elementPtr);
}


/* =============================================================================
 * element_getNumEdge
 * =============================================================================
 */
long
element_getNumEdge (element_t* elementPtr)
{
    return elementPtr->numEdge;
}


/* =============================================================================
 * element_getEdge
 *
 * Returned edgePtr is sorted; i.e., coordinate_compare(first, second) < 0
 * =============================================================================
 */
edge_t*
element_getEdge (element_t* elementPtr, long i)
{
    if (i < 0 || i > elementPtr->numEdge) {
        return NULL;
    }

    return &elementPtr->edges[i];
}


/* =============================================================================
 * element_compareEdge
 * =============================================================================
 */
static long
compareEdge (edge_t* aEdgePtr, edge_t* bEdgePtr)
{
    long diffFirst = coordinate_compare((coordinate_t*)aEdgePtr->firstPtr,
                                        (coordinate_t*)bEdgePtr->firstPtr);

    return ((diffFirst != 0) ?
            (diffFirst) :
            (coordinate_compare((coordinate_t*)aEdgePtr->secondPtr,
                                (coordinate_t*)bEdgePtr->secondPtr)));
}


/* ============================================================================
 * element_listCompareEdge
 *
 * For use in list_t
 * ============================================================================
 */
long
element_listCompareEdge (const void* aPtr, const void* bPtr)
{
    edge_t* aEdgePtr = (edge_t*)(aPtr);
    edge_t* bEdgePtr = (edge_t*)(bPtr);

    return compareEdge(aEdgePtr, bEdgePtr);
}


/* =============================================================================
 * element_mapCompareEdge
 *
  * For use in MAP_T
 * =============================================================================
 */
long
element_mapCompareEdge (const pair_t* aPtr, const pair_t* bPtr)
{
    edge_t* aEdgePtr = (edge_t*)(aPtr->firstPtr);
    edge_t* bEdgePtr = (edge_t*)(bPtr->firstPtr);

    return compareEdge(aEdgePtr, bEdgePtr);
}


/* =============================================================================
 * element_heapCompare
 *
 * For use in heap_t. Consider using minAngle instead of "do not care".
 * =============================================================================
 */
long
element_heapCompare (const void* aPtr, const void* bPtr)
{
    element_t* aElementPtr = (element_t*)aPtr;
    element_t* bElementPtr = (element_t*)bPtr;

    if (aElementPtr->encroachedEdgePtr) {
        if (bElementPtr->encroachedEdgePtr) {
            return 0; /* do not care */
        } else {
            return 1;
        }
    }

    if (bElementPtr->encroachedEdgePtr) {
        return -1;
    }

    return 0; /* do not care */
}


/* =============================================================================
 * element_isInCircumCircle
 * =============================================================================
 */
bool_t
element_isInCircumCircle (element_t* elementPtr, coordinate_t* coordinatePtr)
{
    double distance = coordinate_distance(coordinatePtr,
                                          &elementPtr->circumCenter);

    return ((distance <= elementPtr->circumRadius) ? TRUE : FALSE);
}


/* =============================================================================
 * isEncroached
 * =============================================================================
 */
static bool_t
isEncroached (element_t* elementPtr)
{
    return ((elementPtr->encroachedEdgePtr != NULL) ? TRUE : FALSE);
}


/* =============================================================================
 * element_setEncroached
 * =============================================================================
 */
void
element_clearEncroached (element_t* elementPtr)
{
    elementPtr->encroachedEdgePtr = NULL;
}


/* =============================================================================
 * element_getEncroachedPtr
 * =============================================================================
 */
edge_t*
element_getEncroachedPtr (element_t* elementPtr)
{
    return elementPtr->encroachedEdgePtr;
}


/* =============================================================================
 * element_isSkinny
 * =============================================================================
 */
bool_t
element_isSkinny (element_t* elementPtr)
{
    return ((elementPtr->isSkinny) ? TRUE : FALSE);
}


/* =============================================================================
 * element_isBad
 * -- Does it need to be refined?
 * =============================================================================
 */
bool_t
element_isBad (element_t* elementPtr)
{
    return ((isEncroached(elementPtr) || element_isSkinny(elementPtr)) ?
            TRUE : FALSE);
}


/* =============================================================================
 * element_isReferenced
 * -- Held by another data structure?
 * =============================================================================
 */
bool_t
element_isReferenced (element_t* elementPtr)
{
    return elementPtr->isReferenced;
}


/* =============================================================================
 * TMelement_isReferenced
 * -- Held by another data structure?
 * =============================================================================
 */
bool_t
TMelement_isReferenced (TM_ARGDECL  element_t* elementPtr)
{
    return (bool_t)TM_SHARED_READ(elementPtr->isReferenced);
}


/* =============================================================================
 * element_setIsReferenced
 * =============================================================================
 */
void
element_setIsReferenced (element_t* elementPtr, bool_t status)
{
    elementPtr->isReferenced = status;
}


/* =============================================================================
 * TMelement_setIsReferenced
 * =============================================================================
 */
void
TMelement_setIsReferenced (TM_ARGDECL  element_t* elementPtr, bool_t status)
{
    TM_SHARED_WRITE(elementPtr->isReferenced, status);
}


/* =============================================================================
 * element_isGarbage
 * -- Can we deallocate?
 * =============================================================================
 */
bool_t
element_isGarbage (element_t* elementPtr)
{
    return elementPtr->isGarbage;
}


/* =============================================================================
 * TMelement_isGarbage
 * -- Can we deallocate?
 * =============================================================================
 */
bool_t
TMelement_isGarbage (TM_ARGDECL  element_t* elementPtr)
{
    return (bool_t)TM_SHARED_READ(elementPtr->isGarbage);
}


/* =============================================================================
 * element_setIsGarbage
 * =============================================================================
 */
void
element_setIsGarbage (element_t* elementPtr, bool_t status)
{
    elementPtr->isGarbage = status;
}


/* =============================================================================
 * TMelement_setIsGarbage
 * =============================================================================
 */
void
TMelement_setIsGarbage (TM_ARGDECL  element_t* elementPtr, bool_t status)
{
    TM_SHARED_WRITE(elementPtr->isGarbage, status);
}


/* =============================================================================
 * element_addNeighbor
 * =============================================================================
 */
void
element_addNeighbor (element_t* elementPtr, element_t* neighborPtr)
{
    list_insert(elementPtr->neighborListPtr, (void*)neighborPtr);
    assert(list_getSize(elementPtr->neighborListPtr) <= elementPtr->numEdge);
}


/* =============================================================================
 * TMelement_addNeighbor
 * =============================================================================
 */
void
TMelement_addNeighbor (TM_ARGDECL  element_t* elementPtr, element_t* neighborPtr)
{
    TMLIST_INSERT(elementPtr->neighborListPtr, (void*)neighborPtr);
}


/* =============================================================================
 * element_getNeighborListPtr
 * =============================================================================
 */
list_t*
element_getNeighborListPtr (element_t* elementPtr)
{
    return elementPtr->neighborListPtr;
}


/* =============================================================================
 * element_getCommonEdge
 *
 * Returns pointer to aElementPtr's shared edge
 * =============================================================================
 */
edge_t*
element_getCommonEdge (element_t* aElementPtr, element_t* bElementPtr)
{
    edge_t* aEdges = aElementPtr->edges;
    edge_t* bEdges = bElementPtr->edges;
    long aNumEdge = aElementPtr->numEdge;
    long bNumEdge = bElementPtr->numEdge;
    long a;
    long b;

    for (a = 0; a < aNumEdge; a++) {
        edge_t* aEdgePtr = &aEdges[a];
        for (b = 0; b < bNumEdge; b++) {
            edge_t* bEdgePtr = &bEdges[b];
            if (compareEdge(aEdgePtr, bEdgePtr) == 0) {
                return aEdgePtr;
            }
        }
    }

    return NULL;
}


/* =============================================================================
 * element_getNewPoint
 * -- Either the element is encroached or is skinny, so get the new point to add
 * =============================================================================
 */
coordinate_t
element_getNewPoint (element_t* elementPtr)
{
    edge_t* encroachedEdgePtr = elementPtr->encroachedEdgePtr;

    if (encroachedEdgePtr) {
        long e;
        long numEdge = elementPtr->numEdge;
        edge_t* edges = elementPtr->edges;
        for (e = 0; e < numEdge; e++) {
            if (compareEdge(encroachedEdgePtr, &edges[e]) == 0) {
                return elementPtr->midpoints[e];
            }
        }
        assert(0);
    }

    return elementPtr->circumCenter;
}


/* =============================================================================
 * element_checkAngles
 *
 * Return FALSE if minimum angle constraint not met
 * =============================================================================
 */
bool_t
element_checkAngles (element_t* elementPtr)
{
    long numCoordinate = elementPtr->numCoordinate;
    double angleConstraint = global_angleConstraint;

    if (numCoordinate == 3) {
        long i;
        coordinate_t* coordinates = elementPtr->coordinates;
        for (i = 0; i < 3; i++) {
            double angle = coordinate_angle(&coordinates[i],
                                            &coordinates[(i + 1) % 3],
                                            &coordinates[(i + 2) % 3]);
            if (angle < angleConstraint) {
                return FALSE;
            }
        }
    }

    return TRUE;
}


/* =============================================================================
 * element_print
 * =============================================================================
 */
void
element_print (element_t* elementPtr)
{
    long c;
    long numCoordinate = elementPtr->numCoordinate;
    coordinate_t* coordinates = elementPtr->coordinates;

    for (c = 0; c < numCoordinate; c++) {
        coordinate_print(&coordinates[c]);
        printf(" ");
    }
}


/* =============================================================================
 * element_printEdge
 * =============================================================================
 */
void
element_printEdge (edge_t* edgePtr)
{
    coordinate_print((coordinate_t*)edgePtr->firstPtr);
    printf(" -> ");
    coordinate_print((coordinate_t*)edgePtr->secondPtr);
}


/* =============================================================================
 * element_printAngles
 * =============================================================================
 */
void
element_printAngles (element_t* elementPtr)
{
    long numCoordinate = elementPtr->numCoordinate;

    if (numCoordinate == 3) {
        long i;
        coordinate_t* coordinates = elementPtr->coordinates;
        for (i = 0; i < 3; i++) {
            double angle = coordinate_angle(&coordinates[i],
                                            &coordinates[(i + 1) % 3],
                                            &coordinates[(i + 2) % 3]);
            printf("%0.3lf ", angle);
        }
    }

}


#ifdef TEST_ELEMENT
/* =============================================================================
 * TEST_ELEMENT
 * =============================================================================
 */


#include <assert.h>
#include <stdio.h>


static void
printElement (element_t* elementPtr)
{
    long numCoordinate = elementPtr->numCoordinate;
    coordinate_t* coordinates = elementPtr->coordinates;
    long i;

    printf("%li: ", elementPtr->numCoordinate);
    for (i = 0; i < numCoordinate; i++) {
        printf("(%.2lf, %.2lf) ", coordinates[i].x, coordinates[i].y);
    }
    printf("| (%.2lf, %.2lf)",
            elementPtr->circumCenter.x, elementPtr->circumCenter.y);
    printf(" | isBad = %li", (long)element_isBad(elementPtr));
    puts("");
}


int
main (int argc, char* argv[])
{
    element_t* elementPtr;
    coordinate_t coordinates[4];

    coordinates[0].x = 1;
    coordinates[0].y = 0;
    coordinates[1].x = 0;
    coordinates[1].y = 1;
    coordinates[2].x = 0;
    coordinates[2].y = 0;
    coordinates[3].x = -2;
    coordinates[3].y = -2;

    elementPtr = element_alloc(coordinates, 3);
    assert(elementPtr);
    printElement(elementPtr);
    element_free(elementPtr);

    elementPtr = element_alloc(coordinates, 2);
    assert(elementPtr);
    printElement(elementPtr);
    element_free(elementPtr);

    elementPtr = element_alloc(coordinates+1, 3);
    assert(elementPtr);
    printElement(elementPtr);
    element_free(elementPtr);

    return 0;
}


#endif /* TEST_ELEMENT */


/* =============================================================================
 *
 * End of element.c
 *
 * =============================================================================
 */
