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


#ifndef EXERESPE_H_INCLUDED
#define EXERESPE_H_INCLUDED

#include "watcom.h"
#include "exepe.h"
#include "wresall.h"
#include "rcstr.h"

typedef struct PEResDirEntry {
    resource_dir_header Head;
    int                 NumUnused;
    struct PEResEntry * Children;
} PEResDirEntry;

typedef struct PEResDataEntry {
    resource_entry      Entry;
    WResDirWindow       Wind;           /* window into the current WResDir */
} PEResDataEntry;

typedef struct PEResEntry {
    resource_dir_entry  Entry;
    void *              Name;
    char                IsDirEntry;
    union {
        PEResDataEntry  Data;
        PEResDirEntry   Dir;
    } u;
} PEResEntry;

typedef struct PEResDir {
    PEResDirEntry   Root;
    uint_32         DirSize;
    pe_va           ResRVA;
    uint_32         ResOffset;
    uint_32         ResSize;
    StringBlock     String;
} PEResDir;

struct ResFileInfo;     // ANSI/gcc
struct ExeFileInfo;

int BuildResourceObject( struct ExeFileInfo *exeinfo,
                         struct ResFileInfo *resinfo,
                         pe_object *res_obj, unsigned_32 rva,
                         unsigned_32 offset, int writebyfile );
int RcBuildResourceObject( void );

#endif
