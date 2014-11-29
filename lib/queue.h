/* =============================================================================
 *
 * queue.h
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


#ifndef QUEUE_H
#define QUEUE_H 1


#include "random.h"
#include "tm.h"
#include "types.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct queue queue_t;


/* =============================================================================
 * queue_alloc
 * =============================================================================
 */
queue_t*
queue_alloc (long initCapacity);


/* =============================================================================
 * Pqueue_alloc
 * =============================================================================
 */
queue_t*
Pqueue_alloc (long initCapacity);


/* =============================================================================
 * TMqueue_alloc
 * =============================================================================
 */
queue_t*
TMqueue_alloc (TM_ARGDECL  long initCapacity);


/* =============================================================================
 * queue_free
 * =============================================================================
 */
void
queue_free (queue_t* queuePtr);


/* =============================================================================
 * Pqueue_free
 * =============================================================================
 */
void
Pqueue_free (queue_t* queuePtr);


/* =============================================================================
 * TMqueue_free
 * =============================================================================
 */
void
TMqueue_free (TM_ARGDECL  queue_t* queuePtr);


/* =============================================================================
 * queue_isEmpty
 * =============================================================================
 */
bool_t
queue_isEmpty (queue_t* queuePtr);


/* =============================================================================
 * TMqueue_isEmpty
 * =============================================================================
 */
TM_CALLABLE
bool_t
TMqueue_isEmpty (TM_ARGDECL  queue_t* queuePtr);


/* =============================================================================
 * queue_clear
 * =============================================================================
 */
void
queue_clear (queue_t* queuePtr);


/* =============================================================================
 * queue_shuffle
 * =============================================================================
 */
void
queue_shuffle (queue_t* queuePtr, random_t* randomPtr);


/* =============================================================================
 * queue_push
 * =============================================================================
 */
bool_t
queue_push (queue_t* queuePtr, void* dataPtr);


/* =============================================================================
 * Pqueue_push
 * =============================================================================
 */
bool_t
Pqueue_push (queue_t* queuePtr, void* dataPtr);


/* =============================================================================
 * TMqueue_push
 * =============================================================================
 */
TM_CALLABLE
bool_t
TMqueue_push (TM_ARGDECL  queue_t* queuePtr, void* dataPtr);


/* =============================================================================
 * queue_pop
 * =============================================================================
 */
void*
queue_pop (queue_t* queuePtr);


/* =============================================================================
 * TMqueue_pop
 * =============================================================================
 */
TM_CALLABLE
void*
TMqueue_pop (TM_ARGDECL  queue_t* queuePtr);


#define PQUEUE_ALLOC(c)     Pqueue_alloc(c)
#define PQUEUE_FREE(q)      Pqueue_free(q)
#define PQUEUE_ISEMPTY(q)   queue_isEmpty(q)
#define PQUEUE_CLEAR(q)     queue_clear(q)
#define PQUEUE_SHUFFLE(q)   queue_shuffle(q, randomPtr);
#define PQUEUE_PUSH(q, d)   Pqueue_push(q, (void*)(d))
#define PQUEUE_POP(q)       queue_pop(q)

#define TMQUEUE_ALLOC(c)    TMqueue_alloc(TM_ARG_ALONE  c)
#define TMQUEUE_FREE(q)     TMqueue_free(TM_ARG  q)
#define TMQUEUE_ISEMPTY(q)  TMqueue_isEmpty(TM_ARG  q)
#define TMQUEUE_PUSH(q, d)  TMqueue_push(TM_ARG  q, (void*)(d))
#define TMQUEUE_POP(q)      TMqueue_pop(TM_ARG  q)


#ifdef __cplusplus
}
#endif


#endif /* QUEUE_H */


/* =============================================================================
 *
 * End of queue.h
 *
 * =============================================================================
 */

