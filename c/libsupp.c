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
 *  LIBSUPP   : Library support
 *
 */

#include <string.h>
#include "linkstd.h"
#include "pcobj.h"
#include "msg.h"
#include "objcache.h"
#include "distrib.h"
#include "symtrace.h"
#include "specials.h"
#include "library.h"
#include "procfile.h"


static bool ProcLibFile( file_list *lib, char *name )
/***************************************************/
{
    mod_entry *     lp;
    mod_entry **    prev;
    unsigned long   dummy;

    DEBUG(( DBG_OLD, "ProcLibFile( %s ) enter, mod=%s, calling SearchLib()", name, CurrMod->name ));
    lp = SearchLib( lib, name );
    if( lp == NULL ) {
        CacheClose( lib, 1 );
        return( FALSE );
    }
    lib->status |= STAT_LIB_USED;
    if( FmtData.type & MK_OVERLAYS && FmtData.u.dos.distribute ) {
        if( lib->status & STAT_LIB_FIXED ) {
            lp->modinfo |= MOD_FIXED;
        }
        AddModTable( lp, lib->ovlref );
    } else {
        prev = &LibModules;
        while( *prev != NULL ) {/*  find end of list */
            prev = &(*prev)->n.next_mod;
        }
        *prev = lp;
    }
    CurrMod = lp;
    CurrMod->name = IdentifyObject( lp->f.source, &lp->location, &dummy );
    CurrMod->modinfo |= ObjFormat & FMT_OBJ_FMT_MASK;
    DEBUG(( DBG_OLD, "ProcLibFile( %s ): symbol found in %s, modinfo=%h", name, CurrMod->name, CurrMod->modinfo ));
    ObjPass1();
    CacheClose( lp->f.source, 1 );
    if( FmtData.type & MK_OVERLAYS && FmtData.u.dos.distribute ) {
        FinishArcs( lp );
    }
    if( FindLibTrace( lp ) ) {
        TraceSymList( lp->publist );
    }
    return( TRUE );
}

#define PREFIX_LEN (sizeof(ImportSymPrefix) - 1)

bool LibFind( char *name, bool old_sym )
/*********************************************/
/* Search for a file in a library */
{
    file_list * lib;
    bool        found;
    bool        isimpsym;

    DEBUG(( DBG_OLD, "LibFind( %s, %d )", name, old_sym ));
    isimpsym = FmtData.type & MK_PE &&
                memcmp( name, ImportSymPrefix, PREFIX_LEN ) == 0;
    found = FALSE;
    for( lib = ObjLibFiles; lib != NULL; lib = lib->next_file ) {
        if( lib->file->flags & INSTAT_IOERR ) continue;
        if( old_sym && lib->status & STAT_OLD_LIB ) continue;
        found = ProcLibFile( lib, name );
        if( found ) break;
        if( isimpsym ) {
            found = ProcLibFile( lib, name + PREFIX_LEN );
            if( found ) break;
        }
    }
    return found;
}

bool ModNameCompare( char *tname, char *membname )
/*******************************************************/
// check if a THEADR record name is equal to a library member name
{
    unsigned    lentheadr;
    unsigned    lenmember;
    char        *namestart;

    namestart = RemovePath( tname, &lentheadr );
    lenmember = strlen( membname );
    if( lentheadr == lenmember ) {
        if( memicmp( namestart, membname, lenmember ) == 0 ) {
            return( TRUE );
        }
    }
    return( FALSE );
}
