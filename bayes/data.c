/* =============================================================================
 *
 * data.c
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
#include <string.h>
#include "bitmap.h"
#include "data.h"
#include "net.h"
#include "random.h"
#include "sort.h"
#include "types.h"
#include "vector.h"

enum data_config {
    DATA_PRECISION = 100,
    DATA_INIT      = 2 /* not 0 or 1 */
};


/* =============================================================================
 * data_alloc
 * =============================================================================
 */
data_t*
data_alloc (long numVar, long numRecord, random_t* randomPtr)
{
    data_t* dataPtr;

    dataPtr = (data_t*)malloc(sizeof(data_t));
    if (dataPtr) {
        long numDatum = numVar * numRecord;
        dataPtr->records = (char*)malloc(numDatum * sizeof(char));
        if (dataPtr->records == NULL) {
            free(dataPtr);
            return NULL;
        }
        memset(dataPtr->records, DATA_INIT, (numDatum * sizeof(char)));
        dataPtr->numVar = numVar;
        dataPtr->numRecord = numRecord;
        dataPtr->randomPtr = randomPtr;
    }

    return dataPtr;
}


/* =============================================================================
 * data_free
 * =============================================================================
 */
void
data_free (data_t* dataPtr)
{
    free(dataPtr->records);
    free(dataPtr);
}


/* =============================================================================
 * data_generate
 * -- Binary variables of random PDFs
 * -- If seed is <0, do not reseed
 * -- Returns random network
 * =============================================================================
 */
net_t*
data_generate (data_t* dataPtr, long seed, long maxNumParent, long percentParent)
{
    random_t* randomPtr = dataPtr->randomPtr;
    if (seed >= 0) {
        random_seed(randomPtr, seed);
    }

    /*
     * Generate random Bayesian network
     */

    long numVar = dataPtr->numVar;
    net_t* netPtr = net_alloc(numVar);
    assert(netPtr);
    net_generateRandomEdges(netPtr, maxNumParent, percentParent, randomPtr);

    /*
     * Create a threshold for each of the possible permutation of variable
     * value instances
     */

    long** thresholdsTable = (long**)malloc(numVar * sizeof(long*));
    assert(thresholdsTable);
    long v;
    for (v = 0; v < numVar; v++) {
        list_t* parentIdListPtr = net_getParentIdListPtr(netPtr, v);
        long numThreshold = 1 << list_getSize(parentIdListPtr);
        long* thresholds = (long*)malloc(numThreshold * sizeof(long));
        assert(thresholds);
        long t;
        for (t = 0; t < numThreshold; t++) {
            long threshold = random_generate(randomPtr) % (DATA_PRECISION + 1);
            thresholds[t] = threshold;
        }
        thresholdsTable[v] = thresholds;
    }

    /*
     * Create variable dependency ordering for record generation
     */

    long* order = (long*)malloc(numVar * sizeof(long));
    assert(order);
    long numOrder = 0;

    queue_t* workQueuePtr = queue_alloc(-1);
    assert(workQueuePtr);

    vector_t* dependencyVectorPtr = vector_alloc(1);
    assert(dependencyVectorPtr);

    bitmap_t* orderedBitmapPtr = bitmap_alloc(numVar);
    assert(orderedBitmapPtr);
    bitmap_clearAll(orderedBitmapPtr);

    bitmap_t* doneBitmapPtr = bitmap_alloc(numVar);
    assert(doneBitmapPtr);
    bitmap_clearAll(doneBitmapPtr);
    v = -1;
    while ((v = bitmap_findClear(doneBitmapPtr, (v + 1))) >= 0) {
        list_t* childIdListPtr = net_getChildIdListPtr(netPtr, v);
        long numChild = list_getSize(childIdListPtr);
        if (numChild == 0) {

            bool_t status;

            /*
             * Use breadth-first search to find net connected to this leaf
             */

            queue_clear(workQueuePtr);
            status = queue_push(workQueuePtr, (void*)v);
            assert(status);
            while (!queue_isEmpty(workQueuePtr)) {
                long id = (long)queue_pop(workQueuePtr);
                status = bitmap_set(doneBitmapPtr, id);
                assert(status);
                status = vector_pushBack(dependencyVectorPtr, (void*)id);
                assert(status);
                list_t* parentIdListPtr = net_getParentIdListPtr(netPtr, id);
                list_iter_t it;
                list_iter_reset(&it, parentIdListPtr);
                while (list_iter_hasNext(&it, parentIdListPtr)) {
                    long parentId = (long)list_iter_next(&it, parentIdListPtr);
                    status = queue_push(workQueuePtr, (void*)parentId);
                    assert(status);
                }
            }

            /*
             * Create ordering
             */

            long i;
            long n = vector_getSize(dependencyVectorPtr);
            for (i = 0; i < n; i++) {
                long id = (long)vector_popBack(dependencyVectorPtr);
                if (!bitmap_isSet(orderedBitmapPtr, id)) {
                    bitmap_set(orderedBitmapPtr, id);
                    order[numOrder++] = id;
                }
            }

        }
    }
    assert(numOrder == numVar);

    /*
     * Create records
     */

    char* record = dataPtr->records;
    long r;
    long numRecord = dataPtr->numRecord;
    for (r = 0; r < numRecord; r++) {
        long o;
        for (o = 0; o < numOrder; o++) {
            long v = order[o];
            list_t* parentIdListPtr = net_getParentIdListPtr(netPtr, v);
            long index = 0;
            list_iter_t it;
            list_iter_reset(&it, parentIdListPtr);
            while (list_iter_hasNext(&it, parentIdListPtr)) {
                long parentId = (long)list_iter_next(&it, parentIdListPtr);
                long value = record[parentId];
                assert(value != DATA_INIT);
                index = (index << 1) + value;
            }
            long rnd = random_generate(randomPtr) % DATA_PRECISION;
            long threshold = thresholdsTable[v][index];
            record[v] = ((rnd < threshold) ? 1 : 0);
        }
        record += numVar;
        assert(record <= (dataPtr->records + numRecord * numVar));
    }

    /*
     * Clean up
     */

    bitmap_free(doneBitmapPtr);
    bitmap_free(orderedBitmapPtr);
    vector_free(dependencyVectorPtr);
    queue_free(workQueuePtr);
    free(order);
    for (v = 0; v < numVar; v++) {
        free(thresholdsTable[v]);
    }
    free(thresholdsTable);

    return netPtr;
}


/* =============================================================================
 * data_getRecord
 * -- Returns NULL if invalid index
 * =============================================================================
 */
char*
data_getRecord (data_t* dataPtr, long index)
{
    if (index < 0 || index >= (dataPtr->numRecord)) {
        return NULL;
    }

    return &dataPtr->records[index * dataPtr->numVar];
}


/* =============================================================================
 * data_copy
 * -- Returns FALSE on failure
 * =============================================================================
 */
bool_t
data_copy (data_t* dstPtr, data_t* srcPtr)
{
    long numDstDatum = dstPtr->numVar * dstPtr->numRecord;
    long numSrcDatum = srcPtr->numVar * srcPtr->numRecord;
    if (numDstDatum != numSrcDatum) {
        free(dstPtr->records);
        dstPtr->records = (char*)calloc(numSrcDatum, sizeof(char));
        if (dstPtr->records == NULL) {
            return FALSE;
        }
    }

    dstPtr->numVar    = srcPtr->numVar;
    dstPtr->numRecord = srcPtr->numRecord;
    memcpy(dstPtr->records, srcPtr->records, (numSrcDatum * sizeof(char)));

    return TRUE;
}


/* =============================================================================
 * compareRecord
 * =============================================================================
 */
static int
compareRecord (const void* p1, const void* p2, long n, long offset)
{
    long i = n - offset;
    const char* s1 = (const char*)p1 + offset;
    const char* s2 = (const char*)p2 + offset;

    while (i-- > 0) {
        unsigned char u1 = (unsigned char)*s1++;
        unsigned char u2 = (unsigned char)*s2++;
        if (u1 != u2) {
            return (u1 - u2);
        }
    }

    return 0;
}


/* =============================================================================
 * data_sort
 * -- In place
 * =============================================================================
 */
void
data_sort (data_t* dataPtr,
           long start,
           long num,
           long offset)
{
    assert(start >= 0 && start <= dataPtr->numRecord);
    assert(num >= 0 && num <= dataPtr->numRecord);
    assert(start + num >= 0 && start + num <= dataPtr->numRecord);

    long numVar = dataPtr->numVar;

    sort((dataPtr->records + (start * numVar)),
          num,
          numVar,
          &compareRecord,
          numVar,
          offset);
}


/* =============================================================================
 * data_findSplit
 * -- Call data_sort first with proper start, num, offset
 * -- Returns number of zeros in offset column
 * =============================================================================
 */
long
data_findSplit (data_t* dataPtr, long start, long num, long offset)
{
    long low = start;
    long high = start + num - 1;

    long numVar = dataPtr->numVar;
    char* records = dataPtr->records;

    while (low <= high) {
        long mid = (low + high) / 2;
        if (records[numVar * mid + offset] == 0) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    return (low - start);
}


/* #############################################################################
 * TEST_DATA
 * #############################################################################
 */
#ifdef TEST_DATA

#include <stdio.h>
#include <string.h>


static void
printRecords (data_t* dataPtr)
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
    puts("");
}


static void
testAll (long numVar, long numRecord, long numMaxParent, long percentParent)
{
    random_t* randomPtr = random_alloc();

    puts("Starting...");

    data_t* dataPtr = data_alloc(numVar, numRecord, randomPtr);
    assert(dataPtr);

    puts("Init:");
    net_t* netPtr = data_generate(dataPtr, 0, numMaxParent, percentParent);
    net_free(netPtr);
    printRecords(dataPtr);

    puts("Sort first half from 0:");
    data_sort(dataPtr, 0, numRecord/2, 0);
    printRecords(dataPtr);

    puts("Sort second half from 0:");
    data_sort(dataPtr, numRecord/2, numRecord-numRecord/2, 0);
    printRecords(dataPtr);

    puts("Sort all from mid:");
    data_sort(dataPtr, 0, numRecord, numVar/2);
    printRecords(dataPtr);

    long split = data_findSplit(dataPtr, 0, numRecord, numVar/2);
    printf("Split = %li\n", split);

    long v;
    for (v = 0; v < numVar; v++) {
        data_sort(dataPtr, 0, numRecord, v);
        long s = data_findSplit(dataPtr, 0, numRecord, v);
        if (s < numRecord) {
            assert(dataPtr->records[numVar * s + v] == 1);
        }
        if (s > 0) {
            assert(dataPtr->records[numVar * (s - 1)  + v] == 0);
        }
    }

    memset(dataPtr->records, 0, dataPtr->numVar * dataPtr->numRecord);
    for (v = 0; v < numVar; v++) {
        data_sort(dataPtr, 0, numRecord, v);
        long s = data_findSplit(dataPtr, 0, numRecord, v);
        if (s < numRecord) {
            assert(dataPtr->records[numVar * s + v] == 1);
        }
        if (s > 0) {
            assert(dataPtr->records[numVar * (s - 1)  + v] == 0);
        }
        assert(s == numRecord);
    }

    memset(dataPtr->records, 1, dataPtr->numVar * dataPtr->numRecord);
    for (v = 0; v < numVar; v++) {
        data_sort(dataPtr, 0, numRecord, v);
        long s = data_findSplit(dataPtr, 0, numRecord, v);
        if (s < numRecord) {
            assert(dataPtr->records[numVar * s + v] == 1);
        }
        if (s > 0) {
            assert(dataPtr->records[numVar * (s - 1)  + v] == 0);
        }
        assert(s == 0);
    }

    data_free(dataPtr);
}


static void
testBasic (long numVar, long numRecord, long numMaxParent, long percentParent)
{
    random_t* randomPtr = random_alloc();

    puts("Starting...");

    data_t* dataPtr = data_alloc(numVar, numRecord, randomPtr);
    assert(dataPtr);

    puts("Init:");
    data_generate(dataPtr, 0, numMaxParent, percentParent);

    long v;
    for (v = 0; v < numVar; v++) {
        data_sort(dataPtr, 0, numRecord, v);
        long s = data_findSplit(dataPtr, 0, numRecord, v);
        if (s < numRecord) {
            assert(dataPtr->records[numVar * s + v] == 1);
        }
        if (s > 0) {
            assert(dataPtr->records[numVar * (s - 1)  + v] == 0);
        }
    }

    memset(dataPtr->records, 0, dataPtr->numVar * dataPtr->numRecord);
    for (v = 0; v < numVar; v++) {
        data_sort(dataPtr, 0, numRecord, v);
        long s = data_findSplit(dataPtr, 0, numRecord, v);
        if (s < numRecord) {
            assert(dataPtr->records[numVar * s + v] == 1);
        }
        if (s > 0) {
            assert(dataPtr->records[numVar * (s - 1)  + v] == 0);
        }
        assert(s == numRecord);
    }

    memset(dataPtr->records, 1, dataPtr->numVar * dataPtr->numRecord);
    for (v = 0; v < numVar; v++) {
        data_sort(dataPtr, 0, numRecord, v);
        long s = data_findSplit(dataPtr, 0, numRecord, v);
        if (s < numRecord) {
            assert(dataPtr->records[numVar * s + v] == 1);
        }
        if (s > 0) {
            assert(dataPtr->records[numVar * (s - 1)  + v] == 0);
        }
        assert(s == 0);
    }

    data_free(dataPtr);
}


int
main ()
{
    puts("Test 1:");
    testAll(10, 20, 10, 10);

    puts("Test 2:");
    testBasic(20, 80, 10, 20);

    puts("Done");

    return 0;
}


#endif /* TEST_DATA */


/* =============================================================================
 *
 * End of data.c
 *
 * =============================================================================
 */
