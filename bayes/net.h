/* =============================================================================
 *
 * net.h
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


#ifndef NET_H
#define NET_H 1


#include "bitmap.h"
#include "list.h"
#include "operation.h"
#include "queue.h"
#include "tm.h"

typedef struct net net_t;


/* =============================================================================
 * net_alloc
 * =============================================================================
 */
net_t*
net_alloc (long numNode);


/* =============================================================================
 * net_free
 * =============================================================================
 */
void
net_free (net_t* netPtr);


/* =============================================================================
 * net_applyOperation
 * =============================================================================
 */
void
net_applyOperation (net_t* netPtr, operation_t op, long fromId, long toId);


/* =============================================================================
 * TMnet_applyOperation
 * =============================================================================
 */
void
TMnet_applyOperation (TM_ARGDECL
                      net_t* netPtr, operation_t op, long fromId, long toId);


/* =============================================================================
 * net_hasEdge
 * =============================================================================
 */
bool_t
net_hasEdge (net_t* netPtr, long fromId, long toId);


/* =============================================================================
 * TMnet_hasEdge
 * =============================================================================
 */
bool_t
TMnet_hasEdge (TM_ARGDECL  net_t* netPtr, long fromId, long toId);


/* =============================================================================
 * net_isPath
 * =============================================================================
 */
bool_t
net_isPath (net_t* netPtr,
            long fromId,
            long toId,
            bitmap_t* visitedBitmapPtr,
            queue_t* workQueuePtr);


/* =============================================================================
 * TMnet_isPath
 * =============================================================================
 */
bool_t
TMnet_isPath (TM_ARGDECL
              net_t* netPtr,
              long fromId,
              long toId,
              bitmap_t* visitedBitmapPtr,
              queue_t* workQueuePtr);


/* =============================================================================
 * net_isCycle
 * =============================================================================
 */
bool_t
net_isCycle (net_t* netPtr);


/* =============================================================================
 * net_getParentIdListPtr
 * =============================================================================
 */
list_t*
net_getParentIdListPtr (net_t* netPtr, long id);


/* =============================================================================
 * net_getChildIdListPtr
 * =============================================================================
 */
list_t*
net_getChildIdListPtr (net_t* netPtr, long id);


/* =============================================================================
 * net_findAncestors
 * -- Contents of bitmapPtr set to 1 if parent, else 0
 * -- Returns false if id is not root node (i.e., has cycle back id)
 * =============================================================================
 */
bool_t
net_findAncestors (net_t* netPtr,
                   long id,
                   bitmap_t* ancestorBitmapPtr,
                   queue_t* workQueuePtr);


/* =============================================================================
 * TMnet_findAncestors
 * -- Contents of bitmapPtr set to 1 if parent, else 0
 * -- Returns false if id is not root node (i.e., has cycle back id)
 * =============================================================================
 */
bool_t
TMnet_findAncestors (TM_ARGDECL
                     net_t* netPtr,
                     long id,
                     bitmap_t* ancestorBitmapPtr,
                     queue_t* workQueuePtr);


/* =============================================================================
 * net_findDescendants
 * -- Contents of bitmapPtr set to 1 if descendants, else 0
 * -- Returns false if id is not root node (i.e., has cycle back id)
 * =============================================================================
 */
bool_t
net_findDescendants (net_t* netPtr,
                     long id,
                     bitmap_t* descendantBitmapPtr,
                     queue_t* workQueuePtr);


/* =============================================================================
 * TMnet_findDescendants
 * -- Contents of bitmapPtr set to 1 if descendants, else 0
 * -- Returns false if id is not root node (i.e., has cycle back id)
 * =============================================================================
 */
bool_t
TMnet_findDescendants (TM_ARGDECL
                       net_t* netPtr,
                       long id,
                       bitmap_t* descendantBitmapPtr,
                       queue_t* workQueuePtr);


/* =============================================================================
 * net_generateRandomEdges
 * =============================================================================
 */
void
net_generateRandomEdges (net_t* netPtr,
                         long maxNumParent,
                         long percentParent,
                         random_t* randomPtr);


#define TMNET_APPLYOPERATION(net, op, from, to)     TMnet_applyOperation(TM_ARG \
                                                                         net, \
                                                                         op, \
                                                                         from, \
                                                                         to)
#define TMNET_HASEDGE(net, from, to)                TMnet_hasEdge(TM_ARG \
                                                                  net, \
                                                                  from, \
                                                                  to)
#define TMNET_ISPATH(net, from, to, bmp, wq)        TMnet_isPath(TM_ARG \
                                                                 net, \
                                                                 from, \
                                                                 to, \
                                                                 bmp, \
                                                                 wq)
#define TMNET_FINDANCESTORS(net, id, bmp, wq)       TMnet_findAncestors(TM_ARG \
                                                                        net, \
                                                                        id, \
                                                                        bmp, \
                                                                        wq)
#define TMNET_FINDDESCENDANTS(net, id, bmp, wq)     TMnet_findDescendants(TM_ARG \
                                                                          net, \
                                                                          id, \
                                                                          bmp, \
                                                                          wq)

#endif /* NET_H */


/* =============================================================================
 *
 * End of net.h
 *
 * =============================================================================
 */
