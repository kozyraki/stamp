/* =============================================================================
 *
 * getUserParameters.c
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
#include "getUserParameters.h"
#include "globals.h"


/* =============================================================================
 * displayUsage
 * =============================================================================
 */
static void
displayUsage (const char* appName)
{
    printf("Usage: %s [options]\n", appName);
    puts("\nOptions:                                       (defaults)\n");
    printf("    i <FLT>    Probability [i]nter-clique      (%f)\n",  PROB_INTERCL_EDGES);
    printf("    k <UINT>   [k]ind: 0=array 1=list 2=vector (%li)\n", K3_DS);
    printf("    l <UINT>   Max path [l]ength               (%li)\n", SUBGR_EDGE_LENGTH);
    printf("    p <UINT>   Max [p]arallel edges            (%li)\n", MAX_PARAL_EDGES);
    printf("    s <UINT>   Problem [s]cale                 (%li)\n", SCALE);
    printf("    t <UINT>   Number of [t]hreads             (%li)\n", THREADS);
    printf("    u <FLT>    Probability [u]nidirectional    (%f)\n",  PROB_UNIDIRECTIONAL);
    printf("    w <FLT>    Fraction integer [w]eights      (%f)\n",  PERC_INT_WEIGHTS);
    exit(1);
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

    while ((opt = getopt(argc, argv, "i:k:l:p:s:t:u:w:")) != -1) {
        switch (opt) {
            case 'i':
                PROB_INTERCL_EDGES = atof(optarg);
                break;
            case 'k':
                K3_DS = atol(optarg);
                assert(K3_DS >= 0 && K3_DS <= 2);
                break;
            case 'l':
                SUBGR_EDGE_LENGTH = atol(optarg);
                break;
            case 'p':
                MAX_PARAL_EDGES = atol(optarg);
                break;
            case 's':
                SCALE = atol(optarg);
                break;
            case 't':
                THREADS = atol(optarg);
                break;
            case 'u':
                PROB_UNIDIRECTIONAL = atof(optarg);
                break;
            case 'w':
                PERC_INT_WEIGHTS = atof(optarg);
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
 * getUserParameters
 * =============================================================================
 */
void
getUserParameters (long argc, char* const argv[])
{
    /*
     * Scalable Data Generator parameters - defaults
     */

    THREADS             = 1;
    SCALE               = 20;              /* binary scaling heuristic */
    MAX_PARAL_EDGES     = 3;               /* between vertices. */
    PERC_INT_WEIGHTS    = 0.6;             /* % int (vs. string) edge weights */
    PROB_UNIDIRECTIONAL = 0.1;
    PROB_INTERCL_EDGES  = 0.5;             /* Init probability link between cliques */

    SUBGR_EDGE_LENGTH   = 3;               /* Kernel 3: max. path length,       */
                                           /* measured by num edges in subgraph */
                                           /* generated from the end Vertex of  */
                                           /* SI and SC lists                   */

    /*
     * Some implementation-specific vars, nothing to do with the specs
     */

    K3_DS               = 2;               /* 0 - Array         */
                                           /* 1 - Linked List   */
                                           /* 2 - Dynamic Array */

    parseArgs(argc, argv); /* overrides default values set above */


    TOT_VERTICES        = (1<<SCALE);
    MAX_CLIQUE_SIZE     = (1<<(SCALE/3));
    MAX_INT_WEIGHT      = (1<<SCALE);      /* Max int value in edge weight */
    MAX_STRLEN          = SCALE;

    SOUGHT_STRING       = "";              /* Kernel 2: Character string sought:  */
                                           /* specify here, else it is picked     */
                                           /* picked from randomly selected entry */
                                           /* in genScalData.c                    */



    MAX_CLUSTER_SIZE    = MAX_CLIQUE_SIZE; /* Kernel 4: Clustering search box size */

}


/* =============================================================================
 *
 * End of getUserParameters.c
 *
 * =============================================================================
 */

