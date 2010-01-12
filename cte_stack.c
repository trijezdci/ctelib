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
// Range checks
// ---------------------------------------------------------------------------

#if (CTE_DEFAULT_STACK_SIZE < 1)
#error CTE_DEFAULT_STACK_SIZE must not be zero, recommended minimum is 8
#elif (CTE_DEFAULT_STACK_SIZE > CTE_MAXIMUM_STACK_SIZE)
#error CTE_DEFAULT_STACK_SIZE must not be larger than CTE_MAXIMUM_STACK_SIZE
#endif

#if (CTE_DEFAULT_STACK_SIZE < 8)
#warning CTE_DEFAULT_STACK_SIZE is unreasonably low, factory setting is 100
#elif (CTE_DEFAULT_STACK_SIZE > 65535)
#warning CTE_DEFAULT_STACK_SIZE is unreasonably high, factory setting is 100
#endif


// ---------------------------------------------------------------------------
// Template context storage type
// ---------------------------------------------------------------------------

typedef struct /* cte_context_s */ {
        char *str;
    cardinal index;
} cte_context_s;


// ---------------------------------------------------------------------------
// Template context stack entry pointer type for self referencing declaration
// ---------------------------------------------------------------------------

struct _cte_stack_entry_s; /* FORWARD */

typedef struct _cte_stack_entry_s *cte_stack_entry_p;


// ---------------------------------------------------------------------------
// Template context stack entry type
// ---------------------------------------------------------------------------

struct _cte_stack_entry_s {
        cte_context_s context;
    cte_stack_entry_p next;
};

typedef struct _cte_stack_entry_s cte_stack_entry_s;


// ---------------------------------------------------------------------------
// Template context stack type
// ---------------------------------------------------------------------------

typedef struct /* cte_stack_s */ {
    cte_stack_entry_s *overflow;
     cte_stack_size_t entry_count;
     cte_stack_size_t array_size;
        cte_context_s context[0];
} cte_stack_s;


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
                        cte_stack_status_t *status) {
    cte_stack_s *stack;
    
    // zero size means default
    if (initial_size == 0) {
        initial_size = CTE_DEFAULT_STACK_SIZE;
    } // end if
    
    // bail out if initial size is too high
    if (initial_size > CTE_MAXIMUM_STACK_SIZE) {
        ASSIGN_BY_REF(status, CTE_STACK_STATUS_INVALID_SIZE);
        return NULL;
    } // end if
    
    // allocate new stack
    stack =
        ALLOCATE(sizeof(cte_stack_s) + initial_size * sizeof(cte_context_s));
    
    // bail out if allocation failed
    if (stack == NULL) {
        ASSIGN_BY_REF(status, CTE_STACK_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // initialise meta data
    stack->array_size = initial_size;
    stack->entry_count = 0;
    stack->overflow = NULL;
        
    // pass status and new stack to caller
    ASSIGN_BY_REF(status, CTE_STACK_STATUS_SUCCESS);
    return (cte_stack_t) stack;
} // end cte_new_stack


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
                     cte_stack_status_t *status) {
    
    #define this_stack ((cte_stack_s *)stack)
    cte_stack_entry_s *new_entry;
    
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
    if (this_stack->entry_count >= CTE_MAXIMUM_STACK_SIZE) {
        ASSIGN_BY_REF(status, CTE_STACK_STATUS_STACK_OVERFLOW);
        return;
    } // end if
    
    // check if index falls within array segment
    if (this_stack->entry_count < this_stack->array_size) {
        
        // store context in arr array segment
        this_stack->context[this_stack->entry_count].str = template_str;
        this_stack->context[this_stack->entry_count].index = index;
    }
    else /* index falls within overflow segment */ {
        
        // allocate new entry slot
        new_entry = ALLOCATE(sizeof(cte_stack_entry_s));
        
        // bail out if allocation failed
        if (new_entry == NULL) {
            ASSIGN_BY_REF(status, CTE_STACK_STATUS_ALLOCATION_FAILED);
            return;
        } // end if
        
        // store context in new_entry
        new_entry->context.str = template_str;
        new_entry->context.index = index;
        
        // link new entry into overflow list
        new_entry->next = this_stack->overflow;
        this_stack->overflow = new_entry;
    } // end if
    
    // update entry counter
    this_stack->entry_count++;
    
    ASSIGN_BY_REF(status, CTE_STACK_STATUS_SUCCESS);
    return;
    
    #undef this_stack
} // end cte_stack_push_context


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
                     cte_stack_status_t *status) {
    
    #define this_stack ((cte_stack_s *)stack)
    cte_stack_entry_s *this_entry;
    char *template_str;
    
    // bail out if stack is NULL
    if (stack == NULL) {
        ASSIGN_BY_REF(status, CTE_STACK_STATUS_INVALID_STACK);
        return NULL;
    } // end if

    // bail out if index is NULL
    if (index == NULL) {
        ASSIGN_BY_REF(status, CTE_STACK_STATUS_INVALID_INDEX);
        return NULL;
    } // end if
    
    if (this_stack->entry_count == 0) {
        ASSIGN_BY_REF(status, CTE_STACK_STATUS_STACK_EMPTY);
        return NULL;
    } // end if
    
    this_stack->entry_count--;
    
    // check if index falls within array segment
    if (this_stack->entry_count < this_stack->array_size) {
        
        // return value and status to caller
        ASSIGN_BY_REF(status, CTE_STACK_STATUS_SUCCESS);
        *index = this_stack->context[this_stack->entry_count].index;
        return this_stack->context[this_stack->entry_count].str;
        
    }
    else /* index falls within overflow segment */ {
        
        // get context from first entry in overflow list
        template_str = this_stack->overflow->context.str;
        *index = this_stack->overflow->context.index;
        
        // remember first entry in overflow list
        this_entry = this_stack->overflow;
        this_stack->overflow = this_stack->overflow->next;
        
        // remove the entry
        DEALLOCATE(this_entry);
        
        ASSIGN_BY_REF(status, CTE_STACK_STATUS_SUCCESS);
        return template_str;
    } // end if
    
    #undef this_stack
} // end cte_stack_pop_context


// ---------------------------------------------------------------------------
// function:  cte_stack_size( stack )
// ---------------------------------------------------------------------------
//
// Returns the number of context slots of stack <stack>,  returns zero if NULL
// is passed in for <stack>.

cte_stack_size_t cte_stack_size(cte_stack_t stack) {
    #define this_stack ((cte_stack_s *)stack)
    
    // bail out if stack is NULL
    if (stack == NULL)
        return 0;
    
    if (this_stack->entry_count < this_stack->array_size)
        return this_stack->array_size;
    else
        return this_stack->entry_count;
    
    #undef this_stack
} // end cte_stack_size


// ---------------------------------------------------------------------------
// function:  cte_stack_number_of_entries( stack )
// ---------------------------------------------------------------------------
//
// Returns  the number of  template contexts  saved on stack <stack>,  returns
// zero if NULL is passed in for <stack>.

cte_stack_size_t cte_stack_number_of_entries(cte_stack_t stack) {
    #define this_stack ((cte_stack_s *)stack)
    
    // bail out if stack is NULL
    if (stack == NULL)
        return 0;
    
    return this_stack->entry_count;
    
    #undef this_stack
} // end cte_stack_number_of_entries


// ---------------------------------------------------------------------------
// function:  cte_dispose_stack( stack )
// ---------------------------------------------------------------------------
//
// Disposes of stack object <stack>.  Returns NULL.

cte_stack_t cte_dispose_stack(cte_stack_t stack) {
    #define this_stack ((cte_stack_s *)stack)
    cte_stack_entry_s *this_entry;
    
    // bail out if stack is NULL
    if (stack == NULL)
        return NULL;
    
    // deallocate any entries in stack's overflow list
    while (this_stack->overflow != NULL) {
        
        // isolate first entry in overflow list
        this_entry = this_stack->overflow;
        this_stack->overflow = this_stack->overflow->next;
        
        // deallocate the entry
        DEALLOCATE(this_entry);
    } // end while
    
    // deallocate stack object and pass NULL to caller
    DEALLOCATE(stack);
    return NULL;
    
    #undef this_stack
} // end cte_dispose_stack


// END OF FILE
