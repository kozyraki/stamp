/* =============================================================================
 *
 * hashtable.h
 *
 * =============================================================================
 *
 * Copyright (C) Stanford University, 2006.  All Rights Reserved.
 * Author: Chi Cao Minh
 *
 * =============================================================================
 *
 * Options:
 *
 * LIST_NO_DUPLICATES (default: allow duplicates)
 *
 * HASHTABLE_RESIZABLE (enable dynamically increasing number of buckets)
 *
 * HASHTABLE_SIZE_FIELD (size is explicitely stored in
 *     hashtable and not implicitly defined by the sizes of
 *     all bucket lists => more conflicts in case of parallel access)
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


#ifndef HASHTABLE_H
#define HASHTABLE_H 1


#include "list.h"
#include "pair.h"
#include "tm.h"
#include "types.h"


#ifdef __cplusplus
extern "C" {
#endif


enum hashtable_config {
    HASHTABLE_DEFAULT_RESIZE_RATIO  = 3,
    HASHTABLE_DEFAULT_GROWTH_FACTOR = 3
};

typedef struct hashtable {
    list_t** buckets;
    long numBucket;
#ifdef HASHTABLE_SIZE_FIELD
    long size;
#endif
    ulong_t (*hash)(const void*);
    long (*comparePairs)(const pair_t*, const pair_t*);
    long resizeRatio;
    long growthFactor;
    /* comparePairs should return <0 if before, 0 if equal, >0 if after */
} hashtable_t;


typedef struct hashtable_iter {
    long bucket;
    list_iter_t it;
} hashtable_iter_t;


/* =============================================================================
 * hashtable_iter_reset
 * =============================================================================
 */
void
hashtable_iter_reset (hashtable_iter_t* itPtr, hashtable_t* hashtablePtr);


/* =============================================================================
 * TMhashtable_iter_reset
 * =============================================================================
 */
void
TMhashtable_iter_reset (TM_ARGDECL
                        hashtable_iter_t* itPtr, hashtable_t* hashtablePtr);


/* =============================================================================
 * hashtable_iter_hasNext
 * =============================================================================
 */
bool_t
hashtable_iter_hasNext (hashtable_iter_t* itPtr, hashtable_t* hashtablePtr);


/* =============================================================================
 * TMhashtable_iter_hasNext
 * =============================================================================
 */
bool_t
TMhashtable_iter_hasNext (TM_ARGDECL
                          hashtable_iter_t* itPtr, hashtable_t* hashtablePtr);


/* =============================================================================
 * hashtable_iter_next
 * =============================================================================
 */
void*
hashtable_iter_next (hashtable_iter_t* itPtr, hashtable_t* hashtablePtr);


/* =============================================================================
 * TMhashtable_iter_next
 * =============================================================================
 */
void*
TMhashtable_iter_next (TM_ARGDECL
                       hashtable_iter_t* itPtr, hashtable_t* hashtablePtr);


/* =============================================================================
 * hashtable_alloc
 * -- Returns NULL on failure
 * -- Negative values for resizeRatio or growthFactor select default values
 * =============================================================================
 */
hashtable_t*
hashtable_alloc (long initNumBucket,
                 ulong_t (*hash)(const void*),
                 long (*comparePairs)(const pair_t*, const pair_t*),
                 long resizeRatio,
                 long growthFactor);


/* =============================================================================
 * TMhashtable_alloc
 * -- Returns NULL on failure
 * -- Negative values for resizeRatio or growthFactor select default values
 * =============================================================================
 */
hashtable_t*
TMhashtable_alloc (TM_ARGDECL
                   long initNumBucket,
                   ulong_t (*hash)(const void*),
                   long (*comparePairs)(const pair_t*, const pair_t*),
                   long resizeRatio,
                   long growthFactor);


/* =============================================================================
 * hashtable_free
 * =============================================================================
 */
void
hashtable_free (hashtable_t* hashtablePtr);


/* =============================================================================
 * TMhashtable_free
 * =============================================================================
 */
void
TMhashtable_free (TM_ARGDECL  hashtable_t* hashtablePtr);


/* =============================================================================
 * hashtable_isEmpty
 * =============================================================================
 */
bool_t
hashtable_isEmpty (hashtable_t* hashtablePtr);


/* =============================================================================
 * TMhashtable_isEmpty
 * =============================================================================
 */
bool_t
TMhashtable_isEmpty (TM_ARGDECL  hashtable_t* hashtablePtr);


/* =============================================================================
 * hashtable_getSize
 * -- Returns number of elements in hash table
 * =============================================================================
 */
long
hashtable_getSize (hashtable_t* hashtablePtr);


/* =============================================================================
 * TMhashtable_getSize
 * -- Returns number of elements in hash table
 * =============================================================================
 */
long
TMhashtable_getSize (TM_ARGDECL  hashtable_t* hashtablePtr);


/* =============================================================================
 * hashtable_containsKey
 * =============================================================================
 */
bool_t
hashtable_containsKey (hashtable_t* hashtablePtr, void* keyPtr);


/* =============================================================================
 * TMhashtable_containsKey
 * =============================================================================
 */
bool_t
TMhashtable_containsKey (TM_ARGDECL  hashtable_t* hashtablePtr, void* keyPtr);


/* =============================================================================
 * hashtable_find
 * -- Returns NULL on failure, else pointer to data associated with key
 * =============================================================================
 */
void*
hashtable_find (hashtable_t* hashtablePtr, void* keyPtr);


/* =============================================================================
 * TMhashtable_find
 * -- Returns NULL on failure, else pointer to data associated with key
 * =============================================================================
 */
void*
TMhashtable_find (TM_ARGDECL  hashtable_t* hashtablePtr, void* keyPtr);


/* =============================================================================
 * hashtable_insert
 * =============================================================================
 */
bool_t
hashtable_insert (hashtable_t* hashtablePtr, void* keyPtr, void* dataPtr);


/* =============================================================================
 * TMhashtable_insert
 * =============================================================================
 */
bool_t
TMhashtable_insert (TM_ARGDECL
                    hashtable_t* hashtablePtr, void* keyPtr, void* dataPtr);


/* =============================================================================
 * hashtable_remove
 * -- Returns TRUE if successful, else FALSE
 * =============================================================================
 */
bool_t
hashtable_remove (hashtable_t* hashtablePtr, void* keyPtr);


/* =============================================================================
 * TMhashtable_remove
 * -- Returns TRUE if successful, else FALSE
 * =============================================================================
 */
bool_t
TMhashtable_remove (TM_ARGDECL  hashtable_t* hashtablePtr, void* keyPtr);


#define TMHASHTABLE_ITER_RESET(it, ht)    TMhashtable_iter_reset(TM_ARG  it, ht)
#define TMHASHTABLE_ITER_HASNEXT(it, ht)  TMhashtable_iter_hasNext(TM_ARG  it, ht)
#define TMHASHTABLE_ITER_NEXT(it, ht)     TMhashtable_iter_next(TM_ARG  it, ht)
#define TMHASHTABLE_ALLOC(i, h, c, r, g)  TMhashtable_alloc(TM_ARG, i, h, c, r, g)
#define TMHASHTABLE_FREE(ht)              TMhashtable_free(TM_ARG  ht)
#define TMHASHTABLE_ISEMPTY(ht)           TMhashtable_isEmpty(TM_ARG  ht)
#define TMHASHTABLE_GETSIZE(ht)           TMhashtable_getSize(TM_ARG  ht)
#define TMHASHTABLE_FIND(ht, k)           TMhashtable_find(TM_ARG  ht, k)
#define TMHASHTABLE_INSERT(ht, k, d)      TMhashtable_insert(TM_ARG  ht, k, d)
#define TMHASHTABLE_REMOVE(ht)            TMhashtable_remove(TM_ARG  ht)


#ifdef __cplusplus
}
#endif


#endif /* HASHTABLE_H */


/* =============================================================================
 *
 * End of hashtable.h
 *
 * =============================================================================
 */
