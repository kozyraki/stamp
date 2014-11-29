/* =============================================================================
 *
 * getStartLists.c
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
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "createPartition.h"
#include "defs.h"
#include "getStartLists.h"
#include "globals.h"
#include "thread.h"
#include "tm.h"
#include "utility.h"

static LONGINT_T global_maxWeight          = 0;
static long*     global_i_edgeStartCounter = NULL;
static long*     global_i_edgeEndCounter   = NULL;
static edge*     global_maxIntWtList       = NULL;
static edge*     global_soughtStrWtList    = NULL;


/* =============================================================================
 * getStartLists
 * =============================================================================
 */
void
getStartLists (void* argPtr)
{
    TM_THREAD_ENTER();

    graph* GPtr                = ((getStartLists_arg_t*)argPtr)->GPtr;
    edge** maxIntWtListPtr     = ((getStartLists_arg_t*)argPtr)->maxIntWtListPtr;
    long*  maxIntWtListSize    = ((getStartLists_arg_t*)argPtr)->maxIntWtListSize;
    edge** soughtStrWtListPtr  = ((getStartLists_arg_t*)argPtr)->soughtStrWtListPtr;
    long*  soughtStrWtListSize = ((getStartLists_arg_t*)argPtr)->soughtStrWtListSize;

    long myId = thread_getId();
    long numThread = thread_getNumThread();

    /*
     * Find Max Wt on each thread
     */

    LONGINT_T maxWeight = 0;

    long i;
    long i_start;
    long i_stop;
    createPartition(0, GPtr->numEdges, myId, numThread, &i_start, &i_stop);

    for (i = i_start; i < i_stop; i++) {
        if (GPtr->intWeight[i] > maxWeight) {
            maxWeight = GPtr->intWeight[i];
        }
    }

    TM_BEGIN();
    long tmp_maxWeight = (long)TM_SHARED_READ(global_maxWeight);
    if (maxWeight > tmp_maxWeight) {
        TM_SHARED_WRITE(global_maxWeight, maxWeight);
    }
    TM_END();

    thread_barrier_wait();

    maxWeight = global_maxWeight;

    /*
     * Create partial lists
     */

    /*
     * Allocate mem. for temp edge list for each thread
     */
    long numTmpEdge = (5+ceil(1.5*(GPtr->numIntEdges)/MAX_INT_WEIGHT));
    edge* tmpEdgeList = (edge*)P_MALLOC(numTmpEdge * sizeof(edge));

    long i_edgeCounter = 0;

    for (i = i_start; i < i_stop; i++) {

        if (GPtr->intWeight[i] == maxWeight) {

            /* Find the corresponding endVertex */
            long j;
            for (j = 0; j < GPtr->numDirectedEdges; j++) {
                if (GPtr->paralEdgeIndex[j] > i) {
                    break;
                }
            }
            tmpEdgeList[i_edgeCounter].endVertex = GPtr->outVertexList[j-1];
            tmpEdgeList[i_edgeCounter].edgeNum = j-1;

            long t;
            for (t = 0; t < GPtr->numVertices; t++) {
                if (GPtr->outVertexIndex[t] > j-1) {
                    break;
                }
            }
            tmpEdgeList[i_edgeCounter].startVertex = t-1;

            i_edgeCounter++;

        }
    }

    /*
     * Merge partial edge lists
     */

    long* i_edgeStartCounter;
    long* i_edgeEndCounter;

    if (myId == 0) {
        i_edgeStartCounter = (long*)P_MALLOC(numThread * sizeof(long));
        assert(i_edgeStartCounter);
        global_i_edgeStartCounter = i_edgeStartCounter;
        i_edgeEndCounter = (long*)P_MALLOC(numThread * sizeof(long));
        assert(i_edgeEndCounter);
        global_i_edgeEndCounter = i_edgeEndCounter;

        *maxIntWtListSize = 0;
    }

    thread_barrier_wait();

    i_edgeStartCounter = global_i_edgeStartCounter;
    i_edgeEndCounter = global_i_edgeEndCounter;

    i_edgeEndCounter[myId] = i_edgeCounter;
    i_edgeStartCounter[myId] = 0;

    thread_barrier_wait();

    if (myId == 0) {
        for (i = 1; i < numThread; i++) {
            i_edgeEndCounter[i] = i_edgeEndCounter[i-1] + i_edgeEndCounter[i];
            i_edgeStartCounter[i] = i_edgeEndCounter[i-1];
        }
    }

    *maxIntWtListSize += i_edgeCounter;

    thread_barrier_wait();

    edge* maxIntWtList;

    if (myId == 0) {
        P_FREE(*maxIntWtListPtr);
        maxIntWtList = (edge*)P_MALLOC((*maxIntWtListSize) * sizeof(edge));
        assert(maxIntWtList);
        global_maxIntWtList = maxIntWtList;
    }

    thread_barrier_wait();

    maxIntWtList = global_maxIntWtList;

    for (i = i_edgeStartCounter[myId]; i<i_edgeEndCounter[myId]; i++) {
      (maxIntWtList[i]).startVertex = tmpEdgeList[i-i_edgeStartCounter[myId]].startVertex;
      (maxIntWtList[i]).endVertex = tmpEdgeList[i-i_edgeStartCounter[myId]].endVertex;
      (maxIntWtList[i]).edgeNum = tmpEdgeList[i-i_edgeStartCounter[myId]].edgeNum;
    }

    if (myId == 0) {
        *maxIntWtListPtr = maxIntWtList;
    }

    i_edgeCounter = 0;

    createPartition(0, GPtr->numStrEdges, myId, numThread, &i_start, &i_stop);

    for (i = i_start; i < i_stop; i++) {

        if (strncmp(GPtr->strWeight+i*MAX_STRLEN,
                    SOUGHT_STRING,
                    MAX_STRLEN) == 0)
        {
            /*
             * Find the corresponding endVertex
             */

            long t;
            for (t = 0; t < GPtr->numEdges; t++) {
                if (GPtr->intWeight[t] == -i) {
                    break;
                }
            }

            long j;
            for (j = 0; j < GPtr->numDirectedEdges; j++) {
            if (GPtr->paralEdgeIndex[j] > t) {
                    break;
                }
            }
            tmpEdgeList[i_edgeCounter].endVertex = GPtr->outVertexList[j-1];
            tmpEdgeList[i_edgeCounter].edgeNum = j-1;

            for (t = 0; t < GPtr->numVertices; t++) {
                if (GPtr->outVertexIndex[t] > j-1) {
                    break;
                }
            }
            tmpEdgeList[i_edgeCounter].startVertex = t-1;
            i_edgeCounter++;
        }

    }

    thread_barrier_wait();

    i_edgeEndCounter[myId] = i_edgeCounter;
    i_edgeStartCounter[myId] = 0;

    if (myId == 0) {
        *soughtStrWtListSize = 0;
    }

    thread_barrier_wait();

    if (myId == 0) {
        for (i = 1; i < numThread; i++) {
            i_edgeEndCounter[i] = i_edgeEndCounter[i-1] + i_edgeEndCounter[i];
            i_edgeStartCounter[i] = i_edgeEndCounter[i-1];
        }
    }

    *soughtStrWtListSize += i_edgeCounter;

    thread_barrier_wait();

    edge* soughtStrWtList;

    if (myId == 0) {
        P_FREE(*soughtStrWtListPtr);
        soughtStrWtList = (edge*)P_MALLOC((*soughtStrWtListSize) * sizeof(edge));
        assert(soughtStrWtList);
        global_soughtStrWtList = soughtStrWtList;
    }

    thread_barrier_wait();

    soughtStrWtList = global_soughtStrWtList;

    for (i = i_edgeStartCounter[myId]; i < i_edgeEndCounter[myId]; i++) {
        (soughtStrWtList[i]).startVertex =
            tmpEdgeList[i-i_edgeStartCounter[myId]].startVertex;
        (soughtStrWtList[i]).endVertex =
            tmpEdgeList[i-i_edgeStartCounter[myId]].endVertex;
        (soughtStrWtList[i]).edgeNum =
            tmpEdgeList[i-i_edgeStartCounter[myId]].edgeNum;
    }

    thread_barrier_wait();

    if (myId == 0) {
        *soughtStrWtListPtr = soughtStrWtList;
        P_FREE(i_edgeStartCounter);
        P_FREE(i_edgeEndCounter);
    }

    P_FREE(tmpEdgeList);

    TM_THREAD_EXIT();
}


/* =============================================================================
 *
 * End of getStartLists.c
 *
 * =============================================================================
 */
