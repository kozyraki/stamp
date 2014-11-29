/* =============================================================================
 *
 * random.c
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


#include <stdlib.h>
#include "mt19937ar.h"
#include "random.h"
#include "tm.h"


/* =============================================================================
 * random_alloc
 * -- Returns NULL if failure
 * =============================================================================
 */
random_t*
random_alloc (void)
{
    random_t* randomPtr = (random_t*)malloc(sizeof(random_t));
    if (randomPtr != NULL) {
        randomPtr->mti = N;
        init_genrand(randomPtr->mt, &(randomPtr->mti), RANDOM_DEFAULT_SEED);
    }

    return randomPtr;
}


/* =============================================================================
 * Prandom_alloc
 * -- Returns NULL if failure
 * =============================================================================
 */
random_t*
Prandom_alloc (void)
{
    random_t* randomPtr = (random_t*)P_MALLOC(sizeof(random_t));
    if (randomPtr != NULL) {
        randomPtr->mti = N;
        init_genrand(randomPtr->mt, &(randomPtr->mti), RANDOM_DEFAULT_SEED);
    }

    return randomPtr;
}


/* =============================================================================
 * random_free
 * =============================================================================
 */
void
random_free (random_t* randomPtr)
{
    free(randomPtr);
}


/* =============================================================================
 * Prandom_free
 * =============================================================================
 */
void
Prandom_free (random_t* randomPtr)
{
    P_FREE(randomPtr);
}


/* =============================================================================
 * random_seed
 * =============================================================================
 */
void
random_seed (random_t* randomPtr, unsigned long seed)
{
    init_genrand(randomPtr->mt, &(randomPtr->mti), seed);
}


/* =============================================================================
 * random_generate
 * =============================================================================
 */
unsigned long
random_generate (random_t* randomPtr)
{
    return genrand_int32(randomPtr->mt, &(randomPtr->mti));
}


/* =============================================================================
 * TEST_RANDOM
 * =============================================================================
 */
#ifdef TEST_RANDOM


#include <stdio.h>
#include <assert.h>
#include "mt19937ar.c"


#define NUM_ITERATIONS 10


int
main ()
{
    random_t* random1Ptr;
    random_t* random2Ptr;
    random_t* random3Ptr;
    long i;

    puts("Starting...");

    random1Ptr = random_alloc();
    random2Ptr = random_alloc();
    random3Ptr = random_alloc();

    random_seed(random2Ptr, (RANDOM_DEFAULT_SEED + 1));
    random_seed(random3Ptr, (RANDOM_DEFAULT_SEED + 1));

    for (i = 0; i < NUM_ITERATIONS; i++) {
        unsigned long rand1 = random_generate(random1Ptr);
        unsigned long rand2 = random_generate(random2Ptr);
        unsigned long rand3 = random_generate(random3Ptr);
        printf("i = %2li, rand1 = %12lu, rand2 = %12lu, rand2 = %12lu\n",
                i, rand1, rand2, rand3);
        assert(rand1 != rand2);
        assert(rand2 == rand3);
    }

    random_free(random1Ptr);
    random_free(random2Ptr);

    puts("Done.");

    return 0;
}


#endif /* TEST_VECTOR */


/* =============================================================================
 *
 * End of random.c
 *
 * =============================================================================
 */
