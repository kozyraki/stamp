/* =============================================================================
 *
 * net.c
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
#include <stdlib.h>
#include "bitmap.h"
#include "learner.h"
#include "list.h"
#include "net.h"
#include "operation.h"
#include "queue.h"
#include "tm.h"
#include "vector.h"

typedef enum net_node_mark {
    NET_NODE_MARK_INIT = 0,
    NET_NODE_MARK_DONE = 1,
    NET_NODE_MARK_TEST = 2,
} net_node_mark_t;

typedef struct net_node {
    long id;
    list_t* parentIdListPtr;
    list_t* childIdListPtr;
    net_node_mark_t mark;
} net_node_t;

struct net {
    vector_t* nodeVectorPtr;
};


/* =============================================================================
 * compareId
 * =============================================================================
 */
static long
compareId (const void* aPtr, const void* bPtr)
{
    long a = (long)aPtr;
    long b = (long)bPtr;

    return (a - b);
}


/* =============================================================================
 * allocNode
 * =============================================================================
 */
static net_node_t*
allocNode (long id)
{
    net_node_t* nodePtr;

    nodePtr = (net_node_t*)malloc(sizeof(net_node_t));
    if (nodePtr) {
        nodePtr->parentIdListPtr = list_alloc(&compareId);
        if (nodePtr->parentIdListPtr == NULL) {
            free(nodePtr);
            return NULL;
        }
        nodePtr->childIdListPtr = list_alloc(&compareId);
        if (nodePtr->childIdListPtr == NULL) {
            list_free(nodePtr->parentIdListPtr);
            free(nodePtr);
            return NULL;
        }
        nodePtr->id = id;
    }

    return nodePtr;
}


/* =============================================================================
 * freeNode
 * =============================================================================
 */
static void
freeNode (net_node_t* nodePtr)
{
    Plist_free(nodePtr->childIdListPtr);
    Plist_free(nodePtr->parentIdListPtr);
    free(nodePtr);
}


/* =============================================================================
 * net_alloc
 * =============================================================================
 */
net_t*
net_alloc (long numNode)
{
    net_t* netPtr;

    netPtr = (net_t*)malloc(sizeof(net_t));
    if (netPtr) {
        vector_t* nodeVectorPtr = vector_alloc(numNode);
        if (nodeVectorPtr == NULL) {
            free(netPtr);
            return NULL;
        }
        long i;
        for (i = 0; i < numNode; i++) {
            net_node_t* nodePtr = allocNode(i);
            if (nodePtr == NULL) {
                long j;
                for (j = 0; j < i; j++) {
                    nodePtr = (net_node_t*)vector_at(nodeVectorPtr, j);
                    freeNode(nodePtr);
                }
                vector_free(nodeVectorPtr);
                free(netPtr);
                return NULL;
            }
            bool_t status = vector_pushBack(nodeVectorPtr, (void*)nodePtr);
            assert(status);
        }
        netPtr->nodeVectorPtr = nodeVectorPtr;
    }

    return netPtr;
}


/* =============================================================================
 * net_free
 * =============================================================================
 */
void
net_free (net_t* netPtr)
{
    long i;
    vector_t* nodeVectorPtr = netPtr->nodeVectorPtr;
    long numNode = vector_getSize(nodeVectorPtr);
    for (i = 0; i < numNode; i++) {
        net_node_t* nodePtr = (net_node_t*)vector_at(nodeVectorPtr, i);
        freeNode(nodePtr);
    }
    vector_free(netPtr->nodeVectorPtr);
    free(netPtr);
}


/* =============================================================================
 * insertEdge
 * =============================================================================
 */
static void
insertEdge (net_t* netPtr, long fromId, long toId)
{
    vector_t* nodeVectorPtr = netPtr->nodeVectorPtr;
    bool_t status;

    net_node_t* childNodePtr = (net_node_t*)vector_at(nodeVectorPtr, toId);
    list_t* parentIdListPtr = childNodePtr->parentIdListPtr;
    status = list_insert(parentIdListPtr, (void*)fromId);
    assert(status);

    net_node_t* parentNodePtr = (net_node_t*)vector_at(nodeVectorPtr, fromId);
    list_t* childIdListPtr = parentNodePtr->childIdListPtr;
    status = list_insert(childIdListPtr, (void*)toId);
    assert(status);
}


/* =============================================================================
 * TMinsertEdge
 * =============================================================================
 */
static void
TMinsertEdge (TM_ARGDECL  net_t* netPtr, long fromId, long toId)
{
    vector_t* nodeVectorPtr = netPtr->nodeVectorPtr;
    bool_t status;

    net_node_t* childNodePtr = (net_node_t*)vector_at(nodeVectorPtr, toId);
    list_t* parentIdListPtr = childNodePtr->parentIdListPtr;
    status = TMLIST_INSERT(parentIdListPtr, (void*)fromId);
    assert(status);

    net_node_t* parentNodePtr = (net_node_t*)vector_at(nodeVectorPtr, fromId);
    list_t* childIdListPtr = parentNodePtr->childIdListPtr;
    status = TMLIST_INSERT(childIdListPtr, (void*)toId);
    assert(status);
}


/* =============================================================================
 * removeEdge
 * =============================================================================
 */
static void
removeEdge (net_t* netPtr, long fromId, long toId)
{
    vector_t* nodeVectorPtr = netPtr->nodeVectorPtr;
    bool_t status;

    net_node_t* childNodePtr = (net_node_t*)vector_at(nodeVectorPtr, toId);
    list_t* parentIdListPtr = childNodePtr->parentIdListPtr;
    status = list_remove(parentIdListPtr, (void*)fromId);
    assert(status);

    net_node_t* parentNodePtr = (net_node_t*)vector_at(nodeVectorPtr, fromId);
    list_t* childIdListPtr = parentNodePtr->childIdListPtr;
    status = list_remove(childIdListPtr, (void*)toId);
    assert(status);
}


/* =============================================================================
 * TMremoveEdge
 * =============================================================================
 */
static void
TMremoveEdge (TM_ARGDECL  net_t* netPtr, long fromId, long toId)
{
    vector_t* nodeVectorPtr = netPtr->nodeVectorPtr;
    bool_t status;

    net_node_t* childNodePtr = (net_node_t*)vector_at(nodeVectorPtr, toId);
    list_t* parentIdListPtr = childNodePtr->parentIdListPtr;
    status = TMLIST_REMOVE(parentIdListPtr, (void*)fromId);
    assert(status);

    net_node_t* parentNodePtr = (net_node_t*)vector_at(nodeVectorPtr, fromId);
    list_t* childIdListPtr = parentNodePtr->childIdListPtr;
    status = TMLIST_REMOVE(childIdListPtr, (void*)toId);
    assert(status);
}


/* =============================================================================
 * reverseEdge
 * =============================================================================
 */
static void
reverseEdge (net_t* netPtr, long fromId, long toId)
{
    removeEdge(netPtr, fromId, toId);
    insertEdge(netPtr, toId, fromId);
}


/* =============================================================================
 * TMreverseEdge
 * =============================================================================
 */
static void
TMreverseEdge (TM_ARGDECL  net_t* netPtr, long fromId, long toId)
{
    TMremoveEdge(TM_ARG  netPtr, fromId, toId);
    TMinsertEdge(TM_ARG  netPtr, toId, fromId);
}


/* =============================================================================
 * net_applyOperation
 * =============================================================================
 */
void
net_applyOperation (net_t* netPtr, operation_t op, long fromId, long toId)
{
    switch (op) {
        case OPERATION_INSERT:  insertEdge(netPtr, fromId, toId);  break;
        case OPERATION_REMOVE:  removeEdge(netPtr, fromId, toId);  break;
        case OPERATION_REVERSE: reverseEdge(netPtr, fromId, toId); break;
        default:
            assert(0);
    }
}


/* =============================================================================
 * TMnet_applyOperation
 * =============================================================================
 */
void
TMnet_applyOperation (TM_ARGDECL
                      net_t* netPtr, operation_t op, long fromId, long toId)
{
    switch (op) {
        case OPERATION_INSERT:  TMinsertEdge(TM_ARG   netPtr, fromId, toId); break;
        case OPERATION_REMOVE:  TMremoveEdge(TM_ARG   netPtr, fromId, toId); break;
        case OPERATION_REVERSE: TMreverseEdge(TM_ARG  netPtr, fromId, toId); break;
        default:
            assert(0);
    }
}


/* =============================================================================
 * net_hasEdge
 * =============================================================================
 */
bool_t
net_hasEdge (net_t* netPtr, long fromId, long toId)
{
    vector_t* nodeVectorPtr = netPtr->nodeVectorPtr;
    net_node_t* childNodePtr = (net_node_t*)vector_at(nodeVectorPtr, toId);
    list_t* parentIdListPtr = childNodePtr->parentIdListPtr;

    list_iter_t it;
    list_iter_reset(&it, parentIdListPtr);
    while (list_iter_hasNext(&it, parentIdListPtr)) {
        long parentId = (long)list_iter_next(&it, parentIdListPtr);
        if (parentId == fromId) {
            return TRUE;
        }
    }

    return FALSE;
}


/* =============================================================================
 * TMnet_hasEdge
 * =============================================================================
 */
bool_t
TMnet_hasEdge (TM_ARGDECL  net_t* netPtr, long fromId, long toId)
{
    vector_t* nodeVectorPtr = netPtr->nodeVectorPtr;
    net_node_t* childNodePtr = (net_node_t*)vector_at(nodeVectorPtr, toId);
    list_t* parentIdListPtr = childNodePtr->parentIdListPtr;

    list_iter_t it;
    TMLIST_ITER_RESET(&it, parentIdListPtr);
    while (TMLIST_ITER_HASNEXT(&it, parentIdListPtr)) {
        long parentId = (long)TMLIST_ITER_NEXT(&it, parentIdListPtr);
        if (parentId == fromId) {
            return TRUE;
        }
    }

    return FALSE;
}


/* =============================================================================
 * net_isPath
 * =============================================================================
 */
bool_t
net_isPath (net_t* netPtr,
            long fromId,
            long toId,
            bitmap_t* visitedBitmapPtr,
            queue_t* workQueuePtr)
{
    bool_t status;

    vector_t* nodeVectorPtr = netPtr->nodeVectorPtr;
    assert(visitedBitmapPtr->numBit == vector_getSize(nodeVectorPtr));

    bitmap_clearAll(visitedBitmapPtr);
    queue_clear(workQueuePtr);

    status = queue_push(workQueuePtr, (void*)fromId);
    assert(status);

    while (!queue_isEmpty(workQueuePtr)) {
        long id = (long)queue_pop(workQueuePtr);
        if (id == toId) {
            queue_clear(workQueuePtr);
            return TRUE;
        }
        status = bitmap_set(visitedBitmapPtr, id);
        assert(status);
        net_node_t* nodePtr = (net_node_t*)vector_at(nodeVectorPtr, id);
        list_t* childIdListPtr = nodePtr->childIdListPtr;
        list_iter_t it;
        list_iter_reset(&it, childIdListPtr);
        while (list_iter_hasNext(&it, childIdListPtr)) {
            long childId = (long)list_iter_next(&it, childIdListPtr);
            if (!bitmap_isSet(visitedBitmapPtr, childId)) {
                status = queue_push(workQueuePtr, (void*)childId);
                assert(status);
            }
        }
    }

    return FALSE;
}


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
              queue_t* workQueuePtr)
{
    bool_t status;

    vector_t* nodeVectorPtr = netPtr->nodeVectorPtr;
    assert(visitedBitmapPtr->numBit == vector_getSize(nodeVectorPtr));

    PBITMAP_CLEARALL(visitedBitmapPtr);
    PQUEUE_CLEAR(workQueuePtr);

    status = PQUEUE_PUSH(workQueuePtr, (void*)fromId);
    assert(status);

    while (!PQUEUE_ISEMPTY(workQueuePtr)) {
        long id = (long)queue_pop(workQueuePtr);
        if (id == toId) {
            queue_clear(workQueuePtr);
            return TRUE;
        }
        status = PBITMAP_SET(visitedBitmapPtr, id);
        assert(status);
        net_node_t* nodePtr = (net_node_t*)vector_at(nodeVectorPtr, id);
        list_t* childIdListPtr = nodePtr->childIdListPtr;
        list_iter_t it;
        TMLIST_ITER_RESET(&it, childIdListPtr);
        while (TMLIST_ITER_HASNEXT(&it, childIdListPtr)) {
            long childId = (long)TMLIST_ITER_NEXT(&it, childIdListPtr);
            if (!PBITMAP_ISSET(visitedBitmapPtr, childId)) {
                status = PQUEUE_PUSH(workQueuePtr, (void*)childId);
                assert(status);
            }
        }
    }

    return FALSE;
}


/* =============================================================================
 * isCycle
 * =============================================================================
 */
static bool_t
isCycle (vector_t* nodeVectorPtr, net_node_t* nodePtr)
{
    switch (nodePtr->mark) {
        case NET_NODE_MARK_INIT: {
            nodePtr->mark = NET_NODE_MARK_TEST;
            list_t* childIdListPtr = nodePtr->childIdListPtr;
            list_iter_t it;
            list_iter_reset(&it, childIdListPtr);
            while (list_iter_hasNext(&it, childIdListPtr)) {
                long childId = (long)list_iter_next(&it, childIdListPtr);
                net_node_t* childNodePtr =
                    (net_node_t*)vector_at(nodeVectorPtr, childId);
                if (isCycle(nodeVectorPtr, childNodePtr)) {
                    return TRUE;
                }
            }
            break;
        }
        case NET_NODE_MARK_TEST:
            return TRUE;
        case NET_NODE_MARK_DONE:
            return FALSE;
            break;
        default:
            assert(0);
    }

    nodePtr->mark = NET_NODE_MARK_DONE;
    return FALSE;
}


/* =============================================================================
 * net_isCycle
 * =============================================================================
 */
bool_t
net_isCycle (net_t* netPtr)
{
    vector_t* nodeVectorPtr = netPtr->nodeVectorPtr;
    long numNode = vector_getSize(nodeVectorPtr);
    long n;
    for (n = 0; n < numNode; n++) {
        net_node_t* nodePtr = (net_node_t*)vector_at(nodeVectorPtr, n);
        nodePtr->mark = NET_NODE_MARK_INIT;
    }

    for (n = 0; n < numNode; n++) {
        net_node_t* nodePtr = (net_node_t*)vector_at(nodeVectorPtr, n);
        switch (nodePtr->mark) {
            case NET_NODE_MARK_INIT:
                if (isCycle(nodeVectorPtr, nodePtr)) {
                    return TRUE;
                }
                break;
            case NET_NODE_MARK_DONE:
                /* do nothing */
                break;
            case NET_NODE_MARK_TEST:
                assert(0);
                break;
            default:
                assert(0);
                break;
        }
    }

    return FALSE;
}


/* =============================================================================
 * net_getParentIdListPtr
 * =============================================================================
 */
list_t*
net_getParentIdListPtr (net_t* netPtr, long id)
{
    net_node_t* nodePtr = (net_node_t*)vector_at(netPtr->nodeVectorPtr, id);
    assert(nodePtr);

    return nodePtr->parentIdListPtr;
}


/* =============================================================================
 * net_getChildIdListPtr
 * =============================================================================
 */
list_t*
net_getChildIdListPtr (net_t* netPtr, long id)
{
    net_node_t* nodePtr = (net_node_t*)vector_at(netPtr->nodeVectorPtr, id);
    assert(nodePtr);

    return nodePtr->childIdListPtr;
}


/* =============================================================================
 * net_findAncestors
 * -- Contents of bitmapPtr set to 1 if ancestor, else 0
 * -- Returns false if id is not root node (i.e., has cycle back id)
 * =============================================================================
 */
bool_t
net_findAncestors (net_t* netPtr,
                   long id,
                   bitmap_t* ancestorBitmapPtr,
                   queue_t* workQueuePtr)
{
    bool_t status;

    vector_t* nodeVectorPtr = netPtr->nodeVectorPtr;
    assert(ancestorBitmapPtr->numBit == vector_getSize(nodeVectorPtr));

    bitmap_clearAll(ancestorBitmapPtr);
    queue_clear(workQueuePtr);

    {
        net_node_t* nodePtr = (net_node_t*)vector_at(nodeVectorPtr, id);
        list_t* parentIdListPtr = nodePtr->parentIdListPtr;
        list_iter_t it;
        list_iter_reset(&it, parentIdListPtr);
        while (list_iter_hasNext(&it, parentIdListPtr)) {
            long parentId = (long)list_iter_next(&it, parentIdListPtr);
            status = bitmap_set(ancestorBitmapPtr, parentId);
            assert(status);
            status = queue_push(workQueuePtr, (void*)parentId);
            assert(status);
        }
    }

    while (!queue_isEmpty(workQueuePtr)) {
        long parentId = (long)queue_pop(workQueuePtr);
        if (parentId == id) {
            queue_clear(workQueuePtr);
            return FALSE;
        }
        net_node_t* nodePtr = (net_node_t*)vector_at(nodeVectorPtr, parentId);
        list_t* grandParentIdListPtr = nodePtr->parentIdListPtr;
        list_iter_t it;
        list_iter_reset(&it, grandParentIdListPtr);
        while (list_iter_hasNext(&it, grandParentIdListPtr)) {
            long grandParentId = (long)list_iter_next(&it, grandParentIdListPtr);
            if (!bitmap_isSet(ancestorBitmapPtr, grandParentId)) {
                status = bitmap_set(ancestorBitmapPtr, grandParentId);
                assert(status);
                status = queue_push(workQueuePtr, (void*)grandParentId);
                assert(status);
            }
        }
    }

    return TRUE;
}


/* =============================================================================
 * TMnet_findAncestors
 * -- Contents of bitmapPtr set to 1 if ancestor, else 0
 * -- Returns false if id is not root node (i.e., has cycle back id)
 * =============================================================================
 */
bool_t
TMnet_findAncestors (TM_ARGDECL
                     net_t* netPtr,
                     long id,
                     bitmap_t* ancestorBitmapPtr,
                     queue_t* workQueuePtr)
{
    bool_t status;

    vector_t* nodeVectorPtr = netPtr->nodeVectorPtr;
    assert(ancestorBitmapPtr->numBit == vector_getSize(nodeVectorPtr));

    PBITMAP_CLEARALL(ancestorBitmapPtr);
    PQUEUE_CLEAR(workQueuePtr);

    {
        net_node_t* nodePtr = (net_node_t*)vector_at(nodeVectorPtr, id);
        list_t* parentIdListPtr = nodePtr->parentIdListPtr;
        list_iter_t it;
        TMLIST_ITER_RESET(&it, parentIdListPtr);
        while (TMLIST_ITER_HASNEXT(&it, parentIdListPtr)) {
            long parentId = (long)TMLIST_ITER_NEXT(&it, parentIdListPtr);
            status = PBITMAP_SET(ancestorBitmapPtr, parentId);
            assert(status);
            status = PQUEUE_PUSH(workQueuePtr, (void*)parentId);
            assert(status);
        }
    }

    while (!PQUEUE_ISEMPTY(workQueuePtr)) {
        long parentId = (long)PQUEUE_POP(workQueuePtr);
        if (parentId == id) {
            PQUEUE_CLEAR(workQueuePtr);
            return FALSE;
        }
        net_node_t* nodePtr = (net_node_t*)vector_at(nodeVectorPtr, parentId);
        list_t* grandParentIdListPtr = nodePtr->parentIdListPtr;
        list_iter_t it;
        TMLIST_ITER_RESET(&it, grandParentIdListPtr);
        while (TMLIST_ITER_HASNEXT(&it, grandParentIdListPtr)) {
            long grandParentId = (long)TMLIST_ITER_NEXT(&it, grandParentIdListPtr);
            if (!PBITMAP_ISSET(ancestorBitmapPtr, grandParentId)) {
                status = PBITMAP_SET(ancestorBitmapPtr, grandParentId);
                assert(status);
                status = PQUEUE_PUSH(workQueuePtr, (void*)grandParentId);
                assert(status);
            }
        }
    }

    return TRUE;
}


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
                     queue_t* workQueuePtr)
{
    bool_t status;

    vector_t* nodeVectorPtr = netPtr->nodeVectorPtr;
    assert(descendantBitmapPtr->numBit == vector_getSize(nodeVectorPtr));

    bitmap_clearAll(descendantBitmapPtr);
    queue_clear(workQueuePtr);

    {
        net_node_t* nodePtr = (net_node_t*)vector_at(nodeVectorPtr, id);
        list_t* childIdListPtr = nodePtr->childIdListPtr;
        list_iter_t it;
        list_iter_reset(&it, childIdListPtr);
        while (list_iter_hasNext(&it, childIdListPtr)) {
            long childId = (long)list_iter_next(&it, childIdListPtr);
            status = bitmap_set(descendantBitmapPtr, childId);
            assert(status);
            status = queue_push(workQueuePtr, (void*)childId);
            assert(status);
        }
    }

    while (!queue_isEmpty(workQueuePtr)) {
        long childId = (long)queue_pop(workQueuePtr);
        if (childId == id) {
            queue_clear(workQueuePtr);
            return FALSE;
        }
        net_node_t* nodePtr = (net_node_t*)vector_at(nodeVectorPtr, childId);
        list_t* grandChildIdListPtr = nodePtr->childIdListPtr;
        list_iter_t it;
        list_iter_reset(&it, grandChildIdListPtr);
        while (list_iter_hasNext(&it, grandChildIdListPtr)) {
            long grandChildId = (long)list_iter_next(&it, grandChildIdListPtr);
            if (!bitmap_isSet(descendantBitmapPtr, grandChildId)) {
                status = bitmap_set(descendantBitmapPtr, grandChildId);
                assert(status);
                status = queue_push(workQueuePtr, (void*)grandChildId);
                assert(status);
            }
        }
    }

    return TRUE;
}


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
                       queue_t* workQueuePtr)
{
    bool_t status;

    vector_t* nodeVectorPtr = netPtr->nodeVectorPtr;
    assert(descendantBitmapPtr->numBit == vector_getSize(nodeVectorPtr));

    PBITMAP_CLEARALL(descendantBitmapPtr);
    PQUEUE_CLEAR(workQueuePtr);

    {
        net_node_t* nodePtr = (net_node_t*)vector_at(nodeVectorPtr, id);
        list_t* childIdListPtr = nodePtr->childIdListPtr;
        list_iter_t it;
        TMLIST_ITER_RESET(&it, childIdListPtr);
        while (TMLIST_ITER_HASNEXT(&it, childIdListPtr)) {
            long childId = (long)TMLIST_ITER_NEXT(&it, childIdListPtr);
            status = PBITMAP_SET(descendantBitmapPtr, childId);
            assert(status);
            status = PQUEUE_PUSH(workQueuePtr, (void*)childId);
            assert(status);
        }
    }

    while (!PQUEUE_ISEMPTY(workQueuePtr)) {
        long childId = (long)PQUEUE_POP(workQueuePtr);
        if (childId == id) {
            queue_clear(workQueuePtr);
            return FALSE;
        }
        net_node_t* nodePtr = (net_node_t*)vector_at(nodeVectorPtr, childId);
        list_t* grandChildIdListPtr = nodePtr->childIdListPtr;
        list_iter_t it;
        TMLIST_ITER_RESET(&it, grandChildIdListPtr);
        while (TMLIST_ITER_HASNEXT(&it, grandChildIdListPtr)) {
            long grandChildId = (long)TMLIST_ITER_NEXT(&it, grandChildIdListPtr);
            if (!PBITMAP_ISSET(descendantBitmapPtr, grandChildId)) {
                status = PBITMAP_SET(descendantBitmapPtr, grandChildId);
                assert(status);
                status = PQUEUE_PUSH(workQueuePtr, (void*)grandChildId);
                assert(status);
            }
        }
    }

    return TRUE;
}


/* =============================================================================
 * net_generateRandomEdges
 * =============================================================================
 */
void
net_generateRandomEdges (net_t* netPtr,
                         long maxNumParent,
                         long percentParent,
                         random_t* randomPtr)
{
    vector_t* nodeVectorPtr = netPtr->nodeVectorPtr;

    long numNode = vector_getSize(nodeVectorPtr);
    bitmap_t* visitedBitmapPtr = bitmap_alloc(numNode);
    assert(visitedBitmapPtr);
    queue_t* workQueuePtr = queue_alloc(-1);

    long n;

    for (n = 0; n < numNode; n++) {
        long p;
        for (p = 0; p < maxNumParent; p++) {
            long value = random_generate(randomPtr) % 100;
            if (value < percentParent) {
                long parent = random_generate(randomPtr) % numNode;
                if ((parent != n) &&
                    !net_hasEdge(netPtr, parent, n) &&
                    !net_isPath(netPtr, n, parent, visitedBitmapPtr, workQueuePtr))
                {
#ifdef TEST_NET
                    printf("node=%li parent=%li\n", n, parent);
#endif
                    insertEdge(netPtr, parent, n);
                }
            }
        }
    }

    assert(!net_isCycle(netPtr));

    bitmap_free(visitedBitmapPtr);
    queue_free(workQueuePtr);
}


/* #############################################################################
 * TEST_NET
 * #############################################################################
 */
#ifdef TEST_NET

#include <stdio.h>


int
main ()
{
    long numNode = 100;

    puts("Starting tests...");

    bool_t status;

    net_t* netPtr = net_alloc(numNode);
    assert(netPtr);
    bitmap_t* visitedBitmapPtr = bitmap_alloc(numNode);
    assert(visitedBitmapPtr);
    queue_t* workQueuePtr = queue_alloc(-1);
    assert(workQueuePtr);

    assert(!net_isCycle(netPtr));

    long aId = 31;
    long bId = 14;
    long cId = 5;
    long dId = 92;

    net_applyOperation(netPtr, OPERATION_INSERT, aId, bId);
    assert(net_isPath(netPtr, aId, bId, visitedBitmapPtr, workQueuePtr));
    assert(!net_isPath(netPtr, bId, aId, visitedBitmapPtr, workQueuePtr));
    assert(!net_isPath(netPtr, aId, cId, visitedBitmapPtr, workQueuePtr));
    assert(!net_isPath(netPtr, aId, dId, visitedBitmapPtr, workQueuePtr));
    assert(!net_isCycle(netPtr));

    net_applyOperation(netPtr, OPERATION_INSERT, bId, cId);
    net_applyOperation(netPtr, OPERATION_INSERT, aId, cId);
    net_applyOperation(netPtr, OPERATION_INSERT, dId, aId);
    assert(!net_isCycle(netPtr));
    net_applyOperation(netPtr, OPERATION_INSERT, cId, dId);
    assert(net_isCycle(netPtr));
    net_applyOperation(netPtr, OPERATION_REVERSE, cId, dId);
    assert(!net_isCycle(netPtr));
    net_applyOperation(netPtr, OPERATION_REVERSE, dId, cId);
    assert(net_isCycle(netPtr));
    assert(net_isPath(netPtr, aId, dId, visitedBitmapPtr, workQueuePtr));
    net_applyOperation(netPtr, OPERATION_REMOVE, cId, dId);
    assert(!net_isPath(netPtr, aId, dId, visitedBitmapPtr, workQueuePtr));

    bitmap_t* ancestorBitmapPtr = bitmap_alloc(numNode);
    assert(ancestorBitmapPtr);
    status = net_findAncestors(netPtr, cId, ancestorBitmapPtr, workQueuePtr);
    assert(status);
    assert(bitmap_isSet(ancestorBitmapPtr, aId));
    assert(bitmap_isSet(ancestorBitmapPtr, bId));
    assert(bitmap_isSet(ancestorBitmapPtr, dId));
    assert(bitmap_getNumSet(ancestorBitmapPtr) == 3);

    bitmap_t* descendantBitmapPtr = bitmap_alloc(numNode);
    assert(descendantBitmapPtr);
    status = net_findDescendants(netPtr, aId, descendantBitmapPtr, workQueuePtr);
    assert(status);
    assert(bitmap_isSet(descendantBitmapPtr, bId));
    assert(bitmap_isSet(descendantBitmapPtr, cId));
    assert(bitmap_getNumSet(descendantBitmapPtr) == 2);

    bitmap_free(visitedBitmapPtr);
    queue_free(workQueuePtr);
    bitmap_free(ancestorBitmapPtr);
    bitmap_free(descendantBitmapPtr);
    net_free(netPtr);

    random_t* randomPtr = random_alloc();
    assert(randomPtr);
    netPtr = net_alloc(numNode);
    assert(netPtr);
    net_generateRandomEdges(netPtr, 10, 10, randomPtr);
    net_free(netPtr);

    puts("All tests passed.");

    return 0;
}


#endif /* TEST_NET */


/* =============================================================================
 *
 * End of net.c
 *
 * =============================================================================
 */
