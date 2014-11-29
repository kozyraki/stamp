/* =============================================================================
 *
 * avltree.h
 * -- AVL balanced tree library
 *
 * =============================================================================
 *
 * AVL balanced tree library
 *
 * > Created (Julienne Walker): June 17, 2003
 * > Modified (Julienne Walker): September 24, 2005
 *
 * This code is in the public domain. Anyone may
 * use it or change it in any way that they see
 * fit. The author assumes no responsibility for
 * damages incurred through use of the original
 * code or any variations thereof.
 *
 * It is requested, but not required, that due
 * credit is given to the original author and
 * anyone who has modified the code through
 * a header comment, such as this one.
 *
 * =============================================================================
 *
 * Modified May 5, 2006 by Chi Cao Minh
 *
 * - Changed to not need functions to duplicate and release the data pointer
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



#ifndef JSW_AVLTREE_H
#define JSW_AVLTREE_H


#ifdef __cplusplus
#include <cstddef>

using std::size_t;

extern "C" {
#else
#include <stddef.h>
#endif

#include "tm.h"

/* Opaque types */
typedef struct jsw_avltree jsw_avltree_t;
typedef struct jsw_avltrav jsw_avltrav_t;

/* User-defined item handling */
typedef long   (*cmp_f) ( const void *p1, const void *p2 );
#if USE_DUP_AND_REL
typedef void *(*dup_f) ( void *p );
typedef void  (*rel_f) ( void *p );
#endif

/* AVL tree functions */
#if USE_DUP_AND_REL
jsw_avltree_t *jsw_avlnew ( cmp_f cmp, dup_f dup, rel_f rel );
jsw_avltree_t *Pjsw_avlnew ( cmp_f cmp, dup_f dup, rel_f rel );
#else
jsw_avltree_t *jsw_avlnew ( cmp_f cmp );
jsw_avltree_t *Pjsw_avlnew ( cmp_f cmp );
#endif
void           jsw_avldelete ( jsw_avltree_t *tree );
void           Pjsw_avldelete ( jsw_avltree_t *tree );
void          *jsw_avlfind ( jsw_avltree_t *tree, void *data );
long           jsw_avlinsert ( jsw_avltree_t *tree, void *data );
long           Pjsw_avlinsert ( jsw_avltree_t *tree, void *data );
long           jsw_avlerase ( jsw_avltree_t *tree, void *data );
long           Pjsw_avlerase ( jsw_avltree_t *tree, void *data );
size_t         jsw_avlsize ( jsw_avltree_t *tree );

/* Traversal functions */
jsw_avltrav_t *jsw_avltnew ( void );
void           jsw_avltdelete ( jsw_avltrav_t *trav );
void          *jsw_avltfirst ( jsw_avltrav_t *trav, jsw_avltree_t *tree );
void          *jsw_avltlast ( jsw_avltrav_t *trav, jsw_avltree_t *tree );
void          *jsw_avltnext ( jsw_avltrav_t *trav );
void          *jsw_avltprev ( jsw_avltrav_t *trav );

#ifdef __cplusplus
}
#endif

#endif



/* =============================================================================
 *
 * End of avltree.h
 *
 * =============================================================================
 */
