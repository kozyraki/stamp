/* =============================================================================
 *
 * findSubGraphs.c
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
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include "createPartition.h"
#include "defs.h"
#include "findSubGraphs.h"
#include "globals.h"
#include "thread.h"
#include "tm.h"


/* =============================================================================
 * findSubGraphs0
 * =============================================================================
 */
void
findSubGraphs0 (void* argPtr)
{
    graph* GPtr                = ((findSubGraphs0_arg_t*)argPtr)->GPtr;
    V*     intWtVList          = ((findSubGraphs0_arg_t*)argPtr)->intWtVList;
    V*     strWtVList          = ((findSubGraphs0_arg_t*)argPtr)->strWtVList;
    edge*  maxIntWtList        = ((findSubGraphs0_arg_t*)argPtr)->maxIntWtList;
    long   maxIntWtListSize    = ((findSubGraphs0_arg_t*)argPtr)->maxIntWtListSize;
    edge*  soughtStrWtList     = ((findSubGraphs0_arg_t*)argPtr)->soughtStrWtList;
    long   soughtStrWtListSize = ((findSubGraphs0_arg_t*)argPtr)->soughtStrWtListSize;

    long myId = thread_getId();
    long numThread = thread_getNumThread();

    long i;
    long i_start;
    long i_stop;

    createPartition(0,
                    (maxIntWtListSize + soughtStrWtListSize),
                    myId,
                    numThread,
                    &i_start,
                    &i_stop);

    for (i = i_start; i < i_stop; i++) {
        if (i < maxIntWtListSize) {
            long j;
            for (j = 0; j < GPtr->numVertices; j++) {
                intWtVList[i*(GPtr->numVertices)+j].num = 0;
                intWtVList[i*(GPtr->numVertices)+j].depth = 0;
            }
        } else {
            long t = i - maxIntWtListSize;
            long j;
            for (j = 0; j < GPtr->numVertices; j++) {
                strWtVList[t*(GPtr->numVertices)+j].num = 0;
                strWtVList[t*(GPtr->numVertices)+j].depth = 0;
            }
        }
    }

    thread_barrier_wait();

    LONGINT_T* visited = (LONGINT_T*)P_MALLOC(GPtr->numVertices * sizeof(LONGINT_T));
    assert(visited);

    /*
     * Each thread runs a BFS from endvertex of maxIntWtList edgeList
     */


    for (i = i_start; i < i_stop; i++) {

        long k;
        for (k = 0; k < GPtr->numVertices; k++) {
            visited[k] = 0;
        }

        if (i < maxIntWtListSize) {

            intWtVList[i*(GPtr->numVertices)+0].num = maxIntWtList[i].startVertex;
            intWtVList[i*(GPtr->numVertices)+0].depth = -1;

            intWtVList[i*(GPtr->numVertices)+1].num = maxIntWtList[i].endVertex;
            intWtVList[i*(GPtr->numVertices)+1].depth = 1;

            visited[(intWtVList[i*(GPtr->numVertices)+0]).num] = 1;
            visited[(intWtVList[i*(GPtr->numVertices)+1]).num] = 1;

            long depth = 1;
            long verticesVisited = 2;
            long currIndex = 1;

            while ((depth < SUBGR_EDGE_LENGTH) ||
                   (verticesVisited == GPtr->numVertices))
            {
                long intWtListIndex = i*(GPtr->numVertices)+currIndex;
                depth = intWtVList[intWtListIndex].depth + 1;
                long j;
                long j_start = GPtr->outVertexIndex[intWtVList[intWtListIndex].num];
                long j_stop = j_start + GPtr->outDegree[intWtVList[intWtListIndex].num];
                for (j = j_start; j < j_stop; j++) {
                    if (visited[GPtr->outVertexList[j]] == 0) {
                        visited[GPtr->outVertexList[j]] = 1;
                        intWtVList[i*(GPtr->numVertices)+verticesVisited].num =
                            GPtr->outVertexList[j];
                        intWtVList[i*(GPtr->numVertices)+verticesVisited].depth =
                            depth;
                        verticesVisited++;
                    }
                }
                if ((currIndex < verticesVisited - 1) &&
                    (verticesVisited < GPtr->numVertices))
                {
                    currIndex++;
                    depth = intWtVList[i*(GPtr->numVertices)+currIndex].depth;
                } else {
                    break;
                }
            }

        } else {

            long t = i - maxIntWtListSize;

            strWtVList[t*(GPtr->numVertices)+0].num =
                (soughtStrWtList[t]).startVertex;
            strWtVList[t*(GPtr->numVertices)+0].depth = -1;

            strWtVList[t*(GPtr->numVertices)+1].num =
                (soughtStrWtList[t]).endVertex;
            strWtVList[t*(GPtr->numVertices)+1].depth = 1;

            visited[(strWtVList[t*(GPtr->numVertices)+0]).num] = 1;
            visited[(strWtVList[t*(GPtr->numVertices)+1]).num] = 1;

            long depth = 1;
            long verticesVisited = 2;
            long currIndex = 1;

            while ((depth < SUBGR_EDGE_LENGTH) ||
                   (verticesVisited == GPtr->numVertices))
            {
                long strWtVListIndex = t*(GPtr->numVertices)+currIndex;
                depth = strWtVList[strWtVListIndex].depth + 1;
                long j;
                long j_start = GPtr->outVertexIndex[strWtVList[strWtVListIndex].num];
                long j_stop = j_start + GPtr->outDegree[strWtVList[strWtVListIndex].num];
                for (j = j_start; j < j_stop; j++) {
                    if (visited[GPtr->outVertexList[j]] == 0) {
                        visited[GPtr->outVertexList[j]] = 1;
                        strWtVList[t*(GPtr->numVertices)+verticesVisited].num =
                            GPtr->outVertexList[j];
                        strWtVList[t*(GPtr->numVertices)+verticesVisited].depth =
                            depth;
                        verticesVisited++;
                    }
                }
                if (currIndex < verticesVisited - 1) {
                    currIndex++;
                    depth = strWtVList[t*(GPtr->numVertices)+currIndex].depth;
                } else {
                    break;
                }
            }

        }

    }

    P_FREE(visited);
}


/* =============================================================================
 * findSubGraphs1
 * =============================================================================
 */
void
findSubGraphs1 (void* argPtr)
{
    graph* GPtr                = ((findSubGraphs1_arg_t*)argPtr)->GPtr;
    Vl**   intWtVLList         = ((findSubGraphs1_arg_t*)argPtr)->intWtVLList;
    Vl**   strWtVLList         = ((findSubGraphs1_arg_t*)argPtr)->strWtVLList;
    edge*  maxIntWtList        = ((findSubGraphs1_arg_t*)argPtr)->maxIntWtList;
    long   maxIntWtListSize    = ((findSubGraphs1_arg_t*)argPtr)->maxIntWtListSize;
    edge*  soughtStrWtList     = ((findSubGraphs1_arg_t*)argPtr)->soughtStrWtList;
    long   soughtStrWtListSize = ((findSubGraphs1_arg_t*)argPtr)->soughtStrWtListSize;

    long myId = thread_getId();
    long numThread = thread_getNumThread();

    char* visited = (char*)P_MALLOC(GPtr->numVertices * sizeof(char));
    assert(visited);

    long i;
    long i_start;
    long i_stop;
    createPartition(0,
                    (maxIntWtListSize + soughtStrWtListSize),
                    myId,
                    numThread,
                    &i_start,
                    &i_stop);

    for (i = i_start; i < i_stop; i++) {

        long k;
        for (k = 0; k < GPtr->numVertices; k++)  {
            visited[k] = 'u';
        }

        if (i < maxIntWtListSize) {

            intWtVLList[i] = (Vl*)P_MALLOC(sizeof(Vl));
            assert(intWtVLList[i]);
            (intWtVLList[i])->num = maxIntWtList[i].startVertex;
            (intWtVLList[i])->depth = 0;
            visited[(intWtVLList[i])->num] = 'v';

            (intWtVLList[i])->next = (Vl*)P_MALLOC(sizeof(Vl));
            P_FREE((intWtVLList[i])->next);
            ((intWtVLList[i])->next)->num = maxIntWtList[i].endVertex;
            ((intWtVLList[i])->next)->depth = 1;
            visited[((intWtVLList[i])->next)->num] = 'v';

            Vl* currV = (intWtVLList[i])->next;
            Vl* startV = (intWtVLList[i])->next;

            long depth = 1;
            long verticesVisited = 2;
            long currIndex = 1;

            while ((startV->depth < SUBGR_EDGE_LENGTH) ||
                   (verticesVisited == GPtr->numVertices))
            {
                depth = startV->depth + 1;
                long j;
                long j_start = GPtr->outVertexIndex[startV->num];
                long j_stop = j_start + GPtr->outDegree[startV->num];
                for (j = j_start; j < j_stop; j++) {
                    if (visited[GPtr->outVertexList[j]] == 'u') {
                        visited[GPtr->outVertexList[j]] = 'v';
                        currV->next = (Vl*)P_MALLOC(sizeof(Vl));
                        assert(currV->next);
                        (currV->next)->num = GPtr->outVertexList[j];
                        (currV->next)->depth = depth;
                        verticesVisited++;
                        currV = currV->next;
                    }
                }

                if ((currIndex < verticesVisited - 1) && (verticesVisited < GPtr->numVertices)) {
                currIndex++;
                startV = startV->next;

                } else {
                break;
                }
            }
            currV->next = NULL;

        } else {

            long t = i - maxIntWtListSize;
            strWtVLList[t] = (Vl*)P_MALLOC(sizeof(Vl));
            assert(strWtVLList[t]);
            (strWtVLList[t])->num = soughtStrWtList[t].startVertex;
            (strWtVLList[t])->depth = 0;
            visited[(strWtVLList[t])->num] = 'v';

            (strWtVLList[t])->next = (Vl*)P_MALLOC(sizeof(Vl));
            assert((strWtVLList[t])->next);
            ((strWtVLList[t])->next)->num = soughtStrWtList[t].endVertex;
            ((strWtVLList[t])->next)->depth = 1;
            visited[((strWtVLList[t])->next)->num] = 'v';

            Vl* currV = (strWtVLList[t])->next;
            Vl* startV = (strWtVLList[t])->next;

            long depth = 1;
            long verticesVisited = 2;
            long currIndex = 1;

            while ((startV->depth < SUBGR_EDGE_LENGTH) ||
                   (verticesVisited == GPtr->numVertices))
            {
                depth = startV->depth + 1;

                long j;
                long j_start = GPtr->outVertexIndex[startV->num];
                long j_stop = j_start + GPtr->outDegree[startV->num];
                for (j = j_start; j < j_stop; j++) {
                    if (visited[GPtr->outVertexList[j]] == 'u') {
                        visited[GPtr->outVertexList[j]] = 'v';
                        currV->next = (Vl*)P_MALLOC(sizeof(Vl));
                        assert(currV->next);
                        (currV->next)->num = GPtr->outVertexList[j];
                        (currV->next)->depth = depth;
                        verticesVisited++;
                        currV = currV->next;
                    }
                }

                if ((currIndex < verticesVisited - 1) &&
                    (verticesVisited < GPtr->numVertices))
                {
                    currIndex++;
                    startV = startV->next;
                } else {
                    break;
                }
            }

            currV->next = NULL;

        }

    } /* for i */

    P_FREE(visited);
}


/* =============================================================================
 * findSubGraphs2
 * =============================================================================
 */
void
findSubGraphs2 (void* argPtr)
{
    graph* GPtr                = ((findSubGraphs2_arg_t*)argPtr)->GPtr;
    Vd*    intWtVDList         = ((findSubGraphs2_arg_t*)argPtr)->intWtVDList;
    Vd*    strWtVDList         = ((findSubGraphs2_arg_t*)argPtr)->strWtVDList;
    edge*  maxIntWtList        = ((findSubGraphs2_arg_t*)argPtr)->maxIntWtList;
    long   maxIntWtListSize    = ((findSubGraphs2_arg_t*)argPtr)->maxIntWtListSize;
    edge*  soughtStrWtList     = ((findSubGraphs2_arg_t*)argPtr)->soughtStrWtList;
    long   soughtStrWtListSize = ((findSubGraphs2_arg_t*)argPtr)->soughtStrWtListSize;

    long myId = thread_getId();
    long numThread = thread_getNumThread();


    long numSubArray = 30;
    long arraySize = 5*MAX_CLUSTER_SIZE;

    long i;
    long i_start;
    long i_stop;
    createPartition(0,
                    (maxIntWtListSize + soughtStrWtListSize),
                    myId,
                    numThread,
                    &i_start,
                    &i_stop);

    for (i = i_start; i < i_stop; i++) {
        if (i < maxIntWtListSize) {
            /* Initialize the DS and create one sub-array */
            intWtVDList[i].numArrays = 1;
            intWtVDList[i].arraySize =
                (ULONGINT_T*)P_MALLOC(numSubArray * sizeof(ULONGINT_T));
            assert(intWtVDList[i].arraySize);
            intWtVDList[i].arraySize[0] = 0;
            intWtVDList[i].vList = (V**)P_MALLOC(numSubArray*sizeof(V *));
            assert(intWtVDList[i].vList);
            intWtVDList[i].vList[0] = (V*)P_MALLOC(arraySize*sizeof(V));
            assert(intWtVDList[i].vList[0]);
            long j;
            for (j = 0; j < arraySize; j++) {
                intWtVDList[i].vList[0][j].num = 0;
                intWtVDList[i].vList[0][j].depth = 0;
            }
        } else {
            long t = i - maxIntWtListSize;
            strWtVDList[t].numArrays = 1;
            strWtVDList[t].arraySize =
                (ULONGINT_T*)P_MALLOC(numSubArray * sizeof(ULONGINT_T));
            assert(strWtVDList[t].arraySize);
            strWtVDList[t].arraySize[0] = 0;
            strWtVDList[t].vList = (V**)P_MALLOC(numSubArray * sizeof(V*));
            assert(strWtVDList[t].vList);
            strWtVDList[t].vList[0] = (V*)P_MALLOC(arraySize * sizeof(V));
            assert(strWtVDList[t].vList[0]);
            long j;
            for (j = 0; j < arraySize; j++) {
                strWtVDList[t].vList[0][j].num = 0;
                strWtVDList[t].vList[0][j].depth = 0;
            }
        }
    }

    thread_barrier_wait();

    char* visited = (char*)P_MALLOC(GPtr->numVertices * sizeof(char));
    assert(visited);

    /*
     * Each thread runs a BFS from endvertex of maxIntWtList edgeList
     */

    for (i = i_start; i < i_stop; i++) {

        long k;
        for (k = 0; k < GPtr->numVertices; k++) {
            visited[k] = 'u';
        }

        if (i < maxIntWtListSize) {

            intWtVDList[i].vList[0][0].num = maxIntWtList[i].startVertex;
            intWtVDList[i].vList[0][0].depth = -1;

            intWtVDList[i].vList[0][1].num = maxIntWtList[i].endVertex;
            intWtVDList[i].vList[0][1].depth = 1;

            intWtVDList[i].arraySize[0] = 2;

            visited[intWtVDList[i].vList[0][0].num] = 'v';
            visited[intWtVDList[i].vList[0][1].num] = 'v';

            long depth = 1;
            long verticesVisited = 2;
            long currIndex = 1;

            while ((depth < SUBGR_EDGE_LENGTH) ||
                   (verticesVisited == GPtr->numVertices))
            {
                long currVListX = currIndex / arraySize;
                long currVListY = currIndex % arraySize;
                V* currV = &intWtVDList[i].vList[currVListX][currVListY];
                long vNum = currV->num;
                depth = currV->depth + 1;
                long j;
                long j_start = GPtr->outVertexIndex[vNum];
                long j_stop = j_start + GPtr->outDegree[vNum];
                for (j = j_start; j < j_stop; j++) {
                    if (visited[GPtr->outVertexList[j]] == 'u') {
                        visited[GPtr->outVertexList[j]] = 'v';
                        long vListX = verticesVisited/arraySize;
                        long vListY = verticesVisited % arraySize;
                        V* v = &intWtVDList[i].vList[vListX][vListY];
                        v->num = GPtr->outVertexList[j];
                        v->depth = depth;
                        intWtVDList[i].arraySize[vListX]++;
                        verticesVisited++;
                    }
                }

                /* Check if we need to create a new array */
                if (((float) verticesVisited / (float) arraySize) > 0.5) {
                    /* create a new sub-array */
                    if (intWtVDList[i].numArrays !=
                        (verticesVisited/arraySize + 2))
                    {
                        intWtVDList[i].numArrays++;
                        intWtVDList[i].vList[intWtVDList[i].numArrays-1] =
                            (V*)P_MALLOC(arraySize * sizeof(V));
                        assert(intWtVDList[i].vList[intWtVDList[i].numArrays-1]);
                        intWtVDList[i].arraySize[intWtVDList[i].numArrays-1] = 0;
                    }
                }

                if ((currIndex < verticesVisited - 1) &&
                    (verticesVisited < GPtr->numVertices))
                {
                    currIndex++;
                    long vListX = currIndex / arraySize;
                    long vListY = currIndex % arraySize;
                    depth = intWtVDList[i].vList[vListX][vListY].depth;
                } else {
                    break;
                }
            }

        } else {

            long t = i - maxIntWtListSize;

            strWtVDList[t].vList[0][0].num = soughtStrWtList[t].startVertex;
            strWtVDList[t].vList[0][0].depth = -1;

            strWtVDList[t].vList[0][1].num = soughtStrWtList[t].endVertex;
            strWtVDList[t].vList[0][1].depth = 1;

            strWtVDList[t].arraySize[0] = 2;

            visited[strWtVDList[t].vList[0][0].num] = 'v';
            visited[strWtVDList[t].vList[0][1].num] = 'v';

            long depth = 1;
            long verticesVisited = 2;
            long currIndex = 1;

            while ((depth < SUBGR_EDGE_LENGTH) ||
                   (verticesVisited == GPtr->numVertices))
            {
                long currVListX = currIndex / arraySize;
                long currVListY = currIndex % arraySize;
                V* currV = &strWtVDList[t].vList[currVListX][currVListY];
                long vNum = currV->num;
                depth = currV->depth + 1;
                long j;
                long j_start = GPtr->outVertexIndex[vNum];
                long j_stop = j_start + GPtr->outDegree[vNum];
                for (j = j_start; j < j_stop; j++) {
                    if (visited[GPtr->outVertexList[j]] == 'u') {
                        visited[GPtr->outVertexList[j]] = 'v';
                        long vListX = verticesVisited / arraySize;
                        long vListY = verticesVisited % arraySize;
                        V* v = &strWtVDList[t].vList[vListX][vListY];
                        v->num = GPtr->outVertexList[j];
                        v->depth = depth;
                        strWtVDList[t].arraySize[vListX]++;
                        verticesVisited++;
                    }
                }

                /* Check if we need to create a new array */
                if (((float)verticesVisited /(float) arraySize) > 0.5) {
                    /* create a new sub-array */
                    if (strWtVDList[t].numArrays !=
                        (verticesVisited/arraySize + 2))
                    {
                        strWtVDList[t].numArrays++;
                        strWtVDList[t].vList[strWtVDList[t].numArrays-1] =
                            (V*)P_MALLOC(arraySize * sizeof(V));
                        assert(strWtVDList[t].vList[strWtVDList[t].numArrays-1]);
                        strWtVDList[t].arraySize[strWtVDList[t].numArrays-1] = 0;
                    }
                }

                if ((currIndex < verticesVisited - 1) &&
                    (verticesVisited < GPtr->numVertices))
                {
                    currIndex++;
                    long currVListX = currIndex / arraySize;
                    long currVListY = currIndex % arraySize;
                    depth = strWtVDList[t].vList[currVListX][currVListY].depth;
                } else {
                    break;
                }
            }

        }

    } /* for i */

    P_FREE(visited);
}


/* =============================================================================
 *
 * End of findSubGraphs.c
 *
 * =============================================================================
 */
