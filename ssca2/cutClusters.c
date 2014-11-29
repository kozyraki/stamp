/* =============================================================================
 *
 * cutClusters.c
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
#include <stdio.h>
#include "alg_radix_smp.h"
#include "createPartition.h"
#include "cutClusters.h"
#include "defs.h"
#include "globals.h"
#include "thread.h"
#include "tm.h"

static ULONGINT_T* global_Index                = NULL;
static ULONGINT_T* global_neighbourArray       = NULL;
static ULONGINT_T* global_IndexSorted          = NULL;
static ULONGINT_T* global_neighbourArraySorted = NULL;
static long*       global_vStatus              = NULL;
static edge*       global_pCutSet              = NULL;
static ULONGINT_T* global_startV               = NULL;
static ULONGINT_T* global_clusterSize          = NULL;
static ULONGINT_T* global_edgeStartCounter     = NULL;
static ULONGINT_T* global_edgeEndCounter       = NULL;
static edge*       global_cutSet               = NULL;

static long       global_iter;
static ULONGINT_T global_cliqueSize = 0;
static ULONGINT_T global_cutSetIndex = 0;


/* =============================================================================
 * cutClusters
 * =============================================================================
 */
void
cutClusters (void* argPtr)
{
    TM_THREAD_ENTER();

    graph* GPtr = (graph*)argPtr;

    long myId = thread_getId();
    long numThread = thread_getNumThread();

    /*
     * Sort the vertex list by their degree
     */

    ULONGINT_T* Index;
    ULONGINT_T* neighbourArray;
    ULONGINT_T* IndexSorted;
    ULONGINT_T* neighbourArraySorted;

    if (myId == 0) {
        long numByte = GPtr->numVertices * sizeof(ULONGINT_T);
        Index = (ULONGINT_T*)P_MALLOC(numByte);
        assert(Index);
        global_Index = Index;
        neighbourArray = (ULONGINT_T*)P_MALLOC(numByte);
        assert(neighbourArray);
        global_neighbourArray = neighbourArray;
        IndexSorted = (ULONGINT_T*)P_MALLOC(numByte);
        assert(IndexSorted);
        global_IndexSorted = IndexSorted;
        neighbourArraySorted = (ULONGINT_T*)P_MALLOC(numByte);
        assert(neighbourArraySorted);
        global_neighbourArraySorted = neighbourArraySorted;
    }

    thread_barrier_wait();

    Index = global_Index;
    neighbourArray = global_neighbourArray;
    IndexSorted = global_IndexSorted;
    neighbourArraySorted = global_neighbourArraySorted;

    long i;
    long i_start;
    long i_stop;
    createPartition(0, GPtr->numVertices, myId, numThread, &i_start, &i_stop);

    for (i = i_start; i < i_stop; i++) {
        neighbourArray[i] = GPtr->inDegree[i] + GPtr->outDegree[i];
        Index[i] = i;
    }

    thread_barrier_wait();


    all_radixsort_node_aux_s3(GPtr->numVertices,
                              neighbourArray,
                              neighbourArraySorted,
                              Index,
                              IndexSorted);

    thread_barrier_wait();

    /*
     * Global array to keep track of vertex status:
     * -1 if a vertex hasn't been assigned to a cluster yet
     * t if it belongs to a cluster; t = iteration*numThread + myId
     */
    long* vStatus;

    edge* pCutSet;
    ULONGINT_T* startV;
    ULONGINT_T* clusterSize;

    if (myId == 0) {

        P_FREE(Index);
        P_FREE(neighbourArray);

        vStatus = (long*)P_MALLOC(GPtr->numVertices * sizeof(long));
        assert(vStatus);
        global_vStatus = vStatus;

        /*
         * Allocate mem. for the cut set list
         * Maintain local arrays initially and merge them in the end
         */

        if (SCALE < 12) {
            pCutSet =(edge*)P_MALLOC((1*(GPtr->numDirectedEdges)/numThread)
                                     * sizeof(edge));
        } else {
            pCutSet = (edge*)P_MALLOC((0.2*(GPtr->numDirectedEdges)/numThread)
                                      * sizeof(edge));
        }
        assert(pCutSet);
        global_pCutSet = pCutSet;

        /*
         * Vertex to start from, on each thread
         */
        startV = (ULONGINT_T*)P_MALLOC(numThread * sizeof(ULONGINT_T));
        assert(startV);
        global_startV = startV;
        clusterSize = (ULONGINT_T*)P_MALLOC(numThread * sizeof(ULONGINT_T));
        assert(clusterSize);
        global_clusterSize = clusterSize;
    }

    thread_barrier_wait();

    vStatus     = global_vStatus;
    pCutSet     = global_pCutSet;
    startV      = global_startV;
    clusterSize = global_clusterSize;

    for (i = i_start; i < i_stop; i++) {
        vStatus[i] = -1;
    }

    thread_barrier_wait();

    ULONGINT_T verticesVisited = 0;

#ifdef WRITE_RESULT_FILES
    FILE* outfp1 = NULL;
    if (myId == 0) {
        outfp1 = fopen("clusters.txt", "w");
        fprintf(outfp1, "\nKernel 4 - Extracted Clusters\n");
    }
#endif

    long iter = 0;
    ULONGINT_T currIndex = 0;
    ULONGINT_T cutSetIndex = 0;

    while (verticesVisited < GPtr->numVertices) {

        /* Clear start vertex array */
        startV[myId] = -1;
        clusterSize[myId] = 0;

        if (currIndex == GPtr->numVertices) {
            currIndex = 0;
        }

        thread_barrier_wait();

        /*
         * Choose vertices to start from
         * Done sequentially right now, can be parallelized
         */
        if (myId == 0) {
            long t;
            for (t = 0; t < numThread; t++) {
                long r;
                for (r = currIndex; r < GPtr->numVertices; r++) {
                    if (vStatus[IndexSorted[GPtr->numVertices - r - 1]] == -1) {
                        startV[t] = IndexSorted[GPtr->numVertices - r - 1];
                        vStatus[startV[t]] = iter * numThread + t;
                        long j;
                        for (j = 0; j < GPtr->outDegree[startV[t]]; j++) {
                            long outVertexListIndex =
                                j+GPtr->outVertexIndex[startV[t]];
                            long vStatusIndex =
                                GPtr->outVertexList[outVertexListIndex];
                            if (vStatus[vStatusIndex] == -1) {
                                vStatus[vStatusIndex] = iter * numThread + t;
                                clusterSize[t]++;
                            }
                        }
                        for (j = 0; j < GPtr->inDegree[startV[t]]; j++) {
                            long inVertexIndex = j+GPtr->inVertexIndex[startV[t]];
                            long vStatusIndex = GPtr->inVertexList[inVertexIndex];
                            if (vStatus[vStatusIndex] == -1) {
                                vStatus[vStatusIndex] = iter * numThread + t;
                                clusterSize[t]++;
                            }
                        }
                        currIndex = r+1;
                        break;
                    }
                }
            }
        }

        thread_barrier_wait();

        /*
         * Determine clusters and cut sets in parallel
         */

        i = startV[myId];

        ULONGINT_T cliqueSize = 0;

        /* If the thread has some vertex to start from */
        if (i != -1)  {

            cliqueSize = 1;

            /* clusterSize[myId] gives the no. of 'unassigned' vertices adjacent to the current vertex */
            if ((clusterSize[myId] >= 0.6*(GPtr->inDegree[i]+GPtr->outDegree[i])) ||
                ((iter > (GPtr->numVertices)/(numThread*MAX_CLUSTER_SIZE)) &&
                 (clusterSize[myId] > 0)))
            {

                /*
                 * Most of the adjacent vertices are unassigned,
                 * should be able to extract a cluster easily
                 */

                /* Inspect adjacency list */
                long j;
                for (j = 0; j < GPtr->outDegree[i]; j++) {

                    ULONGINT_T clusterCounter = 0;
                    ULONGINT_T cutSetIndexPrev = cutSetIndex;
                    ULONGINT_T cutSetCounter = 0;

                    if (vStatus[GPtr->outVertexList[j+GPtr->outVertexIndex[i]]] ==
                        iter * numThread + myId)
                    {

                        long v = GPtr->outVertexList[j+GPtr->outVertexIndex[i]];

                        /*
                         * Inspect vertices adjacent to v and determine if it belongs
                         * to a cluster or not
                         */
                        long k;
                        for (k = 0; k < GPtr->outDegree[v]; k++) {
                            long outVertexListIndex = k+GPtr->outVertexIndex[v];
                            long vStatusIndex = GPtr->outVertexList[outVertexListIndex];
                            if (vStatus[vStatusIndex] == (iter * numThread + myId)) {
                                clusterCounter++;
                            } else {
                                cutSetCounter++;
                                if (vStatus[vStatusIndex] == -1) {
                                    /* Ensure that an edge is not added twice to the list */
                                    pCutSet[cutSetIndex].startVertex = v;
                                    pCutSet[cutSetIndex].endVertex = vStatusIndex;
                                    cutSetIndex++;
                                }
                            }
                        }

                        if ((cutSetCounter >= clusterCounter) ||
                            ((SCALE < 9) &&
                             (clusterCounter <= 2) &&
                             (GPtr->inDegree[v]+GPtr->outDegree[v] >
                              clusterCounter + cutSetCounter) &&
                             (clusterSize[myId] > clusterCounter + 2)) ||
                            ((SCALE > 9) &&
                             (clusterCounter < 0.5*clusterSize[myId])))
                        {

                            /* v doesn't belong to this clique, free it */
                            vStatus[v] = -1;

                            /* Also add this edge to cutset list, removing previously added edges */
                            cutSetIndex = cutSetIndexPrev;
                            pCutSet[cutSetIndex].startVertex = i;
                            pCutSet[cutSetIndex].endVertex = v;
                            cutSetIndex++;

                        } else {

                            cliqueSize++;
                             /* Add edges in inVertexList also to cut Set */
                            for (k = 0; k < GPtr->inDegree[v]; k++) {
                                long inVertexListIndex = k+GPtr->inVertexIndex[v];
                                long vStatusIndex = GPtr->inVertexList[inVertexListIndex];
                                if (vStatus[vStatusIndex] == -1) {
                                    pCutSet[cutSetIndex].startVertex = v;
                                    pCutSet[cutSetIndex].endVertex = vStatusIndex;
                                    cutSetIndex++;
                                }
                            }

                        }

                    }

                }

                /* Do the same for the implied edges too */
                for (j = 0; j < GPtr->inDegree[i]; j++) {

                    ULONGINT_T clusterCounter = 0;
                    ULONGINT_T cutSetIndexPrev = cutSetIndex;
                    ULONGINT_T cutSetCounter = 0;

                    if (vStatus[GPtr->inVertexList[j+GPtr->inVertexIndex[i]]] ==
                        iter*numThread+myId)
                    {
                        long v = GPtr->inVertexList[j+GPtr->inVertexIndex[i]];

                        /* Inspect vertices adjacent to v and determine if it belongs to a cluster or not */
                        long k;
                        for (k = 0; k < GPtr->outDegree[v]; k++) {
                            long outVertexListIndex = k+GPtr->outVertexIndex[v];
                            long vStatusIndex = GPtr->outVertexList[outVertexListIndex];
                            if (vStatus[vStatusIndex] == iter*numThread+myId) {
                                clusterCounter++;
                            } else {
                                cutSetCounter++;
                                if (vStatus[vStatusIndex] == -1) {
                                    /* To ensure that an edge is not added twice to the list */
                                    pCutSet[cutSetIndex].startVertex = v;
                                    pCutSet[cutSetIndex].endVertex = vStatusIndex;
                                    cutSetIndex++;
                                }
                            }
                        }

                        if ((cutSetCounter >= clusterCounter) ||
                            ((SCALE < 9) &&
                             (clusterCounter <= 2) &&
                             (GPtr->inDegree[v]+GPtr->outDegree[v] >
                              clusterCounter + cutSetCounter)  &&
                             (clusterSize[myId] > clusterCounter + 2)) ||
                            ((SCALE > 9) &&
                             (clusterCounter < 0.5*clusterSize[myId])))
                        {
                            /* v doesn't belong to this clique, free it */
                            vStatus[v] = -1;
                            cutSetIndex = cutSetIndexPrev;
                            pCutSet[cutSetIndex].startVertex = i;
                            pCutSet[cutSetIndex].endVertex = v;
                            cutSetIndex++;

                        } else {

                            cliqueSize++;
                            /* Add edges in inVertexList also to cut Set */
                            for (k = 0; k < GPtr->inDegree[v]; k++) {
                                long inVertexListIndex = k+GPtr->inVertexIndex[v];
                                long vStatusIndex = GPtr->inVertexList[inVertexListIndex];
                                if (vStatus[vStatusIndex] == -1) {
                                    pCutSet[cutSetIndex].startVertex = v;
                                    pCutSet[cutSetIndex].endVertex = vStatusIndex;
                                    cutSetIndex++;
                                }
                            }

                        }

                    }

                }

            } /* i != -1 */

            if (clusterSize[myId] == 0)  {

              /* Only one vertex in cluster */
              cliqueSize = 1;

            } else {

                if ((clusterSize[myId] < 0.6*(GPtr->inDegree[i]+GPtr->outDegree[i])) &&
                    (iter <= GPtr->numVertices/(numThread*MAX_CLUSTER_SIZE)))
                {
                    /* High perc. of intra-clique edges, do not commit clique */
                    cliqueSize = 0;
                    vStatus[i] = -1;

                    long j;
                    for (j=0; j<GPtr->outDegree[i]; j++) {
                        long outVertexListIndex = j+GPtr->outVertexIndex[i];
                        long vStatusIndex = GPtr->outVertexList[outVertexListIndex];
                        if (vStatus[vStatusIndex] == iter*numThread+myId) {
                            vStatus[vStatusIndex] = -1;
                        }
                    }

                    for (j=0; j<GPtr->inDegree[i]; j++) {
                        long inVertexListIndex = j+GPtr->inVertexIndex[i];
                        long vStatusIndex = GPtr->inVertexList[inVertexListIndex];
                        if (vStatus[vStatusIndex] == iter*numThread+myId) {
                            vStatus[vStatusIndex] = -1;
                        }
                    }
                }

            }
        } /* if i != -1 */

        if (myId == 0) {
            global_cliqueSize = 0;
        }

        thread_barrier_wait();

#ifdef WRITE_RESULT_FILES
        /* Print to results.clq file */

        if (myId == 0) {
            long t;
            for (t = 0; t < numThread; t++) {
                if (startV[t] != -1) {
                    if (vStatus[startV[t]] == iter*numThread+t) {
                        fprintf(outfp1, "%lu ", startV[t]);
                        long j;
                        for (j = 0; j < GPtr->outDegree[startV[t]]; j++) {
                            long outVertexListIndex = j+GPtr->outVertexIndex[startV[t]];
                            long vStatusIndex = GPtr->outVertexList[outVertexListIndex];
                            if (vStatus[vStatusIndex] == iter*numThread+t) {
                                fprintf(outfp1, "%lu ", vStatusIndex);
                            }
                        }
                        for (j = 0; j < GPtr->inDegree[startV[t]]; j++) {
                            long inVertexListIndex = j+GPtr->inVertexIndex[startV[t]];
                            long vStatusIndex = GPtr->inVertexList[inVertexListIndex];
                            if (vStatus[vStatusIndex] == iter*numThread+t) {
                                fprintf(outfp1, "%lu ", vStatusIndex);
                            }
                        }
                        fprintf(outfp1, "\n");
                    }
                }
            }
        }

        thread_barrier_wait();
#endif /* WRITE_RESULTS_FILE */

        if (myId == 0) {
            iter++;
            global_iter = iter;
        }

        TM_BEGIN();
        long tmp_cliqueSize = (long)TM_SHARED_READ(global_cliqueSize);
        TM_SHARED_WRITE(global_cliqueSize, (tmp_cliqueSize + cliqueSize));
        TM_END();

        thread_barrier_wait();

        iter = global_iter;
        verticesVisited += global_cliqueSize;

        if ((verticesVisited >= 0.95*GPtr->numVertices) ||
            (iter > GPtr->numVertices/2))
        {
            break;
        }

    } /* while (verticesVisited < GPtr->numVertices) */

    thread_barrier_wait();

#ifdef WRITE_RESULT_FILES
    /* Take care of unmarked vertices */
    if (myId == 0) {
        if (verticesVisited < GPtr->numVertices) {
            for(i = 0; i < GPtr->numVertices; i++) {
                if (vStatus[i] == -1) {
                    vStatus[i] = iter*numThread+myId;
                    fprintf(outfp1, "%lu\n", i);
                    iter++;
                }
            }
        }
    }

    thread_barrier_wait();
#endif

    /*
     * Merge partial Cutset Lists
     */

    /* Temp vars for merging edge lists */
    ULONGINT_T* edgeStartCounter;
    ULONGINT_T* edgeEndCounter;

    if (myId == 0) {
        edgeStartCounter = (ULONGINT_T*)P_MALLOC(numThread * sizeof(ULONGINT_T));
        assert(edgeStartCounter);
        global_edgeStartCounter = edgeStartCounter;
        edgeEndCounter = (ULONGINT_T*)P_MALLOC(numThread * sizeof(ULONGINT_T));
        assert(edgeEndCounter);
        global_edgeEndCounter = edgeEndCounter;
    }

    thread_barrier_wait();

    edgeStartCounter = global_edgeStartCounter;
    edgeEndCounter   = global_edgeEndCounter;

    edgeEndCounter[myId] = cutSetIndex;
    edgeStartCounter[myId] = 0;

    thread_barrier_wait();

    if (myId == 0) {
        long t;
        for (t = 1; t < numThread; t++) {
            edgeEndCounter[t] = edgeEndCounter[t-1] + edgeEndCounter[t];
            edgeStartCounter[t] = edgeEndCounter[t-1];
        }
    }

    TM_BEGIN();
    long tmp_cutSetIndex = (long)TM_SHARED_READ(global_cutSetIndex);
    TM_SHARED_WRITE(global_cutSetIndex, (tmp_cutSetIndex + cutSetIndex));
    TM_END();

    thread_barrier_wait();

    cutSetIndex = global_cutSetIndex;
    ULONGINT_T cutSetCounter = cutSetIndex;

    /* Data struct. for storing edgeCut */
    edge* cutSet;

    if (myId == 0) {
        cutSet = (edge*)P_MALLOC(cutSetCounter * sizeof(edge));
        assert(cutSet);
        global_cutSet = cutSet;
    }

    thread_barrier_wait();

    cutSet = global_cutSet;

    long j;
    for (j = edgeStartCounter[myId]; j < edgeEndCounter[myId]; j++) {
        cutSet[j].startVertex = pCutSet[j-edgeStartCounter[myId]].startVertex;
        cutSet[j].endVertex = pCutSet[j-edgeStartCounter[myId]].endVertex;
    }

    thread_barrier_wait();

#ifdef WRITE_RESULT_FILES
    FILE* outfp2 = NULL;
    if (myId == 0) {
        outfp2 = fopen("edgeCut.txt", "w");
        fprintf(outfp2, "\nEdges in Cut Set - \n");
        for (i = 0; i < cutSetCounter; i++) {
            fprintf(outfp2, "[%lu %lu] ",
                    cutSet[i].startVertex, cutSet[i].endVertex);
        }
        fclose(outfp2);
        fclose(outfp1);
    }
#endif

    if (myId == 0) {
        P_FREE(edgeStartCounter);
        P_FREE(edgeEndCounter);
        P_FREE(pCutSet);
        P_FREE(IndexSorted);
        P_FREE(neighbourArraySorted);
        P_FREE(startV);
        P_FREE(clusterSize);
        P_FREE(cutSet);
        P_FREE(vStatus);
    }

    TM_THREAD_EXIT();
}


/* =============================================================================
 *
 * End of cutClusters.c
 *
 * =============================================================================
 */
