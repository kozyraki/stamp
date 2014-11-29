/* =============================================================================
 *
 * customer.h
 * -- Representation of customer
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


#ifndef CUSTOMER_H
#define CUSTOMER_H 1


#include "list.h"
#include "reservation.h"
#include "tm.h"
#include "types.h"

typedef struct customer {
    long id;
    list_t* reservationInfoListPtr;
} customer_t;


/* =============================================================================
 * customer_alloc
 * =============================================================================
 */
customer_t*
customer_alloc (TM_ARGDECL  long id);

customer_t*
customer_alloc_seq (long id);


/* =============================================================================
 * customer_compare
 * -- Returns -1 if A < B, 0 if A = B, 1 if A > B
 * =============================================================================
 */
long
customer_compare (customer_t* aPtr, customer_t* bPtr);


/* =============================================================================
 * customer_hash
 * =============================================================================
 */
unsigned long
customer_hash (customer_t* customerPtr);


/* =============================================================================
 * customer_free
 * =============================================================================
 */
void
customer_free (TM_ARGDECL  customer_t* customerPtr);


/* =============================================================================
 * customer_addReservationInfo
 * -- Returns TRUE if success, else FALSE
 * =============================================================================
 */
TM_CALLABLE
bool_t
customer_addReservationInfo (TM_ARGDECL
                             customer_t* customerPtr,
                             reservation_type_t type, long id, long price);

bool_t
customer_addReservationInfo_seq (customer_t* customerPtr,
                                 reservation_type_t type, long id, long price);


/* =============================================================================
 * customer_removeReservationInfo
 * -- Returns TRUE if success, else FALSE
 * =============================================================================
 */
bool_t
customer_removeReservationInfo (TM_ARGDECL
                                customer_t* customerPtr,
                                reservation_type_t type, long id);


/* =============================================================================
 * customer_getBill
 * -- Returns total cost of reservations
 * =============================================================================
 */
TM_CALLABLE
long
customer_getBill (TM_ARGDECL  customer_t* customerPtr);

long
customer_getBill_seq (customer_t* customerPtr);


#define CUSTOMER_ALLOC(id) \
    customer_alloc(TM_ARG  id)
#define CUSTOMER_ADD_RESERVATION_INFO(cust, type, id, price)  \
    customer_addReservationInfo(TM_ARG  cust, type, id, price)
#define CUSTOMER_REMOVE_RESERVATION_INFO(cust, type, id) \
    customer_removeReservationInfo(TM_ARG  cust, type, id)
#define CUSTOMER_GET_BILL(cust) \
    customer_getBill(TM_ARG  cust)
#define CUSTOMER_FREE(cust) \
    customer_free(TM_ARG  cust)


#endif /* CUSTOMER_H */


/* =============================================================================
 *
 * End of customer.h
 *
 * =============================================================================
 */
