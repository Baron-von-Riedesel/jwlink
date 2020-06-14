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


#ifndef SEMACCEL_INCLUDED
#define SEMACCEL_INCLUDED

#include "wresall.h"
#include "watcom.h"
#include "os2res.h"

/**** Semantic structures ****/
typedef struct FullAccelEntry {
    uint_8              Win32;
    union {
        AccelTableEntry     entry;
        AccelTableEntry32   entry32;
    } u;
    uint_32             startoftable;
} FullAccelEntry;

typedef struct FullAccelFlags {
    AccelFlags          flags;
    uint_8              typegiven;
} FullAccelFlags;

typedef struct AccelEvent {
    int                 event;
    uint_8              strevent;
} AccelEvent;

typedef struct FullAccelFlagsOS2 {
    uint_16             flags;
    uint_8              typegiven;
} FullAccelFlagsOS2;

typedef struct FullAccelEntryOS2 {
    struct FullAccelEntryOS2    *next;
    struct FullAccelEntryOS2    *prev;
    AccelTableEntryOS2          entry;
} FullAccelEntryOS2;

typedef struct FullAccelTableOS2 {
    struct FullAccelEntryOS2    *head;
    struct FullAccelEntryOS2    *tail;
} FullAccelTableOS2;

/**** Semantic variables ****/
extern const FullAccelFlags     DefaultAccelFlags;
extern const FullAccelFlagsOS2  DefaultAccelFlagsOS2;

/**** Semantic routines ****/
extern int SemStrToAccelEvent( char * string );
extern FullAccelEntry SemMakeAccItem( AccelEvent event, unsigned long idval,
                    FullAccelFlags flags );
extern void SemWriteAccelEntry( FullAccelEntry entry );
extern void SemWriteLastAccelEntry( FullAccelEntry entry );

extern int SemOS2StrToAccelEvent( char *string );
extern FullAccelTableOS2 *SemOS2NewAccelTable( FullAccelEntryOS2 );
extern FullAccelTableOS2 *SemOS2AddAccelEntry( FullAccelEntryOS2,
                                               FullAccelTableOS2 * );
extern FullAccelEntryOS2 SemOS2MakeAccItem( AccelEvent event, unsigned long idval,
                    FullAccelFlagsOS2 flags );
extern void SemOS2WriteAccelTable( WResID *name, ResMemFlags flags,
                            uint_32 codepage, FullAccelTableOS2 *acctable );
#endif
