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

/* MS 32-bit .RSRC file format if type + name ordinal
 * uint_32  +0 Size
 * uint_32  +4 HeaderSize
 * uint_16  +8 ffff, ordinal follows
 * uint_16 +10 resource type
 * uint_16 +12 ffff. ordinal follows
 * uint_16 +14 resource ID
 * uint_32 +16 data version
 * uint_16 +20 memory flags
 * uint_16 +22 language ID
 * uint_32 +24 version
 * uint_32 +28 flags
 */

extern M32ResResourceHeader *M32ResReadResourceHeader( WResFileID handle )
/************************************************************************/
{
    M32ResResourceHeader     *newhead;
    int                       error = FALSE;
    uint_16                   tmp16;
    uint_32                   tmp32;

    newhead = WRESALLOC( sizeof( M32ResResourceHeader ) );
    if( newhead == NULL ) {
        error = TRUE;
        WRES_ERROR( WRS_MALLOC_FAILED );
    }
    newhead->head16 = WRESALLOC( sizeof( MResResourceHeader ) );
    if( newhead->head16 == NULL ) {
        error = TRUE;
        WRES_ERROR( WRS_MALLOC_FAILED );
    }
    if( !error ) {
        error = ResPadDWord( handle );
    }
    if( !error ) {
        error = ResReadUint32( &tmp32, handle );
        newhead->head16->Size = tmp32;
    }
    if( !error ) {
        error = ResReadUint32( &(newhead->HeaderSize), handle );
    }
    if( !error ) {
        newhead->head16->Type = ResRead32NameOrOrdinal( handle );
        error = (newhead->head16->Type == NULL );
    }
    if( !error ) {
        newhead->head16->Name = ResRead32NameOrOrdinal( handle );
        error = (newhead->head16->Name == NULL );
    }
    if( !error ) {
        error = ResPadDWord( handle );
    }
    if( !error ) {
        error = ResReadUint32( &tmp32, handle );
        newhead->head16->DataVersion = tmp32;
    }
    if( !error ) {
        error = ResReadUint16( &tmp16, handle );
        newhead->head16->MemoryFlags = tmp16;
    }
    if( !error ) {
        error = ResReadUint16( &tmp16, handle );
        newhead->head16->LanguageId = tmp16;
    }
    if( !error ) {
        error = ResReadUint32( &tmp32, handle );
        newhead->head16->Version = tmp32;
    }
    if( !error ) {
        error = ResReadUint32( &tmp32, handle );
        newhead->head16->Characteristics = tmp32;
    }
    if( error && newhead != NULL ) {
        WRESFREE( newhead->head16 );
        WRESFREE( newhead );
        newhead = NULL;
    }

    return( newhead );
}

MResResourceHeader *MResReadResourceHeader( WResFileID handle )
/*************************************************************/
{
    MResResourceHeader     *newhead;
    int                     error;
    uint_16                 tmp16;
    uint_32                 tmp32;

    newhead = WRESALLOC( sizeof(MResResourceHeader) );
    if( newhead == NULL ) {
        error = TRUE;
        WRES_ERROR( WRS_MALLOC_FAILED );
    } else {
        error = FALSE;
    }

    if (!error) {
        newhead->Type = ResReadNameOrOrdinal( handle );
        error = (newhead->Type == NULL);
    }
    if (!error) {
        newhead->Name = ResReadNameOrOrdinal( handle );
        error = (newhead->Name == NULL);
    }
    if (!error) {
        error = ResReadUint16( &tmp16, handle);
        newhead->MemoryFlags = tmp16;
    }
    if (!error) {
        error = ResReadUint32( &tmp32, handle );
        newhead->Size = tmp32;
    }

    if (error && newhead != NULL) {
        WRESFREE( newhead );
        newhead = NULL;
    }

    return( newhead );
} /* MResReadResourceHeader */
