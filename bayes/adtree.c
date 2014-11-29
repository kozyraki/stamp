/* =============================================================================
 *
 * adtree.c
 *
 * =============================================================================
 *
 * Copyright (C) Stanford University, 2006.  All Rights Reserved.
 * Author: Chi Cao Minh
 *
 * =============================================================================
 *
 * Reference:
 *
 * A. Moore and M.-S. Lee. Cached sufficient statistics for efficient machine
 * learning with large datasets. Journal of Artificial Intelligence Research 8
 * (1998), pp 67-91.
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
#include "adtree.h"
#include "data.h"
#include "query.h"
#include "utility.h"
#include "vector.h"


/* =============================================================================
 * allocNode
 * =============================================================================
 */
adtree_node_t*
allocNode (long index)
{
    adtree_node_t* nodePtr;

    nodePtr = (adtree_node_t*)malloc(sizeof(adtree_node_t));
    if (nodePtr) {
        nodePtr->varyVectorPtr = vector_alloc(1);
        if (nodePtr->varyVectorPtr == NULL) {
            free(nodePtr);
            return NULL;
        }
        nodePtr->index = index;
        nodePtr->value = -1;
        nodePtr->count = -1;
    }

    return nodePtr;
}


/* =============================================================================
 * freeNode
 * =============================================================================
 */
void
freeNode (adtree_node_t* nodePtr)
{
    vector_free(nodePtr->varyVectorPtr);
    free(nodePtr);
}


/* =============================================================================
 * allocVary
 * =============================================================================
 */
adtree_vary_t*
allocVary (long index)
{
    adtree_vary_t* varyPtr;

    varyPtr = (adtree_vary_t*)malloc(sizeof(adtree_vary_t));
    if (varyPtr) {
        varyPtr->index = index;
        varyPtr->mostCommonValue = -1;
        varyPtr->zeroNodePtr = NULL;
        varyPtr->oneNodePtr = NULL;
    }

    return varyPtr;
}


/* =============================================================================
 * freeVary
 * =============================================================================
 */
void
freeVary (adtree_vary_t* varyPtr)
{
    free(varyPtr);
}


/* =============================================================================
 * adtree_alloc
 * =============================================================================
 */
adtree_t*
adtree_alloc ()
{
    adtree_t* adtreePtr;

    adtreePtr = (adtree_t*)malloc(sizeof(adtree_t));
    if (adtreePtr) {
        adtreePtr->numVar = -1L;
        adtreePtr->numRecord = -1L;
        adtreePtr->rootNodePtr = NULL;
    }

    return adtreePtr;
}


/* =============================================================================
 * freeNodes
 * =============================================================================
 */
static void
freeNodes (adtree_node_t* nodePtr)
{
    if (nodePtr) {
        vector_t* varyVectorPtr = nodePtr->varyVectorPtr;
        long v;
        long numVary = vector_getSize(varyVectorPtr);
        for (v = 0; v < numVary; v++) {
            adtree_vary_t* varyPtr = (adtree_vary_t*)vector_at(varyVectorPtr, v);
            freeNodes(varyPtr->zeroNodePtr);
            freeNodes(varyPtr->oneNodePtr);
            freeVary(varyPtr);
        }
        freeNode(nodePtr);
    }
}


/* =============================================================================
 * adtree_free
 * =============================================================================
 */
void
adtree_free (adtree_t* adtreePtr)
{
    freeNodes(adtreePtr->rootNodePtr);
    free(adtreePtr);
}


static adtree_vary_t*
makeVary (long parentIndex,
          long index,
          long start,
          long numRecord,
          data_t* dataPtr);

static adtree_node_t*
makeNode (long parentIndex,
          long index,
          long start,
          long numRecord,
          data_t* dataPtr);


/* =============================================================================
 * makeVary
 * =============================================================================
 */
static adtree_vary_t*
makeVary (long parentIndex,
          long index,
          long start,
          long numRecord,
          data_t* dataPtr)
{
    adtree_vary_t* varyPtr = allocVary(index);
    assert(varyPtr);

    if ((parentIndex + 1 != index) && (numRecord > 1)) {
        data_sort(dataPtr, start, numRecord, index);
    }

    long num0 = data_findSplit(dataPtr, start, numRecord, index);
    long num1 = numRecord - num0;

    long mostCommonValue = ((num0 >= num1) ? 0 : 1);
    varyPtr->mostCommonValue = mostCommonValue;

    if (num0 == 0 || mostCommonValue == 0) {
        varyPtr->zeroNodePtr = NULL;
    } else {
        varyPtr->zeroNodePtr =
            makeNode(index, index, start, num0, dataPtr);
        varyPtr->zeroNodePtr->value = 0;
    }

    if (num1 == 0 || mostCommonValue == 1) {
        varyPtr->oneNodePtr = NULL;
    } else {
        varyPtr->oneNodePtr =
            makeNode(index, index, (start + num0), num1, dataPtr);
        varyPtr->oneNodePtr->value = 1;
    }

    return varyPtr;
}


/* =============================================================================
 * makeNode
 * =============================================================================
 */
static adtree_node_t*
makeNode (long parentIndex,
          long index,
          long start,
          long numRecord,
          data_t* dataPtr)
{
    adtree_node_t* nodePtr = allocNode(index);
    assert(nodePtr);

    nodePtr->count = numRecord;

    vector_t* varyVectorPtr = nodePtr->varyVectorPtr;

    long v;
    long numVar = dataPtr->numVar;
    for (v = (index + 1); v < numVar; v++) {
        adtree_vary_t* varyPtr =
            makeVary(parentIndex, v, start, numRecord, dataPtr);
        assert(varyPtr);
        bool_t status = vector_pushBack(varyVectorPtr, (void*)varyPtr);
        assert(status);
    }

    return nodePtr;
}


/* =============================================================================
 * adtree_make
 * -- Records in dataPtr will get rearranged
 * =============================================================================
 */
void
adtree_make (adtree_t* adtreePtr, data_t* dataPtr)
{
    long numRecord = dataPtr->numRecord;
    adtreePtr->numVar = dataPtr->numVar;
    adtreePtr->numRecord = dataPtr->numRecord;
    data_sort(dataPtr, 0, numRecord, 0);
    adtreePtr->rootNodePtr = makeNode(-1, -1, 0, numRecord, dataPtr);
}


/* =============================================================================
 * getCount
 * =============================================================================
 */
static long
getCount (adtree_node_t* nodePtr,
          long i,
          long q,
          vector_t* queryVectorPtr,
          long lastQueryIndex,
          adtree_t* adtreePtr)
{
    if (nodePtr == NULL) {
        return 0;
    }

    long nodeIndex = nodePtr->index;
    if (nodeIndex >= lastQueryIndex) {
        return nodePtr->count;
    }

    long count = 0L;

    query_t* queryPtr = (query_t*)vector_at(queryVectorPtr, q);
    if (!queryPtr) {
        return nodePtr->count;
    }
    long queryIndex = queryPtr->index;
    assert(queryIndex <= lastQueryIndex);
    vector_t* varyVectorPtr = nodePtr->varyVectorPtr;
    adtree_vary_t* varyPtr =
        (adtree_vary_t*)vector_at(varyVectorPtr,
                                  (queryIndex - nodeIndex - 1));
    assert(varyPtr);

    long queryValue = queryPtr->value;

    if (queryValue == varyPtr->mostCommonValue) {

        /*
         * We do not explicitly store the counts for the most common value.
         * We can calculate it by finding the count of the query without
         * the current (superCount) and subtracting the count for the
         * query with the current toggled (invertCount).
         */
        long numQuery = vector_getSize(queryVectorPtr);
        vector_t* superQueryVectorPtr = PVECTOR_ALLOC(numQuery - 1);
        assert(superQueryVectorPtr);

        long qq;
        for (qq = 0; qq < numQuery; qq++) {
            if (qq != q) {
                bool_t status = vector_pushBack(superQueryVectorPtr,
                                                vector_at(queryVectorPtr, qq));
                assert(status);
            }
        }
        long superCount = adtree_getCount(adtreePtr, superQueryVectorPtr);

        PVECTOR_FREE(superQueryVectorPtr);

        long invertCount;
        if (queryValue == 0) {
            queryPtr->value = 1;
            invertCount = getCount(nodePtr,
                                   i,
                                   q,
                                   queryVectorPtr,
                                   lastQueryIndex,
                                   adtreePtr);
            queryPtr->value = 0;
        } else {
            queryPtr->value = 0;
            invertCount = getCount(nodePtr,
                                   i,
                                   q,
                                   queryVectorPtr,
                                   lastQueryIndex,
                                   adtreePtr);
            queryPtr->value = 1;
        }
        count += superCount - invertCount;

    } else {

        if (queryValue == 0) {
            count += getCount(varyPtr->zeroNodePtr,
                              (i + 1),
                              (q + 1),
                              queryVectorPtr,
                              lastQueryIndex,
                              adtreePtr);
        } else if (queryValue == 1) {
            count += getCount(varyPtr->oneNodePtr,
                              (i + 1),
                              (q + 1),
                              queryVectorPtr,
                              lastQueryIndex,
                              adtreePtr);
        } else { /* QUERY_VALUE_WILDCARD */
#if 0
            count += getCount(varyPtr->zeroNodePtr,
                              (i + 1),
                              (q + 1),
                              queryVectorPtr,
                              lastQueryIndex,
                              adtreePtr);
            count += getCount(varyPtr->oneNodePtr,
                              (i + 1),
                              (q + 1),
                              queryVectorPtr,
                              lastQueryIndex,
                              adtreePtr);
#else
            assert(0); /* catch bugs in learner */
#endif
        }

    }

    return count;
}


/* =============================================================================
 * adtree_getCount
 * -- queryVector must consist of queries sorted by id
 * =============================================================================
 */
long
adtree_getCount (adtree_t* adtreePtr, vector_t* queryVectorPtr)
{
    adtree_node_t* rootNodePtr = adtreePtr->rootNodePtr;
    if (rootNodePtr == NULL) {
        return 0;
    }

    long lastQueryIndex = -1L;
    long numQuery = vector_getSize(queryVectorPtr);
    if (numQuery > 0) {
        query_t* lastQueryPtr = (query_t*)vector_at(queryVectorPtr, (numQuery - 1));
        lastQueryIndex = lastQueryPtr->index;
    }

    return getCount(rootNodePtr,
                    -1,
                    0,
                    queryVectorPtr,
                    lastQueryIndex,
                    adtreePtr);
}


/* #############################################################################
 * TEST_ADTREE
 * #############################################################################
 */
#ifdef TEST_ADTREE

#include <stdio.h>
#include "timer.h"

static void printNode (adtree_node_t* nodePtr);
static void printVary (adtree_vary_t* varyPtr);

bool_t global_doPrint = FALSE;


static void
printData (data_t* dataPtr)
{
    long numVar = dataPtr->numVar;
    long numRecord = dataPtr->numRecord;

    long r;
    for (r = 0; r < numRecord; r++) {
        printf("%4li: ", r);
        char* record = data_getRecord(dataPtr, r);
        assert(record);
        long v;
        for (v = 0; v < numVar; v++) {
            printf("%li", (long)record[v]);
        }
        puts("");
    }
}


static void
printNode (adtree_node_t* nodePtr)
{
    if (nodePtr) {
        printf("[node] index=%li value=%li count=%li\n",
               nodePtr->index, nodePtr->value, nodePtr->count);
        vector_t* varyVectorPtr = nodePtr->varyVectorPtr;
        long v;
        long numVary = vector_getSize(varyVectorPtr);
        for (v = 0; v < numVary; v++) {
            adtree_vary_t* varyPtr = (adtree_vary_t*)vector_at(varyVectorPtr, v);
            printVary(varyPtr);
        }
    }
    puts("[up]");
}


static void
printVary (adtree_vary_t* varyPtr)
{
    if (varyPtr) {
        printf("[vary] index=%li\n", varyPtr->index);
        printNode(varyPtr->zeroNodePtr);
        printNode(varyPtr->oneNodePtr);
    }
    puts("[up]");
}


static void
printAdtree (adtree_t* adtreePtr)
{
    printNode(adtreePtr->rootNodePtr);
}


static void
printQuery (vector_t* queryVectorPtr)
{
    printf("[");
    long q;
    long numQuery = vector_getSize(queryVectorPtr);
    for (q = 0; q < numQuery; q++) {
        query_t* queryPtr = (query_t*)vector_at(queryVectorPtr, q);
        printf("%li:%li ", queryPtr->index, queryPtr->value);
    }
    printf("]");
}


static long
countData (data_t* dataPtr, vector_t* queryVectorPtr)
{
    long count = 0;
    long numQuery = vector_getSize(queryVectorPtr);

    long r;
    long numRecord = dataPtr->numRecord;
    for (r = 0; r < numRecord; r++) {
        char* record = data_getRecord(dataPtr, r);
        bool_t isMatch = TRUE;
        long q;
        for (q = 0; q < numQuery; q++) {
            query_t* queryPtr = (query_t*)vector_at(queryVectorPtr, q);
            long queryValue = queryPtr->value;
            if ((queryValue != QUERY_VALUE_WILDCARD) &&
                ((char)queryValue) != record[queryPtr->index])
            {
                isMatch = FALSE;
                break;
            }
        }
        if (isMatch) {
            count++;
        }
    }

    return count;
}


static void
testCount (adtree_t* adtreePtr,
           data_t* dataPtr,
           vector_t* queryVectorPtr,
           long index,
           long numVar)
{
    if (index >= numVar) {
        return;
    }

    long count1 = adtree_getCount(adtreePtr, queryVectorPtr);
    long count2 = countData(dataPtr, queryVectorPtr);
    if (global_doPrint) {
        printQuery(queryVectorPtr);
        printf(" count1=%li count2=%li\n", count1, count2);
        fflush(stdout);
    }
    assert(count1 == count2);

    query_t query;

    long i;
    for (i = 1; i < numVar; i++) {
        query.index = index + i;
        bool_t status = vector_pushBack(queryVectorPtr, (void*)&query);
        assert(status);

        query.value = 0;
        testCount(adtreePtr, dataPtr, queryVectorPtr, query.index, numVar);

        query.value = 1;
        testCount(adtreePtr, dataPtr, queryVectorPtr, query.index, numVar);

        vector_popBack(queryVectorPtr);
    }
}


static void
testCounts (adtree_t* adtreePtr, data_t* dataPtr)
{
    long numVar = dataPtr->numVar;
    vector_t* queryVectorPtr = vector_alloc(numVar);
    long v;
    for (v = -1; v < numVar; v++) {
        testCount(adtreePtr, dataPtr, queryVectorPtr, v, dataPtr->numVar);
    }
    vector_free(queryVectorPtr);
}


static void
test (long numVar, long numRecord)
{
    random_t* randomPtr = random_alloc();
    data_t* dataPtr = data_alloc(numVar, numRecord, randomPtr);
    assert(dataPtr);
    data_generate(dataPtr, 0, 10, 10);
    if (global_doPrint) {
        printData(dataPtr);
    }

    data_t* copyDataPtr = data_alloc(numVar, numRecord, randomPtr);
    assert(copyDataPtr);
    data_copy(copyDataPtr, dataPtr);

    adtree_t* adtreePtr = adtree_alloc();
    assert(adtreePtr);

    TIMER_T start;
    TIMER_READ(start);

    adtree_make(adtreePtr, copyDataPtr);

    TIMER_T stop;
    TIMER_READ(stop);

    printf("%lf\n", TIMER_DIFF_SECONDS(start, stop));

    if (global_doPrint) {
        printAdtree(adtreePtr);
    }

    testCounts(adtreePtr, dataPtr);

    adtree_free(adtreePtr);
    random_free(randomPtr);
    data_free(dataPtr);
}


int
main ()
{
    puts("Starting...");

    puts("Test 1:");
    test(3, 8);

    puts("Test 2:");
    test(4, 64);

    puts("Test 3:");
    test(8, 256);

    puts("Test 4:");
    test(12, 256);

    puts("Test 5:");
    test(48, 1024);

    puts("All tests passed.");

    return 0;
}


#endif /* TEST_ADTREE */


/* =============================================================================
 *
 * End of adtree.c
 *
 * =============================================================================
 */
