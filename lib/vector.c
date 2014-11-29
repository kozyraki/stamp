/* =============================================================================
 *
 * vector.c
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
#include <string.h>
#include "tm.h"
#include "types.h"
#include "utility.h"
#include "vector.h"


/* =============================================================================
 * vector_alloc
 * -- Returns NULL if failed
 * =============================================================================
 */
vector_t*
vector_alloc (long initCapacity)
{
    vector_t* vectorPtr;
    long capacity = MAX(initCapacity, 1);

    vectorPtr = (vector_t*)malloc(sizeof(vector_t));

    if (vectorPtr != NULL) {
        vectorPtr->size = 0;
        vectorPtr->capacity = capacity;
        vectorPtr->elements = (void**)malloc(capacity * sizeof(void*));
        if (vectorPtr->elements == NULL) {
            return NULL;
        }
    }

    return vectorPtr;
}


/* =============================================================================
 * Pvector_alloc
 * -- Returns NULL if failed
 * =============================================================================
 */
vector_t*
Pvector_alloc (long initCapacity)
{
    vector_t* vectorPtr;
    long capacity = MAX(initCapacity, 1);

    vectorPtr = (vector_t*)P_MALLOC(sizeof(vector_t));

    if (vectorPtr != NULL) {
        vectorPtr->size = 0;
        vectorPtr->capacity = capacity;
        vectorPtr->elements = (void**)P_MALLOC(capacity * sizeof(void*));
        if (vectorPtr->elements == NULL) {
            return NULL;
        }
    }

    return vectorPtr;
}


/* =============================================================================
 * vector_free
 * =============================================================================
 */
void
vector_free (vector_t* vectorPtr)
{
    free(vectorPtr->elements);
    free(vectorPtr);
}


/* =============================================================================
 * Pvector_free
 * =============================================================================
 */
void
Pvector_free (vector_t* vectorPtr)
{
    P_FREE(vectorPtr->elements);
    P_FREE(vectorPtr);
}


/* =============================================================================
 * vector_at
 * -- Returns NULL if failed
 * =============================================================================
 */
void*
vector_at (vector_t* vectorPtr, long i)
{
    if ((i < 0) || (i >= vectorPtr->size)) {
        return NULL;
    }

    return (vectorPtr->elements[i]);
}


/* =============================================================================
 * vector_pushBack
 * -- Returns FALSE if fail, else TRUE
 * =============================================================================
 */
bool_t
vector_pushBack (vector_t* vectorPtr, void* dataPtr)
{
    if (vectorPtr->size == vectorPtr->capacity) {
        long i;
        long newCapacity = vectorPtr->capacity * 2;
        void** newElements = (void**)malloc(newCapacity * sizeof(void*));
        if (newElements == NULL) {
            return FALSE;
        }
        vectorPtr->capacity = newCapacity;
        for (i = 0; i < vectorPtr->size; i++) {
            newElements[i] = vectorPtr->elements[i];
        }
        free(vectorPtr->elements);
        vectorPtr->elements = newElements;
    }

    vectorPtr->elements[vectorPtr->size++] = dataPtr;

    return TRUE;
}


/* =============================================================================
 * Pvector_pushBack
 * -- Returns FALSE if fail, else TRUE
 * =============================================================================
 */
bool_t
Pvector_pushBack (vector_t* vectorPtr, void* dataPtr)
{
    if (vectorPtr->size == vectorPtr->capacity) {
        long i;
        long newCapacity = vectorPtr->capacity * 2;
        void** newElements = (void**)P_MALLOC(newCapacity * sizeof(void*));
        if (newElements == NULL) {
            return FALSE;
        }
        vectorPtr->capacity = newCapacity;
        for (i = 0; i < vectorPtr->size; i++) {
            newElements[i] = vectorPtr->elements[i];
        }
        P_FREE(vectorPtr->elements);
        vectorPtr->elements = newElements;
    }

    vectorPtr->elements[vectorPtr->size++] = dataPtr;

    return TRUE;
}


/* =============================================================================
 * vector_popBack
 * Returns NULL if fail, else returns last element
 * =============================================================================
 */
void*
vector_popBack (vector_t* vectorPtr)
{
    if (vectorPtr->size < 1) {
        return NULL;
    }

    return (vectorPtr->elements[--(vectorPtr->size)]);
}


/* =============================================================================
 * vector_getSize
 * =============================================================================
 */
long
vector_getSize (vector_t* vectorPtr)
{
    return (vectorPtr->size);
}


/* =============================================================================
 * vector_clear
 * =============================================================================
 */
void
vector_clear (vector_t* vectorPtr)
{
    vectorPtr->size = 0;
}


/* =============================================================================
 * vector_sort
 * =============================================================================
 */
void
vector_sort (vector_t* vectorPtr, int (*compare) (const void*, const void*))
{
    qsort((void*)vectorPtr->elements, vectorPtr->size, sizeof(void**), compare);
}


/* =============================================================================
 * vector_copy
 * =============================================================================
 */
bool_t
vector_copy (vector_t* dstVectorPtr, vector_t* srcVectorPtr)
{
    long dstCapacity = dstVectorPtr->capacity;
    long srcSize = srcVectorPtr->size;
    if (dstCapacity < srcSize) {
        long srcCapacity = srcVectorPtr->capacity;
        void** elements = (void**)malloc(srcCapacity * sizeof(void*));
        if (elements == NULL) {
            return FALSE;
        }
        free(dstVectorPtr->elements);
        dstVectorPtr->elements = elements;
        dstVectorPtr->capacity = srcCapacity;
    }

    memcpy(dstVectorPtr->elements,
           srcVectorPtr->elements,
           (srcSize * sizeof(void*)));
    dstVectorPtr->size = srcSize;

    return TRUE;
}


/* =============================================================================
 * Pvector_copy
 * =============================================================================
 */
bool_t
Pvector_copy (vector_t* dstVectorPtr, vector_t* srcVectorPtr)
{
    long dstCapacity = dstVectorPtr->capacity;
    long srcSize = srcVectorPtr->size;
    if (dstCapacity < srcSize) {
        long srcCapacity = srcVectorPtr->capacity;
        void** elements = (void**)P_MALLOC(srcCapacity * sizeof(void*));
        if (elements == NULL) {
            return FALSE;
        }
        P_FREE(dstVectorPtr->elements);
        dstVectorPtr->elements = elements;
        dstVectorPtr->capacity = srcCapacity;
    }

    memcpy(dstVectorPtr->elements,
           srcVectorPtr->elements,
           (srcSize * sizeof(void*)));
    dstVectorPtr->size = srcSize;

    return TRUE;
}


/* =============================================================================
 * TEST_VECTOR
 * =============================================================================
 */
#ifdef TEST_VECTOR


#include <stdio.h>

static void
printVector (vector_t* vectorPtr)
{
    long i;
    long size = vector_getSize(vectorPtr);

    printf("[");
    for (i = 0; i < size; i++) {
        printf("%li ", *((long*)vector_at(vectorPtr, i)));
    }
    puts("]");
}


static void
insertInt (vector_t* vectorPtr, long* data)
{
    printf("Inserting: %li\n", *data);
    vector_pushBack(vectorPtr, (void*)data);
    printVector(vectorPtr);
}


static void
removeInt (vector_t* vectorPtr)
{
    printf("Removing: %li\n", *((long*)vector_popBack(vectorPtr)));
    printVector(vectorPtr);

}


static int
compareInt (const void* aPtr, const void* bPtr)
{
    long a = *((long*)(*(void**)aPtr));
    long b = *((long*)(*(void**)bPtr));

    return (a - b);
}


int
main ()
{
    vector_t* vectorPtr;
    vector_t* copyVectorPtr;
    long data[] = {3, 1, 4, 1, 5, -1};
    long i;

    puts("Starting...");

    vectorPtr = vector_alloc(1);
    copyVectorPtr = vector_alloc(1);

    for (i = 0; data[i] >= 0; i++) {
        insertInt(vectorPtr, &data[i]);
    }

    vector_copy(copyVectorPtr, vectorPtr);

    while (i-- > 0) {
        removeInt(vectorPtr);
    }

    printf("copy ");
    printVector(copyVectorPtr);
    printf("sort ");
    vector_sort(copyVectorPtr, &compareInt);
    printVector(copyVectorPtr);

    vector_free(vectorPtr);
    vector_free(copyVectorPtr);

    puts("Done.");

    return 0;
}


#endif /* TEST_VECTOR */


/* =============================================================================
 *
 * End of vector.c
 *
 * =============================================================================
 */
