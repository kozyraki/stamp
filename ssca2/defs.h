/* =============================================================================
 *
 * defs.h
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


#ifndef DEFS_H
#define DEFS_H 1

typedef unsigned long int  ULONGINT_T;
typedef long int           LONGINT_T;
typedef short int          SHORTINT_T;

#define CACHELOG           7
#define NOSHARE(x)         ((x)<<CACHELOG)


typedef struct
{
    ULONGINT_T* startVertex;
    ULONGINT_T* endVertex;

    LONGINT_T* intWeight;

    /* The idea is to store the index of the string weights (as a negative value)
    * in the long Weight array. A negative value because we need to sort on
    * the intWeights in Kernel 2. Hence the long long
    */

    char* strWeight;
    ULONGINT_T numEdgesPlaced;

} graphSDG;

typedef struct /*the graph data structure*/
{
    ULONGINT_T numVertices;
    ULONGINT_T numEdges;

    ULONGINT_T numDirectedEdges;
    ULONGINT_T numUndirectedEdges;

    ULONGINT_T numIntEdges;
    ULONGINT_T numStrEdges;

    LONGINT_T*  outDegree;
    ULONGINT_T* outVertexIndex;
    ULONGINT_T* outVertexList;
    ULONGINT_T* paralEdgeIndex;

    LONGINT_T*  inDegree;
    ULONGINT_T* inVertexIndex;
    ULONGINT_T* inVertexList;

    LONGINT_T* intWeight;
    char* strWeight;

} graph;

typedef struct /* edge structure for Kernel 2 */
{
    ULONGINT_T startVertex;
    ULONGINT_T endVertex;
    ULONGINT_T edgeNum;
} edge;

typedef struct /* Vertex list returned by Kernel 3 */
{
    ULONGINT_T num;
    long depth;
} V;

typedef struct l /* A Linked list for Kernel 3 */
{
    ULONGINT_T num;
    SHORTINT_T depth;
    struct l* next;
} Vl;

typedef struct /* A dynamic array for Kernel 3 */
{
    ULONGINT_T numArrays;
    ULONGINT_T* arraySize;
    V** vList;
} Vd;


#endif /* DEFS_H */


/* =============================================================================
 *
 * End of defs.h
 *
 * =============================================================================
 */
