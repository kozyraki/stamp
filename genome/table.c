/* =============================================================================
 *
 * table.c
 * -- Fixed-size hash table
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
#include "list.h"
#include "table.h"
#include "types.h"


/* =============================================================================
 * table_alloc
 * -- Returns NULL on failure
 * =============================================================================
 */
table_t*
table_alloc (long numBucket, long (*compare)(const void*, const void*))
{
    table_t* tablePtr;
    long i;

    tablePtr = (table_t*)malloc(sizeof(table_t));
    if (tablePtr == NULL) {
        return NULL;
    }

    tablePtr->buckets = (list_t**)malloc(numBucket * sizeof(list_t*));
    if (tablePtr->buckets == NULL) {
        return NULL;
    }

    for (i = 0; i < numBucket; i++) {
        tablePtr->buckets[i] = list_alloc(compare);
        if (tablePtr->buckets[i] == NULL) {
            return NULL;
        }
    }

    tablePtr->numBucket = numBucket;

    return tablePtr;
}


/* =============================================================================
 * table_insert
 * -- Returns TRUE if successful, else FALSE
 * =============================================================================
 */
bool_t
table_insert (table_t* tablePtr, ulong_t hash, void* dataPtr)
{
    long i = hash % tablePtr->numBucket;

    if (!list_insert(tablePtr->buckets[i], dataPtr)) {
        return FALSE;
    }

    return TRUE;
}


/* =============================================================================
 * TMtable_insert
 * -- Returns TRUE if successful, else FALSE
 * =============================================================================
 */
bool_t
TMtable_insert (TM_ARGDECL  table_t* tablePtr, ulong_t hash, void* dataPtr)
{
    long i = hash % tablePtr->numBucket;

    if (!TMLIST_INSERT(tablePtr->buckets[i], dataPtr)) {
        return FALSE;
    }

    return TRUE;
}


/* =============================================================================
 * table_remove
 * -- Returns TRUE if successful, else FALSE
 * =============================================================================
 */
bool_t
table_remove (table_t* tablePtr, ulong_t hash, void* dataPtr)
{
    long i = hash % tablePtr->numBucket;

    if (!list_remove(tablePtr->buckets[i], dataPtr)) {
        return FALSE;
    }

    return TRUE;
}


/* =============================================================================
 * table_free
 * =============================================================================
 */
void
table_free (table_t* tablePtr)
{
#if 0
   /* TODO: fix mixed sequential/parallel allocation */
    long i;

    for (i = 0; i < tablePtr->numBucket; i++) {
        list_free(tablePtr->buckets[i]);
    }
#endif

    free(tablePtr);
}


/* =============================================================================
 * TEST_TABLE
 * =============================================================================
 */
#ifdef TEST_TABLE


#include <stdio.h>


static void
printTable (table_t* tablePtr)
{
    long i;

    for (i = 0; i < tablePtr->numBucket; i++) {
        list_iter_t it;
        printf("%2i: [", i);
        list_iter_reset(&it, tablePtr->buckets[i]);
        while (list_iter_hasNext(&it, tablePtr->buckets[i])) {
            printf("%li ", *(long*)list_iter_next(&it, tablePtr->buckets[i]));
        }
        puts("]");
    }
}


int
main ()
{
    table_t* tablePtr;
    long hash[] = {3, 1, 4, 1, 5, 9, 2, 6, 8, 7, -1};
    long i;

    bool_t status = memory_init(1, 4, 2);
    assert(status);

    puts("Starting...");

    tablePtr = table_alloc(8, NULL);

    for (i = 0; hash[i] >= 0; i++ ) {
        bool_t status = table_insert(tablePtr,
                                     (ulong_t)hash[i],
                                     (void*)&hash[i])
        assert(status);
        printTable(tablePtr);
        puts("");
    }

    for (i = 0; hash[i] >= 0; i++ ) {
        bool_t status = table_remove(tablePtr,
                                     (ulong_t)hash[i],
                                     (void*)&hash[i])
        assert(status);
        printTable(tablePtr);
        puts("");
    }

    table_free(tablePtr);

    puts("Done.");

    return 0;
}


#endif /* TEST_TABLE */


/* =============================================================================
 *
 * End of table.c
 *
 * =============================================================================
 */
