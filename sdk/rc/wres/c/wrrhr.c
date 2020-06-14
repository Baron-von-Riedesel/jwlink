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


#include "wresrtns.h"
#include "read.h"
#include "reserr.h"

int WResReadHeaderRecord( WResHeader * header, WResFileID handle )
/****************************************************************/
{
    off_t   currpos;
    int     error;
    int     numread;

    error = TRUE;
    currpos = WRESSEEK( handle, 0, SEEK_SET );
    if( currpos == -1L ) {
        WRES_ERROR( WRS_SEEK_FAILED );
    } else {
        numread = WRESREAD( handle, header, sizeof(WResHeader) );
        if( numread != sizeof(WResHeader) ) {
            WRES_ERROR( numread == -1 ? WRS_READ_FAILED:WRS_READ_INCOMPLETE );
        } else {
            currpos = WRESSEEK( handle, currpos, SEEK_SET );
            if( currpos == -1L ) {
                WRES_ERROR( WRS_SEEK_FAILED );
            } else {
                error = FALSE;
            }
        }
    }
    return( error );
}

int WResReadExtHeader( WResExtHeader * head, WResFileID handle )
/**************************************************************/
{
    int     numread;

    numread = WRESREAD( handle, head, sizeof(WResExtHeader) );
    if( numread != sizeof(WResExtHeader) ) {
        WRES_ERROR( numread == -1 ? WRS_READ_FAILED:WRS_READ_INCOMPLETE );
        return( TRUE );
    } else {
        return( FALSE );
    }
}
