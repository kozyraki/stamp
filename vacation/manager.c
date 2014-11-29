/* =============================================================================
 *
 * manager.c
 * -- Travel reservation resource manager
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
#include "customer.h"
#include "map.h"
#include "pair.h"
#include "manager.h"
#include "reservation.h"
#include "tm.h"
#include "types.h"

/* =============================================================================
 * DECLARATION OF TM_CALLABLE FUNCTIONS
 * =============================================================================
 */

TM_CALLABLE
static long 
queryNumFree (TM_ARGDECL  MAP_T* tablePtr, long id);

TM_CALLABLE
static long 
queryPrice (TM_ARGDECL  MAP_T* tablePtr, long id);

TM_CALLABLE
static bool_t 
reserve (TM_ARGDECL MAP_T* tablePtr, MAP_T* customerTablePtr, long customerId, long id, reservation_type_t type);

TM_CALLABLE
bool_t
addReservation (TM_ARGDECL  MAP_T* tablePtr, long id, long num, long price);


/* =============================================================================
 * tableAlloc
 * =============================================================================
 */
static MAP_T*
tableAlloc ()
{
    return MAP_ALLOC(NULL, NULL);
}


/* =============================================================================
 * manager_alloc
 * =============================================================================
 */
manager_t*
manager_alloc ()
{
    manager_t* managerPtr;

    managerPtr = (manager_t*)malloc(sizeof(manager_t));
    assert(managerPtr != NULL);

    managerPtr->carTablePtr = tableAlloc();
    managerPtr->roomTablePtr = tableAlloc();
    managerPtr->flightTablePtr = tableAlloc();
    managerPtr->customerTablePtr = tableAlloc();
    assert(managerPtr->carTablePtr != NULL);
    assert(managerPtr->roomTablePtr != NULL);
    assert(managerPtr->flightTablePtr != NULL);
    assert(managerPtr->customerTablePtr != NULL);

    return managerPtr;
}


/* =============================================================================
 * tableFree
 * -- Note: contents are not deallocated
 * =============================================================================
 */
static void
tableFree (MAP_T* mapPtr)
{
    MAP_FREE(mapPtr);
}



/* =============================================================================
 * manager_free
 * =============================================================================
 */
void
manager_free (manager_t* managerPtr)
{
    tableFree(managerPtr->carTablePtr);
    tableFree(managerPtr->roomTablePtr);
    tableFree(managerPtr->flightTablePtr);
    tableFree(managerPtr->customerTablePtr);
}


/* =============================================================================
 * ADMINISTRATIVE INTERFACE
 * =============================================================================
 */


/* =============================================================================
 * addReservation
 * -- If 'num' > 0 then add, if < 0 remove
 * -- Adding 0 seats is error if does not exist
 * -- If 'price' < 0, do not update price
 * -- Returns TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
addReservation (TM_ARGDECL  MAP_T* tablePtr, long id, long num, long price)
{
    reservation_t* reservationPtr;

    reservationPtr = (reservation_t*)TMMAP_FIND(tablePtr, id);
    if (reservationPtr == NULL) {
        /* Create new reservation */
        if (num < 1 || price < 0) {
            return FALSE;
        }
        reservationPtr = RESERVATION_ALLOC(id, num, price);
        assert(reservationPtr != NULL);
        TMMAP_INSERT(tablePtr, id, reservationPtr);
    } else {
        /* Update existing reservation */
        if (!RESERVATION_ADD_TO_TOTAL(reservationPtr, num)) {
            return FALSE;
        }
        if ((long)TM_SHARED_READ(reservationPtr->numTotal) == 0) {
            bool_t status = TMMAP_REMOVE(tablePtr, id);
            if (status == FALSE) {
                TM_RESTART();
            }
            RESERVATION_FREE(reservationPtr);
        } else {
            RESERVATION_UPDATE_PRICE(reservationPtr, price);
        }
    }

    return TRUE;
}


bool_t
addReservation_seq (MAP_T* tablePtr, long id, long num, long price)
{
    reservation_t* reservationPtr;
    bool_t status;

    reservationPtr = (reservation_t*)MAP_FIND(tablePtr, id);
    if (reservationPtr == NULL) {
        /* Create new reservation */
        if (num < 1 || price < 0) {
            return FALSE;
        }
        reservationPtr = reservation_alloc_seq(id, num, price);
        assert(reservationPtr != NULL);
        status = MAP_INSERT(tablePtr, id, reservationPtr);
        assert(status);
    } else {
        /* Update existing reservation */
        if (!reservation_addToTotal_seq(reservationPtr, num)) {
            return FALSE;
        }
        if (reservationPtr->numTotal == 0) {
            status = MAP_REMOVE(tablePtr, id);
            assert(status);
        } else {
            reservation_updatePrice_seq(reservationPtr, price);
        }
    }

    return TRUE;
}


/* =============================================================================
 * manager_addCar
 * -- Add cars to a city
 * -- Adding to an existing car overwrite the price if 'price' >= 0
 * -- Returns TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
manager_addCar (TM_ARGDECL
                manager_t* managerPtr, long carId, long numCars, long price)
{
    return addReservation(TM_ARG  managerPtr->carTablePtr, carId, numCars, price);
}


bool_t
manager_addCar_seq (manager_t* managerPtr, long carId, long numCars, long price)
{
    return addReservation_seq(managerPtr->carTablePtr, carId, numCars, price);
}


/* =============================================================================
 * manager_deleteCar
 * -- Delete cars from a city
 * -- Decreases available car count (those not allocated to a customer)
 * -- Fails if would make available car count negative
 * -- If decresed to 0, deletes entire entry
 * -- Returns TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
manager_deleteCar (TM_ARGDECL  manager_t* managerPtr, long carId, long numCar)
{
    /* -1 keeps old price */
    return addReservation(TM_ARG  managerPtr->carTablePtr, carId, -numCar, -1);
}


/* =============================================================================
 * manager_addRoom
 * -- Add rooms to a city
 * -- Adding to an existing room overwrite the price if 'price' >= 0
 * -- Returns TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
manager_addRoom (TM_ARGDECL
                 manager_t* managerPtr, long roomId, long numRoom, long price)
{
    return addReservation(TM_ARG  managerPtr->roomTablePtr, roomId, numRoom, price);
}


bool_t
manager_addRoom_seq (manager_t* managerPtr, long roomId, long numRoom, long price)
{
    return addReservation_seq(managerPtr->roomTablePtr, roomId, numRoom, price);
}



/* =============================================================================
 * manager_deleteRoom
 * -- Delete rooms from a city
 * -- Decreases available room count (those not allocated to a customer)
 * -- Fails if would make available room count negative
 * -- If decresed to 0, deletes entire entry
 * -- Returns TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
manager_deleteRoom (TM_ARGDECL  manager_t* managerPtr, long roomId, long numRoom)
{
    /* -1 keeps old price */
    return addReservation(TM_ARG  managerPtr->roomTablePtr, roomId, -numRoom, -1);
}


/* =============================================================================
 * manager_addFlight
 * -- Add seats to a flight
 * -- Adding to an existing flight overwrite the price if 'price' >= 0
 * -- Returns TRUE on success, FALSE on failure
 * =============================================================================
 */
bool_t
manager_addFlight (TM_ARGDECL
                   manager_t* managerPtr, long flightId, long numSeat, long price)
{
    return addReservation(TM_ARG
                          managerPtr->flightTablePtr, flightId, numSeat, price);
}


bool_t
manager_addFlight_seq (manager_t* managerPtr, long flightId, long numSeat, long price)
{
    return addReservation_seq(managerPtr->flightTablePtr, flightId, numSeat, price);
}


/* =============================================================================
 * manager_deleteFlight
 * -- Delete an entire flight
 * -- Fails if customer has reservation on this flight
 * -- Returns TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
manager_deleteFlight (TM_ARGDECL  manager_t* managerPtr, long flightId)
{
    reservation_t* reservationPtr;

    reservationPtr = (reservation_t*)TMMAP_FIND(managerPtr->flightTablePtr, flightId);
    if (reservationPtr == NULL) {
        return FALSE;
    }

    if ((long)TM_SHARED_READ(reservationPtr->numUsed) > 0) {
        return FALSE; /* somebody has a reservation */
    }

    return addReservation(TM_ARG
                          managerPtr->flightTablePtr,
                          flightId,
                          -1*(long)TM_SHARED_READ(reservationPtr->numTotal),
                          -1 /* -1 keeps old price */);
}


/* =============================================================================
 * manager_addCustomer
 * -- If customer already exists, returns failure
 * -- Returns TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
manager_addCustomer (TM_ARGDECL  manager_t* managerPtr, long customerId)
{
    customer_t* customerPtr;
    bool_t status;

    if (TMMAP_CONTAINS(managerPtr->customerTablePtr, customerId)) {
        return FALSE;
    }

    customerPtr = CUSTOMER_ALLOC(customerId);
    assert(customerPtr != NULL);
    status = TMMAP_INSERT(managerPtr->customerTablePtr, customerId, customerPtr);
    if (status == FALSE) {
        TM_RESTART();
    }

    return TRUE;
}


bool_t
manager_addCustomer_seq (manager_t* managerPtr, long customerId)
{
    customer_t* customerPtr;
    bool_t status;

    if (MAP_CONTAINS(managerPtr->customerTablePtr, customerId)) {
        return FALSE;
    }

    customerPtr = customer_alloc_seq(customerId);
    assert(customerPtr != NULL);
    status = MAP_INSERT(managerPtr->customerTablePtr, customerId, customerPtr);
    assert(status);

    return TRUE;
}


/* =============================================================================
 * manager_deleteCustomer
 * -- Delete this customer and associated reservations
 * -- If customer does not exist, returns success
 * -- Returns TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
manager_deleteCustomer (TM_ARGDECL  manager_t* managerPtr, long customerId)
{
    customer_t* customerPtr;
    MAP_T* reservationTables[NUM_RESERVATION_TYPE];
    list_t* reservationInfoListPtr;
    list_iter_t it;
    bool_t status;

    customerPtr = (customer_t*)TMMAP_FIND(managerPtr->customerTablePtr, customerId);
    if (customerPtr == NULL) {
        return FALSE;
    }

    reservationTables[RESERVATION_CAR] = managerPtr->carTablePtr;
    reservationTables[RESERVATION_ROOM] = managerPtr->roomTablePtr;
    reservationTables[RESERVATION_FLIGHT] = managerPtr->flightTablePtr;

    /* Cancel this customer's reservations */
    reservationInfoListPtr = customerPtr->reservationInfoListPtr;
    TMLIST_ITER_RESET(&it, reservationInfoListPtr);
    while (TMLIST_ITER_HASNEXT(&it, reservationInfoListPtr)) {
        reservation_info_t* reservationInfoPtr;
        reservation_t* reservationPtr;
        reservationInfoPtr =
            (reservation_info_t*)TMLIST_ITER_NEXT(&it, reservationInfoListPtr);
        reservationPtr =
            (reservation_t*)TMMAP_FIND(reservationTables[reservationInfoPtr->type],
                                     reservationInfoPtr->id);
        if (reservationPtr == NULL) {
            TM_RESTART();
        }
        status = RESERVATION_CANCEL(reservationPtr);
        if (status == FALSE) {
            TM_RESTART();
        }
        RESERVATION_INFO_FREE(reservationInfoPtr);
    }

    status = TMMAP_REMOVE(managerPtr->customerTablePtr, customerId);
    if (status == FALSE) {
        TM_RESTART();
    }
    CUSTOMER_FREE(customerPtr);

    return TRUE;
}


/* =============================================================================
 * QUERY INTERFACE
 * =============================================================================
 */


/* =============================================================================
 * queryNumFree
 * -- Return numFree of a reservation, -1 if failure
 * =============================================================================
 */
static long
queryNumFree (TM_ARGDECL  MAP_T* tablePtr, long id)
{
    long numFree = -1;
    reservation_t* reservationPtr;

    reservationPtr = (reservation_t*)TMMAP_FIND(tablePtr, id);
    if (reservationPtr != NULL) {
        numFree = (long)TM_SHARED_READ(reservationPtr->numFree);
    }

    return numFree;
}


/* =============================================================================
 * queryPrice
 * -- Return price of a reservation, -1 if failure
 * =============================================================================
 */
static long
queryPrice (TM_ARGDECL  MAP_T* tablePtr, long id)
{
    long price = -1;
    reservation_t* reservationPtr;

    reservationPtr = (reservation_t*)TMMAP_FIND(tablePtr, id);
    if (reservationPtr != NULL) {
        price = (long)TM_SHARED_READ(reservationPtr->price);
    }

    return price;
}


/* =============================================================================
 * manager_queryCar
 * -- Return the number of empty seats on a car
 * -- Returns -1 if the car does not exist
 * =============================================================================
 */
long
manager_queryCar (TM_ARGDECL  manager_t* managerPtr, long carId)
{
    return queryNumFree(TM_ARG  managerPtr->carTablePtr, carId);
}


/* =============================================================================
 * manager_queryCarPrice
 * -- Return the price of the car
 * -- Returns -1 if the car does not exist
 * =============================================================================
 */
long
manager_queryCarPrice (TM_ARGDECL  manager_t* managerPtr, long carId)
{
    return queryPrice(TM_ARG  managerPtr->carTablePtr, carId);
}


/* =============================================================================
 * manager_queryRoom
 * -- Return the number of empty seats on a room
 * -- Returns -1 if the room does not exist
 * =============================================================================
 */
long
manager_queryRoom (TM_ARGDECL  manager_t* managerPtr, long roomId)
{
    return queryNumFree(TM_ARG  managerPtr->roomTablePtr, roomId);
}


/* =============================================================================
 * manager_queryRoomPrice
 * -- Return the price of the room
 * -- Returns -1 if the room does not exist
 * =============================================================================
 */
long
manager_queryRoomPrice (TM_ARGDECL  manager_t* managerPtr, long roomId)
{
    return queryPrice(TM_ARG  managerPtr->roomTablePtr, roomId);
}


/* =============================================================================
 * manager_queryFlight
 * -- Return the number of empty seats on a flight
 * -- Returns -1 if the flight does not exist
 * =============================================================================
 */
long
manager_queryFlight (TM_ARGDECL  manager_t* managerPtr, long flightId)
{
    return queryNumFree(TM_ARG  managerPtr->flightTablePtr, flightId);
}


/* =============================================================================
 * manager_queryFlightPrice
 * -- Return the price of the flight
 * -- Returns -1 if the flight does not exist
 * =============================================================================
 */
long
manager_queryFlightPrice (TM_ARGDECL  manager_t* managerPtr, long flightId)
{
    return queryPrice(TM_ARG  managerPtr->flightTablePtr, flightId);
}


/* =============================================================================
 * manager_queryCustomerBill
 * -- Return the total price of all reservations held for a customer
 * -- Returns -1 if the customer does not exist
 * =============================================================================
 */
long
manager_queryCustomerBill (TM_ARGDECL  manager_t* managerPtr, long customerId)
{
    long bill = -1;
    customer_t* customerPtr;

    customerPtr = (customer_t*)TMMAP_FIND(managerPtr->customerTablePtr, customerId);

    if (customerPtr != NULL) {
        bill = CUSTOMER_GET_BILL(customerPtr);
    }

    return bill;
}


/* =============================================================================
 * RESERVATION INTERFACE
 * =============================================================================
 */


/* =============================================================================
 * reserve
 * -- Customer is not allowed to reserve same (type, id) multiple times
 * -- Returns TRUE on success, else FALSE
 * =============================================================================
 */
static bool_t
reserve (TM_ARGDECL
         MAP_T* tablePtr, MAP_T* customerTablePtr,
         long customerId, long id, reservation_type_t type)
{
    customer_t* customerPtr;
    reservation_t* reservationPtr;

    customerPtr = (customer_t*)TMMAP_FIND(customerTablePtr, customerId);
    if (customerPtr == NULL) {
        return FALSE;
    }

    reservationPtr = (reservation_t*)TMMAP_FIND(tablePtr, id);
    if (reservationPtr == NULL) {
        return FALSE;
    }

    if (!RESERVATION_MAKE(reservationPtr)) {
        return FALSE;
    }

    if (!CUSTOMER_ADD_RESERVATION_INFO(
            customerPtr,
            type,
            id,
            (long)TM_SHARED_READ(reservationPtr->price)))
    {
        /* Undo previous successful reservation */
        bool_t status = RESERVATION_CANCEL(reservationPtr);
        if (status == FALSE) {
            TM_RESTART();
        }
        return FALSE;
    }

    return TRUE;
}


/* =============================================================================
 * manager_reserveCar
 * -- Returns failure if the car or customer does not exist
 * -- Returns TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
manager_reserveCar (TM_ARGDECL  manager_t* managerPtr, long customerId, long carId)
{
    return reserve(TM_ARG
                   managerPtr->carTablePtr,
                   managerPtr->customerTablePtr,
                   customerId,
                   carId,
                   RESERVATION_CAR);
}


/* =============================================================================
 * manager_reserveRoom
 * -- Returns failure if the room or customer does not exist
 * -- Returns TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
manager_reserveRoom (TM_ARGDECL  manager_t* managerPtr, long customerId, long roomId)
{
    return reserve(TM_ARG
                   managerPtr->roomTablePtr,
                   managerPtr->customerTablePtr,
                   customerId,
                   roomId,
                   RESERVATION_ROOM);
}


/* =============================================================================
 * manager_reserveFlight
 * -- Returns failure if the flight or customer does not exist
 * -- Returns TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
manager_reserveFlight (TM_ARGDECL
                       manager_t* managerPtr, long customerId, long flightId)
{
    return reserve(TM_ARG
                   managerPtr->flightTablePtr,
                   managerPtr->customerTablePtr,
                   customerId,
                   flightId,
                   RESERVATION_FLIGHT);
}


/* =============================================================================
 * cancel
 * -- Customer is not allowed to cancel multiple times
 * -- Returns TRUE on success, else FALSE
 * =============================================================================
 */
static bool_t
cancel (TM_ARGDECL
        MAP_T* tablePtr, MAP_T* customerTablePtr,
        long customerId, long id, reservation_type_t type)
{
    customer_t* customerPtr;
    reservation_t* reservationPtr;

    customerPtr = (customer_t*)TMMAP_FIND(customerTablePtr, customerId);
    if (customerPtr == NULL) {
        return FALSE;
    }

    reservationPtr = (reservation_t*)TMMAP_FIND(tablePtr, id);
    if (reservationPtr == NULL) {
        return FALSE;
    }

    if (!RESERVATION_CANCEL(reservationPtr)) {
        return FALSE;
    }

    if (!CUSTOMER_REMOVE_RESERVATION_INFO(customerPtr, type, id)) {
        /* Undo previous successful cancellation */
        bool_t status = RESERVATION_MAKE(reservationPtr);
        if (status == FALSE) {
            TM_RESTART();
        }
        return FALSE;
    }

    return TRUE;
}


/* =============================================================================
 * manager_cancelCar
 * -- Returns failure if the car, reservation, or customer does not exist
 * -- Returns TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
manager_cancelCar (TM_ARGDECL  manager_t* managerPtr, long customerId, long carId)
{
    return cancel(TM_ARG
                  managerPtr->carTablePtr,
                  managerPtr->customerTablePtr,
                  customerId,
                  carId,
                  RESERVATION_CAR);
}


/* =============================================================================
 * manager_cancelRoom
 * -- Returns failure if the room, reservation, or customer does not exist
 * -- Returns TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
manager_cancelRoom (TM_ARGDECL  manager_t* managerPtr, long customerId, long roomId)
{
    return cancel(TM_ARG
                  managerPtr->roomTablePtr,
                  managerPtr->customerTablePtr,
                  customerId,
                  roomId,
                  RESERVATION_ROOM);
}



/* =============================================================================
 * manager_cancelFlight
 * -- Returns failure if the flight, reservation, or customer does not exist
 * -- Returns TRUE on success, else FALSE
 * =============================================================================
 */
bool_t
manager_cancelFlight (TM_ARGDECL
                      manager_t* managerPtr, long customerId, long flightId)
{
    return cancel(TM_ARG
                  managerPtr->flightTablePtr,
                  managerPtr->customerTablePtr,
                  customerId,
                  flightId,
                  RESERVATION_FLIGHT);
}


/* =============================================================================
 * TEST_MANAGER
 * =============================================================================
 */
#ifdef TEST_MANAGER


#include <assert.h>
#include <stdio.h>


int
main ()
{
    manager_t* managerPtr;

    assert(memory_init(1, 4, 2));

    puts("Starting...");

    managerPtr = manager_alloc();

    /* Test administrative interface for cars */
    assert(!manager_addCar(managerPtr, 0, -1, 0)); /* negative num */
    assert(!manager_addCar(managerPtr, 0, 0, -1)); /* negative price */
    assert(!manager_addCar(managerPtr, 0, 0, 0)); /* zero num */
    assert(manager_addCar(managerPtr, 0, 1, 1));
    assert(!manager_deleteCar(managerPtr, 1, 0)); /* does not exist */
    assert(!manager_deleteCar(managerPtr, 0, 2)); /* cannot remove that many */
    assert(manager_addCar(managerPtr, 0, 1, 0));
    assert(manager_deleteCar(managerPtr, 0, 1));
    assert(manager_deleteCar(managerPtr, 0, 1));
    assert(!manager_deleteCar(managerPtr, 0, 1)); /* none left */
    assert(manager_queryCar(managerPtr, 0) == -1); /* does not exist */

    /* Test administrative interface for rooms */
    assert(!manager_addRoom(managerPtr, 0, -1, 0)); /* negative num */
    assert(!manager_addRoom(managerPtr, 0, 0, -1)); /* negative price */
    assert(!manager_addRoom(managerPtr, 0, 0, 0)); /* zero num */
    assert(manager_addRoom(managerPtr, 0, 1, 1));
    assert(!manager_deleteRoom(managerPtr, 1, 0)); /* does not exist */
    assert(!manager_deleteRoom(managerPtr, 0, 2)); /* cannot remove that many */
    assert(manager_addRoom(managerPtr, 0, 1, 0));
    assert(manager_deleteRoom(managerPtr, 0, 1));
    assert(manager_deleteRoom(managerPtr, 0, 1));
    assert(!manager_deleteRoom(managerPtr, 0, 1)); /* none left */
    assert(manager_queryRoom(managerPtr, 0) ==  -1); /* does not exist */

    /* Test administrative interface for flights and customers */
    assert(!manager_addFlight(managerPtr, 0, -1, 0));  /* negative num */
    assert(!manager_addFlight(managerPtr, 0, 0, -1));  /* negative price */
    assert(!manager_addFlight(managerPtr, 0, 0, 0));
    assert(manager_addFlight(managerPtr, 0, 1, 0));
    assert(!manager_deleteFlight(managerPtr, 1)); /* does not exist */
    assert(!manager_deleteFlight(managerPtr, 2)); /* cannot remove that many */
    assert(!manager_cancelFlight(managerPtr, 0, 0)); /* do not have reservation */
    assert(!manager_reserveFlight(managerPtr, 0, 0)); /* customer does not exist */
    assert(!manager_deleteCustomer(managerPtr, 0)); /* does not exist */
    assert(manager_addCustomer(managerPtr, 0));
    assert(!manager_addCustomer(managerPtr, 0)); /* already exists */
    assert(manager_reserveFlight(managerPtr, 0, 0));
    assert(manager_addFlight(managerPtr, 0, 1, 0));
    assert(!manager_deleteFlight(managerPtr, 0)); /* someone has reservation */
    assert(manager_cancelFlight(managerPtr, 0, 0));
    assert(manager_deleteFlight(managerPtr, 0));
    assert(!manager_deleteFlight(managerPtr, 0)); /* does not exist */
    assert(manager_queryFlight(managerPtr, 0) ==  -1); /* does not exist */
    assert(manager_deleteCustomer(managerPtr, 0));

    /* Test query interface for cars */
    assert(manager_addCustomer(managerPtr, 0));
    assert(manager_queryCar(managerPtr, 0) == -1); /* does not exist */
    assert(manager_queryCarPrice(managerPtr, 0) == -1); /* does not exist */
    assert(manager_addCar(managerPtr, 0, 1, 2));
    assert(manager_queryCar(managerPtr, 0) == 1);
    assert(manager_queryCarPrice(managerPtr, 0) == 2);
    assert(manager_addCar(managerPtr, 0, 1, -1));
    assert(manager_queryCar(managerPtr, 0) == 2);
    assert(manager_reserveCar(managerPtr, 0, 0));
    assert(manager_queryCar(managerPtr, 0) == 1);
    assert(manager_deleteCar(managerPtr, 0, 1));
    assert(manager_queryCar(managerPtr, 0) == 0);
    assert(manager_queryCarPrice(managerPtr, 0) == 2);
    assert(manager_addCar(managerPtr, 0, 1, 1));
    assert(manager_queryCarPrice(managerPtr, 0) == 1);
    assert(manager_deleteCustomer(managerPtr, 0));
    assert(manager_queryCar(managerPtr, 0) == 2);
    assert(manager_deleteCar(managerPtr, 0, 2));

    /* Test query interface for rooms */
    assert(manager_addCustomer(managerPtr, 0));
    assert(manager_queryRoom(managerPtr, 0) == -1); /* does not exist */
    assert(manager_queryRoomPrice(managerPtr, 0) == -1); /* does not exist */
    assert(manager_addRoom(managerPtr, 0, 1, 2));
    assert(manager_queryRoom(managerPtr, 0) == 1);
    assert(manager_queryRoomPrice(managerPtr, 0) == 2);
    assert(manager_addRoom(managerPtr, 0, 1, -1));
    assert(manager_queryRoom(managerPtr, 0) == 2);
    assert(manager_reserveRoom(managerPtr, 0, 0));
    assert(manager_queryRoom(managerPtr, 0) == 1);
    assert(manager_deleteRoom(managerPtr, 0, 1));
    assert(manager_queryRoom(managerPtr, 0) == 0);
    assert(manager_queryRoomPrice(managerPtr, 0) == 2);
    assert(manager_addRoom(managerPtr, 0, 1, 1));
    assert(manager_queryRoomPrice(managerPtr, 0) == 1);
    assert(manager_deleteCustomer(managerPtr, 0));
    assert(manager_queryRoom(managerPtr, 0) == 2);
    assert(manager_deleteRoom(managerPtr, 0, 2));

    /* Test query interface for flights */
    assert(manager_addCustomer(managerPtr, 0));
    assert(manager_queryFlight(managerPtr, 0) == -1); /* does not exist */
    assert(manager_queryFlightPrice(managerPtr, 0) == -1); /* does not exist */
    assert(manager_addFlight(managerPtr, 0, 1, 2));
    assert(manager_queryFlight(managerPtr, 0) == 1);
    assert(manager_queryFlightPrice(managerPtr, 0) == 2);
    assert(manager_addFlight(managerPtr, 0, 1, -1));
    assert(manager_queryFlight(managerPtr, 0) == 2);
    assert(manager_reserveFlight(managerPtr, 0, 0));
    assert(manager_queryFlight(managerPtr, 0) == 1);
    assert(manager_addFlight(managerPtr, 0, 1, 1));
    assert(manager_queryFlightPrice(managerPtr, 0) == 1);
    assert(manager_deleteCustomer(managerPtr, 0));
    assert(manager_queryFlight(managerPtr, 0) == 3);
    assert(manager_deleteFlight(managerPtr, 0));

    /* Test query interface for customer bill */

    assert(manager_queryCustomerBill(managerPtr, 0) == -1); /* does not exist */
    assert(manager_addCustomer(managerPtr, 0));
    assert(manager_queryCustomerBill(managerPtr, 0) == 0);
    assert(manager_addCar(managerPtr, 0, 1, 1));
    assert(manager_addRoom(managerPtr, 0, 1, 2));
    assert(manager_addFlight(managerPtr, 0, 1, 3));

    assert(manager_reserveCar(managerPtr, 0, 0));
    assert(manager_queryCustomerBill(managerPtr, 0) == 1);
    assert(!manager_reserveCar(managerPtr, 0, 0));
    assert(manager_queryCustomerBill(managerPtr, 0) == 1);
    assert(manager_addCar(managerPtr, 0, 0, 2));
    assert(manager_queryCar(managerPtr, 0) == 0);
    assert(manager_queryCustomerBill(managerPtr, 0) == 1);

    assert(manager_reserveRoom(managerPtr, 0, 0));
    assert(manager_queryCustomerBill(managerPtr, 0) == 3);
    assert(!manager_reserveRoom(managerPtr, 0, 0));
    assert(manager_queryCustomerBill(managerPtr, 0) == 3);
    assert(manager_addRoom(managerPtr, 0, 0, 2));
    assert(manager_queryRoom(managerPtr, 0) == 0);
    assert(manager_queryCustomerBill(managerPtr, 0) == 3);

    assert(manager_reserveFlight(managerPtr, 0, 0));
    assert(manager_queryCustomerBill(managerPtr, 0) == 6);
    assert(!manager_reserveFlight(managerPtr, 0, 0));
    assert(manager_queryCustomerBill(managerPtr, 0) == 6);
    assert(manager_addFlight(managerPtr, 0, 0, 2));
    assert(manager_queryFlight(managerPtr, 0) == 0);
    assert(manager_queryCustomerBill(managerPtr, 0) == 6);

    assert(manager_deleteCustomer(managerPtr, 0));
    assert(manager_deleteCar(managerPtr, 0, 1));
    assert(manager_deleteRoom(managerPtr, 0, 1));
    assert(manager_deleteFlight(managerPtr, 0));

   /* Test reservation interface */

    assert(manager_addCustomer(managerPtr, 0));
    assert(manager_queryCustomerBill(managerPtr, 0) == 0);
    assert(manager_addCar(managerPtr, 0, 1, 1));
    assert(manager_addRoom(managerPtr, 0, 1, 2));
    assert(manager_addFlight(managerPtr, 0, 1, 3));

    assert(!manager_cancelCar(managerPtr, 0, 0)); /* do not have reservation */
    assert(manager_reserveCar(managerPtr, 0, 0));
    assert(manager_queryCar(managerPtr, 0) == 0);
    assert(manager_cancelCar(managerPtr, 0, 0));
    assert(manager_queryCar(managerPtr, 0) == 1);

    assert(!manager_cancelRoom(managerPtr, 0, 0)); /* do not have reservation */
    assert(manager_reserveRoom(managerPtr, 0, 0));
    assert(manager_queryRoom(managerPtr, 0) == 0);
    assert(manager_cancelRoom(managerPtr, 0, 0));
    assert(manager_queryRoom(managerPtr, 0) == 1);

    assert(!manager_cancelFlight(managerPtr, 0, 0)); /* do not have reservation */
    assert(manager_reserveFlight(managerPtr, 0, 0));
    assert(manager_queryFlight(managerPtr, 0) == 0);
    assert(manager_cancelFlight(managerPtr, 0, 0));
    assert(manager_queryFlight(managerPtr, 0) == 1);

    assert(manager_deleteCar(managerPtr, 0, 1));
    assert(manager_deleteRoom(managerPtr, 0, 1));
    assert(manager_deleteFlight(managerPtr, 0));
    assert(manager_deleteCustomer(managerPtr, 0));

    manager_free(managerPtr);

    puts("All tests passed.");

    return 0;
}


#endif /* TEST_MANAGER */


/* =============================================================================
 *
 * End of manager.c
 *
 * =============================================================================
 */
