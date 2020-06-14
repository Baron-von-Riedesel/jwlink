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
#include "resutil.h"
#include "read.h"
#include "reserr.h"

extern off_t ResSeek( WResFileID handle, off_t offset, int origin )
/*****************************************************************/
/* cover function for seek */
{
    off_t               posn;

    posn = WRESSEEK( handle, offset, origin );
    if( posn == -1L ) {
        WRES_ERROR( WRS_SEEK_FAILED );
    }
    return( posn );
}

extern int ResPadDWord( WResFileID handle )
/*****************************************/
/* advances in the file to the next DWORD boundry */
{
    off_t   curr_pos;
    off_t   padding;
    int     error;

    error = FALSE;
    curr_pos = WRESTELL( handle );
    if( curr_pos == -1 ) {
        WRES_ERROR( WRS_TELL_FAILED );
        error = TRUE;
    } else {
        padding = RES_PADDING( curr_pos, sizeof(uint_32) );
        curr_pos = WRESSEEK( handle, padding, SEEK_CUR );
        if( curr_pos == -1L ) {
            WRES_ERROR( WRS_SEEK_FAILED );
            error = TRUE;
        }
    }
    return( error );
}
