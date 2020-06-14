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


#include <string.h>
#include "wresrtns.h"
#include "read.h"
#include "reserr.h"

WResTypeInfo * WResReadTypeRecord( WResFileID handle )
/****************************************************/
/* reads in the fields of a type info record from the current position in */
/* the file identified by fp */
{
    WResTypeInfo        newtype;
    WResTypeInfo *      newptr;
    int                 numread;
    int                 numcharsleft;
    int                 error;

    error = WResReadFixedTypeRecord( &newtype, handle );
    if (error) {
        return( NULL );
    }

    if (newtype.TypeName.IsName) {
        numcharsleft = newtype.TypeName.ID.Name.NumChars - 1;
    } else {
        numcharsleft = 0;
    }
    newptr = WRESALLOC( sizeof(WResTypeInfo) + numcharsleft );
    if (newptr == NULL) {
        WRES_ERROR( WRS_MALLOC_FAILED );
    } else {
        memcpy( newptr, &newtype, sizeof(WResTypeInfo) );
        if (numcharsleft != 0) {
            numread = (* WRESREAD) ( handle,
                    &(newptr->TypeName.ID.Name.Name[1]), numcharsleft );
            if (numread != numcharsleft) {
                WRES_ERROR( numread == -1 ? WRS_READ_FAILED:WRS_READ_INCOMPLETE );
                WRESFREE( newptr );
                newptr = NULL;
            }
        }
    }

    return( newptr );
} /* WResReadTypeRecord */
