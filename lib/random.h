/* =============================================================================
 *
 * random.h
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


#ifndef RANDOM_H
#define RANDOM_H 1


#include "mt19937ar.h"


#ifdef __cplusplus
extern "C" {
#endif


#define RANDOM_DEFAULT_SEED (0)

typedef struct random {
    unsigned long (*rand)(unsigned long*, unsigned long*);
    unsigned long mt[N];
    unsigned long mti;
} random_t;


/* =============================================================================
 * random_alloc
 * -- allocates and initialize datastructure
 * -- Returns NULL if failure
 * =============================================================================
 */
random_t*
random_alloc ();


/* =============================================================================
 * Prandom_alloc
 * -- allocates and initialize datastructure
 * -- Returns NULL if failure
 * =============================================================================
 */
random_t*
Prandom_alloc ();


/* =============================================================================
 * random_free
 * =============================================================================
 */
void
random_free (random_t* randomPtr);


/* =============================================================================
 * Prandom_free
 * =============================================================================
 */
void
Prandom_free (random_t* randomPtr);


/* =============================================================================
 * random_seed
 * =============================================================================
 */
void
random_seed (random_t* randomPtr, unsigned long seed);


/* =============================================================================
 * random_generate
 * =============================================================================
 */
unsigned long
random_generate (random_t* randomPtr);


#define PRANDOM_ALLOC()                 Prandom_alloc()
#define PRANDOM_FREE(r)                 Prandom_free(r)
#define PRANDOM_SEED(r, s)              random_seed(r, s)
#define PRANDOM_GENERATE(r)             random_generate(r)


#ifdef __cplusplus
}
#endif


#endif /* RANDOM_H */


/* =============================================================================
 *
 * End of random.h
 *
 * =============================================================================
 */
