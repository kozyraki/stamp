/* =============================================================================
 *
 * stream.h
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


#ifndef STREAM_H
#define STREAM_H 1

#include "dictionary.h"
#include "tm.h"

typedef struct stream stream_t;


/* =============================================================================
 * stream_alloc
 * =============================================================================
 */
stream_t*
stream_alloc (long percentAttack);


/* =============================================================================
 * stream_free
 * =============================================================================
 */
void
stream_free (stream_t* streamPtr);


/* =============================================================================
 * stream_generate
 * -- Returns number of attacks generated
 * =============================================================================
 */
long
stream_generate (stream_t* streamPtr,
                 dictionary_t* dictionaryPtr,
                 long numFlow,
                 long seed,
                 long maxLength);


/* =============================================================================
 * stream_getPacket
 * -- If none, returns NULL
 * =============================================================================
 */
char*
stream_getPacket (stream_t* streamPtr);


/* =============================================================================
 * TMstream_getPacket
 * -- If none, returns NULL
 * =============================================================================
 */
char*
TMstream_getPacket (TM_ARGDECL stream_t* streamPtr);


/* =============================================================================
 * stream_isAttack
 * =============================================================================
 */
bool_t
stream_isAttack (stream_t* streamPtr, long flowId);


#define TMSTREAM_GETPACKET(s)           TMstream_getPacket(TM_ARG  s)

#endif /* STREAM_H */


/* =============================================================================
 *
 * End of stream.h
 *
 * =============================================================================
 */
