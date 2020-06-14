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
* Description:  routines for producing codeview information in the load file
*
****************************************************************************/


#include <string.h>
#include "linkstd.h"
#include "exepe.h"
#include "alloc.h"
#include "cv4w.h"
#include "virtmem.h"
#include "objnode.h"
#include "loadfile.h"
#include "objcalc.h"
#include "dbgcomm.h"
#include "loadqnx.h"    // for ToQNXIndex
#include "ring.h"
#include "dbgcv.h"
#include "msg.h"
#include "wlnkmsg.h"
#include "specials.h"

typedef struct cvmodinfo {
    unsigned_32 pubsize;
    virt_mem    segloc;
    unsigned_16 numlines;
    unsigned_16 modidx;
    unsigned_16 numsegs;
} cvmodinfo;

#pragma pack( 1 )

// Cheesy implementation of sstSrcModule structures.
// Only allow one file and one segment.
typedef struct {
    unsigned_32 start;
    unsigned_32 end;
} offset_range;

typedef struct {
    unsigned_16         cFile;
    unsigned_16         cSeg;
    unsigned_32         baseSrcFile[1];
    offset_range        range[1];
    unsigned_16         seg[1];
    unsigned_16         pad;
} cheesy_module_header;

typedef struct {
    unsigned_16         cSeg;
    unsigned_16         pad;
    unsigned_32         baseSrcLn[1];
    offset_range        range[1];
    unsigned_8          name[1];        /* variable sized */
} cheesy_file_table;

typedef struct {
    unsigned_16         Seg;
    unsigned_16         cPair;
//  unsigned_32         offset[];
//  unsigned_32         linenumber[];
} cheesy_mapping_table;

#pragma pack()

// global information needed to keep track of line number information

typedef struct {
    virt_mem            linestart;
    virt_mem            offbase;
    virt_mem            numbase;
    offset_range        range;
    unsigned_32         prevaddr;
    unsigned_16         seg;
    unsigned_8          needsort : 1;
} cvlineinfo;

// split codeview up into a number of different "sections" to keep track
// of where we write bits of the information.

enum {
    CVSECT_MODULE,
    CVSECT_MISC,
    CVSECT_MODDIR,
    CVSECT_DIRECTORY,
    NUM_CV_SECTS
};

// keeps track of where to write bits of information.  Also used in pass 1
// to collect sizes of different blocks.

static virt_mem SectAddrs[NUM_CV_SECTS];

// the codeview information is just one big memory block.  This is the start
// of it, and the length of it

static virt_mem         CVBase;
unsigned_32             CVSize; // external linkage since NT wants to know.
unsigned_32             CVDebugDirEntryPos = 0;

static unsigned         TempIndex;
static cvlineinfo       LineInfo;


void CVInit( void )
/************************/
// called just after command file parsing
{
    DEBUG(( DBG_OLD, "CVInit() enter" ));
    memset( SectAddrs, 0, sizeof( virt_mem ) * NUM_CV_SECTS );
    memset( &LineInfo, 0, sizeof( cvlineinfo ) );
    TempIndex = 0;
}

void CVInitModule( mod_entry *obj )
/****************************************/
// called before pass 1 is done on the module
{
    _PermAlloc( obj->d.cv, sizeof( cvmodinfo ) );
    memset( obj->d.cv, 0, sizeof( cvmodinfo ) );
}

static void DumpInfo( unsigned sect, void *data, unsigned_32 len )
/****************************************************************/
{
    PutInfo( SectAddrs[sect], data, len );
    SectAddrs[sect] += len;
}

static segment GetCVSegment( seg_leader *seg )
/********************************************/
{
    int                 index;
    group_entry *       group;

    if( ( seg == NULL ) || ( seg->group == NULL ) ) {
        return( 0 );
    }
    if( FmtData.type & ( MK_REAL_MODE | MK_FLAT | MK_ID_SPLIT ) ) {
        index = 1;
        for( group = Groups; group != NULL; group = group->next_group ) {
            if( group == seg->group )
                return( index );
            index++;
        }
    } else if( FmtData.type & MK_QNX ) {
        return( ToQNXIndex( seg->seg_addr.seg ) );
    } else {
        return( seg->seg_addr.seg );
    }
    return( 0 );
}

static void AddSubSection( bool ismodule )
/****************************************/
{
    unsigned    sect;

    if( ismodule ) {
        sect = CVSECT_MODDIR;
    } else {
        sect = CVSECT_DIRECTORY;
    }
    SectAddrs[sect] += sizeof( cv_directory_entry );
}

static void GenSubSection( sst sect, unsigned_32 size )
/*****************************************************/
// generate a subsection entry
{
    cv_directory_entry  entry;

    entry.subsection = sect;
    if( CurrMod == NULL ) {
        entry.iMod = 0xFFFF;
    } else {
        entry.iMod = CurrMod->d.cv->modidx;
    }
    entry.cb = size;
    if( sect == sstModule ) {
        entry.lfo = SectAddrs[CVSECT_MODULE] - CVBase;
        DumpInfo( CVSECT_MODDIR, &entry, sizeof( cv_directory_entry ) );
    } else {
        entry.lfo = SectAddrs[CVSECT_MISC] - CVBase;
        DumpInfo( CVSECT_DIRECTORY, &entry, sizeof( cv_directory_entry ) );
    }
}

void CVP1ModuleScanned( void )
/***********************************/
{
}

static void CVAddLines( lineinfo *info )
/**************************************/
// called during pass 1 linnum processing
{
    CurrMod->d.cv->numlines += DBICalcLineQty( info );
}

void CVP1ModuleFinished( mod_entry *obj )
/**********************************************/
// calculate size of the sstModule
{
    byte        namelen;
    unsigned_32 temp;
    unsigned_32 size;

    if( MOD_NOT_DEBUGGABLE( obj ) )
        return;
    TempIndex++;
    CurrMod = obj;
    obj->d.cv->modidx = TempIndex;
    if( CurrMod->modinfo & DBI_LINE ) {
        DBILineWalk( obj->lines, CVAddLines );
    }
    Ring2Walk( obj->publist, DBIModGlobal );
    namelen = strlen( obj->name );
    //  required alignment ???
    size = sizeof( cv_sst_module ) + namelen + 1 - sizeof( cv_seginfo );
    size = ROUND_UP( size, 4 );
    SectAddrs[CVSECT_MODULE] += size;
    //  required alignment ???
    AddSubSection( TRUE );
    if( obj->d.cv->pubsize > 0 ) {
        AddSubSection( FALSE );
        SectAddrs[CVSECT_MISC] += sizeof( unsigned_32 );
        obj->d.cv->pubsize += sizeof( unsigned_32 );
    }
    if( obj->d.cv->numlines > 0 ) {
        AddSubSection( FALSE );
        temp = sizeof( cheesy_module_header );
        temp += ROUND_UP( sizeof( cheesy_file_table ) + namelen, 4 );
        temp += sizeof( cheesy_mapping_table );
        temp += ROUND_UP( obj->d.cv->numlines
                    * ( sizeof( unsigned_32 ) + sizeof( unsigned_16 ) ), 4 );
        SectAddrs[CVSECT_MISC] += temp;
    }
}

void CVAddModule( mod_entry *obj, section *sect )
/******************************************************/
// called just before publics have been assigned addresses between p1 & p2
{
    unsigned_32         sig;
    cv_sst_module       mod;
    unsigned_32         size;
    byte                namelen;

    sect = sect;
	DEBUG(( DBG_OLD, "CVAddModule(%s): numsegs=%d pubsize=%d", obj->name, obj->d.cv->numsegs, obj->d.cv->pubsize ));
    if( obj->d.cv->pubsize > 0 ) {
        GenSubSection( sstPublicSym, obj->d.cv->pubsize );
        sig = 1;
        DumpInfo( CVSECT_MISC, &sig, sizeof( unsigned_32 ) );
    }
    namelen = strlen( obj->name );
    size = sizeof( cv_sst_module ) + namelen + 1
                + ( obj->d.cv->numsegs - 1 ) * sizeof( cv_seginfo );
    //  begin padding required ???
    size = ROUND_UP( size, 4 );
    //  end padding required ???
    GenSubSection( sstModule, size );
    mod.ovlNumber = 0;
    mod.iLib = 0;
    mod.cSeg = obj->d.cv->numsegs;
    mod.Style = CV_DEBUG_STYLE;
    DumpInfo( CVSECT_MODULE, &mod, sizeof( cv_sst_module ) - sizeof( cv_seginfo ) );
    obj->d.cv->segloc = SectAddrs[CVSECT_MODULE];
    SectAddrs[CVSECT_MODULE] += sizeof( cv_seginfo ) * obj->d.cv->numsegs;
    DumpInfo( CVSECT_MODULE, &namelen, 1 );
    DumpInfo( CVSECT_MODULE, obj->name, namelen );
	DEBUG(( DBG_OLD, "CVAddModule(%s): exit", obj->name ));
}

static int RelocCompare( virt_mem a, virt_mem b )
/***********************************************/
{
    unsigned_32 a32;
    unsigned_32 b32;

    GET32INFO( a, a32 );
    GET32INFO( b, b32 );
    return( (signed_32)a32 - b32 );
}

static void SwapRelocs( virt_mem a, virt_mem b )
/**********************************************/
{
    unsigned    diffa;
    unsigned    diffb;
    unsigned_16 a16;
    unsigned_16 b16;
    unsigned_32 a32;
    unsigned_32 b32;

    GET32INFO( a, a32 );
    GET32INFO( b, b32 );
    PUT32INFO( a, b32 );
    PUT32INFO( b, a32 );
    diffa = ( a - LineInfo.offbase ) / sizeof( unsigned_32 );
    diffa *= sizeof( unsigned_16 );
    diffa += LineInfo.numbase;
    diffb = ( b - LineInfo.offbase ) / sizeof( unsigned_32 );
    diffb *= sizeof( unsigned_16 );
    diffb += LineInfo.numbase;
    GET16INFO( diffa, a16 );
    GET16INFO( diffb, b16 );
    PUT16INFO( diffa, b16 );
    PUT16INFO( diffb, a16 );
}

static void SortRelocs( void )
/****************************/
{
    LineInfo.offbase -= CurrMod->d.cv->numlines * sizeof( unsigned_32 );
    LineInfo.numbase -= CurrMod->d.cv->numlines * sizeof( unsigned_16 );
    VMemQSort( LineInfo.offbase, CurrMod->d.cv->numlines, sizeof( unsigned_32 ),
                SwapRelocs, RelocCompare );
}

static void GenSrcModHeader( void )
/*********************************/
// emit header for line number information now that we know where everything
// is.
{
    cheesy_module_header        mod_hdr;
    cheesy_file_table           file_tbl;
    cheesy_mapping_table        map_tbl;
    unsigned                    adjust;
    unsigned_32                 buff;

    if( LineInfo.linestart == 0 )
        return;
    memset( &mod_hdr, 0, sizeof( mod_hdr ) );
    mod_hdr.cFile = 1;
    mod_hdr.cSeg = 1;
    mod_hdr.range[0] = LineInfo.range;
    mod_hdr.baseSrcFile[0] = sizeof( mod_hdr );
    mod_hdr.seg[0] = LineInfo.seg;
    mod_hdr.pad = 0;
    PutInfo( LineInfo.linestart, &mod_hdr, sizeof( mod_hdr ) );
    LineInfo.linestart += sizeof( mod_hdr );
    file_tbl.cSeg = 1;
    file_tbl.pad = 0;
    file_tbl.range[0] = LineInfo.range;
    file_tbl.name[0] = strlen( CurrMod->name );
    file_tbl.baseSrcLn[0] = sizeof( mod_hdr ) +
                    ROUND_UP( sizeof( file_tbl ) + file_tbl.name[0], 4 );
    PutInfo( LineInfo.linestart, &file_tbl, sizeof( file_tbl ) );
    LineInfo.linestart += sizeof( file_tbl );
    PutInfo( LineInfo.linestart, CurrMod->name, file_tbl.name[0] );
    LineInfo.linestart += file_tbl.name[0];
    adjust = file_tbl.baseSrcLn[0] - sizeof( mod_hdr ) - sizeof( file_tbl )
                - file_tbl.name[0];
    if( adjust != 0 ) {
        buff = 0;
        PutInfo( LineInfo.linestart, &buff, adjust );
        LineInfo.linestart += adjust;
    }
    map_tbl.Seg = mod_hdr.seg[0];
    map_tbl.cPair = CurrMod->d.cv->numlines;
    PutInfo( LineInfo.linestart, &map_tbl, sizeof( map_tbl ) );
    memset( &LineInfo, 0, sizeof( LineInfo ) );
}

void CVGenModule( void )
/*****************************/
// generate an sstSrcModule
{
    if( LineInfo.needsort ) {
        SortRelocs();
    }
    GenSrcModHeader();
}

void CVAddLocal( seg_leader *seg, offset length )
/*******************************************************/
// called during pass 1 final segment processing.
{
    if( IS_DBG_INFO( seg ) ) {
        //if( length > 0xFFFF ) {
        //    LnkMsg( WRN+LOC+MSG_DEBUG_TOO_LARGE, "s", "Codeview" );
        //} else {
            AddSubSection( FALSE );
        //}
    }
}

void CVAddGlobal( symbol *sym )
/************************************/
// called during pass 1 symbol definition
{
    unsigned    size;

    if( !( sym->info & SYM_STATIC ) ) {
        if( ( sym->p.seg == NULL )
            || IS_SYM_IMPORTED( sym )
            || sym->p.seg->is32bit ) {
            size = sizeof( s_pub32 );
        } else {
            size = sizeof( s_pub16 );
        }
        size = ROUND_UP( size + strlen( sym->name ) + 1, 4 );
        CurrMod->d.cv->pubsize += size;
        SectAddrs[CVSECT_MISC] += size;
    }
}

void CVGenGlobal( symbol * sym, section *sect )
/****************************************************/
// called during symbol address calculation (between pass 1 & pass 2)
// also called by loadpe between passes
{
    s_pub16     pub16;
    s_pub32     pub32;
    unsigned    size;
    unsigned    pad;
    unsigned_32 buf;
    byte        namelen;

    sect = sect;
    if( sym->info & SYM_STATIC )
        return;
    namelen = strlen( sym->name );
    size = namelen + 1;

    if( ( sym->p.seg == NULL )
        || IS_SYM_IMPORTED( sym )
        || sym->p.seg->is32bit ) {
        size += sizeof( s_pub32 );
        pub32.common.length = ROUND_UP( size, 4 );
        pad = pub32.common.length - size;
        pub32.common.length -= 2;
        pub32.common.code = S_PUB32;
        pub32.f.offset = sym->addr.off;
        pub32.f.segment = GetCVSegment( sym->p.seg->u.leader );
        pub32.f.type = 0;
        DumpInfo( CVSECT_MISC, &pub32, sizeof( s_pub32 ) );
    } else {
        size += sizeof( s_pub16 );
        pub16.common.length = ROUND_UP( size, 4 );
        pad = pub16.common.length - size;
        pub16.common.length -= 2;
        pub16.common.code = S_PUB16;
        pub16.f.offset = sym->addr.off;
        pub16.f.segment = GetCVSegment( sym->p.seg->u.leader );
        pub16.f.type = 0;
        DumpInfo( CVSECT_MISC, &pub16, sizeof( s_pub16 ) );
    }
    DumpInfo( CVSECT_MISC, &namelen, 1 );
    DumpInfo( CVSECT_MISC, sym->name, namelen );
    if( pad > 0 ) {
        buf = 0;
        DumpInfo( CVSECT_MISC, &buf, pad );
    }
}

void CVGenLines( lineinfo *info )
/*******************************/
// called during pass 2 linnum processing
{
    ln_off_pair UNALIGN *pair;
    unsigned_32         temp_off;
    unsigned_16         temp_num;
    offset              adjust;
    unsigned long       cvsize;
    unsigned            size;
    segdata             *seg;

    seg = info->seg;
    size = info->size & ~LINE_IS_32BIT;

    if( !( CurrMod->modinfo & DBI_LINE ) )
        return;
    adjust = seg->a.delta + seg->u.leader->seg_addr.off;
    if( LineInfo.offbase == 0 ) { // this is our first time through.
        LineInfo.seg = GetCVSegment( seg->u.leader );
        LineInfo.linestart = SectAddrs[CVSECT_MISC];
        cvsize = sizeof( cheesy_module_header ) + sizeof( cheesy_mapping_table )
            + ROUND_UP( sizeof( cheesy_file_table ) + strlen( CurrMod->name ), 4 );
        LineInfo.offbase = SectAddrs[CVSECT_MISC] + cvsize;
        LineInfo.numbase = LineInfo.offbase
                                + CurrMod->d.cv->numlines * sizeof( unsigned_32 );
        cvsize += CurrMod->d.cv->numlines * sizeof( unsigned_32 );
        cvsize += ROUND_UP( CurrMod->d.cv->numlines * sizeof( unsigned_16 ), 4 );
        GenSubSection( sstSrcModule, cvsize );
        SectAddrs[CVSECT_MISC] += cvsize;
        LineInfo.range.start = adjust;
        LineInfo.range.end = adjust + seg->length;
    } else {
        if( adjust < LineInfo.range.start ) {
            LineInfo.range.start = adjust;
        }
        if( adjust + seg->length > LineInfo.range.end ) {
            LineInfo.range.end = adjust + seg->length;
        }
    }
    pair = (ln_off_pair *)info->data;
    if( info->size & LINE_IS_32BIT ) {
        while( size > 0 ) {
            pair->_386.off += adjust;
            if( pair->_386.off < LineInfo.prevaddr ) {
                LineInfo.needsort = TRUE;
            }
            LineInfo.prevaddr = pair->_386.off;
            _HostU32toTarg( pair->_386.off, temp_off );
            _HostU16toTarg( pair->_386.linnum, temp_num );
            PutInfo( LineInfo.offbase, &temp_off, sizeof( unsigned_32 ) );
            PutInfo( LineInfo.numbase, &temp_num, sizeof( unsigned_16 ) );
            LineInfo.offbase += sizeof( unsigned_32 );
            LineInfo.numbase += sizeof( unsigned_16 );
            pair = (void *)( (char *)pair + sizeof( ln_off_386 ) );
            size -= sizeof( ln_off_386 );
        }
    } else {
        while( size > 0 ) {
            pair->_286.off += adjust;
            if( pair->_286.off < LineInfo.prevaddr ) {
                LineInfo.needsort = TRUE;
            }
            LineInfo.prevaddr = pair->_286.off;
            _HostU16toTarg( pair->_286.off, temp_off );
            _HostU16toTarg( pair->_286.linnum, temp_num );
            PutInfo( LineInfo.offbase, &temp_off, sizeof( unsigned_32 ) );
            PutInfo( LineInfo.numbase, &temp_num, sizeof( unsigned_16 ) );
            LineInfo.offbase += sizeof( unsigned_32 );
            LineInfo.numbase += sizeof( unsigned_16 );
            pair = (void *)( (char *)pair + sizeof( ln_off_286 ) );
            size -= sizeof( ln_off_286 );
        }
    }
}

static void CVAddAddrInit( segdata *sdata, void *cookie )
/*******************************************************/
{
    sdata = sdata;
    cookie = cookie;
}

static void CVAddAddrAdd( segdata *sdata, offset delta, offset size,
                          void *cookie, bool isnewmod )
/******************************************************************/
{
	DEBUG(( DBG_OLD, "CVAddAddrAdd(%s): numsegs=%d", sdata->u.leader->segname, sdata->o.mod->d.cv->numsegs ));
    delta = delta;
    size = size;
    cookie = cookie;
    if( !isnewmod )
        return;
    sdata->o.mod->d.cv->numsegs++;
    SectAddrs[CVSECT_MODULE] += sizeof( cv_seginfo );
}

void CVAddAddrInfo( seg_leader *seg )
/******************************************/
{
    DEBUG(( DBG_OLD, "CVAddAddrInfo(%s) enter, info=%x", seg->segname, seg->info ));
    if( !( seg->info & SEG_CODE ) )
        return;
    DBIAddrInfoScan( seg, CVAddAddrInit, CVAddAddrAdd, NULL );
}

static void CVGenAddrInit( segdata *sdata, void *_info )
/******************************************************/
{
    cv_seginfo *info = _info;
    info->Seg = GetCVSegment( sdata->u.leader );
    info->pad = 0;
    info->offset = sdata->u.leader->seg_addr.off + sdata->a.delta;
}

static void CVGenAddrAdd( segdata *sdata, offset delta, offset size,
                          void *_info, bool isnewmod )
/******************************************************************/
{
    cv_seginfo *info = _info;
    if( !isnewmod )
        return;
    info->cbSeg = size;
    PutInfo( sdata->o.mod->d.cv->segloc, info, sizeof( cv_seginfo ) );
    sdata->o.mod->d.cv->segloc += sizeof( cv_seginfo );
    info->offset = sdata->u.leader->seg_addr.off + delta;
}

void CVGenAddrInfo( seg_leader *seg )
/******************************************/
{
    cv_seginfo          info;

    if( !( seg->info & SEG_CODE ) )
        return;
    DBIAddrInfoScan( seg, CVGenAddrInit, CVGenAddrAdd, &info );
}

void CVDefClass( class_entry *class, unsigned_32 size )
/************************************************************/
{
    group_entry *group;

    if( ( class->flags & CLASS_DEBUG_INFO ) == CLASS_DWARF )
        return;
    SectAddrs[CVSECT_MISC] += size;
    group = AllocGroup( AutoGrpName, &DBIGroups );
    group->g.class = class;
    group->grp_addr.seg = 0;
}

static void DefLocal( void *_sdata )
/**********************************/
/* NOTE: this assumes that codeview segments are byte aligned! */
{
    segdata    *sdata = _sdata;
    seg_leader *leader;
    sst         sect;

    leader = sdata->u.leader;
    if( !sdata->isdead && IS_DBG_INFO( leader ) ) {
        if( leader->dbgtype == MS_TYPE ) {
            sect = sstTypes;
        } else {
            sect = sstSymbols;
        }
        CurrMod = sdata->o.mod;
        GenSubSection( sect, sdata->length );
        CopyInfo( SectAddrs[CVSECT_MISC], sdata->data, sdata->length );
        sdata->data = SectAddrs[CVSECT_MISC];   // FIXME: inefficient
        SectAddrs[CVSECT_MISC] += sdata->length;
    }
}

static bool DefLeader( void *_leader, void *group )
/*************************************************/
{
    seg_leader *leader = _leader;

    leader->group = group;
    RingWalk( leader->pieces, DefLocal );
    return( FALSE );
}

void CVAddrStart( void )
/*****************************/
{
    cv_subsection_directory dir;
    int         index;
    cv_trailer  start;
    virt_mem    currpos;
    unsigned_32 size;
    unsigned_32 numentries;
    group_entry *group;
    int         myNumGroups;
    group_entry *currgrp;

    /* Numgroups is expected to contain the FINAL number of groups here!
     * However, this is not quite correct, since groups with totalsize==0
     * are NOT contained in NumGroups (see objcalc.c).
     * To avoid problems, we just count groups again and IGNORE Numgroups
     */
    for( myNumGroups = 0, currgrp = Groups; currgrp != NULL; currgrp = currgrp->next_group, myNumGroups++ );
    DEBUG(( DBG_OLD, "CVAddrStart() enter, NumGroups=%d, real # of groups=%d", NumGroups, myNumGroups ));

    AddSubSection( FALSE );     // for sstSegMap
    numentries = ( SectAddrs[CVSECT_MODDIR] + SectAddrs[CVSECT_DIRECTORY] )
                        / sizeof( cv_directory_entry );
    SectAddrs[CVSECT_MISC] += sizeof( cv_sst_seg_map )
    //                        + ( NumGroups - 1 ) * sizeof( seg_desc );
                            + ( myNumGroups - 1 ) * sizeof( seg_desc );
    SectAddrs[CVSECT_MODDIR] += sizeof( cv_subsection_directory );
    CVSize = 2 * sizeof( cv_trailer );
    for( index = 0; index < NUM_CV_SECTS; index++ ) {
        CVSize += SectAddrs[index];
        DEBUG(( DBG_OLD, "CVAddrStart(): CVSize=%h", CVSize ));
    }
    CVBase = DBIAlloc( CVSize );
    currpos = CVBase + sizeof( cv_trailer );
    for( index = 0; index < NUM_CV_SECTS; index++ ) {
        DEBUG(( DBG_OLD, "CVAddrStart(): currpos=%h", currpos ));
        size = SectAddrs[index];
        SectAddrs[index] = currpos;
        currpos += size;
    }
    memcpy( start.sig, CV4_NB05, CV_SIG_SIZE );
    start.offset = SectAddrs[CVSECT_MODDIR] - CVBase;
    PutInfo( CVBase, &start, sizeof( cv_trailer ) );
    start.offset = CVSize;
    PutInfo( CVBase + CVSize - sizeof( cv_trailer ), &start, sizeof( cv_trailer ) );
    dir.cbDirHeader = sizeof( cv_subsection_directory );
    dir.cbDirEntry = sizeof( cv_directory_entry );
    dir.cDir = numentries;
    dir.lfoNextDir = 0;
    dir.flags = 0;
    DumpInfo( CVSECT_MODDIR, &dir, sizeof( cv_subsection_directory ) );
    for( group = DBIGroups; group != NULL; group = group->next_group ) {
        RingLookup( group->g.class->segs, DefLeader, group );
        group->g.class = NULL;
    }
}

void CVFini( section *sect )
/*********************************/
// called after pass 2 is finished, but before load file generation
{
    cv_sst_seg_map      map;
    seg_desc            desc;
    group_entry *       group;
    seg_leader *        leader;
    int                 myNumGroups;
    group_entry         *currgrp;

    DEBUG(( DBG_OLD, "CVFini() enter" ));
    if( sect != Root )
        return;
    CurrMod = NULL;

    for( myNumGroups = 0, currgrp = Groups; currgrp != NULL; currgrp = currgrp->next_group, myNumGroups++ );

    GenSubSection( sstSegMap, sizeof( cv_sst_seg_map )
                            + ( myNumGroups - 1 ) * sizeof( seg_desc ) );
    map.cSeg = myNumGroups;
    map.cSegLog = myNumGroups;
    DumpInfo( CVSECT_MISC, &map, sizeof( cv_sst_seg_map ) - sizeof( seg_desc ) );
    memset( &desc, 0, sizeof( seg_desc ) );
    desc.u.b.fSel = !( LinkFlags & MK_REAL_MODE );
    desc.u.b.fRead = TRUE;
    desc.iSegName = 0xFFFF;
    desc.iClassName = 0xFFFF;
    for( group = Groups; group != NULL; group = group->next_group ) {
        DEBUG(( DBG_OLD, "CVFini(): dump secdesc for grp=%s, totalsize=%h", group->sym->name, group->totalsize ));
#if 0 /* jwlink */
        /* groups with size 0 cannot be ignored - they might still contain a label */
        if ( !group->totalsize )
            continue;
#endif
        desc.frame = group->grp_addr.seg;
        desc.offset = group->grp_addr.off;
        desc.cbseg = group->totalsize;
        if( !( group->segflags & SEG_DATA ) ) {
            desc.u.b.fExecute = 1;
        } else {
            desc.u.b.fExecute = 0;
        }
        if( ( group->segflags & ( SEG_DATA | SEG_READ_ONLY ) ) == SEG_DATA ) {
            desc.u.b.fWrite = 1;
        } else {
            desc.u.b.fWrite = 0;
        }
        leader = Ring2First( group->leaders );
        if( leader->info & USE_32 ) {
            desc.u.b.f32Bit = 1;
        }
        desc.u.b.fSel = 1;
        DumpInfo( CVSECT_MISC, &desc, sizeof( seg_desc ) );
    }
}

void CVWriteDebugTypeMisc( const char *filename )
/***********************************************/
// called during load file generation.  It is assumed that the loadfile is
// positioned to the right spot.
{
    unsigned                namelen;
    unsigned                bufspace;
    debug_misc_dbgdata      dbg_exename;

    DEBUG(( DBG_OLD, "CVWriteDebugTypeMisc() enter" ));
    memset( &dbg_exename, 0, sizeof( dbg_exename ) );
    dbg_exename.data_type = 1;
    dbg_exename.length = sizeof( dbg_exename );
    dbg_exename.unicode = 0;

    if( filename ) {
        namelen = strlen( filename ) + 1;
        bufspace = sizeof( dbg_exename.data ) - 4;
        if( bufspace >= namelen ) {
            memcpy( dbg_exename.data, filename, namelen );
        } else {
            LnkMsg( WRN+LOC+MSG_INTERNAL, "s", "filename too long in CVWriteDebugTypeMisc()" );
        }
    }
    dbg_exename.special_purpose = CVDebugDirEntryPos;
    WriteLoad( &dbg_exename, sizeof( dbg_exename ) );
}

void CVWrite( void )
/****************************/
// called during load file generation.  It is assumed that the loadfile is
// positioned to the right spot.
{
    // write DEBUG_TYPE_CODEVIEW data: CodeView NB05 data
    WriteInfo( CVBase, CVSize );
}
