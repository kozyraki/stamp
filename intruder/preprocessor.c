/* =============================================================================
 *
 * preprocessor.c
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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "preprocessor.h"


/* =============================================================================
 * preprocessor_convertURNHex
 * -- Translate % hex escape sequences
 * =============================================================================
 */
void
preprocessor_convertURNHex (char* str)
{
    char* src = str;
    char* dst = str;
    char c;

    while ((c = *src) != '\0') {
        if (c == '%') {
            char hex[3];
            hex[0] = (char)tolower((int)*(src + 1));
            assert(hex[0]);
            hex[1] = (char)tolower((int)*(src + 2));
            assert(hex[1]);
            hex[2] = '\0';
            int i;
            int n = sscanf(hex, "%x", &i);
            assert(n == 1);
            src += 2;
            *src = (char)i;
        }
        *dst = *src;
        src++;
        dst++;
    }

    *dst = '\0';
}


/* =============================================================================
 * preprocessor_toLower
 * -- Translate uppercase letters to lowercase
 * =============================================================================
 */
void
preprocessor_toLower (char* str)
{
    char* src = str;

    while (*src != '\0') {
        *src = (char)tolower((int)*src);
        src++;
    }
}


/* #############################################################################
 * TEST_PREPROCESSOR
 * #############################################################################
 */
#ifdef TEST_PREPROCESSOR


#include <assert.h>
#include <stdio.h>


int
main ()
{
    puts("Starting...");

    char hex[] = "This%20is %41 test%3F%3f";
    preprocessor_convertURNHex(hex);
    assert(strcmp(hex, "This is A test??") == 0);

    char caps[] = "ThiS is A tEsT??";
    preprocessor_toLower(caps);
    assert(strcmp(caps, "this is a test??") == 0);

    puts("All tests passed.");

    return 0;
}


#endif /* TEST_PREPROCESSOR */


/* =============================================================================
 *
 * End of preprocessor.c
 *
 * =============================================================================
 */
