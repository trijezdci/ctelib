/* C Template Engine
 *
 *  @file CTE.c
 *  CTE implementation
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


#include "CTE.h"
#include "ASCII.h"
#include "hash.h"
#include "alloc.h"
#include "common.h"
#include "bailout.h"
#include "cte_stack.h"


// ---------------------------------------------------------------------------
// Size and growth parameters for target string
// ---------------------------------------------------------------------------

#define CTE_TARGET_SIZE_INITIAL (4*1024) /* 4 KBytes */

#define CTE_TARGET_SIZE_INCREMENT (4*1024) /* 4 KBytes */


// ---------------------------------------------------------------------------
// Maximum length for placeholder names
// ---------------------------------------------------------------------------

#define CTE_MAX_PLACEHOLDER_LENGTH 32


// ---------------------------------------------------------------------------
// Maximum template nesting level
// ---------------------------------------------------------------------------

#define CTE_MAX_NESTING_LEVEL 100


// ---------------------------------------------------------------------------
// Prefix for lines to ignore "%%"
// ---------------------------------------------------------------------------

#define CTE_IGNORE_PFX_CHAR_1 PERCENT
#define CTE_IGNORE_PFX_CHAR_2 PERCENT

static const char _cte_ignore_prefix[] = {
CTE_IGNORE_PFX_CHAR_1,
CTE_IGNORE_PFX_CHAR_2,
CSTRING_TERMINATOR
} /* _cte_ignore_prefix */ ;


// ---------------------------------------------------------------------------
// Delimiter for placeholders "@@"
// ---------------------------------------------------------------------------

#define CTE_DELIMITER_CHAR_1 AT_SIGN
#define CTE_DELIMITER_CHAR_2 AT_SIGN

static const char _cte_delimiter[] = {
CTE_DELIMITER_CHAR_1,
CTE_DELIMITER_CHAR_2,
CSTRING_TERMINATOR
} /* _cte_delimiter */ ;


// ---------------------------------------------------------------------------
// Notification handler
// ---------------------------------------------------------------------------

static cte_notification_f _cte_notify = NULL;


// ===========================================================================
// P R I V A T E   F U N C T I O N   P R O T O T Y P E S   A N D   M A C R O S
// ===========================================================================

static fmacro char *_update_target(char char_to_add, char *initial_str,
                                   cardinal index, cardinal *size, cte_status_t *status);

#define CTE_NOTIFY( _notification, _str, _index_or_size) \
{ if (_cte_notify != NULL) \
_cte_notify( _notification, _str, _index_or_size); }

#define CTE_START_OF_LINE(_str, _index, _nesting_level) \
(((_index == 0) && (_nesting_level == 0)) || \
((_index > 0) && (_str[_index-1] == NEWLINE)))


// ===========================================================================
// P U B L I C   F U N C T I O N   I M P L E M E N T A T I O N S
// ===========================================================================

// ---------------------------------------------------------------------------
// function:  cte_delimiter()
// ---------------------------------------------------------------------------
//
// Returns a pointer to a constant C string  containing the library's built-in
// placeholder delimiter.  The placeholder delimiter may be changed at compile
// time only.  The factory setting is "@@".

inline const char *cte_delimiter(void) {
    return (const char *) &_cte_delimiter;
} // end cte_delimiter


// ---------------------------------------------------------------------------
// function:  cte_ignore_prefix()
// ---------------------------------------------------------------------------
//
// Returns a pointer to a constant C string  containing the library's built-in
// ignore prefix.  The ignore prefix may be changed at compile time only.  The
// factory setting is "%%".

inline const char *cte_ignore_prefix(void) {
    return (const char *) &_cte_ignore_prefix;
} // end cte_ignore_prefix


// ---------------------------------------------------------------------------
// function:  cte_max_placeholder_length()
// ---------------------------------------------------------------------------
//
// Returns the value  of the library's built-in maximum length for placeholder
// identifiers (length not including delimiters).  The factory setting is 32.

inline cardinal cte_max_placeholder_length(void) {
    return CTE_MAX_PLACEHOLDER_LENGTH;
} // end cte_max_placeholder_length


// ---------------------------------------------------------------------------
// function:  cte_max_nesting_level()
// ---------------------------------------------------------------------------
//
// Returns the value  of the library's  built-in  maximum level  for  template
// nesting.  The factory setting is 100.

inline cardinal cte_max_nesting_level(void) {
    return CTE_MAX_NESTING_LEVEL;
} // end cte_max_nesting_level


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

inline void cte_install_notification_handler(cte_notification_f handler) {
    _cte_notify = handler;
    return;
} // end cte_install_notification_handler


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
                               cte_status_t *status) {
    
    char *source; // source string pointer
    char *target; // target string pointer
    cardinal s_index; // source string index
    cardinal t_index; // target string index
    cardinal t_size; // allocated size of target string
    
    cte_stack_t stack; // template context stack
    cardinal nesting_level; // template nesting level
    
    kvs_key_t key; // placeholder key
    cardinal ident_len; // identifier length
    cte_status_t r_status; // intermediate status
    
    
    // bail out if template string is NULL
    if (template == NULL) {
        ASSIGN_BY_REF(status, CTE_STATUS_INVALID_TEMPLATE);
        return NULL;
    } // end if
    
    // bail out if placeholders is NULL
    if (placeholders == NULL) {
        ASSIGN_BY_REF(status, CTE_STATUS_INVALID_PLACEHOLDERS);
        return NULL;
    } // end if
    
    // allocate new target string
    t_size = CTE_TARGET_SIZE_INITIAL;
    target = ALLOCATE(t_size);
    
    // bail out if target allocation failed
    if (target == NULL) {
        CTE_NOTIFY(CTE_NOTIFICATION_TARGET_ALLOCATION_FAILED, template, 0);
        ASSIGN_BY_REF(status, CTE_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    // allocate new recursion stack
    stack = cte_new_stack(CTE_MAX_NESTING_LEVEL, NULL);
    
    // bail out if stack allocation failed
    if (stack == NULL) {
        CTE_NOTIFY(CTE_NOTIFICATION_STACK_ALLOCATION_FAILED, template, 0);
        DEALLOCATE(target);
        ASSIGN_BY_REF(status, CTE_STATUS_ALLOCATION_FAILED);
        return NULL;
    } // end if
    
    
    nesting_level = 0;
    
    source = (char *) template;
    s_index = 0;
    t_index = 0;
    
    // recursively expand source strings
    repeat {
        
        // copy all characters until special character is found
        while ((source[s_index] != BACKSLASH) &&
               (source[s_index] != CTE_DELIMITER_CHAR_1) &&
               (source[s_index] != CTE_IGNORE_PFX_CHAR_1) &&
               (source[s_index] != CSTRING_TERMINATOR)) {
            
            // copy char to target, enlarge if necessary
            target = _update_target(source[s_index],
                                    target, t_index, &t_size, &r_status);
            
            // bail out if allocation failed
            if (r_status == CTE_STATUS_ALLOCATION_FAILED)
                BAILOUT(enlargement_failed);
            
            s_index++;
            t_index++;
        } // end while
        
        // handle special characters
        switch (source[s_index]) {
                
                // backslash may indicate escaped delimiter
            case BACKSLASH :
                
                switch (source[s_index+1]) {
                        
                    // found backslash escaped backslash
                    case BACKSLASH :
                    // copy leading backslash to target, enlarge if necessary
                        target = _update_target(source[s_index],
                                        target, t_index, &t_size, &r_status);
                        
                        // bail out if allocation failed
                        if (r_status == CTE_STATUS_ALLOCATION_FAILED)
                            BAILOUT(enlargement_failed);
                        
                        s_index++;
                        t_index++;
                        
                        break; // case
                        
                        // found backslash escaped delimiter
                    case CTE_DELIMITER_CHAR_1 :
                        // skip leading backslash
                        s_index++;
                        
                        break; // case
                        
                        // found ignore prefix following backslash
                    case CTE_IGNORE_PFX_CHAR_1 :
                        // check if leading backslash is at first row of line
                        if (CTE_START_OF_LINE(source, s_index, nesting_level))
                            // skip leading backslash
                            s_index++;
                        
                        break; // case
                } // end switch
                
                // copy remaining character to target, enlarge if necessary
                target = _update_target(source[s_index],
                                        target, t_index, &t_size, &r_status);
                
                // bail out if allocation failed
                if (r_status == CTE_STATUS_ALLOCATION_FAILED)
                    BAILOUT(enlargement_failed);
                
                s_index++;
                t_index++;
                
                break; // case
                
                // delimiter char may indicate template engine placeholder
            case CTE_DELIMITER_CHAR_1:
                
                // check for opening delimiter followed by letter
                if ((source[s_index+1] == CTE_DELIMITER_CHAR_2) &&
                    (IS_LETTER(source[s_index+2]))) {
                    
                    // calculate key for identifier following delimiter
                    s_index = s_index + 2;
                    key = HASH_INITIAL;
                    ident_len = 0;
                    
                    // calculate key of identifier
                    repeat {
                        key = HASH_NEXT_CHAR(key, source[s_index]);
                        s_index++;
                        ident_len++;
                    } until ((IS_NOT_UNDERSCORE_NOR_ALPHANUM(source[s_index]))
                             || (ident_len > CTE_MAX_PLACEHOLDER_LENGTH));
                    key = HASH_FINAL(key);
                    
                    // check if identifier is a placeholder
                    if ((ident_len <= CTE_MAX_PLACEHOLDER_LENGTH) &&
                        (kvs_entry_exists(placeholders, key, NULL)) &&
                        (source[s_index] == CTE_DELIMITER_CHAR_1) &&
                        (source[s_index+1] == CTE_DELIMITER_CHAR_2)) {
                        
                        // bail out if nesting limit is reached
                        if (nesting_level >= CTE_MAX_NESTING_LEVEL)
                            BAILOUT(nesting_limit_exceeded);
                        
                        // save source and index to recursion stack
                        cte_stack_push_context(stack, source, s_index, NULL);
                        
                        // set source and index to content of placeholder
                        source = kvs_value_for_key(placeholders, key, NULL);
                        s_index = 0;
                        
                        // update template nesting level
                        nesting_level++;
                    }
                    else /* identifier is not a placeholder */ {
                        
                        // restore source index to delimiter position 
                        s_index = s_index - ident_len - 2;
                        
                        CTE_NOTIFY(CTE_NOTIFICATION_UNDEFINED_PLACEHOLDER,
                                   source, s_index);
                        
                        // copy char to target, enlarge if necessary
                        target = _update_target(source[s_index],
                                        target, t_index, &t_size, &r_status);
                        
                        // bail out if allocation failed
                        if (r_status == CTE_STATUS_ALLOCATION_FAILED)
                            BAILOUT(enlargement_failed);
                        
                        s_index++;
                        t_index++;
                    } // end if
                }
                else /* no opening delimiter followed by letter found */ {
                    // copy char to target, enlarge if necessary
                    target = _update_target(source[s_index],
                                        target, t_index, &t_size, &r_status);
                    
                    // bail out if allocation failed
                    if (r_status == CTE_STATUS_ALLOCATION_FAILED)
                        BAILOUT(enlargement_failed);
                    
                    s_index++;
                    t_index++;
                } // end if
                
                break; // case
                
                // prefix char may indicate template engine comment line
            case CTE_IGNORE_PFX_CHAR_1:
                // check for ignore line prefix at first coloumn
                if ((source[s_index+1] == CTE_IGNORE_PFX_CHAR_2) &&
                    CTE_START_OF_LINE(source, s_index, nesting_level)) {
                    
                    // skip all characters until line end without copying
                    while ((source[s_index] != NEWLINE) &&
                           (source[s_index] != CSTRING_TERMINATOR)) {
                        s_index++;
                    } // end while
                }
                else /* no ignore line prefix found at first coloumn */ {
                    // copy char to target, enlarge if necessary
                    target = _update_target(source[s_index],
                                        target, t_index, &t_size, &r_status);
                    
                    // bail out if allocation failed
                    if (r_status == CTE_STATUS_ALLOCATION_FAILED)
                        BAILOUT(enlargement_failed);
                    
                    s_index++;
                    t_index++;
                } // end if
                
                break; // case
                
                // C string terminator indicates end of template string
            case CSTRING_TERMINATOR:
                
                // return from recursion unless nesting level is zero
                if (nesting_level > 0) {
                    // restore source and index from recursion stack
                    source = cte_stack_pop_context(stack, &s_index, NULL);
                    // update template nesting level
                    nesting_level--;
                } // end if
                
                break; // case
        } // end switch
        
    } until ((source[s_index] == CSTRING_TERMINATOR) && (nesting_level == 0));
    
    // clean up
    
    // terminate target string, enlarge if necessary
    target = _update_target(CSTRING_TERMINATOR,
                            target, t_index, &t_size, &r_status);
    
    // bail out if allocation failed
    if (r_status == CTE_STATUS_ALLOCATION_FAILED)
        BAILOUT(enlargement_failed);
    
    /* NORMAL TERMINATION */

    CTE_NOTIFY(CTE_NOTIFICATION_TARGET_SIZE_INFO, target, t_size);
    
    // return expanded string and status to caller
    ASSIGN_BY_REF(status, CTE_STATUS_SUCCESS);
    DEALLOCATE(stack);
    return target;
    
    /* ERROR HANDLING */
    
    ON_ERROR(enlargement_failed) :
        CTE_NOTIFY(CTE_NOTIFICATION_TARGET_ENLARGEMENT_FAILED,
                   source, s_index);
        DEALLOCATE(stack);
        ASSIGN_BY_REF(status, CTE_STATUS_ALLOCATION_FAILED);
        return target;
    
    ON_ERROR(nesting_limit_exceeded) :
        CTE_NOTIFY(CTE_NOTIFICATION_NESTING_LIMIT_EXCEEDED,
                   source, s_index);
        DEALLOCATE(target);
        DEALLOCATE(stack);
        ASSIGN_BY_REF(status, CTE_STATUS_NESTING_LIMIT_EXCEEDED);
        return NULL;
} // cte_string_from_template


// ===========================================================================
// P R I V A T E   F U N C T I O N   I M P L E M E N T A T I O N S
// ===========================================================================

// ---------------------------------------------------------------------------
// private function:  _update_target( ch, initial_str, index, size, status)
// ---------------------------------------------------------------------------
//
// Writes a single character <char_to_add> at position <index> into the target
// string,  pointed to by <initial_target>.  The target string must be a dyna-
// mically allocated string  and its allocation size must be passed in <size>.
// If <index> is equal to or greater than <size>,  then the target string will
// be  dynamically enlarged  by the number of bytes  as defined  in file CTE.c
// by constant CTE_TARGET_SIZE_INCREMENT.  If the  target string  has been en-
// larged,  its  new allocation size  is passed back  in <size>.  A pointer to
// the  updated  target  string  is returned  as the  function result.  In the
// event that enlargement failed,  the target string is deallocated  and  NULL
// is returned.
//
// NOTE: This primitive does  NOT  implicitly terminate the target string.  To
// terminate the target string,  this primitive must be called passing '\0' in
// parameter <char_to_add>.
//
// The status of the operation  is passed back in <status>,  unless  NULL  was
// passed in for <status>.
//
// pre-conditions:
//  o  initial_target  must point to a dynamically allocated character string
//  o  char_to_add  must contain the character to be added to the string
//  o  index must not be larger than size + CTE_TARGET_SIZE_INCREMENT
//  o  size  must point to a cardinal number containing the current allocation
//     size of the string
//  o  status  must either be NULL or point to a variable of type cte_status_t
//
// post-conditions:
//  o  char_to_add has been written to position index in string initial_target
//  o  if the target string has been enlarged,  then the  new  allocation size
//     is passed back in size, otherwise size remains unmodified
//  o  if status is not NULL, then CTE_STATUS_SUCCESS is passed back in status
//  o  a pointer to the modified (and possibly enlarged) string is returned
//
// error-conditions:
//  o  if target string enlargement failed, then the string passed in via
//     initial_target string is deallocated, size is not modified,
//     CTE_STATUS_ALLOCATION_FAILED is passed back in status and
//     NULL is returned

static fmacro char *_update_target(char char_to_add,
                                   char *initial_target,
                                   cardinal index,
                                   cardinal *size,
                                   cte_status_t *status) {
    char *new_target;
    cardinal new_size;
    
    if (index >= *size) {
        new_size = *size + CTE_TARGET_SIZE_INCREMENT;
        new_target = REALLOCATE(initial_target, new_size);
        
        if (new_target != NULL) {
            *size = new_size;
        }
        else /* reallocation failed */ {
            ASSIGN_BY_REF(status, CTE_STATUS_ALLOCATION_FAILED);
            return initial_target;
        } // end if
    } // end if
    
    new_target[index] = char_to_add;
    ASSIGN_BY_REF(status, CTE_STATUS_SUCCESS);
    
    return new_target;
} // _update_target


// END OF FILE
