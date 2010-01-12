/* C Template Engine
 *
 *  @file CTE.h
 *  CTE interface
 *
 *  Template Engine
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


#ifndef CTE_H
#define CTE_H


#include "../KVS/KVS.h"


// ---------------------------------------------------------------------------
// Maximum length for placeholder names
// ---------------------------------------------------------------------------

#define CTE_MAX_PLACEHOLDER_LENGTH 32


// ---------------------------------------------------------------------------
// Maximum template nesting level
// ---------------------------------------------------------------------------

#define CTE_MAX_NESTING_LEVEL 65535


// ---------------------------------------------------------------------------
// Status codes
// ---------------------------------------------------------------------------

typedef /* cte_status_t */ enum {
    CTE_STATUS_SUCCESS = 1,
    CTE_STATUS_INVALID_TEMPLATE,
    CTE_STATUS_INVALID_PLACEHOLDERS,
    CTE_STATUS_ALLOCATION_FAILED,
    CTE_STATUS_NESTING_LIMIT_EXCEEDED,
} cte_status_t;


// ---------------------------------------------------------------------------
// Notification codes
// ---------------------------------------------------------------------------

typedef /* cte_notification_t */ enum {
    CTE_NOTIFICATION_TARGET_SIZE_INFO,
    CTE_NOTIFICATION_TARGET_ALLOCATION_FAILED,
    CTE_NOTIFICATION_TARGET_ENLARGEMENT_FAILED,
    CTE_NOTIFICATION_STACK_ALLOCATION_FAILED,
    CTE_NOTIFICATION_STACK_ENLARGEMENT_FAILED,
    CTE_NOTIFICATION_UNDEFINED_PLACEHOLDER,
    CTE_NOTIFICATION_NESTING_LIMIT_EXCEEDED,
} cte_notification_t;


// ---------------------------------------------------------------------------
// Notification handler type
// ---------------------------------------------------------------------------

typedef void (*cte_notification_f)(cte_notification_t, const char*, cardinal);


// ---------------------------------------------------------------------------
// function:  cte_delimiter()
// ---------------------------------------------------------------------------
//
// Returns a pointer to a constant C string  containing the library's built-in
// placeholder delimiter.  The placeholder delimiter may be changed at compile
// time only.  The factory setting is "@@".

inline const char *cte_delimiter(void);


// ---------------------------------------------------------------------------
// function:  cte_ignore_prefix()
// ---------------------------------------------------------------------------
//
// Returns a pointer to a constant C string  containing the library's built-in
// ignore prefix.  The ignore prefix may be changed at compile time only.  The
// factory setting is "%%".

inline const char *cte_ignore_prefix(void);


// ---------------------------------------------------------------------------
// function:  cte_install_notification_handler( handler )
// ---------------------------------------------------------------------------
//
// Installs function <handler>  as  notification handler.  If  a  notification
// handler is installed,  the template engine calls the handler when a notifi-
// able event occurs while expanding a template.  Notifiable events are either
// informational or warnings or errors.  By default no handler is installed.
//
// The template engine passes the following parameters to the handler:
//
// o  notification code describing the notified event
// o  pointer to the template being expanded when the event occurred
// o  index to the character in the template when the event occurred
//
// A notification handler may be uninstalled by passing in NULL for <handler>.

inline void cte_install_notification_handler(cte_notification_f handler);


// ---------------------------------------------------------------------------
// function:  cte_string_from_template( tmplate, placeholders, status )
// ---------------------------------------------------------------------------
//
// Recursively expands  all placeholder strings  in template string <template>
// and  returns a pointer to a new dynamically allocated string containing the
// resulting string.  The function fails  if NULL is passed in  for <template>
// or <placeholders>  or if allocation fails  or the template nesting limit is
// exceeded.  The function returns NULL if it fails.
//
// When a placeholder string is found in the template, a key is calculated for
// its identifier.  The key is then looked up in the placeholder table  passed
// in <placeholders>.  If the key is found in the placeholder table,  then its
// value is retrieved and the respective placeholder string in the template is
// replaced with the string pointed to by the retrieved value.
//
// The function recognises templates according to the following EBNF grammar:
//
//  template :
//    ( template-comment | escape-sequence | placeholder-string | character )*
//
//  template-comment :
//    '%%' character* end-of-line
//
//  placeholder-string :
//    '@@' identifier '@@'
//
//  identifier :
//    letter ( letter | digit | '_' )*
//
//  letter :
//    'A' .. 'Z' | 'a' .. 'z'
//
//  digit :
//    '0' .. '9'
//
//  end-of-line :
//    ASCII(10)
//
//  escape-sequence :
//    '\' ( '\' | '%' | '@' )
//
//  character :
//    ASCII(0) .. ASCII(127)
//
// Static semantics:
//
// o  template comments are  ONLY  recognised in coloumn #1 of any line,  that
//    is to say,  the prefix "%%" must either occur at index 0 of the top most
//    template,  or it must immediately follow a newline control character.
//
// o  identifiers must not exceed the value of cte_max_placeholder_length().
//
// o  template nesting must not exceed the value of cte_max_nesting_level().
//
// o  escape sequences are reproduced as follows:
//    "\\" produces "\\" in the expanded result string
//    "\@" produces "@" in the expanded result string
//    "\%" at coloumn #1 produces "%" in the expanded result string
//    "\%" at coloumns > 1 produces "\%" in the expanded result string
//    
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.

char *cte_string_from_template(const char *template,
                              kvs_table_t placeholders,
                             cte_status_t *status);
    

#endif /* CTE_H */

// END OF FILE