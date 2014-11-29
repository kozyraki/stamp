/* =============================================================================
 *
 * detector.c
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
#include "detector.h"
#include "dictionary.h"
#include "error.h"
#include "preprocessor.h"
#include "tm.h"
#include "vector.h"


struct detector {
    dictionary_t* dictionaryPtr;
    vector_t* preprocessorVectorPtr;
};


/* =============================================================================
 * detector_alloc
 * =============================================================================
 */
detector_t*
detector_alloc ()
{
    detector_t* detectorPtr;

    detectorPtr = (detector_t*)malloc(sizeof(detector_t));
    if (detectorPtr) {
        detectorPtr->dictionaryPtr = dictionary_alloc();
        assert(detectorPtr->dictionaryPtr);
        detectorPtr->preprocessorVectorPtr = vector_alloc(1);
        assert(detectorPtr->preprocessorVectorPtr);
    }

    return detectorPtr;
}


/* =============================================================================
 * Pdetector_alloc
 * =============================================================================
 */
detector_t*
Pdetector_alloc ()
{
    detector_t* detectorPtr;

    detectorPtr = (detector_t*)P_MALLOC(sizeof(detector_t));
    if (detectorPtr) {
        detectorPtr->dictionaryPtr = PDICTIONARY_ALLOC();
        assert(detectorPtr->dictionaryPtr);
        detectorPtr->preprocessorVectorPtr = PVECTOR_ALLOC(1);
        assert(detectorPtr->preprocessorVectorPtr);
    }

    return detectorPtr;
}


/* =============================================================================
 * detector_free
 * =============================================================================
 */
void
detector_free (detector_t* detectorPtr)
{
    dictionary_free(detectorPtr->dictionaryPtr);
    vector_free(detectorPtr->preprocessorVectorPtr);
    free(detectorPtr);
}


/* =============================================================================
 * Pdetector_free
 * =============================================================================
 */
void
Pdetector_free (detector_t* detectorPtr)
{
    PDICTIONARY_FREE(detectorPtr->dictionaryPtr);
    PVECTOR_FREE(detectorPtr->preprocessorVectorPtr);
    P_FREE(detectorPtr);
}


/* =============================================================================
 * detector_addPreprocessor
 * =============================================================================
 */
void
detector_addPreprocessor (detector_t* detectorPtr, preprocessor_t p)
{
    bool_t status = vector_pushBack(detectorPtr->preprocessorVectorPtr,
                                    (void*)p);
    assert(status);
}


/* =============================================================================
 * detector_process
 * =============================================================================
 */
error_t
detector_process (detector_t* detectorPtr, char* str)
{
    /*
     * Apply preprocessors
     */

    vector_t* preprocessorVectorPtr = detectorPtr->preprocessorVectorPtr;
    long p;
    long numPreprocessor = vector_getSize(preprocessorVectorPtr);
    for (p = 0; p < numPreprocessor; p++) {
        preprocessor_t preprocessor =
            (preprocessor_t)vector_at(preprocessorVectorPtr, p);
        preprocessor(str);
    }

    /*
     * Check against signatures of known attacks
     */

    char* signature = dictionary_match(detectorPtr->dictionaryPtr, str);
    if (signature) {
        return ERROR_SIGNATURE;
    }

    return ERROR_NONE;
}


/* #############################################################################
 * TEST_DETECTOR
 * #############################################################################
 */
#ifdef TEST_DETECTOR


#include <assert.h>
#include <stdio.h>


char str1[] = "test";
char str2[] = "abouts";
char str3[] = "aBoUt";
char str4[] = "%41Bout";


int
main ()
{
    puts("Starting...");

    detector_t* detectorPtr = detector_alloc();

    detector_addPreprocessor(detectorPtr, &preprocessor_convertURNHex);
    detector_addPreprocessor(detectorPtr, &preprocessor_toLower);

    assert(detector_process(detectorPtr, str1) == ERROR_NONE);
    assert(detector_process(detectorPtr, str2) == ERROR_SIGNATURE);
    assert(detector_process(detectorPtr, str3) == ERROR_SIGNATURE);
    assert(detector_process(detectorPtr, str4) == ERROR_SIGNATURE);

    detector_free(detectorPtr);

    puts("All tests passed.");

    return 0;
}


#endif /* TEST_DETECTOR */


/* =============================================================================
 *
 * End of detector.c
 *
 * =============================================================================
 */
