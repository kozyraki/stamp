/* =============================================================================
 *
 * ssca2.c
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
#include "computeGraph.h"
#include "cutClusters.h"
#include "defs.h"
#include "findSubGraphs.h"
#include "genScalData.h"
#include "getStartLists.h"
#include "getUserParameters.h"
#include "globals.h"
#include "timer.h"
#include "thread.h"
#include "tm.h"


MAIN(argc, argv)
{
    GOTO_REAL();

    /*
     * Tuple for Scalable Data Generation
     * stores startVertex, endVertex, long weight and other info
     */
    graphSDG* SDGdata;

    /*
     * The graph data structure for this benchmark - see defs.h
     */
    graph* G;

#ifdef ENABLE_KERNEL2

    /*
     * Kernel 2
     */
    edge* maxIntWtList;
    edge* soughtStrWtList;
    long maxIntWtListSize;
    long soughtStrWtListSize;

#endif /* ENABLE_KERNEL2 */

#ifdef ENABLE_KERNEL3

#  ifndef ENABLE_KERNEL2
#    error KERNEL3 requires KERNEL2
#  endif

    /*
     * Kernel 3
     */
    V*   intWtVList  = NULL;
    V*   strWtVList  = NULL;
    Vl** intWtVLList = NULL;
    Vl** strWtVLList = NULL;
    Vd*  intWtVDList = NULL;
    Vd*  strWtVDList = NULL;

#endif /* ENABLE_KERNEL3 */

    double totalTime = 0.0;

    /* -------------------------------------------------------------------------
     * Preamble
     * -------------------------------------------------------------------------
     */

    /*
     * User Interface: Configurable parameters, and global program control
     */

    printf("\nHPCS SSCA #2 Graph Analysis Executable Specification:");
    printf("\nRunning...\n\n");

    getUserParameters(argc, (char** const) argv);

    SIM_GET_NUM_CPU(THREADS);
    TM_STARTUP(THREADS);
    P_MEMORY_STARTUP(THREADS);
    thread_startup(THREADS);

    puts("");
    printf("Number of processors:       %ld\n", THREADS);
    printf("Problem Scale:              %ld\n", SCALE);
    printf("Max parallel edges:         %ld\n", MAX_PARAL_EDGES);
    printf("Percent int weights:        %f\n",  PERC_INT_WEIGHTS);
    printf("Probability unidirectional: %f\n",  PROB_UNIDIRECTIONAL);
    printf("Probability inter-clique:   %f\n",  PROB_INTERCL_EDGES);
    printf("Subgraph edge length:       %ld\n", SUBGR_EDGE_LENGTH);
    printf("Kernel 3 data structure:    %ld\n", K3_DS);
    puts("");

    /*
     * Scalable Data Generator
     */

    printf("\nScalable Data Generator - genScalData() beginning execution...\n");

    SDGdata = (graphSDG*)malloc(sizeof(graphSDG));
    assert(SDGdata);

    TIMER_T start;
    TIMER_READ(start);

#ifdef USE_PARALLEL_DATA_GENERATION
    GOTO_SIM();
#ifdef OTM
#pragma omp parallel
    {
        genScalData((void*)SDGdata);
    }
#else
    thread_start(genScalData, (void*)SDGdata);
#endif
    GOTO_REAL();
#else /* !USE_PARALLEL_DATA_GENERATION */
    genScalData_seq(SDGdata);
#endif /* !USE_PARALLEL_DATA_GENERATION */

    TIMER_T stop;
    TIMER_READ(stop);

    double time = TIMER_DIFF_SECONDS(start, stop);
    totalTime += time;

    printf("\nTime taken for Scalable Data Generation is %9.6f sec.\n\n", time);
    printf("\n\tgenScalData() completed execution.\n");


#ifdef ENABLE_KERNEL1

    /* -------------------------------------------------------------------------
     * Kernel 1 - Graph Construction
     *
     * From the input edges, construct the graph 'G'
     * -------------------------------------------------------------------------
     */

    printf("\nKernel 1 - computeGraph() beginning execution...\n");

    G = (graph*)malloc(sizeof(graph));
    assert(G);

    computeGraph_arg_t computeGraphArgs;
    computeGraphArgs.GPtr       = G;
    computeGraphArgs.SDGdataPtr = SDGdata;

    TIMER_READ(start);

    GOTO_SIM();
#ifdef OTM
#pragma omp parallel
    {
        computeGraph((void*)&computeGraphArgs);
    }
#else
    thread_start(computeGraph, (void*)&computeGraphArgs);
#endif
    GOTO_REAL();

    TIMER_READ(stop);

    time = TIMER_DIFF_SECONDS(start, stop);
    totalTime += time;

    printf("\n\tcomputeGraph() completed execution.\n");
    printf("\nTime taken for kernel 1 is %9.6f sec.\n", time);

#endif /* ENABLE_KERNEL1 */

#ifdef ENABLE_KERNEL2

    /* -------------------------------------------------------------------------
     * Kernel 2 - Find Max weight and sought string
     * -------------------------------------------------------------------------
     */

    printf("\nKernel 2 - getStartLists() beginning execution...\n");

    maxIntWtListSize = 0;
    soughtStrWtListSize = 0;
    maxIntWtList = (edge*)malloc(sizeof(edge));
    assert(maxIntWtList);
    soughtStrWtList = (edge*)malloc(sizeof(edge));
    assert(soughtStrWtList);

    getStartLists_arg_t getStartListsArg;
    getStartListsArg.GPtr                = G;
    getStartListsArg.maxIntWtListPtr     = &maxIntWtList;
    getStartListsArg.maxIntWtListSize    = &maxIntWtListSize;
    getStartListsArg.soughtStrWtListPtr  = &soughtStrWtList;
    getStartListsArg.soughtStrWtListSize = &soughtStrWtListSize;

    TIMER_READ(start);

    GOTO_SIM();
#ifdef OTM
#pragma omp parallel
    {
        getStartLists((void*)&getStartListsArg);
    }
#else
    thread_start(getStartLists, (void*)&getStartListsArg);
#endif
    GOTO_REAL();

    TIMER_READ(stop);

    time = TIMER_DIFF_SECONDS(start, stop);
    totalTime += time;

    printf("\n\tgetStartLists() completed execution.\n");
    printf("\nTime taken for kernel 2 is %9.6f sec.\n\n", time);

#endif /* ENABLE_KERNEL2 */

#ifdef ENABLE_KERNEL3

    /* -------------------------------------------------------------------------
     * Kernel 3 - Graph Extraction
     * -------------------------------------------------------------------------
     */

    printf("\nKernel 3 - findSubGraphs() beginning execution...\n");

    if (K3_DS == 0) {

        intWtVList = (V*)malloc(G->numVertices * maxIntWtListSize * sizeof(V));
        assert(intWtVList);
        strWtVList = (V*)malloc(G->numVertices * soughtStrWtListSize * sizeof(V));
        assert(strWtVList);

        findSubGraphs0_arg_t findSubGraphs0Arg;
        findSubGraphs0Arg.GPtr                = G;
        findSubGraphs0Arg.intWtVList          = intWtVList;
        findSubGraphs0Arg.strWtVList          = strWtVList;
        findSubGraphs0Arg.maxIntWtList        = maxIntWtList;
        findSubGraphs0Arg.maxIntWtListSize    = maxIntWtListSize;
        findSubGraphs0Arg.soughtStrWtList     = soughtStrWtList;
        findSubGraphs0Arg.soughtStrWtListSize = soughtStrWtListSize;

        TIMER_READ(start);

        GOTO_SIM();
#ifdef OTM
#pragma omp parallel
        {
            findSubGraphs0((void*)&findSubGraphs0Arg);
        }
#else
        thread_start(findSubGraphs0, (void*)&findSubGraphs0Arg);
#endif
        GOTO_REAL();

        TIMER_READ(stop);

    } else if (K3_DS == 1) {

        intWtVLList = (Vl**)malloc(maxIntWtListSize * sizeof(Vl*));
        assert(intWtVLList);
        strWtVLList = (Vl**)malloc(soughtStrWtListSize * sizeof(Vl*));
        assert(strWtVLList);

        findSubGraphs1_arg_t findSubGraphs1Arg;
        findSubGraphs1Arg.GPtr                = G;
        findSubGraphs1Arg.intWtVLList         = intWtVLList;
        findSubGraphs1Arg.strWtVLList         = strWtVLList;
        findSubGraphs1Arg.maxIntWtList        = maxIntWtList;
        findSubGraphs1Arg.maxIntWtListSize    = maxIntWtListSize;
        findSubGraphs1Arg.soughtStrWtList     = soughtStrWtList;
        findSubGraphs1Arg.soughtStrWtListSize = soughtStrWtListSize;

        TIMER_READ(start);

        GOTO_SIM();
#ifdef OTM
#pragma omp parallel
        {
            findSubGraphs1((void*)&findSubGraphs1Arg);
        }
#else
        thread_start(findSubGraphs1, (void*)&findSubGraphs1Arg);
#endif
        GOTO_REAL();

        TIMER_READ(stop);

        /*  Verification
        on_one_thread {
          for (i=0; i<maxIntWtListSize; i++) {
            printf("%ld -- ", i);
            currV = intWtVLList[i];
            while (currV != NULL) {
              printf("[%ld %ld] ", currV->num, currV->depth);
              currV = currV->next;
            }
            printf("\n");
          }

          for (i=0; i<soughtStrWtListSize; i++) {
            printf("%ld -- ", i);
            currV = strWtVLList[i];
            while (currV != NULL) {
              printf("[%ld %ld] ", currV->num, currV->depth);
              currV = currV->next;
            }
            printf("\n");
          }

        }
        */

    } else if (K3_DS == 2) {

        intWtVDList = (Vd *) malloc(maxIntWtListSize * sizeof(Vd));
        assert(intWtVDList);
        strWtVDList = (Vd *) malloc(soughtStrWtListSize * sizeof(Vd));
        assert(strWtVDList);

        findSubGraphs2_arg_t findSubGraphs2Arg;
        findSubGraphs2Arg.GPtr                = G;
        findSubGraphs2Arg.intWtVDList         = intWtVDList;
        findSubGraphs2Arg.strWtVDList         = strWtVDList;
        findSubGraphs2Arg.maxIntWtList        = maxIntWtList;
        findSubGraphs2Arg.maxIntWtListSize    = maxIntWtListSize;
        findSubGraphs2Arg.soughtStrWtList     = soughtStrWtList;
        findSubGraphs2Arg.soughtStrWtListSize = soughtStrWtListSize;

        TIMER_READ(start);

        GOTO_SIM();
#ifdef OTM
#pragma omp parallel
        {
            findSubGraphs2((void*)&findSubGraphs2Arg);
        }
#else
        thread_start(findSubGraphs2, (void*)&findSubGraphs2Arg);
#endif
        GOTO_REAL();

        TIMER_READ(stop);

        /* Verification */
        /*
        on_one_thread {
          printf("\nInt weight sub-graphs \n");
          for (i=0; i<maxIntWtListSize; i++) {
            printf("%ld -- ", i);
            for (j=0; j<intWtVDList[i].numArrays; j++) {
              printf("\n [Array %ld] - \n", j);
              for (k=0; k<intWtVDList[i].arraySize[j]; k++) {
                printf("[%ld %ld] ", intWtVDList[i].vList[j][k].num, intWtVDList[i].vList[j][k].depth);
              }

            }
            printf("\n");
          }

          printf("\nStr weight sub-graphs \n");
          for (i=0; i<soughtStrWtListSize; i++) {
            printf("%ld -- ", i);
            for (j=0; j<strWtVDList[i].numArrays; j++) {
              printf("\n [Array %ld] - \n", j);
              for (k=0; k<strWtVDList[i].arraySize[j]; k++) {
                printf("[%ld %ld] ", strWtVDList[i].vList[j][k].num, strWtVDList[i].vList[j][k].depth);
              }

            }
            printf("\n");
          }

        }
       */

    } else {

        assert(0);

    }

    time = TIMER_DIFF_SECONDS(start, stop);
    totalTime += time;

    printf("\n\tfindSubGraphs() completed execution.\n");
    printf("\nTime taken for kernel 3 is %9.6f sec.\n\n", time);

#endif /* ENABLE_KERNEL3 */

#ifdef ENABLE_KERNEL4

    /* -------------------------------------------------------------------------
     * Kernel 4 - Graph Clustering
     * -------------------------------------------------------------------------
     */

    printf("\nKernel 4 - cutClusters() beginning execution...\n");

    TIMER_READ(start);

    GOTO_SIM();
#ifdef OTM
#pragma omp parallel
    {
        cutClusters((void*)G);
    }
#else
    thread_start(cutClusters, (void*)G);
#endif
    GOTO_REAL();

    TIMER_READ(stop);

    time = TIMER_DIFF_SECONDS(start, stop);
    totalTime += time;

    printf("\n\tcutClusters() completed execution.\n");
    printf("\nTime taken for Kernel 4 is %9.6f sec.\n\n", time);

#endif /* ENABLE_KERNEL4 */

    printf("\nTime taken for all is %9.6f sec.\n\n", totalTime);

    /* -------------------------------------------------------------------------
     * Cleanup
     * -------------------------------------------------------------------------
     */

    P_FREE(G->outDegree);
    P_FREE(G->outVertexIndex);
    P_FREE(G->outVertexList);
    P_FREE(G->paralEdgeIndex);
    P_FREE(G->inDegree);
    P_FREE(G->inVertexIndex);
    P_FREE(G->inVertexList);
    P_FREE(G->intWeight);
    P_FREE(G->strWeight);

#ifdef ENABLE_KERNEL3

    LONGINT_T i;
    LONGINT_T j;
    Vl* currV;
    Vl* tempV;

    if (K3_DS == 0) {
        P_FREE(strWtVList);
        P_FREE(intWtVList);
    }

    if (K3_DS == 1) {
        for (i = 0; i < maxIntWtListSize; i++) {
            currV = intWtVLList[i];
            while (currV != NULL) {
                tempV = currV->next;
                P_FREE(currV);
                currV = tempV;
            }
        }
        for (i = 0; i < soughtStrWtListSize; i++) {
            currV = strWtVLList[i];
            while (currV != NULL) {
                tempV = currV->next;
                P_FREE(currV);
                currV = tempV;
            }
        }
        P_FREE(strWtVLList);
        P_FREE(intWtVLList);
    }

    if (K3_DS == 2) {
        for (i = 0; i < maxIntWtListSize; i++) {
            for (j = 0; j < intWtVDList[i].numArrays; j++) {
                P_FREE(intWtVDList[i].vList[j]);
            }
            P_FREE(intWtVDList[i].vList);
            P_FREE(intWtVDList[i].arraySize);
        }
        for (i = 0; i < soughtStrWtListSize; i++) {
            for (j = 0; j < strWtVDList[i].numArrays; j++) {
                P_FREE(strWtVDList[i].vList[j]);
            }
            P_FREE(strWtVDList[i].vList);
            P_FREE(strWtVDList[i].arraySize);
        }
        P_FREE(strWtVDList);
        P_FREE(intWtVDList);
    }

    P_FREE(soughtStrWtList);
    P_FREE(maxIntWtList);

#endif /* ENABLE_KERNEL2 */

    P_FREE(SOUGHT_STRING);
    P_FREE(G);
    P_FREE(SDGdata);

    TM_SHUTDOWN();
    P_MEMORY_SHUTDOWN();

    GOTO_SIM();

    thread_shutdown();

    MAIN_RETURN(0);
}


/* =============================================================================
 *
 * End of ssca2.c
 *
 * =============================================================================
 */
