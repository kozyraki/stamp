/* =============================================================================
 *
 * gene.c
 * -- Create random gene
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
#include "gene.h"
#include "nucleotide.h"
#include "random.h"
#include "tm.h"


/* =============================================================================
 * gene_alloc
 * -- Does all memory allocation necessary for gene creation
 * -- Returns NULL on failure
 * =============================================================================
 */
gene_t*
gene_alloc (long length)
{
    gene_t* genePtr;

    assert(length > 1);

    genePtr = (gene_t*)malloc(sizeof(gene_t));
    if (genePtr == NULL) {
        return NULL;
    }

    genePtr->contents = (char*)malloc((length + 1) * sizeof(char));
    if (genePtr->contents == NULL) {
        return NULL;
    }
    genePtr->contents[length] = '\0';
    genePtr->length = length;

    genePtr->startBitmapPtr = bitmap_alloc(length);
    if (genePtr->startBitmapPtr == NULL) {
        return NULL;
    }

    return genePtr;
}


/* =============================================================================
 * gene_create
 * -- Populate contents with random gene
 * =============================================================================
 */
void
gene_create (gene_t* genePtr, random_t* randomPtr)
{
    long length;
    char* contents;
    long i;
    const char nucleotides[] = {
        NUCLEOTIDE_ADENINE,
        NUCLEOTIDE_CYTOSINE,
        NUCLEOTIDE_GUANINE,
        NUCLEOTIDE_THYMINE,
    };

    assert(genePtr != NULL);
    assert(randomPtr != NULL);

    length = genePtr->length;
    contents = genePtr->contents;

    for (i = 0; i < length; i++) {
        contents[i] =
            nucleotides[(random_generate(randomPtr)% NUCLEOTIDE_NUM_TYPE)];
    }
}


/* =============================================================================
 * gene_free
 * =============================================================================
 */
void
gene_free (gene_t* genePtr)
{
    bitmap_free(genePtr->startBitmapPtr);
    free(genePtr->contents);
    free(genePtr);
}


/* =============================================================================
 * TEST_GENE
 * =============================================================================
 */
#ifdef TEST_GENE


#include <assert.h>
#include <stdio.h>
#include <string.h>


int
main ()
{
    gene_t* gene1Ptr;
    gene_t* gene2Ptr;
    gene_t* gene3Ptr;
    random_t* randomPtr;

    bool_t status = memory_init(1, 4, 2);
    assert(status);

    puts("Starting...");

    gene1Ptr = gene_alloc(10);
    gene2Ptr = gene_alloc(10);
    gene3Ptr = gene_alloc(9);
    randomPtr = random_alloc();

    random_seed(randomPtr, 0);
    gene_create(gene1Ptr, randomPtr);
    random_seed(randomPtr, 1);
    gene_create(gene2Ptr, randomPtr);
    random_seed(randomPtr, 0);
    gene_create(gene3Ptr, randomPtr);

    assert(gene1Ptr->length == strlen(gene1Ptr->contents));
    assert(gene2Ptr->length == strlen(gene2Ptr->contents));
    assert(gene3Ptr->length == strlen(gene3Ptr->contents));

    assert(gene1Ptr->length == gene2Ptr->length);
    assert(strcmp(gene1Ptr->contents, gene2Ptr->contents) != 0);

    assert(gene1Ptr->length == (gene3Ptr->length + 1));
    assert(strcmp(gene1Ptr->contents, gene3Ptr->contents) != 0);
    assert(strncmp(gene1Ptr->contents,
                   gene3Ptr->contents,
                   gene3Ptr->length) == 0);

    gene_free(gene1Ptr);
    gene_free(gene2Ptr);
    gene_free(gene3Ptr);
    random_free(randomPtr);

    puts("All tests passed.");

    return 0;
}


#endif /* TEST_GENE */


/* =============================================================================
 *
 * End of gene.c
 *
 * =============================================================================
 */
