/* =============================================================================
 *
 * maze.c
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
#include <stdlib.h>
#include "coordinate.h"
#include "grid.h"
#include "list.h"
#include "maze.h"
#include "queue.h"
#include "pair.h"
#include "types.h"
#include "vector.h"


/* =============================================================================
 * maze_alloc
 * =============================================================================
 */
maze_t*
maze_alloc ()
{
    maze_t* mazePtr;

    mazePtr = (maze_t*)malloc(sizeof(maze_t));
    if (mazePtr) {
        mazePtr->gridPtr = NULL;
        mazePtr->workQueuePtr = queue_alloc(1024);
        mazePtr->wallVectorPtr = vector_alloc(1);
        mazePtr->srcVectorPtr = vector_alloc(1);
        mazePtr->dstVectorPtr = vector_alloc(1);
        assert(mazePtr->workQueuePtr &&
               mazePtr->wallVectorPtr &&
               mazePtr->srcVectorPtr &&
               mazePtr->dstVectorPtr);
    }

    return mazePtr;
}

/* =============================================================================
 * maze_free
 * =============================================================================
 */
void
maze_free (maze_t* mazePtr)
{
    if (mazePtr->gridPtr != NULL) {
        grid_free(mazePtr->gridPtr);
    }
    queue_free(mazePtr->workQueuePtr);
    vector_free(mazePtr->wallVectorPtr);
    vector_free(mazePtr->srcVectorPtr);
    vector_free(mazePtr->dstVectorPtr);
    free(mazePtr);
}


/* =============================================================================
 * addToGrid
 * =============================================================================
 */
static void
addToGrid (grid_t* gridPtr, vector_t* vectorPtr, char* type)
{
    long i;
    long n = vector_getSize(vectorPtr);
    for (i = 0; i < n; i++) {
        coordinate_t* coordinatePtr = (coordinate_t*)vector_at(vectorPtr, i);
        if (!grid_isPointValid(gridPtr,
                               coordinatePtr->x,
                               coordinatePtr->y,
                               coordinatePtr->z))
        {
            fprintf(stderr, "Error: %s (%li, %li, %li) invalid\n",
                    type, coordinatePtr->x, coordinatePtr->y, coordinatePtr->z);
            exit(1);
        }
    }
    grid_addPath(gridPtr, vectorPtr);
}


/* =============================================================================
 * maze_read
 * -- Return number of path to route
 * =============================================================================
 */
long
maze_read (maze_t* mazePtr, char* inputFileName)
{
    FILE* inputFile = fopen(inputFileName, "rt");
    if (!inputFile) {
        fprintf(stderr, "Error: Could not read %s\n", inputFileName);
        exit(1);
    }

    /*
     * Parse input file
     */
    long lineNumber = 0;
    long height = -1;
    long width  = -1;
    long depth  = -1;
    char line[256];
    list_t* workListPtr = list_alloc(&coordinate_comparePair);
    vector_t* wallVectorPtr = mazePtr->wallVectorPtr;
    vector_t* srcVectorPtr = mazePtr->srcVectorPtr;
    vector_t* dstVectorPtr = mazePtr->dstVectorPtr;

    while (fgets(line, sizeof(line), inputFile)) {

        char code;
        long x1, y1, z1;
        long x2, y2, z2;
        long numToken = sscanf(line, " %c %li %li %li %li %li %li",
                              &code, &x1, &y1, &z1, &x2, &y2, &z2);

        lineNumber++;

        if (numToken < 1) {
            continue;
        }

        switch (code) {
            case '#': { /* comment */
                /* ignore line */
                break;
            }
            case 'd': { /* dimensions (format: d x y z) */
                if (numToken != 4) {
                    goto PARSE_ERROR;
                }
                width  = x1;
                height = y1;
                depth  = z1;
                if (width < 1 || height < 1 || depth < 1) {
                    goto PARSE_ERROR;
                }
                break;
            }
            case 'p': { /* paths (format: p x1 y1 z1 x2 y2 z2) */
                if (numToken != 7) {
                    goto PARSE_ERROR;
                }
                coordinate_t* srcPtr = coordinate_alloc(x1, y1, z1);
                coordinate_t* dstPtr = coordinate_alloc(x2, y2, z2);
                assert(srcPtr);
                assert(dstPtr);
                if (coordinate_isEqual(srcPtr, dstPtr)) {
                    goto PARSE_ERROR;
                }
                pair_t* coordinatePairPtr = pair_alloc(srcPtr, dstPtr);
                assert(coordinatePairPtr);
                bool_t status = list_insert(workListPtr, (void*)coordinatePairPtr);
                assert(status == TRUE);
                vector_pushBack(srcVectorPtr, (void*)srcPtr);
                vector_pushBack(dstVectorPtr, (void*)dstPtr);
                break;
            }
            case 'w': { /* walls (format: w x y z) */
                if (numToken != 4) {
                    goto PARSE_ERROR;
                }
                coordinate_t* wallPtr = coordinate_alloc(x1, y1, z1);
                vector_pushBack(wallVectorPtr, (void*)wallPtr);
                break;
            }
            PARSE_ERROR:
            default: { /* error */
                fprintf(stderr, "Error: line %li of %s invalid\n",
                        lineNumber, inputFileName);
                exit(1);
            }
        }

    } /* iterate over lines in input file */

    /*
     * Initialize grid contents
     */
    if (width < 1 || height < 1 || depth < 1) {
        fprintf(stderr, "Error: Invalid dimensions (%li, %li, %li)\n",
                width, height, depth);
        exit(1);
    }
    grid_t* gridPtr = grid_alloc(width, height, depth);
    assert(gridPtr);
    mazePtr->gridPtr = gridPtr;
    addToGrid(gridPtr, wallVectorPtr, "wall");
    addToGrid(gridPtr, srcVectorPtr,  "source");
    addToGrid(gridPtr, dstVectorPtr,  "destination");
    printf("Maze dimensions = %li x %li x %li\n", width, height, depth);
    printf("Paths to route  = %li\n", list_getSize(workListPtr));

    /*
     * Initialize work queue
     */
    queue_t* workQueuePtr = mazePtr->workQueuePtr;
    list_iter_t it;
    list_iter_reset(&it, workListPtr);
    while (list_iter_hasNext(&it, workListPtr)) {
        pair_t* coordinatePairPtr = (pair_t*)list_iter_next(&it, workListPtr);
        queue_push(workQueuePtr, (void*)coordinatePairPtr);
    }
    list_free(workListPtr);

    return vector_getSize(srcVectorPtr);
}


/* =============================================================================
 * maze_checkPaths
 * =============================================================================
 */
bool_t
maze_checkPaths (maze_t* mazePtr, list_t* pathVectorListPtr, bool_t doPrintPaths)
{
    grid_t* gridPtr = mazePtr->gridPtr;
    long width  = gridPtr->width;
    long height = gridPtr->height;
    long depth  = gridPtr->depth;
    long i;

    /* Mark walls */
    grid_t* testGridPtr = grid_alloc(width, height, depth);
    grid_addPath(testGridPtr, mazePtr->wallVectorPtr);

    /* Mark sources */
    vector_t* srcVectorPtr = mazePtr->srcVectorPtr;
    long numSrc = vector_getSize(srcVectorPtr);
    for (i = 0; i < numSrc; i++) {
        coordinate_t* srcPtr = (coordinate_t*)vector_at(srcVectorPtr, i);
        grid_setPoint(testGridPtr, srcPtr->x, srcPtr->y, srcPtr->z, 0);
    }

    /* Mark destinations */
    vector_t* dstVectorPtr = mazePtr->dstVectorPtr;
    long numDst = vector_getSize(dstVectorPtr);
    for (i = 0; i < numDst; i++) {
        coordinate_t* dstPtr = (coordinate_t*)vector_at(dstVectorPtr, i);
        grid_setPoint(testGridPtr, dstPtr->x, dstPtr->y, dstPtr->z, 0);
    }

    /* Make sure path is contiguous and does not overlap */
    long id = 0;
    list_iter_t it;
    list_iter_reset(&it, pathVectorListPtr);
    while (list_iter_hasNext(&it, pathVectorListPtr)) {
        vector_t* pathVectorPtr = (vector_t*)list_iter_next(&it, pathVectorListPtr);
        long numPath = vector_getSize(pathVectorPtr);
        long i;
        for (i = 0; i < numPath; i++) {
            id++;
            vector_t* pointVectorPtr = (vector_t*)vector_at(pathVectorPtr, i);
            /* Check start */
            long* prevGridPointPtr = (long*)vector_at(pointVectorPtr, 0);
            long x;
            long y;
            long z;
            grid_getPointIndices(gridPtr, prevGridPointPtr, &x, &y, &z);
            if (grid_getPoint(testGridPtr, x, y, z) != 0) {
                grid_free(testGridPtr);
                return FALSE;
            }
            coordinate_t prevCoordinate;
            grid_getPointIndices(gridPtr,
                                 prevGridPointPtr,
                                 &prevCoordinate.x,
                                 &prevCoordinate.y,
                                 &prevCoordinate.z);
            long numPoint = vector_getSize(pointVectorPtr);
            long j;
            for (j = 1; j < (numPoint-1); j++) { /* no need to check endpoints */
                long* currGridPointPtr = (long*)vector_at(pointVectorPtr, j);
                coordinate_t currCoordinate;
                grid_getPointIndices(gridPtr,
                                     currGridPointPtr,
                                     &currCoordinate.x,
                                     &currCoordinate.y,
                                     &currCoordinate.z);
                if (!coordinate_areAdjacent(&currCoordinate, &prevCoordinate)) {
                    grid_free(testGridPtr);
                    return FALSE;
                }
                prevCoordinate = currCoordinate;
                long x = currCoordinate.x;
                long y = currCoordinate.y;
                long z = currCoordinate.z;
                if (grid_getPoint(testGridPtr, x, y, z) != GRID_POINT_EMPTY) {
                    grid_free(testGridPtr);
                    return FALSE;
                } else {
                    grid_setPoint(testGridPtr, x, y, z, id);
                }
            }
            /* Check end */
            long* lastGridPointPtr = (long*)vector_at(pointVectorPtr, j);
            grid_getPointIndices(gridPtr, lastGridPointPtr, &x, &y, &z);
            if (grid_getPoint(testGridPtr, x, y, z) != 0) {
                grid_free(testGridPtr);
                return FALSE;
            }
        } /* iteratate over pathVector */
    } /* iterate over pathVectorList */

    if (doPrintPaths) {
        puts("\nRouted Maze:");
        grid_print(testGridPtr);
    }

    grid_free(testGridPtr);

    return TRUE;
}


/* =============================================================================
 *
 * End of maze.c
 *
 * =============================================================================
 */
