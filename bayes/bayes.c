/* =============================================================================
 *
 * bayes.c
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
#include "data.h"
#include "learner.h"
#include "net.h"
#include "thread.h"
#include "timer.h"
#include "tm.h"
#include "types.h"

enum param_types {
    PARAM_EDGE    = (unsigned char)'e',
    PARAM_INSERT  = (unsigned char)'i',
    PARAM_NUMBER  = (unsigned char)'n',
    PARAM_PERCENT = (unsigned char)'p',
    PARAM_RECORD  = (unsigned char)'r',
    PARAM_SEED    = (unsigned char)'s',
    PARAM_THREAD  = (unsigned char)'t',
    PARAM_VAR     = (unsigned char)'v',
};

enum param_defaults {
    PARAM_DEFAULT_EDGE    = -1,
    PARAM_DEFAULT_INSERT  = 1,
    PARAM_DEFAULT_NUMBER  = 4,
    PARAM_DEFAULT_PERCENT = 10,
    PARAM_DEFAULT_RECORD  = 4096,
    PARAM_DEFAULT_SEED    = 1,
    PARAM_DEFAULT_THREAD  = 1,
    PARAM_DEFAULT_VAR     = 32,
};

#define PARAM_DEFAULT_QUALITY           1.0F

long global_params[256]; /* 256 = ascii limit */
long global_maxNumEdgeLearned = PARAM_DEFAULT_EDGE;
long global_insertPenalty = PARAM_DEFAULT_INSERT;
float global_operationQualityFactor = PARAM_DEFAULT_QUALITY;


/* =============================================================================
 * displayUsage
 * =============================================================================
 */
static void
displayUsage (const char* appName)
{
    printf("Usage: %s [options]\n", appName);
    puts("\nOptions:                                         (defaults)\n");
    printf("    e <UINT>   Max [e]dges learned per variable  (%i)\n",  PARAM_DEFAULT_EDGE);
    printf("    i <UINT>   Edge [i]nsert penalty             (%i)\n",  PARAM_DEFAULT_INSERT);
    printf("    n <UINT>   Max [n]umber of parents           (%i)\n",  PARAM_DEFAULT_NUMBER);
    printf("    p <UINT>   [p]ercent chance of parent        (%i)\n",  PARAM_DEFAULT_PERCENT);
    printf("    q <FLT>    Operation [q]uality factor        (%f)\n",  PARAM_DEFAULT_QUALITY);
    printf("    r <UINT>   Number of [r]ecords               (%i)\n",  PARAM_DEFAULT_RECORD);
    printf("    s <UINT>   Random [s]eed                     (%i)\n",  PARAM_DEFAULT_SEED);
    printf("    t <UINT>   Number of [t]hreads               (%i)\n",  PARAM_DEFAULT_THREAD);
    printf("    v <UINT>   Number of [v]ariables             (%i)\n",  PARAM_DEFAULT_VAR);
    exit(1);
}


/* =============================================================================
 * setDefaultParams
 * =============================================================================
 */
static void
setDefaultParams ()
{
    global_params[PARAM_EDGE]    = PARAM_DEFAULT_EDGE;
    global_params[PARAM_INSERT]  = PARAM_DEFAULT_INSERT;
    global_params[PARAM_NUMBER]  = PARAM_DEFAULT_NUMBER;
    global_params[PARAM_PERCENT] = PARAM_DEFAULT_PERCENT;
    global_params[PARAM_RECORD]  = PARAM_DEFAULT_RECORD;
    global_params[PARAM_SEED]    = PARAM_DEFAULT_SEED;
    global_params[PARAM_THREAD]  = PARAM_DEFAULT_THREAD;
    global_params[PARAM_VAR]     = PARAM_DEFAULT_VAR;
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

    while ((opt = getopt(argc, argv, "e:i:n:p:q:r:s:t:v:")) != -1) {
        switch (opt) {
            case 'e':
            case 'i':
            case 'n':
            case 'p':
            case 'r':
            case 's':
            case 't':
            case 'v':
                global_params[(unsigned char)opt] = atol(optarg);
                break;
            case 'q':
                global_operationQualityFactor = atof(optarg);
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
 * score
 * =============================================================================
 */
static float
score (net_t* netPtr, adtree_t* adtreePtr)
{
    /*
     * Create dummy data structures to conform to learner_score assumptions
     */

    data_t* dataPtr = data_alloc(1, 1, NULL);

    learner_t* learnerPtr = learner_alloc(dataPtr, adtreePtr, 1);

    net_t* tmpNetPtr = learnerPtr->netPtr;
    learnerPtr->netPtr = netPtr;

    float score = learner_score(learnerPtr);

    learnerPtr->netPtr = tmpNetPtr;
    learner_free(learnerPtr);
    data_free(dataPtr);

    return score;
}


/* =============================================================================
 * main
 * =============================================================================
 */
MAIN(argc, argv)
{
    GOTO_REAL();

    /*
     * Initialization
     */

    parseArgs(argc, (char** const)argv);
    long numThread     = global_params[PARAM_THREAD];
    long numVar        = global_params[PARAM_VAR];
    long numRecord     = global_params[PARAM_RECORD];
    long randomSeed    = global_params[PARAM_SEED];
    long maxNumParent  = global_params[PARAM_NUMBER];
    long percentParent = global_params[PARAM_PERCENT];
    global_insertPenalty = global_params[PARAM_INSERT];
    global_maxNumEdgeLearned = global_params[PARAM_EDGE];
    SIM_GET_NUM_CPU(numThread);
    TM_STARTUP(numThread);
    P_MEMORY_STARTUP(numThread);
    thread_startup(numThread);

    printf("Random seed                = %li\n", randomSeed);
    printf("Number of vars             = %li\n", numVar);
    printf("Number of records          = %li\n", numRecord);
    printf("Max num parents            = %li\n", maxNumParent);
    printf("%% chance of parent         = %li\n", percentParent);
    printf("Insert penalty             = %li\n", global_insertPenalty);
    printf("Max num edge learned / var = %li\n", global_maxNumEdgeLearned);
    printf("Operation quality factor   = %f\n", global_operationQualityFactor);
    fflush(stdout);

    /*
     * Generate data
     */

    printf("Generating data... ");
    fflush(stdout);

    random_t* randomPtr = random_alloc();
    assert(randomPtr);
    random_seed(randomPtr, randomSeed);

    data_t* dataPtr = data_alloc(numVar, numRecord, randomPtr);
    assert(dataPtr);
    net_t* netPtr = data_generate(dataPtr, -1, maxNumParent, percentParent);
    puts("done.");
    fflush(stdout);

    /*
     * Generate adtree
     */

    adtree_t* adtreePtr = adtree_alloc();
    assert(adtreePtr);

    printf("Generating adtree... ");
    fflush(stdout);

    TIMER_T adtreeStartTime;
    TIMER_READ(adtreeStartTime);

    adtree_make(adtreePtr, dataPtr);

    TIMER_T adtreeStopTime;
    TIMER_READ(adtreeStopTime);

    puts("done.");
    fflush(stdout);
    printf("Adtree time = %f\n",
           TIMER_DIFF_SECONDS(adtreeStartTime, adtreeStopTime));
    fflush(stdout);

    /*
     * Score original network
     */

    float actualScore = score(netPtr, adtreePtr);
    net_free(netPtr);

    /*
     * Learn structure of Bayesian network
     */

    learner_t* learnerPtr = learner_alloc(dataPtr, adtreePtr, numThread);
    assert(learnerPtr);
    data_free(dataPtr); /* save memory */

    printf("Learning structure...");
    fflush(stdout);

    TIMER_T learnStartTime;
    TIMER_READ(learnStartTime);
    GOTO_SIM();

    learner_run(learnerPtr);

    GOTO_REAL();
    TIMER_T learnStopTime;
    TIMER_READ(learnStopTime);

    puts("done.");
    fflush(stdout);
    printf("Learn time = %f\n",
           TIMER_DIFF_SECONDS(learnStartTime, learnStopTime));
    fflush(stdout);

    /*
     * Check solution
     */

    bool_t status = net_isCycle(learnerPtr->netPtr);
    assert(!status);

#ifndef SIMULATOR
    float learnScore = learner_score(learnerPtr);
    printf("Learn score  = %f\n", learnScore);
#endif
    printf("Actual score = %f\n", actualScore);

    /*
     * Clean up
     */

    fflush(stdout);
    random_free(randomPtr);
#ifndef SIMULATOR
    adtree_free(adtreePtr);
#  if 0    
    learner_free(learnerPtr);
#  endif    
#endif

    TM_SHUTDOWN();
    P_MEMORY_SHUTDOWN();

    GOTO_SIM();

    thread_shutdown();

    MAIN_RETURN(0);
}


/* =============================================================================
 *
 * End of bayes.c
 *
 * =============================================================================
 */
