/* =============================================================================
 *
 * genome.c
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
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gene.h"
#include "random.h"
#include "segments.h"
#include "sequencer.h"
#include "thread.h"
#include "timer.h"
#include "tm.h"
#include "vector.h"


enum param_types {
    PARAM_GENE    = (unsigned char)'g',
    PARAM_NUMBER  = (unsigned char)'n',
    PARAM_SEGMENT = (unsigned char)'s',
    PARAM_THREAD  = (unsigned char)'t',
};


#define PARAM_DEFAULT_GENE    (1L << 14)
#define PARAM_DEFAULT_NUMBER  (1L << 22)
#define PARAM_DEFAULT_SEGMENT (1L << 6)
#define PARAM_DEFAULT_THREAD  (1L)


long global_params[256]; /* 256 = ascii limit */


/* =============================================================================
 * displayUsage
 * =============================================================================
 */
static void
displayUsage (const char* appName)
{
    printf("Usage: %s [options]\n", appName);
    puts("\nOptions:                                (defaults)\n");
    printf("    g <UINT>   Length of [g]ene         (%li)\n", PARAM_DEFAULT_GENE);
    printf("    n <UINT>   Min [n]umber of segments (%li)\n", PARAM_DEFAULT_NUMBER);
    printf("    s <UINT>   Length of [s]egment      (%li)\n", PARAM_DEFAULT_SEGMENT);
    printf("    t <UINT>   Number of [t]hreads      (%li)\n", PARAM_DEFAULT_THREAD);
    puts("");
    puts("The actual number of segments created may be greater than -n");
    puts("in order to completely cover the gene.");
    exit(1);
}


/* =============================================================================
 * setDefaultParams
 * =============================================================================
 */
static void
setDefaultParams( void )
{
    global_params[PARAM_GENE]    = PARAM_DEFAULT_GENE;
    global_params[PARAM_NUMBER]  = PARAM_DEFAULT_NUMBER;
    global_params[PARAM_SEGMENT] = PARAM_DEFAULT_SEGMENT;
    global_params[PARAM_THREAD]  = PARAM_DEFAULT_THREAD;
}


/* =============================================================================
 * parseArgs
 * =============================================================================
 */
static void
parseArgs (long argc, char* const argv[])
{
    long i;
    long opt;

    opterr = 0;

    setDefaultParams();

    while ((opt = getopt(argc, argv, "g:n:s:t:")) != -1) {
        switch (opt) {
            case 'g':
            case 'n':
            case 's':
            case 't':
                global_params[(unsigned char)opt] = atol(optarg);
                break;
            case '?':
            default:
                opterr++;
                break;
        }
    }

    for (i = optind; i < argc; i++) {
        fprintf(stderr, "Non-option argument: %s\n", argv[i]);
        opterr++;
    }

    if (opterr) {
        displayUsage(argv[0]);
    }
}


/* =============================================================================
 * main
 * =============================================================================
 */
MAIN (argc,argv)
{
    TIMER_T start;
    TIMER_T stop;

    GOTO_REAL();

    /* Initialization */
    parseArgs(argc, (char** const)argv);
    SIM_GET_NUM_CPU(global_params[PARAM_THREAD]);

    printf("Creating gene and segments... ");
    fflush(stdout);

    long geneLength = global_params[PARAM_GENE];
    long segmentLength = global_params[PARAM_SEGMENT];
    long minNumSegment = global_params[PARAM_NUMBER];
    long numThread = global_params[PARAM_THREAD];

    TM_STARTUP(numThread);
    P_MEMORY_STARTUP(numThread);
    thread_startup(numThread);

    random_t* randomPtr = random_alloc();
    assert(randomPtr != NULL);
    random_seed(randomPtr, 0);

    gene_t* genePtr = gene_alloc(geneLength);
    assert( genePtr != NULL);
    gene_create(genePtr, randomPtr);
    char* gene = genePtr->contents;

    segments_t* segmentsPtr = segments_alloc(segmentLength, minNumSegment);
    assert(segmentsPtr != NULL);
    segments_create(segmentsPtr, genePtr, randomPtr);
    sequencer_t* sequencerPtr = sequencer_alloc(geneLength, segmentLength, segmentsPtr);
    assert(sequencerPtr != NULL);

    puts("done.");
    printf("Gene length     = %li\n", genePtr->length);
    printf("Segment length  = %li\n", segmentsPtr->length);
    printf("Number segments = %li\n", vector_getSize(segmentsPtr->contentsPtr));
    fflush(stdout);

    /* Benchmark */
    printf("Sequencing gene... ");
    fflush(stdout);
    TIMER_READ(start);
    GOTO_SIM();
#ifdef OTM
#pragma omp parallel
    {
        sequencer_run(sequencerPtr);
    }
#else
    thread_start(sequencer_run, (void*)sequencerPtr);
#endif
    GOTO_REAL();
    TIMER_READ(stop);
    puts("done.");
    printf("Time = %lf\n", TIMER_DIFF_SECONDS(start, stop));
    fflush(stdout);

    /* Check result */
    {
        char* sequence = sequencerPtr->sequence;
        int result = strcmp(gene, sequence);
        printf("Sequence matches gene: %s\n", (result ? "no" : "yes"));
        if (result) {
            printf("gene     = %s\n", gene);
            printf("sequence = %s\n", sequence);
        }
        fflush(stdout);
        assert(strlen(sequence) >= strlen(gene));
    }

    /* Clean up */
    printf("Deallocating memory... ");
    fflush(stdout);
    sequencer_free(sequencerPtr);
    segments_free(segmentsPtr);
    gene_free(genePtr);
    random_free(randomPtr);
    puts("done.");
    fflush(stdout);

    TM_SHUTDOWN();
    P_MEMORY_SHUTDOWN();

    GOTO_SIM();

    thread_shutdown();

    MAIN_RETURN(0);
}



/* =============================================================================
 *
 * End of genome.c
 *
 * =============================================================================
 */
