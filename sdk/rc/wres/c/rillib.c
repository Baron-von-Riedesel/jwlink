/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/


/*
 * MEM2.C - memory allocation/usage routines
 *
 * By:  Craig Eisler
 *      June 4-5,11-12,17 1990
 *      July 16,31 1990
 *      August 1 1990
 *      September 11 1990
 *      October 31 1990
 *      November 4,7,19,22 1990
 *      December 28 1990
 *      January 1 1991
 *      March 30 1991
 *
 * Routines:
 *              ResAddLLItemAtEnd
 *              ResInsertLLItemAfter
 *              ResInsertLLItemBefore
 *              ResDeleteLLItem
 *              ResReplaceLLItem
 */

#include <stdlib.h>
#include "mem2.h"

typedef struct ss {
struct ss *next,*prev;
} ss;


/*
 * ResInsertLLItemBefore - insert an item into a linked list after an item
 */
#ifdef __WATCOMC__
#pragma off (unreferenced)
#endif
void ResInsertLLItemBefore( void **headptr, void **tailptr,
                                void *whoptr, void *itemptr )
#ifdef __WATCOMC__
#pragma on (unreferenced)
#endif
{
    ss          *before_who;
    ss          **head;
    ss          *who;
    ss          *item;

    head = (ss **) headptr;
    who  = (ss *)whoptr;
    item = (ss *)itemptr;
    before_who = who->prev;
    who->prev = item;
    item->next = who;
    item->prev = before_who;

    if( before_who == NULL ) {
        *head = item;
    } else {
        before_who->next = item;
    } /* if */

} /* ResInsertLLItemBefore */
