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
* Description:  Routines for producing DWARF debugging information.
*
****************************************************************************/


#include <string.h>
#include <setjmp.h>
#include "walloca.h"
#include "linkstd.h"
#include "alloc.h"
#include "objcalc.h"
#include "ring.h"
#include "exeelf.h"
#include "strtab.h"
#include "tistrail.h"
#include "dwarf.h"
#include "dw.h"
#include "dwutils.h"
#include "loadfile.h"
#include "virtmem.h"
#include "objnode.h"
#include "dbgcomm.h"
#include "dbgdwarf.h"
#include "fileio.h"
#include "loadelf.h"
#include "specials.h"

static class_entry *    DBIClass;       // Assume there is only one!

typedef struct {
    virt_mem    addr;
    unsigned_32 size;
} dwarfsect;

typedef struct dwarfmodinfo {
    dwarfsect   pubsym;
    dwarfsect   dasi;
    dwarfsect   arange;
} dwarfmodinfo;

// this used to hold information about linker generated debugging information.

typedef struct {
    virt_mem    addr;   // virt. mem. block containing genned info.
    unsigned_32 size;   // size of linker generated info.
    unsigned_32 start;  // start of linker generated info - compiler stuff 1st
    char *      name;
} sectioninfo;

static sectioninfo SectionTable[] = {
    { 0, 0, 0, ".debug_info" },
    { 0, 0, 0, ".debug_abbrev" },
    { 0, 0, 0, ".debug_line" },
    { 0, 0, 0, ".debug_aranges" }
};

enum {
    SECT_DEBUG_INFO,    // assumed to be in the same order as segment flags!
    SECT_DEBUG_ABBREV,
    SECT_DEBUG_LINE,
    SECT_DEBUG_ARANGE,
    SECT_NUM_SECTIONS
};

#pragma pack( 1 )

// die information

typedef struct {
    unsigned_8  abbrev_code;    // the abbrev code used.
//  unsigned_32 stmt_list;      // (optional) offset to the line number info
//    char      name[1];        // name of the module
} compunit_die;

typedef struct {
    unsigned_8  abbrev_code;
    offset      off;
    unsigned_8  isexternal;
//    char      name[1];
} symbol_die;

typedef struct {
    unsigned_8  len;
    unsigned_8  loc_op;
    unsigned_16 seg;
} symbol_seg;

// the abbrev codes used in the linker's autogenned stuff.


#define NULL_ABBREV_CODE        0
#define COMPUNIT_ABBREV_CODE    1
#define CU_NOLINE_ABBREV_CODE   2
#define LABEL_ABBREV_CODE       3
#define VARIABLE_ABBREV_CODE    4
#define LAST_ABBREV_CODE        4

#if LAST_ABBREV_CODE != LAST_LINKER_ABBREV
#error "make sure dwarf.h is updated and dwarf writing library recompiled!"
#endif

static char SegmentedStandardAbbrevs[] = {
    COMPUNIT_ABBREV_CODE,
    DW_TAG_compile_unit,
    DW_CHILDREN_yes,
    DW_AT_stmt_list,    DW_FORM_ref_addr,
    DW_AT_name,         DW_FORM_string,
    0,                  0,
    CU_NOLINE_ABBREV_CODE,
    DW_TAG_compile_unit,
    DW_CHILDREN_yes,
    DW_AT_name,         DW_FORM_string,
    0,                  0,
    LABEL_ABBREV_CODE,
    DW_TAG_label,
    DW_CHILDREN_no,
    DW_AT_low_pc,       DW_FORM_addr,
    DW_AT_external,     DW_FORM_flag,
    DW_AT_segment,      DW_FORM_block1,
    DW_AT_name,         DW_FORM_string,
    0,                  0,
    VARIABLE_ABBREV_CODE,
    DW_TAG_variable,
    DW_CHILDREN_no,
    DW_AT_low_pc,       DW_FORM_addr,
    DW_AT_external,     DW_FORM_flag,
    DW_AT_segment,      DW_FORM_block1,
    DW_AT_name,         DW_FORM_string,
    0,                  0
};

static char FlatStandardAbbrevs[] = {
    COMPUNIT_ABBREV_CODE,
    DW_TAG_compile_unit,
    DW_CHILDREN_yes,
    DW_AT_stmt_list,    DW_FORM_ref_addr,
    DW_AT_name,         DW_FORM_string,
    0,                  0,
    CU_NOLINE_ABBREV_CODE,
    DW_TAG_compile_unit,
    DW_CHILDREN_yes,
    DW_AT_name,         DW_FORM_string,
    0,                  0,
    LABEL_ABBREV_CODE,
    DW_TAG_label,
    DW_CHILDREN_no,
    DW_AT_low_pc,       DW_FORM_addr,
    DW_AT_external,     DW_FORM_flag,
    DW_AT_name,         DW_FORM_string,
    0,                  0,
    VARIABLE_ABBREV_CODE,
    DW_TAG_variable,
    DW_CHILDREN_no,
    DW_AT_low_pc,       DW_FORM_addr,
    DW_AT_external,     DW_FORM_flag,
    DW_AT_name,         DW_FORM_string,
    0,                  0
};
#pragma pack()


void DwarfInit( void )
/***************************/
{
    int index;

    DBIClass = NULL;
    for( index = 0; index < SECT_NUM_SECTIONS; index++ ) {
        SectionTable[index].addr = 0;
        SectionTable[index].size = 0;
        SectionTable[index].start = 0;
    }
}

void DwarfInitModule( mod_entry *mod )
/*******************************************/
{
    _PermAlloc( mod->d.d, sizeof( dwarfmodinfo ) );
    memset( mod->d.d, 0, sizeof( dwarfmodinfo ) );
}

void DwarfP1ModuleScanned( void )
/**************************************/
{
}

static int GetStmtHeaderSize( mod_entry *mod )
/********************************************/
{
    int         len;

    len = sizeof( stmt_prologue );  // fixed size
    len += 1 + strlen( mod->name ) + 1 + 3 + 1;
    return( len );
}

static void DwarfAddLines( lineinfo *info )
/*****************************************/
// calculate the amount of space needed to hold all of the line # info.
{
    ln_off_pair UNALIGN *lineptr;
    unsigned_32         dwsize;
    uint_8              buff[ 3 + 2 * MAX_LEB128 ];
    dw_linenum_delta    linedelta;
    dw_addr_delta       addrdelta;
    ln_off_386          prevline;
    unsigned            size;

    prevline.off = 0;
    prevline.linnum = 1;
    if( FmtData.type & MK_286 ) {
        dwsize = 13;    // start and end sequence with 16-bit offset
    } else {
        dwsize = 15;    // start and end sequence with 32-bit offset
    }
    if( !( FmtData.type & MK_SEGMENTED ) ) {
        dwsize -= 5;    // size of LNE_segment burst
    }
    lineptr = (ln_off_pair *)info->data;
    size = info->size & ~LINE_IS_32BIT;
    while( size > 0 ) {
        if( info->size & LINE_IS_32BIT ) {
            linedelta = (signed_32) lineptr->_386.linnum - prevline.linnum;
            addrdelta = lineptr->_386.off - prevline.off;
            lineptr++;
            size -= sizeof( ln_off_386 );
        } else {
            linedelta = (signed_32) lineptr->_286.linnum - prevline.linnum;
            addrdelta = lineptr->_286.off - prevline.off;
            lineptr = (void *)( (char *)lineptr + sizeof( ln_off_286 ) );
            size -= sizeof( ln_off_286 );
        }
        prevline.linnum += linedelta;
        prevline.off += addrdelta;
        dwsize += DWLineGen( linedelta, addrdelta, buff );
    }
    CurrMod->d.d->dasi.size += dwsize;
}

void DwarfP1ModuleFinished( mod_entry *mod )
/*************************************************/
{
    if( MOD_NOT_DEBUGGABLE( mod ) )
        return;
    CurrMod = mod;
    if( mod->modinfo & MOD_DBI_SEEN )
        return;
    DBILineWalk( mod->lines, DwarfAddLines );
    Ring2Walk( mod->publist, DBIModGlobal );
    if( FmtData.type & MK_SEGMENTED ) {
        SectionTable[SECT_DEBUG_ABBREV].size = sizeof( SegmentedStandardAbbrevs );
    } else {
        SectionTable[SECT_DEBUG_ABBREV].size = sizeof( FlatStandardAbbrevs );
    }
    if( mod->d.d->dasi.size > 0 ) {
        mod->d.d->dasi.addr = SectionTable[SECT_DEBUG_LINE].size;
        mod->d.d->dasi.size += GetStmtHeaderSize( mod );
        SectionTable[SECT_DEBUG_LINE].size += mod->d.d->dasi.size;
        mod->d.d->pubsym.size += sizeof( unsigned_32 ); // DW_AT_STMT_LIST
    }
    mod->d.d->pubsym.addr = SectionTable[SECT_DEBUG_INFO].size;
    mod->d.d->pubsym.size += strlen( mod->name ) + sizeof( compunit_die ) + 1
                             + COMPILE_UNIT_HDR_SIZE + 1;
    SectionTable[SECT_DEBUG_INFO].size += mod->d.d->pubsym.size;
}

void DwarfStoreAddrInfo( mod_entry *mod )
/**********************************************/
{
    if( !( mod->modinfo & MOD_DBI_SEEN ) ) {
        if( mod->d.d->arange.size > 0 ) {
            mod->d.d->arange.addr = SectionTable[SECT_DEBUG_ARANGE].size;
            mod->d.d->arange.size += sizeof( arange_prologue );
            if( FmtData.type & MK_SEGMENTED ) {
                mod->d.d->arange.size += sizeof( segmented_arange_tuple );
            } else {
                mod->d.d->arange.size += sizeof( flat_arange_tuple );
            }
            SectionTable[SECT_DEBUG_ARANGE].size += mod->d.d->arange.size;
        }
    }
}

void DwarfAddModule( mod_entry *mod, section *sect )
/**************************************************/
// this generates the headers needed in the individual sections
{
    arange_prologue     arange_hdr;
    stmt_prologue       stmt_hdr;
    compuhdr_prologue   compuhdr;
    compunit_die        die;
    size_t              namelen;
    char *              buff;
    unsigned_32         stmt_list;
    int                 zero;

    sect = sect;
    if( !( mod->modinfo & MOD_DBI_SEEN ) ) {
        if( mod->d.d->arange.size > 0 ) {
            mod->d.d->arange.addr += SectionTable[SECT_DEBUG_ARANGE].addr;
            arange_hdr.length = mod->d.d->arange.size - sizeof( unsigned_32 );
            arange_hdr.version = 2;
            arange_hdr.debug_offset = mod->d.d->pubsym.addr
                                      + SectionTable[SECT_DEBUG_INFO].start;
            arange_hdr.offset_size = sizeof( offset );
            if( FmtData.type & MK_SEGMENTED ) {
                arange_hdr.segment_size = sizeof( segment );
            } else {
                arange_hdr.segment_size = 0;
            }
//          memset( arange_hdr.padding, 0, sizeof( arange_hdr.padding ) );
            PutInfo( mod->d.d->arange.addr, (void *)&arange_hdr,
                     sizeof( arange_prologue ) );
            mod->d.d->arange.addr += sizeof( arange_prologue );
        }
        mod->d.d->pubsym.addr += SectionTable[SECT_DEBUG_INFO].addr;
        compuhdr.length = mod->d.d->pubsym.size - sizeof( unsigned_32 );
        compuhdr.version = 2;
        compuhdr.abbrev_offset = SectionTable[SECT_DEBUG_ABBREV].start;
        compuhdr.addr_size = sizeof( offset );
        PutInfo( mod->d.d->pubsym.addr, (void *) &compuhdr,
                                        sizeof( compuhdr_prologue ) );
        mod->d.d->pubsym.addr += sizeof( compuhdr_prologue );
        if( mod->d.d->dasi.size > 0 ) {
            die.abbrev_code = COMPUNIT_ABBREV_CODE;
        } else {
            die.abbrev_code = CU_NOLINE_ABBREV_CODE;
        }
        PutInfo( mod->d.d->pubsym.addr, &die, sizeof( compunit_die ) );
        mod->d.d->pubsym.addr += sizeof( compunit_die );
        if( mod->d.d->dasi.size > 0 ) {
            stmt_list = mod->d.d->dasi.addr
                            + SectionTable[SECT_DEBUG_LINE].start;
            PutInfo( mod->d.d->pubsym.addr, &stmt_list, sizeof( unsigned_32 ) );
            mod->d.d->pubsym.addr += sizeof( unsigned_32 );
        }
        namelen = strlen( mod->name ) + 1;
        PutInfo( mod->d.d->pubsym.addr, mod->name, namelen );
        mod->d.d->pubsym.addr += namelen;
        if( mod->d.d->dasi.size > 0 ) {
            mod->d.d->dasi.addr += SectionTable[SECT_DEBUG_LINE].addr;
            stmt_hdr.total_length = mod->d.d->dasi.size - sizeof( unsigned_32 );
            stmt_hdr.version = 2;
            stmt_hdr.prologue_length = GetStmtHeaderSize( mod )
                        - offsetof( stmt_prologue, minimum_instruction_length );
            stmt_hdr.minimum_instruction_length = DW_MIN_INSTR_LENGTH;
            stmt_hdr.default_is_stmt = 1;
            stmt_hdr.line_base = DWLINE_BASE;
            stmt_hdr.line_range = DWLINE_RANGE;
            stmt_hdr.opcode_base = DWLINE_OPCODE_BASE;
            stmt_hdr.standard_opcode_lengths[0] = 0;
            stmt_hdr.standard_opcode_lengths[1] = 1;
            stmt_hdr.standard_opcode_lengths[2] = 1;
            stmt_hdr.standard_opcode_lengths[3] = 1;
            stmt_hdr.standard_opcode_lengths[4] = 1;
            stmt_hdr.standard_opcode_lengths[5] = 0;
            stmt_hdr.standard_opcode_lengths[6] = 0;
            stmt_hdr.standard_opcode_lengths[7] = 0;
            stmt_hdr.standard_opcode_lengths[8] = 0;
            PutInfo( mod->d.d->dasi.addr, (void *) &stmt_hdr, sizeof( stmt_prologue ) );
            mod->d.d->dasi.addr += sizeof( stmt_prologue );
            zero = 0;                       // no include directories;
            PutInfo( mod->d.d->dasi.addr, &zero, 1 );
            mod->d.d->dasi.addr += 1;
            buff = alloca( namelen + 3 );
            memcpy( &buff[0], mod->name, namelen );
            buff[namelen + 0] = 0;          // no directory index
            buff[namelen + 1] = 0;          // no time
            buff[namelen + 2] = 0;          // no length
            PutInfo( mod->d.d->dasi.addr, buff, namelen + 3 );
            mod->d.d->dasi.addr += namelen + 3;
            zero = 0;                       // no more file names
            PutInfo( mod->d.d->dasi.addr, &zero, 1 );
            mod->d.d->dasi.addr += 1;
        }
    }
}

void DwarfGenModule( void )
/********************************/
// write out the addr info, and write out the null die at the end of .debug_info
{
    arange_tuple        tuple;
    unsigned            size;
    unsigned_8          nulldie;

    if( CurrMod->modinfo & MOD_DBI_SEEN )
        return;
    if( CurrMod->d.d->arange.size > 0 ) {       // write out terminator arange
        if( FmtData.type & MK_SEGMENTED ) {
            size = sizeof( segmented_arange_tuple );
        } else {
            size = sizeof( flat_arange_tuple );
        }
        memset( &tuple, 0, size );
        PutInfo( CurrMod->d.d->arange.addr, &tuple, size );
    }
    nulldie = 0;
    PutInfo( CurrMod->d.d->pubsym.addr, &nulldie, sizeof( unsigned_8 ) );
}

static void DefAClass( void *_seg )
/*********************************/
{
    seg_leader  *seg = _seg;
    int         index;

    seg->group = AllocGroup( AutoGrpName, &DBIGroups );
    seg->group->grp_addr.seg = 0;
    if( SectionTable[SECT_DEBUG_INFO].size > 0 ) {
        for( index = 0; index < SECT_NUM_SECTIONS; index++ ) {
            if( stricmp( seg->segname, SectionTable[index].name ) == 0 ) {
                seg->dbgtype = index + DWARF_DEBUG_INFO;
                SectionTable[index].start = seg->size;
                break;
            }
        }
    }
}

void DwarfDefClass( class_entry *cl, unsigned_32 size )
/************************************************************/
// go through the list of dwarf segments, and make sure VM is allocated for
// all of them.
{
    size = size;        // to avoid a warning
    if( ( cl->flags & CLASS_DEBUG_INFO ) != CLASS_DWARF )
        return;
    DBIClass = cl;
    RingWalk( cl->segs, DefAClass );
}

void DwarfAddGlobal( symbol *sym )
/***************************************/
{
    sym = sym;
    CurrMod->d.d->pubsym.size += strlen( sym->name ) + sizeof( symbol_die ) + 1;
    if( FmtData.type & MK_SEGMENTED ) {
        CurrMod->d.d->pubsym.size += sizeof( symbol_seg );
    }
}

static offset GetLinearGroupOffset( group_entry *group )
/************************************************************/
{
    /* gives the absolute offset for ELF, relative for PE and QNX_FLAT;
       perhaps PE and QNX_FLAT should be absolute as well if the
       DWARF format is changed */

    offset      off;

    if( group != NULL ) {
        off = group->linear - group->grp_addr.off;
        if( FmtData.type & MK_ELF ) {
            off += FmtData.base;
        } else {
            off -= Groups->linear;
        }
    }
    return( off );
}

void DwarfGenGlobal( symbol *sym, section *sect )
/******************************************************/
{
    symbol_die  die;
    symbol_seg  symseg;
    virt_mem    vmem_addr;
    size_t      len;

    sect = sect;
    if( !( CurrMod->modinfo & MOD_DBI_SEEN ) ) {
        if( ( sym->p.seg == NULL ) || ( sym->p.seg->iscode ) ) {
            die.abbrev_code = LABEL_ABBREV_CODE;
        } else {
            die.abbrev_code = VARIABLE_ABBREV_CODE;
        }
        vmem_addr = CurrMod->d.d->pubsym.addr;
        die.off = sym->addr.off;
        if( FmtData.type & ( MK_PE | MK_QNX_FLAT | MK_ELF ) ) {
            die.off += GetLinearGroupOffset( sym->p.seg->u.leader->group );
        }
        die.isexternal = !( sym->info & SYM_STATIC );
        PutInfo( vmem_addr, &die, sizeof( symbol_die ) );
        vmem_addr += sizeof( symbol_die );
        if( FmtData.type & MK_SEGMENTED ) {
            symseg.len = 3;
            symseg.loc_op = DW_OP_const2u;
            symseg.seg = sym->addr.seg;
            PutInfo( vmem_addr, &symseg, sizeof( symbol_seg ) );
            vmem_addr += sizeof( symbol_seg );
        }
        len = strlen( sym->name ) + 1;
        PutInfo( vmem_addr, sym->name, len );
        CurrMod->d.d->pubsym.addr = vmem_addr + len;
    }
}

void DwarfGenLines( lineinfo *info )
/**********************************/
{
    ln_off_pair UNALIGN *lineptr;
    unsigned            dwsize;
    dw_linenum_delta    linedelta;
    dw_addr_delta       addrdelta;
    virt_mem            vmem_addr;
    ln_off_386          prevline;
    offset              off;
    uint_8              buff[ 3 + 2 * MAX_LEB128 ];
    unsigned            size;
    segdata             *seg;

    if( CurrMod->modinfo & MOD_DBI_SEEN )
        return;

    seg = info->seg;
    prevline.off = 0;
    prevline.linnum = 1;
    vmem_addr = CurrMod->d.d->dasi.addr;
    if( FmtData.type & MK_286 ) {
        dwsize = 3;
    } else {
        dwsize = 5;
    }
    buff[0] = 0;        // extended opcode
    buff[1] = dwsize;
    buff[2] = DW_LNE_set_address;
    if( FmtData.type & MK_286 ) {
        *( (unsigned_16 *)&buff[3] ) = seg->a.delta + seg->u.leader->seg_addr.off;
    } else {
        off = seg->a.delta + seg->u.leader->seg_addr.off;
        if( FmtData.type & ( MK_PE | MK_QNX_FLAT | MK_ELF ) ) {
            off += GetLinearGroupOffset( seg->u.leader->group );
        }
        *( (unsigned_32 *)&buff[3] ) = off;
    }
    dwsize += 2;
    PutInfo( vmem_addr, buff, dwsize );
    vmem_addr += dwsize;
    if( FmtData.type & MK_SEGMENTED ) {
        buff[1] = 3;
        buff[2] = DW_LNE_set_segment;
        *( (unsigned_16 *)&buff[3] ) = seg->u.leader->seg_addr.seg;
        PutInfo( vmem_addr, buff, 5 );
        vmem_addr += 5;
    }
    size = info->size & ~LINE_IS_32BIT;
    lineptr = (ln_off_pair *)info->data;
    while( size > 0 ) {
        if( info->size & LINE_IS_32BIT ) {
            linedelta = (signed_32) lineptr->_386.linnum - prevline.linnum;
            addrdelta = lineptr->_386.off - prevline.off;
            lineptr++;
            size -= sizeof( ln_off_386 );
        } else {
            linedelta = (signed_32) lineptr->_286.linnum - prevline.linnum;
            addrdelta = lineptr->_286.off - prevline.off;
            lineptr = (void *)( (char *)lineptr + sizeof( ln_off_286 ) );
            size -= sizeof( ln_off_286 );
        }
        prevline.linnum += linedelta;
        prevline.off += addrdelta;
        dwsize = DWLineGen( linedelta, addrdelta, buff );
        PutInfo( vmem_addr, buff, dwsize );
        vmem_addr += dwsize;
    }
    buff[0] = 0;        // extended opcode
    buff[1] = 1;        // size 1
    buff[2] = DW_LNE_end_sequence;
    PutInfo( vmem_addr, buff, 3 );
    CurrMod->d.d->dasi.addr = vmem_addr + 3;
}

static void DwarfAddAddrInit( segdata *sdata, void *cookie )
/**********************************************************/
{
    sdata = sdata;
    cookie = cookie;
}

static void DwarfAddAddrAdd( segdata *sdata, offset delta, offset size,
                             void *cookie, bool isnewmod )
/********************************************************/
{
    delta = delta;
    size = size;
    cookie = cookie;
    if( isnewmod && !( sdata->o.mod->modinfo & MOD_DBI_SEEN ) ) {
        if( FmtData.type & MK_SEGMENTED ) {
            sdata->o.mod->d.d->arange.size += sizeof( segmented_arange_tuple );
        } else {
            sdata->o.mod->d.d->arange.size += sizeof( flat_arange_tuple );
        }
    }
}

void DwarfAddAddrInfo( seg_leader *seg )
/*********************************************/
{
    DBIAddrInfoScan( seg, DwarfAddAddrInit, DwarfAddAddrAdd, NULL );
}

static offset GetNewAddrOffset( segdata *sdata, offset delta )
/************************************************************/
{
    offset      off;

    off = sdata->u.leader->seg_addr.off + delta;
    if( FmtData.type & ( MK_PE | MK_QNX_FLAT | MK_ELF ) ) {
        off += GetLinearGroupOffset( sdata->u.leader->group );
    }
    return( off );
}

static void DwarfGenAddrInit( segdata *sdata, void *_tuple )
/**********************************************************/
{
    arange_tuple *tuple = _tuple;

    if( FmtData.type & MK_SEGMENTED ) {
        tuple->s.segment = sdata->u.leader->seg_addr.seg;
        tuple->s.offset = sdata->u.leader->seg_addr.off + sdata->a.delta;
    } else {
        tuple->f.offset = GetNewAddrOffset( sdata, sdata->a.delta );
    }
}

static void DwarfGenAddrAdd( segdata *sdata, offset delta, offset size,
                             void *_tuple, bool isnewmod )
/*********************************************************************/
{
    arange_tuple *tuple = _tuple;
    mod_entry * mod;
    unsigned    tup_size;

    if( !isnewmod )
        return;
    mod = sdata->o.mod;
    if( !( mod->modinfo & MOD_DBI_SEEN ) ) {
        if( FmtData.type & MK_SEGMENTED ) {
            tuple->s.length = size;
            tup_size = sizeof( segmented_arange_tuple );
            if( sdata->u.leader->class->flags & CLASS_STACK ) {
                tuple->s.length = StackSize;
            }
        } else {
            tuple->f.length = size;
            tup_size = sizeof( flat_arange_tuple );
            if( sdata->u.leader->class->flags & CLASS_STACK ) {
                tuple->f.length = StackSize;
            }
        }
        PutInfo( mod->d.d->arange.addr, tuple, tup_size );
        mod->d.d->arange.addr += tup_size;
    }
    if( FmtData.type & MK_SEGMENTED ) {
        tuple->s.offset = sdata->u.leader->seg_addr.off + delta;
    } else {
        tuple->f.offset = GetNewAddrOffset( sdata, delta );
    }
}

void DwarfGenAddrInfo( seg_leader *seg )
/*********************************************/
{
    arange_tuple tuple;

    DBIAddrInfoScan( seg, DwarfGenAddrInit, DwarfGenAddrAdd, &tuple );
}

void DwarfAddrSectStart( section *sect )
/*********************************************/
// This allocates space for the dwarf information
{
    int index;

    if( ( sect == Root ) && ( SectionTable[SECT_DEBUG_ABBREV].size > 0 ) ) {
        for( index = 0; index < SECT_NUM_SECTIONS; index++ ) {
            if( index != SECT_DEBUG_ABBREV ) {
                SectionTable[index].addr = DBIAlloc( SectionTable[index].size );
            }
        }
    }
}

static void FillHeader( Elf32_Shdr *hdr, char *name, stringtable *strtab,
                        unsigned_32 curr_off )
/***********************************************************************/
{
    hdr->sh_name = GetStringTableSize( strtab );
    AddStringStringTable( strtab, name );
    hdr->sh_type = SHT_PROGBITS;
    hdr->sh_flags = 0;
    hdr->sh_addr = 0;
    hdr->sh_offset = curr_off;
    hdr->sh_link = SHN_UNDEF;
    hdr->sh_info = 0;
    hdr->sh_addralign = 0;
    hdr->sh_entsize = 0;
}

static void WriteDwarfSect( unsigned addidx, unsigned_32 size )
/*************************************************************/
{
    if( addidx == SECT_DEBUG_ABBREV ) {
        if( FmtData.type & MK_SEGMENTED ) {
            WriteLoad( SegmentedStandardAbbrevs, size );
        } else {
            WriteLoad( FlatStandardAbbrevs, size );
        }
    } else {
        WriteInfo( SectionTable[addidx].addr, size );
    }
}

static unsigned_32 WriteELFSections( unsigned_32 file_off, unsigned_32 curr_off,
                                     Elf32_Shdr *hdr, stringtable *strtab )
/******************************************************************************/
// write out all of the individual section in the debugging information
{
    seg_leader *seg;
    unsigned    addidx;
    unsigned_32 addsize;

    if( DBIClass != NULL ) {
        seg = (seg_leader *) RingStep( DBIClass->segs, NULL );
        while( seg != NULL ) {
            addsize = 0;
            addidx = seg->dbgtype - DWARF_DEBUG_INFO;
            if( addidx < SECT_NUM_SECTIONS ) {
                addsize = SectionTable[addidx].size;
            }
            if( seg->size != 0 ) {
                FillHeader( hdr, seg->segname, strtab, curr_off );
                hdr->sh_size = seg->size + addsize;
                WriteLeaderLoad( seg );
                if( addsize != 0 ) {
                    WriteDwarfSect( addidx, addsize );
                }
                curr_off += hdr->sh_size;
                file_off += hdr->sh_size;
                hdr++;
            }
            seg = (seg_leader *) RingStep( DBIClass->segs, seg );
        }
    }
    for( addidx = 0; addidx < SECT_NUM_SECTIONS; addidx++ ) {
        if( ( SectionTable[addidx].size != 0 ) && ( SectionTable[addidx].start == 0 ) ) {
            FillHeader( hdr, SectionTable[addidx].name, strtab, curr_off );
            hdr->sh_size = SectionTable[addidx].size;
            WriteDwarfSect( addidx, hdr->sh_size );
            curr_off += hdr->sh_size;
            hdr++;
        }
    }
    return( curr_off );
}

static bool CheckDBISeg( void *seg, void *num )
/*********************************************/
{
    if( ((seg_leader *)seg)->size != 0 ) {
        (*(int *)num)++;
    }
    return( FALSE );
}

int DwarfCountDebugSections( void )
/****************************************/
{
    int num;
    int index;

    num = 0;
    if( DBIClass != NULL ) {
        RingLookup( DBIClass->segs, CheckDBISeg, &num );
    }
    for( index = 0; index < SECT_NUM_SECTIONS; index++ ) {
        if( ( SectionTable[index].size != 0 ) && ( SectionTable[index].start == 0 ) ) {
            num++;
        }
    }
    if( FmtData.type & MK_OVERLAYS ) {
        num++;
    }
    return( num );
}

offset DwarfWriteTrailer( offset curr_off )
/************************************************/
{
    TISTrailer  trailer;

    trailer.size = curr_off;
    trailer.size += sizeof( TISTrailer );
    trailer.signature = TIS_TRAILER_SIGNATURE;
    trailer.vendor = 0;
    trailer.type = 0;
    WriteLoad( &trailer, sizeof( TISTrailer ) );
    return( sizeof( TISTrailer ) );
}

offset DwarfWriteElf( offset curr_off, stringtable *strtab,
                                Elf32_Shdr *dbgsecbegin )
/*********************************************************/
{
    curr_off = WriteELFSections( curr_off, curr_off, dbgsecbegin, strtab );
    return( curr_off );
}

void DwarfWrite( void )
/*********************/
{
    Elf32_Ehdr          elf_header;
    Elf32_Shdr *        sect_header;
    Elf32_Shdr *        sh;
    unsigned            shdr_size;
    unsigned_32         virt_off;
    stringtable         strtab;
    unsigned long       savepos;

    memcpy( elf_header.e_ident, ELF_SIGNATURE, ELF_SIGNATURE_LEN );
    elf_header.e_ident[EI_CLASS] = ELFCLASS32;
#ifdef __BIG_ENDIAN__
    elf_header.e_ident[EI_DATA] = ELFDATA2MSB;
#else
    elf_header.e_ident[EI_DATA] = ELFDATA2LSB;
#endif
    elf_header.e_ident[EI_VERSION] = EV_CURRENT;
    elf_header.e_ident[EI_OSABI] = ELFOSABI_NONE;
    elf_header.e_ident[EI_ABIVERSION] = 0;
    memset( &elf_header.e_ident[EI_PAD], 0, EI_NIDENT - EI_PAD );
    elf_header.e_type = ET_EXEC;
    elf_header.e_machine = EM_386;
    elf_header.e_version = EV_CURRENT;
    elf_header.e_entry = 0;
    elf_header.e_phoff = 0;
    elf_header.e_shoff = sizeof( elf_header );
    elf_header.e_flags = 0;
    elf_header.e_ehsize = sizeof( elf_header );
    elf_header.e_phentsize = sizeof( Elf32_Phdr );
    elf_header.e_phnum = 0;
    elf_header.e_shentsize = sizeof( Elf32_Shdr );
    elf_header.e_shnum = 2;     // reserve 2 for null header & string table
    elf_header.e_shnum += DwarfCountDebugSections();
    elf_header.e_shstrndx = elf_header.e_shnum - 1;     // last entry
    WriteLoad( &elf_header, sizeof( Elf32_Ehdr ) );
    shdr_size = sizeof( Elf32_Shdr ) * elf_header.e_shnum;
    _ChkAlloc( sect_header, shdr_size );
    savepos = PosLoad() + shdr_size;
    SeekLoad( savepos );
    memset( sect_header, 0, sizeof( Elf32_Shdr ) );   // first fill in the null section
    InitStringTable( &strtab, FALSE );
    AddCharStringTable( &strtab, '\0' );
    virt_off = sizeof( Elf32_Ehdr ) + shdr_size;
    virt_off = WriteELFSections( savepos, virt_off, sect_header+1, &strtab );
    sh = sect_header + elf_header.e_shstrndx;
    FillHeader( sh, ".shstrtab", &strtab, virt_off );
    sh->sh_type = SHT_STRTAB;
    sh->sh_size = GetStringTableSize( &strtab );
    WriteStringTable( &strtab, WriteLoad3, NULL );
    FiniStringTable( &strtab );
    virt_off += sh->sh_size;
    virt_off += DwarfWriteTrailer( virt_off );
    SeekLoad( savepos - shdr_size );
    WriteLoad( sect_header, shdr_size );
    _LnkFree( sect_header );
}
