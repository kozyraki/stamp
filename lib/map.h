/* =============================================================================
 *
 * map.h
 * -- Utility defines to use various data structures as map
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


#ifndef MAP_H
#define MAP_H 1


#include <stdlib.h>
#include "pair.h"
#include "types.h"


#if defined(MAP_USE_HASHTABLE)

#  include "hashtable.h"

#  define MAP_T                       hashtable_t
#  define MAP_ALLOC(hash, cmp)        hashtable_alloc(1, hash, cmp, 2, 2)
#  define MAP_FREE(map)               hashtable_free(map)
#  define MAP_CONTAINS(map, key)      hashtable_containsKey(map, (void*)(key))
#  define MAP_FIND(map, key)          hashtable_find(map, (void*)(key))
#  define MAP_INSERT(map, key, data)  hashtable_insert(map, (void*)(key), (void*)(data))
#  define MAP_REMOVE(map, key)        hashtable_remove(map, (void*)(key))

#elif defined(MAP_USE_ATREE)

#  include "atree.h"

#  define MAP_T                       jsw_atree_t
#  define MAP_ALLOC(hash, cmp)        jsw_anew((cmp_f)cmp)
#  define MAP_FREE(map)               jsw_adelete(map)
#  define MAP_CONTAINS(map, key) \
    ({ \
        bool_t success = FALSE; \
        pair_t searchPair; \
        searchPair.firstPtr = (void*)key; \
        if (jsw_afind(map, (void*)&searchPair) != NULL) { \
            success = TRUE; \
        } \
        success; \
     })
#  define MAP_FIND(map, key) \
    ({ \
        void* dataPtr = NULL; \
        pair_t searchPair; \
        searchPair.firstPtr = (void*)(key); \
        pair_t* pairPtr = (pair_t*)jsw_afind(map, (void*)&searchPair); \
        if (pairPtr != NULL) { \
            dataPtr = pairPtr->secondPtr; \
        } \
        dataPtr; \
     })
#  define MAP_INSERT(map, key, data) \
    ({ \
        bool_t success = FALSE; \
        pair_t* insertPtr = pair_alloc((void*)(key), (void*)data); \
        if (insertPtr != NULL) { \
            if (jsw_ainsert(map, (void*)insertPtr)) { \
                success = TRUE; \
            } \
        } \
        success; \
     })
#  define MAP_REMOVE(map, key) \
    ({ \
        bool_t success = FALSE; \
        pair_t searchPair; \
        searchPair.firstPtr = (void*)(key); \
        pair_t* pairPtr = (pair_t*)jsw_afind(map, (void*)&searchPair); \
        if (jsw_aerase(map, (void*)&searchPair)) { \
            pair_free(pairPtr); \
            success = TRUE; \
        } \
        success; \
     })

#elif defined(MAP_USE_AVLTREE)

#  include "avltree.h"

#  define MAP_T                       jsw_avltree_t
#  define MAP_ALLOC(hash, cmp)        jsw_avlnew((cmp_f)cmp)
#  define MAP_FREE(map)               jsw_avldelete(map)
#  define MAP_CONTAINS(map, key) \
    ({ \
        bool_t success = FALSE; \
        pair_t searchPair; \
        searchPair.firstPtr = (void*)key; \
        if (jsw_avlfind(map, (void*)&searchPair) != NULL) { \
            success = TRUE; \
        } \
        success; \
     })
#  define MAP_FIND(map, key) \
    ({ \
        void* dataPtr = NULL; \
        pair_t searchPair; \
        searchPair.firstPtr = (void*)(key); \
        pair_t* pairPtr = (pair_t*)jsw_avlfind(map, (void*)&searchPair); \
        if (pairPtr != NULL) { \
            dataPtr = pairPtr->secondPtr; \
        } \
        dataPtr; \
     })
#  define MAP_INSERT(map, key, data) \
    ({ \
        bool_t success = FALSE; \
        pair_t* insertPtr = pair_alloc((void*)(key), (void*)data); \
        if (insertPtr != NULL) { \
            if (jsw_avlinsert(map, (void*)insertPtr)) { \
                success = TRUE; \
            } \
        } \
        success; \
     })
#  define MAP_REMOVE(map, key) \
    ({ \
        bool_t success = FALSE; \
        pair_t searchPair; \
        searchPair.firstPtr = (void*)(key); \
        pair_t* pairPtr = (pair_t*)jsw_avlfind(map, (void*)&searchPair); \
        if (jsw_avlerase(map, (void*)&searchPair)) { \
            pair_free(pairPtr); \
            success = TRUE; \
        } \
        success; \
     })

#  define PMAP_ALLOC(hash, cmp)        Pjsw_avlnew((cmp_f)cmp)
#  define PMAP_FREE(map)               Pjsw_avldelete(map)
#  define PMAP_INSERT(map, key, data) \
    ({ \
        bool_t success = FALSE; \
        pair_t* insertPtr = PPAIR_ALLOC((void*)(key), (void*)data); \
        if (insertPtr != NULL) { \
            if (Pjsw_avlinsert(map, (void*)insertPtr)) { \
                success = TRUE; \
            } \
        } \
        success; \
     })
#  define PMAP_REMOVE(map, key) \
    ({ \
        bool_t success = FALSE; \
        pair_t searchPair; \
        searchPair.firstPtr = (void*)(key); \
        pair_t* pairPtr = (pair_t*)jsw_avlfind(map, (void*)&searchPair); \
        if (Pjsw_avlerase(map, (void*)&searchPair)) { \
            PPAIR_FREE(pairPtr); \
            success = TRUE; \
        } \
        success; \
     })


#elif defined(MAP_USE_RBTREE)

#  include "rbtree.h"

#  define MAP_T                       rbtree_t
#  define MAP_ALLOC(hash, cmp)        rbtree_alloc(cmp)
#  define MAP_FREE(map)               rbtree_free(map)

#  define MAP_CONTAINS(map, key)      rbtree_contains(map, (void*)(key))
#  define MAP_FIND(map, key)          rbtree_get(map, (void*)(key))
#  define MAP_INSERT(map, key, data) \
    rbtree_insert(map, (void*)(key), (void*)(data))
#  define MAP_REMOVE(map, key)        rbtree_delete(map, (void*)(key))

#  define TMMAP_CONTAINS(map, key)    TMRBTREE_CONTAINS(map, (void*)(key))
#  define TMMAP_FIND(map, key)        TMRBTREE_GET(map, (void*)(key))
#  define TMMAP_INSERT(map, key, data) \
    TMRBTREE_INSERT(map, (void*)(key), (void*)(data))
#  define TMMAP_REMOVE(map, key)      TMRBTREE_DELETE(map, (void*)(key))


#elif defined(MAP_USE_SKIPLIST)

#  include "skiplist.h"

#  define SKIPLIST_MAX_HEIGHT (64)

#  define MAP_T                       jsw_skip_t
#  define MAP_ALLOC(hash, cmp)        jsw_snew(SKIPLIST_MAX_HEIGHT, (cmp_f)cmp)
#  define MAP_FREE(map)               jsw_sdelete(map)
#  define MAP_CONTAINS(map, key) \
    ({ \
        bool_t success = FALSE; \
        pair_t searchPair; \
        searchPair.firstPtr = (void*)key; \
        if (jsw_sfind(map, (void*)&searchPair) != NULL) { \
            success = TRUE; \
        } \
        success; \
     })
#  define MAP_FIND(map, key) \
    ({ \
        void* dataPtr = NULL; \
        pair_t* pairPtr; \
        pair_t searchPair; \
        searchPair.firstPtr = (void*)(key); \
        pairPtr = (pair_t*)jsw_sfind(map, (void*)&searchPair); \
        if (pairPtr != NULL) { \
            dataPtr = pairPtr->secondPtr; \
        } \
        dataPtr; \
     })
#  define MAP_INSERT(map, key, data) \
    ({ \
        bool_t success = FALSE; \
        pair_t* insertPtr = pair_alloc((void*)(key), (void*)data); \
        if (insertPtr != NULL) { \
            if (jsw_sinsert(map, (void*)insertPtr)) { \
                success = TRUE; \
            } \
        } \
        success; \
     })
#  define MAP_REMOVE(map, key) \
    ({ \
        bool_t success = FALSE; \
        pair_t searchPair; \
        searchPair.firstPtr = (void*)(key); \
        pairPtr = (pair_t*)jsw_sfind(map, (void*)&searchPair); \
        if (jsw_serase(map, (void*)&searchPair)) { \
            pair_free(pairPtr); \
            success = TRUE; \
        } \
        success; \
     })

#else

#  error "MAP type is not specified"

#endif


#endif /* MAP_H */


/* =============================================================================
 *
 * End of map.h
 *
 * =============================================================================
 */
