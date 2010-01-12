/* C Template Engine
 *
 *  @file cte_stack.h
 *  CTE stack interface
 *
 *  Template Context Stack to facilitate nesting of templates
 *
 *  Author: Benjamin Kowarsch
 *
 *  Copyright (C) 2009 Benjamin Kowarsch. All rights reserved.
 *
 *  License:
 *
 *  Redistribution  and  use  in source  and  binary forms,  with  or  without
 *  modification, are permitted provided that the following conditions are met
 *
 *  1) NO FEES may be charged for the provision of the software.  The software
 *     may  NOT  be published  on websites  that contain  advertising,  unless
 *     specific  prior  written  permission has been obtained.
 *
 *  2) Redistributions  of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  3) Redistributions  in binary form  must  reproduce  the  above  copyright
 *     notice,  this list of conditions  and  the following disclaimer  in the
 *     documentation and other materials provided with the distribution.
 *
 *  4) Neither the author's name nor the names of any contributors may be used
 *     to endorse  or  promote  products  derived  from this software  without
 *     specific prior written permission.
 *
 *  5) Where this list of conditions  or  the following disclaimer, in part or
 *     as a whole is overruled  or  nullified by applicable law, no permission
 *     is granted to use the software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY  AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
 * CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT  LIMITED  TO,  PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA,  OR PROFITS; OR BUSINESS
 * INTERRUPTION)  HOWEVER  CAUSED  AND ON ANY THEORY OF LIABILITY,  WHETHER IN
 * CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *  
 */


#ifndef CTE_STACK_H
#define CTE_STACK_H


#include "common.h"


// ---------------------------------------------------------------------------
// Default stack size
// ---------------------------------------------------------------------------

#define CTE_DEFAULT_STACK_SIZE 100


// ---------------------------------------------------------------------------
// Maximum stack size
// ---------------------------------------------------------------------------

#define CTE_MAXIMUM_STACK_SIZE 0xffffffff  /* more than 2 billion entries */


// ---------------------------------------------------------------------------
// Determine type to hold stack size values
// ---------------------------------------------------------------------------

#if (CTE_MAXIMUM_STACK_SIZE <= ((1 << 8) - 1))
#define CTE_STACK_SIZE_BASE_TYPE uint8_t
#elif (CTE_MAXIMUM_STACK_SIZE <= ((1 << 16) - 1))
#define CTE_STACK_SIZE_BASE_TYPE uint16_t
#elif (CTE_MAXIMUM_STACK_SIZE <= ((1 << 32) - 1))
#define CTE_STACK_SIZE_BASE_TYPE uint32_t
#elif (CTE_MAXIMUM_STACK_SIZE <= ((1 << 64UL) - 1))
#define CTE_STACK_SIZE_BASE_TYPE uint64_t
#else
#error CTE_MAXIMUM_STACK_SIZE out of range
#endif


// ---------------------------------------------------------------------------
// Opaque stack handle type
// ---------------------------------------------------------------------------
//
// WARNING:  Objects of this opaque type should  only be accessed through this
// public interface.  DO NOT EVER attempt to bypass the public interface.
//
// The internal data structure of this opaque type is  HIDDEN  and  MAY CHANGE
// at any time WITHOUT NOTICE.  Accessing the internal data structure directly
// other than  through the  functions  in this public interface is  UNSAFE and
// may result in an inconsistent program state or a crash.

typedef opaque_t cte_stack_t;


// ---------------------------------------------------------------------------
// Stack size type
// ---------------------------------------------------------------------------

typedef CTE_STACK_SIZE_BASE_TYPE cte_stack_size_t;


// ---------------------------------------------------------------------------
// Status codes
// ---------------------------------------------------------------------------

typedef enum /* cte_stack_status_t */ {
    CTE_STACK_STATUS_SUCCESS = 1,
    CTE_STACK_STATUS_INVALID_SIZE,
    CTE_STACK_STATUS_INVALID_STACK,
    CTE_STACK_STATUS_INVALID_INDEX,
    CTE_STACK_STATUS_INVALID_DATA,
    CTE_STACK_STATUS_STACK_OVERFLOW,
    CTE_STACK_STATUS_STACK_EMPTY,
    CTE_STACK_STATUS_ALLOCATION_FAILED
} cte_stack_status_t;


// ---------------------------------------------------------------------------
// function:  cte_new_stack( initial_size, status )
// ---------------------------------------------------------------------------
//
// Creates and returns a new CTE template context stack object with an initial
// capacity of <initial_size>.  If zero is passed in for <initial_size>,  then
// it will be created with an initial capacity of CTE_DEFAULT_STACK_SIZE.  The
// function fails if a value greater than  CTE_MAXIMUM_STACK_SIZE is passed in
// for <initial_size> or if memory could not be allocated.
//
// The  initial capacity  of a stack is the number of context entries that can
// be stored in the stack without enlargement.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

cte_stack_t cte_new_stack(cte_stack_size_t initial_size,
                        cte_stack_status_t *status);


// ---------------------------------------------------------------------------
// function:  cte_stack_push_context( stack, template, index, status )
// ---------------------------------------------------------------------------
//
// Saves a template context to the stack passed in <stack>.  The context para-
// meters are passed in  <template_str>  and <index>.  The operation will fail
// if NULL is passed in for <stack> or <template_str> or if the stack size has
// reached CTE_MAXIMUM_STACK_SIZE.
//
// New entries are allocated dynamically  if the number of entries exceeds the
// initial capacity of the stack.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

void cte_stack_push_context(cte_stack_t stack,
                                   char *template_str,
                               cardinal index,
                     cte_stack_status_t *status);


// ---------------------------------------------------------------------------
// function:  cte_stack_pop_context( stack, index, status )
// ---------------------------------------------------------------------------
//
// Removes the top most template context from the stack passed in <stack>  and
// returns its  template pointer.  Its index  is passed back  in <index>.  The
// operation fails if NULL is passed in for <stack> or <index>.
//
// Entries which were allocated dynamically  (above the initial capacity)  are
// deallocated when their values are popped.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

char *cte_stack_pop_context(cte_stack_t stack,
                               cardinal *index,
                     cte_stack_status_t *status);


// ---------------------------------------------------------------------------
// function:  cte_stack_size( stack )
// ---------------------------------------------------------------------------
//
// Returns the number of context slots of stack <stack>,  returns zero if NULL
// is passed in for <stack>.

cte_stack_size_t cte_stack_size(cte_stack_t stack);


// ---------------------------------------------------------------------------
// function:  cte_stack_number_of_entries( stack )
// ---------------------------------------------------------------------------
//
// Returns  the number of  template contexts  saved on stack <stack>,  returns
// zero if NULL is passed in for <stack>.

cte_stack_size_t cte_stack_number_of_entries(cte_stack_t stack);


// ---------------------------------------------------------------------------
// function:  cte_dispose_stack( stack )
// ---------------------------------------------------------------------------
//
// Disposes of stack object <stack>.  Returns NULL.

cte_stack_t cte_dispose_stack(cte_stack_t stack);


#endif /* CTE_STACK_H */

// END OF FILE