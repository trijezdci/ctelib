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
// Status codes
// ---------------------------------------------------------------------------

typedef enum /* lifo_status_t */ {
    CTE_STACK_STATUS_SUCCESS = 1,
    CTE_STACK_STATUS_INVALID_STACK,
    CTE_STACK_STATUS_INVALID_DATA,
    CTE_STACK_STATUS_STACK_OVERFLOW,
    CTE_STACK_STATUS_STACK_EMPTY,
    CTE_STACK_ALLOCATION_FAILED
} cte_stack_status_t;


// ---------------------------------------------------------------------------
// function:  cte_new_stack( size, status )
// ---------------------------------------------------------------------------
//
// Creates and returns a new stack object with size <size>.  If zero is passed
// in <size>,  then the new stack will be created  with the default stack size
// as defined by CTE_DEFAULT_STACK_SIZE.  Returns NULL if the new stack object
// could not be created.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

cte_stack_t cte_new_stack(cardinal size, cte_stack_status_t *status);


// ---------------------------------------------------------------------------
// function:  cte_stack_push_context( stack, template, index, status )
// ---------------------------------------------------------------------------
//
// Saves a template context to the stack passed in <stack>.  The context para-
// meters are passed in  <template_str>  and <index>.  The operation will fail
// if NULL is passed in for <stack> or <template_str> or if the stack is full.
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
// Removes the  top most  template context  from the stack  passed in <stack>,
// returns its template pointer  as function result  and passes its index back
// in <index>  unless NULL was passed in for <index>.  The operation will fail
// if NULL is passed in for <stack>.
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

cardinal cte_stack_size(cte_stack_t stack);


// ---------------------------------------------------------------------------
// function:  cte_stack_number_of_entries( stack )
// ---------------------------------------------------------------------------
//
// Returns  the number of  template contexts  saved on stack <stack>,  returns
// zero if NULL is passed in for <stack>.

cardinal cte_stack_number_of_entries(cte_stack_t stack);


// ---------------------------------------------------------------------------
// function:  cte_dispose_stack( stack )
// ---------------------------------------------------------------------------
//
// Disposes of stack object <stack>.

void cte_dispose_stack(cte_stack_t stack);


#endif /* CTE_STACK_H */

// END OF FILE