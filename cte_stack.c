/* C Template Engine
 *
 *  @file cte_stack.c
 *  CTE stack implementation
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


#include "cte_stack.h"
#include "alloc.h"


// ---------------------------------------------------------------------------
// Template context storage type
// ---------------------------------------------------------------------------

typedef struct /* cte_context_s */ {
        char *str;
    cardinal index;
} cte_context_s;


// ---------------------------------------------------------------------------
// Template context stack type
// ---------------------------------------------------------------------------

typedef struct /* cte_stack_s */ {
         cardinal size;
         cardinal pointer;
    cte_context_s context[0];
} cte_stack_s;


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

cte_stack_t cte_new_stack(cardinal size, cte_stack_status_t *status) {
    cte_stack_s *stack;
    
    if (size == 0) {
        size = CTE_DEFAULT_STACK_SIZE;
    } // end if
    
    // allocate new stack
    stack = ALLOCATE(sizeof(cte_stack_s) + size * sizeof(cte_context_s));
    
    // bail out if allocation failed
    if (stack == NULL) {
        ASSIGN_BY_REF(status, CTE_STACK_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // initialise size and stack pointer
    stack->size = size;
    stack->pointer = 0;
    
    // initialise bottom of stack
    stack->context[0].str = NULL;
    stack->context[0].index = 0;
    
    // pass status and stack to caller
    ASSIGN_BY_REF(status, CTE_STACK_STATUS_SUCCESS);
    return (cte_stack_t) stack;
} // end cte_new_stack


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
                     cte_stack_status_t *status) {
    
    cte_stack_s *this_stack = (cte_stack_s *) stack;
    
    // bail out if stack is NULL
    if (stack == NULL) {
        ASSIGN_BY_REF(status, CTE_STACK_STATUS_INVALID_STACK);
        return;
    } // end if
    
    // bail out if value is NULL
    if (template_str == NULL) {
        ASSIGN_BY_REF(status, CTE_STACK_STATUS_INVALID_DATA);
        return;
    } // end if
    
    // bail out if stack is full
    if (this_stack->pointer >= this_stack->size) {
        ASSIGN_BY_REF(status, CTE_STACK_STATUS_STACK_OVERFLOW);
        return;
    } // end if
    
    this_stack->context[this_stack->pointer].str = template_str;
    this_stack->context[this_stack->pointer].index = index;
    this_stack->pointer++;
    
    ASSIGN_BY_REF(status, CTE_STACK_STATUS_SUCCESS);
    return;
} // end cte_stack_push_context


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
                     cte_stack_status_t *status) {
    
    cte_stack_s *this_stack = (cte_stack_s *) stack;
    
    // bail out if stack is NULL
    if (stack == NULL) {
        ASSIGN_BY_REF(status, CTE_STACK_STATUS_INVALID_STACK);
        return NULL;
    } // end if
    
    if (this_stack->pointer == 0) {
        ASSIGN_BY_REF(status, CTE_STACK_STATUS_STACK_EMPTY);
        return NULL;
    } // end if
    
    this_stack->pointer--;
    
    ASSIGN_BY_REF(status, CTE_STACK_STATUS_SUCCESS);
    *index = this_stack->context[this_stack->pointer].index;
    return this_stack->context[this_stack->pointer].str;
} // end cte_stack_pop_context


// ---------------------------------------------------------------------------
// function:  cte_stack_size( stack )
// ---------------------------------------------------------------------------
//
// Returns the number of context slots of stack <stack>,  returns zero if NULL
// is passed in for <stack>.

cardinal cte_stack_size(cte_stack_t stack) {
    
    cte_stack_s *this_stack = (cte_stack_s *) stack;
    
    // bail out if stack is NULL
    if (stack == NULL)
        return 0;
    
    return this_stack->size;
} // end cte_stack_size


// ---------------------------------------------------------------------------
// function:  cte_stack_number_of_entries( stack )
// ---------------------------------------------------------------------------
//
// Returns  the number of  template contexts  saved on stack <stack>,  returns
// zero if NULL is passed in for <stack>.

cardinal cte_stack_number_of_entries(cte_stack_t stack) {
    
    cte_stack_s *this_stack = (cte_stack_s *) stack;
    
    // bail out if stack is NULL
    if (stack == NULL)
        return 0;
    
    return this_stack->pointer;
} // end cte_stack_number_of_entries


// ---------------------------------------------------------------------------
// function:  cte_dispose_stack( stack )
// ---------------------------------------------------------------------------
//
// Disposes of stack object <stack>.

void cte_dispose_stack(cte_stack_t stack) {
    
    DEALLOCATE(stack);
    stack = NULL;
    
    return;
} // end cte_dispose_stack


// END OF FILE
