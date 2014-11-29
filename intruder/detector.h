/* =============================================================================
 *
 * detector.h
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


#ifndef DETECTOR_H
#define DETECTOR_H 1


#include "error.h"
#include "preprocessor.h"

typedef struct detector detector_t;


/* =============================================================================
 * detector_alloc
 * =============================================================================
 */
detector_t*
detector_alloc ();


/* =============================================================================
 * Pdetector_alloc
 * =============================================================================
 */
detector_t*
Pdetector_alloc ();


/* =============================================================================
 * detector_free
 * =============================================================================
 */
void
detector_free (detector_t* detectorPtr);


/* =============================================================================
 * Pdetector_free
 * =============================================================================
 */
void
Pdetector_free (detector_t* detectorPtr);


/* =============================================================================
 * detector_addPreprocessor
 * =============================================================================
 */
void
detector_addPreprocessor (detector_t* detectorPtr, preprocessor_t p);


/* =============================================================================
 * detector_process
 * =============================================================================
 */
error_t
detector_process (detector_t* detectorPtr, char* str);


#define PDETECTOR_ALLOC()               Pdetector_alloc()
#define PDETECTOR_FREE(d)               Pdetector_free(d)
#define PDETECTOR_PROCESS(d, s)         detector_process(d, s)
#define PDETECTOR_ADDPREPROCESSOR(d, s) detector_addPreprocessor(d, s)


#endif /* DETECTOR_H */


/* =============================================================================
 *
 * End of detector.h
 *
 * =============================================================================
 */
