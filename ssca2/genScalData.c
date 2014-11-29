/* =============================================================================
 *
 * genScalData.c
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alg_radix_smp.h"
#include "createPartition.h"
#include "defs.h"
#include "genScalData.h"
#include "globals.h"
#include "random.h"
#include "thread.h"
#include "tm.h"

static ULONGINT_T* global_permV              = NULL;
static long*       global_cliqueSizes        = NULL;
static long        global_totCliques         = 0;
static ULONGINT_T* global_firstVsInCliques   = NULL;
static ULONGINT_T* global_lastVsInCliques    = NULL;
static ULONGINT_T* global_i_edgeStartCounter = NULL;
static ULONGINT_T* global_i_edgeEndCounter   = NULL;
static long        global_edgeNum            = 0;
static ULONGINT_T  global_numStrWtEdges      = 0;
static ULONGINT_T* global_startVertex        = NULL;
static ULONGINT_T* global_endVertex          = NULL;
static ULONGINT_T* global_tempIndex          = NULL;


/* =============================================================================
 * genScalData_seq
 * =============================================================================
 */
void
genScalData_seq (graphSDG* SDGdataPtr)
{
    /*
     * STEP 0: Create the permutations required to randomize the vertices
     */

    random_t* stream = random_alloc();
    assert(stream);
    random_seed(stream, 0);

    ULONGINT_T* permV; /* the vars associated with the graph tuple */
    permV = (ULONGINT_T*)malloc(TOT_VERTICES * sizeof(ULONGINT_T));
    assert(permV);

    long i;

    /* Initialize the array */
    for (i = 0; i < TOT_VERTICES; i++) {
        permV[i] = i;
    }

    for (i = 0; i < TOT_VERTICES; i++) {
        long t1 = random_generate(stream);
        long t = i + t1 % (TOT_VERTICES - i);
        if (t != i) {
            ULONGINT_T t2 = permV[t];
            permV[t] = permV[i];
            permV[i] = t2;
        }
    }

    /*
     * STEP 1: Create Cliques
     */

    long* cliqueSizes;

    long estTotCliques = ceil(1.5 * TOT_VERTICES / ((1+MAX_CLIQUE_SIZE)/2));

    /*
     * Allocate mem for Clique array
     * Estimate number of clique required and pad by 50%
     */
    cliqueSizes = (long*)malloc(estTotCliques * sizeof(long));
    assert(cliqueSizes);


    /* Generate random clique sizes. */
    for (i = 0; i < estTotCliques; i++) {
        cliqueSizes[i] = 1 + (random_generate(stream) % MAX_CLIQUE_SIZE);
    }

    long totCliques = 0;

    /*
     * Allocate memory for cliqueList
     */

    ULONGINT_T* lastVsInCliques;
    ULONGINT_T* firstVsInCliques;

    lastVsInCliques = (ULONGINT_T*)malloc(estTotCliques * sizeof(ULONGINT_T));
    assert(lastVsInCliques);
    firstVsInCliques = (ULONGINT_T*)malloc(estTotCliques * sizeof(ULONGINT_T));
    assert(firstVsInCliques);

    /*
     * Sum up vertices in each clique to determine the lastVsInCliques array
     */

    lastVsInCliques[0] = cliqueSizes[0] - 1;
    for (i = 1; i < estTotCliques; i++) {
        lastVsInCliques[i] = cliqueSizes[i] + lastVsInCliques[i-1];
        if (lastVsInCliques[i] >= TOT_VERTICES-1) {
            break;
        }
    }
    totCliques = i + 1;

    /*
     * Fix the size of the last clique
     */
    cliqueSizes[totCliques-1] =
        TOT_VERTICES - lastVsInCliques[totCliques-2] - 1;
    lastVsInCliques[totCliques-1] = TOT_VERTICES - 1;

    firstVsInCliques[0] = 0;


    /*
     * Compute start Vertices in cliques.
     */
    for (i = 1; i < totCliques; i++) {
        firstVsInCliques[i] = lastVsInCliques[i-1] + 1;
    }

#ifdef WRITE_RESULT_FILES
    /* Write the generated cliques to file for comparison with Kernel 4 */
    FILE* outfp = fopen("cliques.txt", "w");
    fprintf(outfp, "No. of cliques - %lu\n", totCliques);
    for (i = 0; i < totCliques; i++) {
        fprintf(outfp, "Clq %lu - ", i);
        long j;
        for (j = firstVsInCliques[i]; j <= lastVsInCliques[i]; j++) {
            fprintf(outfp, "%lu ", permV[j]);
        }
        fprintf(outfp, "\n");
    }
    fclose(outfp);
#endif

    /*
     * STEP 2: Create the edges within the cliques
     */

    /*
     * Estimate number of edges - using an empirical measure
     */
    long estTotEdges;
    if (SCALE >= 12) {
        estTotEdges = ceil(((MAX_CLIQUE_SIZE-1) * TOT_VERTICES));
    } else {
        estTotEdges = ceil(1.2 * (((MAX_CLIQUE_SIZE-1)*TOT_VERTICES)
                                  * ((1 + MAX_PARAL_EDGES)/2) + TOT_VERTICES*2));
    }

    /*
     * Initialize edge counter
     */
    long i_edgePtr = 0;
    float p = PROB_UNIDIRECTIONAL;

    /*
     * Partial edgeLists
     */

    ULONGINT_T* startV;
    ULONGINT_T* endV;
    long numByte = (estTotEdges) * sizeof(ULONGINT_T);
    startV = (ULONGINT_T*)malloc(numByte);
    endV = (ULONGINT_T*)malloc(numByte);
    assert(startV);
    assert(endV);

    /*
     * Tmp array to keep track of the no. of parallel edges in each direction
     */
    ULONGINT_T** tmpEdgeCounter =
        (ULONGINT_T**)malloc(MAX_CLIQUE_SIZE * sizeof(ULONGINT_T *));
    assert(tmpEdgeCounter);
    for (i = 0; i < MAX_CLIQUE_SIZE; i++) {
        tmpEdgeCounter[i] =
            (ULONGINT_T*)malloc(MAX_CLIQUE_SIZE * sizeof(ULONGINT_T));
        assert(tmpEdgeCounter[i]);
    }

    /*
     * Create edges
     */
     long i_clique;

     for (i_clique = 0; i_clique < totCliques; i_clique++) {

        /*
         * Get current clique parameters
         */

        long i_cliqueSize = cliqueSizes[i_clique];
        long i_firstVsInClique = firstVsInCliques[i_clique];

        /*
         * First create at least one edge between two vetices in a clique
         */

        for (i = 0; i < i_cliqueSize; i++) {

            long j;
            for (j = 0; j < i; j++) {

                float r = (float)(random_generate(stream) % 1000) / (float)1000;
                if (r >= p) {

                    startV[i_edgePtr] = i + i_firstVsInClique;
                    endV[i_edgePtr] = j + i_firstVsInClique;
                    i_edgePtr++;
                    tmpEdgeCounter[i][j] = 1;

                    startV[i_edgePtr] = j + i_firstVsInClique;
                    endV[i_edgePtr] = i + i_firstVsInClique;
                    i_edgePtr++;
                    tmpEdgeCounter[j][i] = 1;

                } else  if (r >= 0.5) {

                    startV[i_edgePtr] = i + i_firstVsInClique;
                    endV[i_edgePtr] = j + i_firstVsInClique;
                    i_edgePtr++;
                    tmpEdgeCounter[i][j] = 1;
                    tmpEdgeCounter[j][i] = 0;

                } else {

                    startV[i_edgePtr] = j + i_firstVsInClique;
                    endV[i_edgePtr] = i + i_firstVsInClique;
                    i_edgePtr++;
                    tmpEdgeCounter[j][i] = 1;
                    tmpEdgeCounter[i][j] = 0;

                }

            } /* for j */
        } /* for i */

        if (i_cliqueSize != 1) {
            long randNumEdges = (long)(random_generate(stream)
                                       % (2*i_cliqueSize*MAX_PARAL_EDGES));
            long i_paralEdge;
            for (i_paralEdge = 0; i_paralEdge < randNumEdges; i_paralEdge++) {
                i = (random_generate(stream) % i_cliqueSize);
                long j = (random_generate(stream) % i_cliqueSize);
                if ((i != j) && (tmpEdgeCounter[i][j] < MAX_PARAL_EDGES)) {
                    float r = (float)(random_generate(stream) % 1000) / (float)1000;
                    if (r >= p) {
                        /* Copy to edge structure. */
                        startV[i_edgePtr] = i + i_firstVsInClique;
                        endV[i_edgePtr] = j + i_firstVsInClique;
                        i_edgePtr++;
                        tmpEdgeCounter[i][j]++;
                    }
                }
            }
        }

    } /* for i_clique */

    for (i = 0; i < MAX_CLIQUE_SIZE; i++) {
        free(tmpEdgeCounter[i]);
    }

    free(tmpEdgeCounter);


    /*
     * Merge partial edge lists
     */

    ULONGINT_T i_edgeStartCounter = 0;
    ULONGINT_T i_edgeEndCounter = i_edgePtr;
    long edgeNum = i_edgePtr;

    /*
     * Initialize edge list arrays
     */

    ULONGINT_T* startVertex;
    ULONGINT_T* endVertex;

    if (SCALE < 10) {
        long numByte = 2 * edgeNum * sizeof(ULONGINT_T);
        startVertex = (ULONGINT_T*)malloc(numByte);
        endVertex = (ULONGINT_T*)malloc(numByte);
    } else {
        long numByte = (edgeNum + MAX_PARAL_EDGES * TOT_VERTICES)
                       * sizeof(ULONGINT_T);
        startVertex = (ULONGINT_T*)malloc(numByte);
        endVertex = (ULONGINT_T*)malloc(numByte);
    }
    assert(startVertex);
    assert(endVertex);

    for (i = i_edgeStartCounter; i < i_edgeEndCounter; i++) {
        startVertex[i] = startV[i-i_edgeStartCounter];
        endVertex[i] = endV[i-i_edgeStartCounter];
    }

    ULONGINT_T numEdgesPlacedInCliques = edgeNum;

    /*
     * STEP 3: Connect the cliques
     */

    i_edgePtr = 0;
    p = PROB_INTERCL_EDGES;

    /*
     * Generating inter-clique edges as given in the specs
     */

    for (i = 0; i < TOT_VERTICES; i++) {

        ULONGINT_T tempVertex1 = i;
        long h = totCliques;
        long l = 0;
        long t = -1;
        while (h - l > 1) {
            long m = (h + l) / 2;
            if (tempVertex1 >= firstVsInCliques[m]) {
                l = m;
            } else {
                if ((tempVertex1 < firstVsInCliques[m]) && (m > 0)) {
                    if (tempVertex1 >= firstVsInCliques[m-1]) {
                        t = m - 1;
                        break;
                    } else {
                        h = m;
                    }
                }
            }
        }

        if (t == -1) {
            long m;
            for (m = (l + 1); m < h; m++) {
                if (tempVertex1<firstVsInCliques[m]) {
                    break;
                }
            }
            t = m-1;
        }

        long t1 = firstVsInCliques[t];

        ULONGINT_T d;
        for (d = 1, p = PROB_INTERCL_EDGES; d < TOT_VERTICES; d *= 2, p /= 2) {

            float r = (float)(random_generate(stream) % 1000) / (float)1000;

            if (r <= p) {

                ULONGINT_T tempVertex2 = (i+d) % TOT_VERTICES;

                h = totCliques;
                l = 0;
                t = -1;
                while (h - l > 1) {
                    long m = (h + l) / 2;
                    if (tempVertex2 >= firstVsInCliques[m]) {
                        l = m;
                    } else {
                        if ((tempVertex2 < firstVsInCliques[m]) && (m > 0)) {
                            if (firstVsInCliques[m-1] <= tempVertex2) {
                                t = m - 1;
                                break;
                            } else {
                                h = m;
                            }
                        }
                    }
                }

                if (t == -1) {
                    long m;
                    for (m = (l + 1); m < h; m++) {
                        if (tempVertex2 < firstVsInCliques[m]) {
                            break;
                        }
                    }
                    t = m - 1;
                }

                long t2 = firstVsInCliques[t];

                if (t1 != t2) {
                    long randNumEdges =
                        random_generate(stream) % MAX_PARAL_EDGES + 1;
                    long j;
                    for (j = 0; j < randNumEdges; j++) {
                        startV[i_edgePtr] = tempVertex1;
                        endV[i_edgePtr] = tempVertex2;
                        i_edgePtr++;
                    }
                }

            } /* r <= p */

            float r0 = (float)(random_generate(stream) % 1000) / (float)1000;

            if ((r0 <= p) && (i-d>=0)) {

                ULONGINT_T tempVertex2 = (i-d) % TOT_VERTICES;

                h = totCliques;
                l = 0;
                t = -1;
                while (h - l > 1) {
                    long m = (h + l) / 2;
                    if (tempVertex2 >= firstVsInCliques[m]) {
                        l = m;
                    } else {
                        if ((tempVertex2 < firstVsInCliques[m]) && (m > 0)) {
                            if (firstVsInCliques[m-1] <= tempVertex2) {
                                t = m - 1;
                                break;
                            } else {
                                h = m;
                            }
                        }
                    }
                }

                if (t == -1) {
                    long m;
                    for (m = (l + 1); m < h; m++) {
                        if (tempVertex2 < firstVsInCliques[m]) {
                            break;
                        }
                    }
                    t = m - 1;
                }

                long t2 = firstVsInCliques[t];

                if (t1 != t2) {
                    long randNumEdges =
                        random_generate(stream) % MAX_PARAL_EDGES + 1;
                    long j;
                    for (j = 0; j < randNumEdges; j++) {
                        startV[i_edgePtr] = tempVertex1;
                        endV[i_edgePtr] = tempVertex2;
                        i_edgePtr++;
                    }
                }

            } /* r0 <= p && (i-d) > 0 */

        } /* for d, p */

    } /* for i */


    i_edgeEndCounter = i_edgePtr;
    i_edgeStartCounter = 0;

    edgeNum = i_edgePtr;
    ULONGINT_T numEdgesPlacedOutside = edgeNum;

    for (i = i_edgeStartCounter; i < i_edgeEndCounter; i++) {
        startVertex[i+numEdgesPlacedInCliques] = startV[i-i_edgeStartCounter];
        endVertex[i+numEdgesPlacedInCliques] = endV[i-i_edgeStartCounter];
    }

    ULONGINT_T  numEdgesPlaced = numEdgesPlacedInCliques + numEdgesPlacedOutside;

    SDGdataPtr->numEdgesPlaced = numEdgesPlaced;

    printf("Finished generating edges\n");
    printf("No. of intra-clique edges - %lu\n", numEdgesPlacedInCliques);
    printf("No. of inter-clique edges - %lu\n", numEdgesPlacedOutside);
    printf("Total no. of edges        - %lu\n", numEdgesPlaced);

    free(cliqueSizes);
    free(firstVsInCliques);
    free(lastVsInCliques);

    free(startV);
    free(endV);

    /*
     * STEP 4: Generate edge weights
     */

    SDGdataPtr->intWeight =
        (LONGINT_T*)malloc(numEdgesPlaced * sizeof(LONGINT_T));
    assert(SDGdataPtr->intWeight);

    p = PERC_INT_WEIGHTS;
    ULONGINT_T numStrWtEdges  = 0;

    for (i = 0; i < numEdgesPlaced; i++) {
        float r = (float)(random_generate(stream) % 1000) / (float)1000;
        if (r <= p) {
            SDGdataPtr->intWeight[i] =
                1 + (random_generate(stream) % (MAX_INT_WEIGHT-1));
        } else {
            SDGdataPtr->intWeight[i] = -1;
            numStrWtEdges++;
        }
    }

    long t = 0;
    for (i = 0; i < numEdgesPlaced; i++) {
        if (SDGdataPtr->intWeight[i] < 0) {
            SDGdataPtr->intWeight[i] = -t;
            t++;
        }
    }

    SDGdataPtr->strWeight =
        (char*)malloc(numStrWtEdges * MAX_STRLEN * sizeof(char));
    assert(SDGdataPtr->strWeight);

    for (i = 0; i < numEdgesPlaced; i++) {
        if (SDGdataPtr->intWeight[i] <= 0) {
            long j;
            for (j = 0; j < MAX_STRLEN; j++) {
                SDGdataPtr->strWeight[(-SDGdataPtr->intWeight[i])*MAX_STRLEN+j] =
                    (char) (1 + random_generate(stream) % 127);
            }
        }
    }

    /*
     * Choose SOUGHT STRING randomly if not assigned
     */

    if (strlen(SOUGHT_STRING) != MAX_STRLEN) {
        SOUGHT_STRING = (char*)malloc(MAX_STRLEN * sizeof(char));
        assert(SOUGHT_STRING);
    }

    t = random_generate(stream) % numStrWtEdges;
    long j;
    for (j = 0; j < MAX_STRLEN; j++) {
        SOUGHT_STRING[j] =
            (char) ((long) SDGdataPtr->strWeight[t*MAX_STRLEN+j]);
    }

    /*
     * STEP 5: Permute Vertices
     */

    for (i = 0; i < numEdgesPlaced; i++) {
        startVertex[i] = permV[(startVertex[i])];
        endVertex[i] = permV[(endVertex[i])];
    }

    /*
     * STEP 6: Sort Vertices
     */

    /*
     * Radix sort with StartVertex as primary key
     */

    numByte = numEdgesPlaced * sizeof(ULONGINT_T);
    SDGdataPtr->startVertex = (ULONGINT_T*)malloc(numByte);
    assert(SDGdataPtr->startVertex);
    SDGdataPtr->endVertex = (ULONGINT_T*)malloc(numByte);
    assert(SDGdataPtr->endVertex);

    all_radixsort_node_aux_s3_seq(numEdgesPlaced,
                                  startVertex,
                                  SDGdataPtr->startVertex,
                                  endVertex,
                                  SDGdataPtr->endVertex);

    free(startVertex);
    free(endVertex);

    if (SCALE < 12) {

        /*
         * Sort with endVertex as secondary key
         */

        long i0 = 0;
        long i1 = 0;
        i = 0;

        while (i < numEdgesPlaced) {

            for (i = i0; i < numEdgesPlaced; i++) {
                if (SDGdataPtr->startVertex[i] !=
                    SDGdataPtr->startVertex[i1])
                {
                    i1 = i;
                    break;
                }
            }

            long j;
            for (j = i0; j < i1; j++) {
                long k;
                for (k = j+1; k < i1; k++) {
                    if (SDGdataPtr->endVertex[k] <
                        SDGdataPtr->endVertex[j])
                    {
                        long t = SDGdataPtr->endVertex[j];
                        SDGdataPtr->endVertex[j] = SDGdataPtr->endVertex[k];
                        SDGdataPtr->endVertex[k] = t;
                    }
                }
            }

            if (SDGdataPtr->startVertex[i0] != TOT_VERTICES-1) {
                i0 = i1;
            } else {
                long j;
                for (j=i0; j<numEdgesPlaced; j++) {
                    long k;
                    for (k=j+1; k<numEdgesPlaced; k++) {
                        if (SDGdataPtr->endVertex[k] <
                            SDGdataPtr->endVertex[j])
                        {
                            long t = SDGdataPtr->endVertex[j];
                            SDGdataPtr->endVertex[j] = SDGdataPtr->endVertex[k];
                            SDGdataPtr->endVertex[k] = t;
                        }
                    }
                }
            }

        } /* while i < numEdgesPlaced */

    } else {

        ULONGINT_T* tempIndex =
            (ULONGINT_T*)malloc((TOT_VERTICES + 1) * sizeof(ULONGINT_T));

        /*
         * Update degree of each vertex
         */

        tempIndex[0] = 0;
        tempIndex[TOT_VERTICES] = numEdgesPlaced;
        long i0 = 0;

        for (i=0; i < TOT_VERTICES; i++) {
            tempIndex[i+1] = tempIndex[i];
            long j;
            for (j = i0; j < numEdgesPlaced; j++) {
                if (SDGdataPtr->startVertex[j] !=
                    SDGdataPtr->startVertex[i0])
                {
                    if (SDGdataPtr->startVertex[i0] == i) {
                        tempIndex[i+1] = j;
                        i0 = j;
                        break;
                    }
                }
            }
        }

        /*
         * Insertion sort for now, replace with something better later on
         */
        for (i = 0; i < TOT_VERTICES; i++) {
            long j;
            for (j = tempIndex[i]; j < tempIndex[i+1]; j++) {
                long k;
                for (k = (j + 1); k < tempIndex[i+1]; k++) {
                    if (SDGdataPtr->endVertex[k] <
                        SDGdataPtr->endVertex[j])
                    {
                        long t = SDGdataPtr->endVertex[j];
                        SDGdataPtr->endVertex[j] = SDGdataPtr->endVertex[k];
                        SDGdataPtr->endVertex[k] = t;
                    }
                }
            }
        }

       free(tempIndex);

    } /* SCALE >= 12 */

    random_free(stream);
    free(permV);
}


/* =============================================================================
 * genScalData
 * =============================================================================
 */
void
genScalData (void* argPtr)
{
    TM_THREAD_ENTER();

    graphSDG* SDGdataPtr = (graphSDG*)argPtr;

    long myId = thread_getId();
    long numThread = thread_getNumThread();

    /*
     * STEP 0: Create the permutations required to randomize the vertices
     */

    random_t* stream = PRANDOM_ALLOC();
    assert(stream);
    PRANDOM_SEED(stream, myId);

    ULONGINT_T* permV; /* the vars associated with the graph tuple */

    if (myId == 0) {
        permV = (ULONGINT_T*)P_MALLOC(TOT_VERTICES * sizeof(ULONGINT_T));
        assert(permV);
        global_permV = permV;
    }

    thread_barrier_wait();

    permV = global_permV;

    long i;
    long i_start;
    long i_stop;
    createPartition(0, TOT_VERTICES, myId, numThread, &i_start, &i_stop);

    /* Initialize the array */
    for (i = i_start; i < i_stop; i++) {
        permV[i] = i;
    }

    thread_barrier_wait();

    for (i = i_start; i < i_stop; i++) {
        long t1 = PRANDOM_GENERATE(stream);
        long t = i + t1 % (TOT_VERTICES - i);
        if (t != i) {
            TM_BEGIN();
            long t2 = (long)TM_SHARED_READ(permV[t]);
            TM_SHARED_WRITE(permV[t], TM_SHARED_READ(permV[i]));
            TM_SHARED_WRITE(permV[i], t2);
            TM_END();
        }
    }

    /*
     * STEP 1: Create Cliques
     */

    long* cliqueSizes;

    long estTotCliques = ceil(1.5 * TOT_VERTICES / ((1+MAX_CLIQUE_SIZE)/2));

    /*
     * Allocate mem for Clique array
     * Estimate number of clique required and pad by 50%
     */
    if (myId == 0) {
        cliqueSizes = (long*)P_MALLOC(estTotCliques * sizeof(long));
        assert(cliqueSizes);
        global_cliqueSizes = cliqueSizes;
    }

    thread_barrier_wait();

    cliqueSizes = global_cliqueSizes;

    createPartition(0, estTotCliques, myId, numThread, &i_start, &i_stop);

    /* Generate random clique sizes. */
    for (i = i_start; i < i_stop; i++) {
        cliqueSizes[i] = 1 + (PRANDOM_GENERATE(stream) % MAX_CLIQUE_SIZE);
    }

    thread_barrier_wait();

    long totCliques = 0;

    /*
     * Allocate memory for cliqueList
     */

    ULONGINT_T* lastVsInCliques;
    ULONGINT_T* firstVsInCliques;

    if (myId == 0) {

        lastVsInCliques = (ULONGINT_T*)P_MALLOC(estTotCliques * sizeof(ULONGINT_T));
        assert(lastVsInCliques);
        global_lastVsInCliques = lastVsInCliques;
        firstVsInCliques = (ULONGINT_T*)P_MALLOC(estTotCliques * sizeof(ULONGINT_T));
        assert(firstVsInCliques);
        global_firstVsInCliques = firstVsInCliques;

        /*
         * Sum up vertices in each clique to determine the lastVsInCliques array
         */

        lastVsInCliques[0] = cliqueSizes[0] - 1;
        for (i = 1; i < estTotCliques; i++) {
            lastVsInCliques[i] = cliqueSizes[i] + lastVsInCliques[i-1];
            if (lastVsInCliques[i] >= TOT_VERTICES-1) {
                break;
            }
        }
        totCliques = i + 1;

        global_totCliques = totCliques;

        /*
         * Fix the size of the last clique
         */
        cliqueSizes[totCliques-1] =
            TOT_VERTICES - lastVsInCliques[totCliques-2] - 1;
        lastVsInCliques[totCliques-1] = TOT_VERTICES - 1;

        firstVsInCliques[0] = 0;

    }

    thread_barrier_wait();

    lastVsInCliques  = global_lastVsInCliques;
    firstVsInCliques = global_firstVsInCliques;
    totCliques = global_totCliques;

    /* Compute start Vertices in cliques. */
    createPartition(1, totCliques, myId, numThread, &i_start, &i_stop);
    for (i = i_start; i < i_stop; i++) {
        firstVsInCliques[i] = lastVsInCliques[i-1] + 1;
    }

#ifdef WRITE_RESULT_FILES
    thread_barrier_wait();

    /* Write the generated cliques to file for comparison with Kernel 4 */
    if (myId == 0) {
        FILE* outfp = fopen("cliques.txt", "w");
        fprintf(outfp, "No. of cliques - %lu\n", totCliques);
        for (i = 0; i < totCliques; i++) {
            fprintf(outfp, "Clq %lu - ", i);
            long j;
            for (j = firstVsInCliques[i]; j <= lastVsInCliques[i]; j++) {
                fprintf(outfp, "%lu ", permV[j]);
            }
            fprintf(outfp, "\n");
        }
        fclose(outfp);
    }

    thread_barrier_wait();
#endif

    /*
     * STEP 2: Create the edges within the cliques
     */

    /*
     * Estimate number of edges - using an empirical measure
     */
    long estTotEdges;
    if (SCALE >= 12) {
        estTotEdges = ceil(((MAX_CLIQUE_SIZE-1) * TOT_VERTICES));
    } else {
        estTotEdges = ceil(1.2 * (((MAX_CLIQUE_SIZE-1)*TOT_VERTICES)
                                  * ((1 + MAX_PARAL_EDGES)/2) + TOT_VERTICES*2));
    }

    /*
     * Initialize edge counter
     */
    long i_edgePtr = 0;
    float p = PROB_UNIDIRECTIONAL;

    /*
     * Partial edgeLists
     */

    ULONGINT_T* startV;
    ULONGINT_T* endV;

    if (numThread > 3) {
        long numByte = 1.5 * (estTotEdges/numThread) * sizeof(ULONGINT_T);
        startV = (ULONGINT_T*)P_MALLOC(numByte);
        endV = (ULONGINT_T*)P_MALLOC(numByte);
    } else  {
        long numByte = (estTotEdges/numThread) * sizeof(ULONGINT_T);
        startV = (ULONGINT_T*)P_MALLOC(numByte);
        endV = (ULONGINT_T*)P_MALLOC(numByte);
    }
    assert(startV);
    assert(endV);

    /*
     * Tmp array to keep track of the no. of parallel edges in each direction
     */
    ULONGINT_T** tmpEdgeCounter =
        (ULONGINT_T**)P_MALLOC(MAX_CLIQUE_SIZE * sizeof(ULONGINT_T *));
    assert(tmpEdgeCounter);
    for (i = 0; i < MAX_CLIQUE_SIZE; i++) {
        tmpEdgeCounter[i] =
            (ULONGINT_T*)P_MALLOC(MAX_CLIQUE_SIZE * sizeof(ULONGINT_T));
        assert(tmpEdgeCounter[i]);
    }

    /*
     * Create edges in parallel
     */
     long i_clique;
     createPartition(0, totCliques, myId, numThread, &i_start, &i_stop);

     for (i_clique = i_start; i_clique < i_stop; i_clique++) {

        /*
         * Get current clique parameters
         */

        long i_cliqueSize = cliqueSizes[i_clique];
        long i_firstVsInClique = firstVsInCliques[i_clique];

        /*
         * First create at least one edge between two vetices in a clique
         */

        for (i = 0; i < i_cliqueSize; i++) {

            long j;
            for (j = 0; j < i; j++) {

                float r = (float)(PRANDOM_GENERATE(stream) % 1000) / (float)1000;
                if (r >= p) {

                    startV[i_edgePtr] = i + i_firstVsInClique;
                    endV[i_edgePtr] = j + i_firstVsInClique;
                    i_edgePtr++;
                    tmpEdgeCounter[i][j] = 1;

                    startV[i_edgePtr] = j + i_firstVsInClique;
                    endV[i_edgePtr] = i + i_firstVsInClique;
                    i_edgePtr++;
                    tmpEdgeCounter[j][i] = 1;

                } else  if (r >= 0.5) {

                    startV[i_edgePtr] = i + i_firstVsInClique;
                    endV[i_edgePtr] = j + i_firstVsInClique;
                    i_edgePtr++;
                    tmpEdgeCounter[i][j] = 1;
                    tmpEdgeCounter[j][i] = 0;

                } else {

                    startV[i_edgePtr] = j + i_firstVsInClique;
                    endV[i_edgePtr] = i + i_firstVsInClique;
                    i_edgePtr++;
                    tmpEdgeCounter[j][i] = 1;
                    tmpEdgeCounter[i][j] = 0;

                }

            } /* for j */
        } /* for i */

        if (i_cliqueSize != 1) {
            long randNumEdges = (long)(PRANDOM_GENERATE(stream)
                                       % (2*i_cliqueSize*MAX_PARAL_EDGES));
            long i_paralEdge;
            for (i_paralEdge = 0; i_paralEdge < randNumEdges; i_paralEdge++) {
                i = (PRANDOM_GENERATE(stream) % i_cliqueSize);
                long j = (PRANDOM_GENERATE(stream) % i_cliqueSize);
                if ((i != j) && (tmpEdgeCounter[i][j] < MAX_PARAL_EDGES)) {
                    float r = (float)(PRANDOM_GENERATE(stream) % 1000) / (float)1000;
                    if (r >= p) {
                        /* Copy to edge structure. */
                        startV[i_edgePtr] = i + i_firstVsInClique;
                        endV[i_edgePtr] = j + i_firstVsInClique;
                        i_edgePtr++;
                        tmpEdgeCounter[i][j]++;
                    }
                }
            }
        }

    } /* for i_clique */

    for (i = 0; i < MAX_CLIQUE_SIZE; i++) {
        P_FREE(tmpEdgeCounter[i]);
    }

    P_FREE(tmpEdgeCounter);


    /*
     * Merge partial edge lists
     */

    ULONGINT_T* i_edgeStartCounter;
    ULONGINT_T* i_edgeEndCounter;

    if (myId == 0) {
        i_edgeStartCounter = (ULONGINT_T*)P_MALLOC(numThread * sizeof(ULONGINT_T));
        assert(i_edgeStartCounter);
        global_i_edgeStartCounter = i_edgeStartCounter;
        i_edgeEndCounter = (ULONGINT_T*)P_MALLOC(numThread * sizeof(ULONGINT_T));
        assert(i_edgeEndCounter);
        global_i_edgeEndCounter = i_edgeEndCounter;
    }

    thread_barrier_wait();

    i_edgeStartCounter = global_i_edgeStartCounter;
    i_edgeEndCounter   = global_i_edgeEndCounter;

    i_edgeEndCounter[myId] = i_edgePtr;
    i_edgeStartCounter[myId] = 0;

    thread_barrier_wait();

    if (myId == 0) {
        for (i = 1; i < numThread; i++) {
            i_edgeEndCounter[i] = i_edgeEndCounter[i-1] + i_edgeEndCounter[i];
            i_edgeStartCounter[i] = i_edgeEndCounter[i-1];
        }
    }

    TM_BEGIN();
    TM_SHARED_WRITE(global_edgeNum,
                    ((long)TM_SHARED_READ(global_edgeNum) + i_edgePtr));
    TM_END();

    thread_barrier_wait();

    long edgeNum = global_edgeNum;

    /*
     * Initialize edge list arrays
     */

    ULONGINT_T* startVertex;
    ULONGINT_T* endVertex;

    if (myId == 0) {
        if (SCALE < 10) {
            long numByte = 2 * edgeNum * sizeof(ULONGINT_T);
            startVertex = (ULONGINT_T*)P_MALLOC(numByte);
            endVertex = (ULONGINT_T*)P_MALLOC(numByte);
        } else {
            long numByte = (edgeNum + MAX_PARAL_EDGES * TOT_VERTICES)
                           * sizeof(ULONGINT_T);
            startVertex = (ULONGINT_T*)P_MALLOC(numByte);
            endVertex = (ULONGINT_T*)P_MALLOC(numByte);
        }
        assert(startVertex);
        assert(endVertex);
        global_startVertex = startVertex;
        global_endVertex = endVertex;
    }

    thread_barrier_wait();

    startVertex = global_startVertex;
    endVertex = global_endVertex;

    for (i = i_edgeStartCounter[myId]; i < i_edgeEndCounter[myId]; i++) {
        startVertex[i] = startV[i-i_edgeStartCounter[myId]];
        endVertex[i] = endV[i-i_edgeStartCounter[myId]];
    }

    ULONGINT_T numEdgesPlacedInCliques = edgeNum;

    thread_barrier_wait();

    /*
     * STEP 3: Connect the cliques
     */

    i_edgePtr = 0;
    p = PROB_INTERCL_EDGES;

    /*
     * Generating inter-clique edges as given in the specs
     */

    createPartition(0, TOT_VERTICES, myId, numThread, &i_start, &i_stop);

    for (i = i_start; i < i_stop; i++) {

        ULONGINT_T tempVertex1 = i;
        long h = totCliques;
        long l = 0;
        long t = -1;
        while (h - l > 1) {
            long m = (h + l) / 2;
            if (tempVertex1 >= firstVsInCliques[m]) {
                l = m;
            } else {
                if ((tempVertex1 < firstVsInCliques[m]) && (m > 0)) {
                    if (tempVertex1 >= firstVsInCliques[m-1]) {
                        t = m - 1;
                        break;
                    } else {
                        h = m;
                    }
                }
            }
        }

        if (t == -1) {
            long m;
            for (m = (l + 1); m < h; m++) {
                if (tempVertex1<firstVsInCliques[m]) {
                    break;
                }
            }
            t = m-1;
        }

        long t1 = firstVsInCliques[t];

        ULONGINT_T d;
        for (d = 1, p = PROB_INTERCL_EDGES; d < TOT_VERTICES; d *= 2, p /= 2) {

            float r = (float)(PRANDOM_GENERATE(stream) % 1000) / (float)1000;

            if (r <= p) {

                ULONGINT_T tempVertex2 = (i+d) % TOT_VERTICES;

                h = totCliques;
                l = 0;
                t = -1;
                while (h - l > 1) {
                    long m = (h + l) / 2;
                    if (tempVertex2 >= firstVsInCliques[m]) {
                        l = m;
                    } else {
                        if ((tempVertex2 < firstVsInCliques[m]) && (m > 0)) {
                            if (firstVsInCliques[m-1] <= tempVertex2) {
                                t = m - 1;
                                break;
                            } else {
                                h = m;
                            }
                        }
                    }
                }

                if (t == -1) {
                    long m;
                    for (m = (l + 1); m < h; m++) {
                        if (tempVertex2 < firstVsInCliques[m]) {
                            break;
                        }
                    }
                    t = m - 1;
                }

                long t2 = firstVsInCliques[t];

                if (t1 != t2) {
                    long randNumEdges =
                        PRANDOM_GENERATE(stream) % MAX_PARAL_EDGES + 1;
                    long j;
                    for (j = 0; j < randNumEdges; j++) {
                        startV[i_edgePtr] = tempVertex1;
                        endV[i_edgePtr] = tempVertex2;
                        i_edgePtr++;
                    }
                }

            } /* r <= p */

            float r0 = (float)(PRANDOM_GENERATE(stream) % 1000) / (float)1000;

            if ((r0 <= p) && (i-d>=0)) {

                ULONGINT_T tempVertex2 = (i-d) % TOT_VERTICES;

                h = totCliques;
                l = 0;
                t = -1;
                while (h - l > 1) {
                    long m = (h + l) / 2;
                    if (tempVertex2 >= firstVsInCliques[m]) {
                        l = m;
                    } else {
                        if ((tempVertex2 < firstVsInCliques[m]) && (m > 0)) {
                            if (firstVsInCliques[m-1] <= tempVertex2) {
                                t = m - 1;
                                break;
                            } else {
                                h = m;
                            }
                        }
                    }
                }

                if (t == -1) {
                    long m;
                    for (m = (l + 1); m < h; m++) {
                        if (tempVertex2 < firstVsInCliques[m]) {
                            break;
                        }
                    }
                    t = m - 1;
                }

                long t2 = firstVsInCliques[t];

                if (t1 != t2) {
                    long randNumEdges =
                        PRANDOM_GENERATE(stream) % MAX_PARAL_EDGES + 1;
                    long j;
                    for (j = 0; j < randNumEdges; j++) {
                        startV[i_edgePtr] = tempVertex1;
                        endV[i_edgePtr] = tempVertex2;
                        i_edgePtr++;
                    }
                }

            } /* r0 <= p && (i-d) > 0 */

        } /* for d, p */

    } /* for i */


    i_edgeEndCounter[myId] = i_edgePtr;
    i_edgeStartCounter[myId] = 0;

    if (myId == 0) {
        global_edgeNum = 0;
    }

    thread_barrier_wait();

    if (myId == 0) {
        for (i = 1; i < numThread; i++) {
            i_edgeEndCounter[i] = i_edgeEndCounter[i-1] + i_edgeEndCounter[i];
            i_edgeStartCounter[i] = i_edgeEndCounter[i-1];
        }
    }

    TM_BEGIN();
    TM_SHARED_WRITE(global_edgeNum,
                    ((long)TM_SHARED_READ(global_edgeNum) + i_edgePtr));
    TM_END();


    thread_barrier_wait();

    edgeNum = global_edgeNum;
    ULONGINT_T numEdgesPlacedOutside = global_edgeNum;

    for (i = i_edgeStartCounter[myId]; i < i_edgeEndCounter[myId]; i++) {
        startVertex[i+numEdgesPlacedInCliques] = startV[i-i_edgeStartCounter[myId]];
        endVertex[i+numEdgesPlacedInCliques] = endV[i-i_edgeStartCounter[myId]];
    }

    thread_barrier_wait();

    ULONGINT_T  numEdgesPlaced = numEdgesPlacedInCliques + numEdgesPlacedOutside;

    if (myId == 0) {

        SDGdataPtr->numEdgesPlaced = numEdgesPlaced;

        printf("Finished generating edges\n");
        printf("No. of intra-clique edges - %lu\n", numEdgesPlacedInCliques);
        printf("No. of inter-clique edges - %lu\n", numEdgesPlacedOutside);
        printf("Total no. of edges        - %lu\n", numEdgesPlaced);

        P_FREE(i_edgeStartCounter);
        P_FREE(i_edgeEndCounter);

        P_FREE(cliqueSizes);
        P_FREE(firstVsInCliques);
        P_FREE(lastVsInCliques);
    }

    thread_barrier_wait();

    P_FREE(startV);
    P_FREE(endV);

    /*
     * STEP 4: Generate edge weights
     */

    if (myId == 0) {
        SDGdataPtr->intWeight =
            (LONGINT_T*)P_MALLOC(numEdgesPlaced * sizeof(LONGINT_T));
        assert(SDGdataPtr->intWeight);
    }

    thread_barrier_wait();

    p = PERC_INT_WEIGHTS;
    ULONGINT_T numStrWtEdges  = 0;

    createPartition(0, numEdgesPlaced, myId, numThread, &i_start, &i_stop);

    for (i = i_start; i < i_stop; i++) {
        float r = (float)(PRANDOM_GENERATE(stream) % 1000) / (float)1000;
        if (r <= p) {
            SDGdataPtr->intWeight[i] =
                1 + (PRANDOM_GENERATE(stream) % (MAX_INT_WEIGHT-1));
        } else {
            SDGdataPtr->intWeight[i] = -1;
            numStrWtEdges++;
        }
    }

    thread_barrier_wait();

    if (myId == 0) {
        long t = 0;
        for (i = 0; i < numEdgesPlaced; i++) {
            if (SDGdataPtr->intWeight[i] < 0) {
                SDGdataPtr->intWeight[i] = -t;
                t++;
            }
        }
    }

    TM_BEGIN();
    TM_SHARED_WRITE(global_numStrWtEdges,
                    ((long)TM_SHARED_READ(global_numStrWtEdges) + numStrWtEdges));
    TM_END();

    thread_barrier_wait();

    numStrWtEdges = global_numStrWtEdges;

    if (myId == 0) {
        SDGdataPtr->strWeight =
            (char*)P_MALLOC(numStrWtEdges * MAX_STRLEN * sizeof(char));
        assert(SDGdataPtr->strWeight);
    }

    thread_barrier_wait();

    createPartition(0, numEdgesPlaced, myId, numThread, &i_start, &i_stop);

    for (i = i_start; i < i_stop; i++) {
        if (SDGdataPtr->intWeight[i] <= 0) {
            long j;
            for (j = 0; j < MAX_STRLEN; j++) {
                SDGdataPtr->strWeight[(-SDGdataPtr->intWeight[i])*MAX_STRLEN+j] =
                    (char) (1 + PRANDOM_GENERATE(stream) % 127);
            }
        }
    }

    /*
     * Choose SOUGHT STRING randomly if not assigned
     */

    if (myId == 0) {

        if (strlen(SOUGHT_STRING) != MAX_STRLEN) {
            SOUGHT_STRING = (char*)P_MALLOC(MAX_STRLEN * sizeof(char));
            assert(SOUGHT_STRING);
        }

        long t = PRANDOM_GENERATE(stream) % numStrWtEdges;
        long j;
        for (j = 0; j < MAX_STRLEN; j++) {
            SOUGHT_STRING[j] =
                (char) ((long) SDGdataPtr->strWeight[t*MAX_STRLEN+j]);
        }

    }

    thread_barrier_wait();

    /*
     * STEP 5: Permute Vertices
     */

    for (i = i_start; i < i_stop; i++) {
        startVertex[i] = permV[(startVertex[i])];
        endVertex[i] = permV[(endVertex[i])];
    }

    thread_barrier_wait();

    /*
     * STEP 6: Sort Vertices
     */

    /*
     * Radix sort with StartVertex as primary key
     */

    if (myId == 0) {
        long numByte = numEdgesPlaced * sizeof(ULONGINT_T);
        SDGdataPtr->startVertex = (ULONGINT_T*)P_MALLOC(numByte);
        assert(SDGdataPtr->startVertex);
        SDGdataPtr->endVertex = (ULONGINT_T*)P_MALLOC(numByte);
        assert(SDGdataPtr->endVertex);
    }

    thread_barrier_wait();

    all_radixsort_node_aux_s3(numEdgesPlaced,
                              startVertex,
                              SDGdataPtr->startVertex,
                              endVertex,
                              SDGdataPtr->endVertex);

    thread_barrier_wait();

    if (myId == 0) {
        P_FREE(startVertex);
        P_FREE(endVertex);
    }

    thread_barrier_wait();

    if (SCALE < 12) {

        /*
         * Sort with endVertex as secondary key
         */

        if (myId == 0) {

            long i0 = 0;
            long i1 = 0;
            i = 0;

            while (i < numEdgesPlaced) {

                for (i = i0; i < numEdgesPlaced; i++) {
                    if (SDGdataPtr->startVertex[i] !=
                        SDGdataPtr->startVertex[i1])
                    {
                        i1 = i;
                        break;
                    }
                }

                long j;
                for (j = i0; j < i1; j++) {
                    long k;
                    for (k = j+1; k < i1; k++) {
                        if (SDGdataPtr->endVertex[k] <
                            SDGdataPtr->endVertex[j])
                        {
                            long t = SDGdataPtr->endVertex[j];
                            SDGdataPtr->endVertex[j] = SDGdataPtr->endVertex[k];
                            SDGdataPtr->endVertex[k] = t;
                        }
                    }
                }

                if (SDGdataPtr->startVertex[i0] != TOT_VERTICES-1) {
                    i0 = i1;
                } else {
                    long j;
                    for (j=i0; j<numEdgesPlaced; j++) {
                        long k;
                        for (k=j+1; k<numEdgesPlaced; k++) {
                            if (SDGdataPtr->endVertex[k] <
                                SDGdataPtr->endVertex[j])
                            {
                                long t = SDGdataPtr->endVertex[j];
                                SDGdataPtr->endVertex[j] = SDGdataPtr->endVertex[k];
                                SDGdataPtr->endVertex[k] = t;
                            }
                        }
                    }
                }

            } /* while i < numEdgesPlaced */

        }

    } else {

        ULONGINT_T* tempIndex;

        if (myId == 0) {

            tempIndex =
                (ULONGINT_T*)P_MALLOC((TOT_VERTICES + 1) * sizeof(ULONGINT_T));
            assert(tempIndex);
            global_tempIndex = tempIndex;

            /*
             * Update degree of each vertex
             */

            tempIndex[0] = 0;
            tempIndex[TOT_VERTICES] = numEdgesPlaced;
            long i0 = 0;

            for (i=0; i < TOT_VERTICES; i++) {
                tempIndex[i+1] = tempIndex[i];
                long j;
                for (j = i0; j < numEdgesPlaced; j++) {
                    if (SDGdataPtr->startVertex[j] !=
                        SDGdataPtr->startVertex[i0])
                    {
                        if (SDGdataPtr->startVertex[i0] == i) {
                            tempIndex[i+1] = j;
                            i0 = j;
                            break;
                        }
                    }
                }
            }
        }

        thread_barrier_wait();

        tempIndex = global_tempIndex;

        /*
         * Insertion sort for now, replace with something better later on
         */
#if 0
        createPartition(0, TOT_VERTICES, myId, numThread, &i_start, &i_stop);

        for (i = i_start; i < i_stop; i++) {
            long j;
            for (j = tempIndex[i]; j < tempIndex[i+1]; j++) {
                long k;
                for (k = (j + 1); k < tempIndex[i+1]; k++) {
                    if (SDGdataPtr->endVertex[k] <
                        SDGdataPtr->endVertex[j])
                    {
                        long t = SDGdataPtr->endVertex[j];
                        SDGdataPtr->endVertex[j] = SDGdataPtr->endVertex[k];
                        SDGdataPtr->endVertex[k] = t;
                    }
                }
            }
        }
#else
        if (myId == 0) {
            for (i = 0; i < TOT_VERTICES; i++) {
                long j;
                for (j = tempIndex[i]; j < tempIndex[i+1]; j++) {
                    long k;
                    for (k = (j + 1); k < tempIndex[i+1]; k++) {
                        if (SDGdataPtr->endVertex[k] <
                            SDGdataPtr->endVertex[j])
                        {
                            long t = SDGdataPtr->endVertex[j];
                            SDGdataPtr->endVertex[j] = SDGdataPtr->endVertex[k];
                            SDGdataPtr->endVertex[k] = t;
                        }
                    }
                }
            }
        }
#endif

        if (myId == 0) {
            P_FREE(tempIndex);
        }

    } /* SCALE >= 12 */

    PRANDOM_FREE(stream);
    if (myId == 0) {
        P_FREE(permV);
    }

    TM_THREAD_EXIT();
}


/* =============================================================================
 *
 * End of genScalData.c
 *
 * =============================================================================
 */
