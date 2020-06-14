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
* Description:  Mainline for Open Watcom linker.
*
****************************************************************************/


#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "linkstd.h"
#include "msg.h"
#include "alloc.h"
#include "wlnkmsg.h"
#include "command.h"
#include "fileio.h"
#include "objpass2.h"
#include "cmdline.h"
#include "loadfile.h"
#include "objfree.h"
#include "mapio.h"
#include "objcalc.h"
#include "procfile.h"
#include "spillio.h"
#include "virtmem.h"
#include "loados2.h"
#include "loadpe.h"
#include "loadqnx.h"
#include "loadnov.h"
#include "loadelf.h"
#include "symtrace.h"
#include "objnode.h"
#include "objio.h"
#include "distrib.h"
#include "objorl.h"
#include "strtab.h"
#include "carve.h"
#include "permdata.h"
#include "toc.h"
#if defined( _DLLHOST )
    #include <malloc.h>
#endif
#include "dbgall.h"
#include "objpass1.h"
#include "obj2supp.h"
#include "cmdall.h"
#include "reloc.h"
#include "salloc.h"
#include "objstrip.h"
#include "symtab.h"
#include "omfreloc.h"
#include "overlays.h"
#include "wcomdef.h"
#include "objomf.h"
#include "wlink.h"
#ifndef __WATCOMC__
    #include "clibext.h"
#endif
#include "library.h"

static void     PreAddrCalcFormatSpec( void );
static void     PostAddrCalcFormatSpec( void );
static void     DoDefaultSystem( void );
static void     FindLibPaths( void );
static void     ResetMisc( void );
static void     ResetSubSystems( void );
static void     DoLink( char * );
static void     CleanSubSystems( void );

#ifdef _INT_DEBUG
/*
 *  I have temporarily left these as extern as they are internal data. On the final pass, either find
 *  a library header that defines these or create one!
 */
extern char     *_edata;
extern char     *_end;
#endif

static char     *ArgSave;

// Not sure what this is for - doesn't seem to be referenced
//extern int              __nheapblk;

#if !defined( _DLLHOST )           // it's the standalone linker
int main( int argc, char ** argv )
/***************************************/
{
    argc = argc;        /* to avoid a warning */
    argv = argv;
#ifndef __WATCOMC__
    _argv = argv;
    _argc = argc;
#endif
    InitSubSystems();
    LinkMainLine( NULL );
    FiniSubSystems();
    return( (LinkState & LINK_ERROR) ? 1 : 0 );
}
#endif

static void LinkMeBaby( void )
/****************************/
{
    ResetSubSystems();
    DoLink( ArgSave );
}

void LinkMainLine( char *cmds )
/************************************/
{
    for(;;) {
        ArgSave = cmds;         // bogus way to pass args to spawn
        Spawn( &LinkMeBaby );
        CleanSubSystems();
        cmds = GetNextLink();
        if( cmds == NULL ) break;
    }
#if defined( _DLLHOST )
    _heapshrink();
#endif
}

void InitSubSystems( void )
/********************************/
{
#ifdef _INT_DEBUG
    memset( _edata, 0xA5, _end - _edata );      // don't rely on BSS == 0
#endif
    LnkMemInit();
    LnkFilesInit();
    InitMsg();
    InitNodes();
    InitTokBuff();
    InitSpillFile();
    InitSym();
    InitObjORL();
    InitCmdFile();
}

static void ResetSubSystems( void )
/*********************************/
{
    ResetPermData();
    ResetMsg();
    VirtMemInit();
    ResetMisc();
    Root = NewSection();
    ResetDBI();
    ResetMapIO();
    ResetCmdAll();
    ResetOvlSupp();
    ResetComdef();
    ResetDistrib();
    ResetLoadNov();
    ResetLoadPE();
    ResetObj2Supp();
    ResetObjIO();
    ResetObjOMF();
    ResetObjPass1();
//    ResetDistrib(); // duplicate call
    ResetObjStrip();
    ResetOMFReloc();
    ResetReloc();
    ResetSymTrace();
    ResetLoadFile();
    ResetAddr();
    ResetToc();
}

static void CleanSubSystems( void )
/*********************************/
{
    if( MapFile != NIL_HANDLE ) {
        QClose( MapFile, MapFName );
        MapFile = NIL_HANDLE;
    }
    FreeOutFiles();
    _LnkFree( MapFName );
    BurnSystemList();
    FreeList( LibPath );
    CloseSpillFile();
    CleanTraces();
    FreePaths();
    FreeUndefs();
    FreeLocalImports();
    CleanLoadFile();
    CleanLinkStruct();
    FreeFormatStuff();
    FreeObjInfo();
    FreeVirtMem();
    CleanToc();
    CleanSym();
    CleanPermData();
}

void FiniSubSystems( void )
/********************************/
{
    FiniLinkStruct();
    FiniMsg();
    FiniSym();
    LnkMemFini();
}

static void DoLink( char *cmdline )
/**********************************/
// cmdline is only used when we are running under watfor.
{
#ifndef __OSI__
    signal( SIGINT, &TrapBreak ); /* so we can clean up */
#endif
    StartTime();
    DoCmdFile( cmdline );
    CheckErr();
    MapInit();
    SetupFakeModule();
    ProcObjFiles(); /* ObjPass1 */
    CheckErr();
    DoDefaultSystem();
    if( LinkState & LIBRARIES_ADDED ) {
        FindLibPaths();
        LinkState |= SEARCHING_LIBRARIES;
        ResolveUndefined();
        LinkState &= ~SEARCHING_LIBRARIES;
        LinkState |= GENERATE_LIB_LIST;
    }
    ProcLocalImports();
    DecideFormat();
    SetFormat();
    ConvertLazyRefs();
    SetSegments();
    CheckErr();
    DefBSSSyms();
    LinkFakeModule();
    PreAddrCalcFormatSpec();
    ReportUndefined();
    CheckClassOrder();
    CalcSegSizes();
    SetStkSize();
    AutoGroup();
    CalcAddresses();
    GetBSSSize();
    GetStkAddr();
    GetStartAddr();
    PostAddrCalcFormatSpec();
    CheckErr();
    InitLoadFile();
    ObjPass2();
    CheckErr();
    FiniLoadFile();
    WritePermData();
    BuildImpLib();
    EndTime();
#ifndef __OSI__
    signal( SIGINT, SIG_IGN ); /* we're going to clean up anyway */
#endif
}

static void PreAddrCalcFormatSpec( void )
/***************************************/
// format specific routines which need to be called before address calculation
{
#ifdef _OS2
    if( FmtData.type & MK_PE ) {
        ChkPEData();
    } else if( FmtData.type & (MK_OS2|MK_WIN_VXD) ) {
        if( IS_PPC_OS2) {
            // Development temporarly on hold:
            // ChkOS2ElfData();
        } else {
            ChkOS2Data();
        }
    }
#endif
#ifdef _NOVELL
    if( FmtData.type & MK_NOVELL ) {
        FindExportedSyms();
    }
#endif
#ifdef _PHARLAP
    if( FmtData.type & MK_PHAR_FLAT && LinkState & HAVE_16BIT_CODE
                                    && !(CmdFlags & CF_HAVE_REALBREAK)) {
        LnkMsg( WRN+MSG_NO_REALBREAK_WITH_16BIT, NULL );
    }
#endif
}

static void PostAddrCalcFormatSpec( void )
/****************************************/
// format specific routines which need to be called after address calculation
{
#ifdef _OS2
    if( FmtData.type & MK_PE ) {
        AllocPETransferTable();
    } else if( FmtData.type & MK_ELF ) {
        ChkElfData();
    } else if( FmtData.type & (MK_OS2|MK_WIN_VXD) ) {
        if( IS_PPC_OS2) {
            // Development temporarly on hold:
            //PrepareOS2Elf();
        } else {
            ChkOS2Exports();
        }
    }
#endif
#ifdef _QNXLOAD
    else if( FmtData.type & MK_QNX ) {
        SetQNXSegFlags();
    }
#endif
}

static void ResetMisc( void )
/***************************/
/* Linker support initialization. */
{
    /* jwlink: default is: multiple defines are NOT ok */
    //LinkFlags = REDEFS_OK | CASE_FLAG | FAR_CALLS_FLAG;
    LinkFlags = CASE_FLAG | FAR_CALLS_FLAG;
    LinkState = MAKE_RELOCS;
    AbsGroups = NULL;
    DataGroup = NULL;
    IDataGroup = NULL;
    MapFile = NIL_HANDLE;
    MapFName = NULL;
    OutFiles = NULL;
    ObjLibFiles = NULL;
    LibModules = NULL;
    Groups = NULL;
    CurrLoc.seg = UNDEFINED;
    CurrLoc.off = 0;
    OvlClasses = NULL;
    OvlVectors = NULL;
    VecNum = 0;
    OvlNum = 0;
    OvlFName = NULL;
    CurrMod = NULL;
    StackSize = 0x1000;
    // set case sensitivity for symbols
    ResetSym();
    SetSymCase();
    SetLibCase();
}

static void DoDefaultSystem( void )
/*********************************/
/* first hint about format being 32-bit vs. 16-bit (might distinguish between
 * os/2 v1 & os/2 v2), and if that doesn't decide it, haul in the default
 * system block */
{
    if( !(LinkState & FMT_DECIDED) ) {
        if( LinkState & FMT_SEEN_32_BIT ) {
            HintFormat( MK_386 );
        } else if ( LinkState & FMT_SEEN_64_BIT ) {
            HintFormat( MK_PE );
        } else {
            HintFormat( MK_286 | MK_QNX );
        }
        if( !(LinkState & FMT_DECIDED) ) {
            if( LinkState & FMT_SPECIFIED ) {
                LnkMsg( FTL+MSG_AMBIG_FORMAT, NULL );
            }
            if( LinkState & FMT_SEEN_32_BIT ) {
                ExecSystem( "386" );
            } else {
                ExecSystem( "286" ); /* no 386 obj's after this */
            }
        }
    }
}

static void FindLibPaths( void )
/******************************/
{
    AddFmtLibPaths();
    if( LinkState & FMT_SEEN_32_BIT ) {
        AddEnvPaths( "LIB386" );
    } else {
        AddEnvPaths( "LIB286" );
        /*
            If we haven't seen a 386 object file by this time, we're
            not going to.
        */
        HintFormat( MK_286 );
    }
    AddEnvPaths( "LIB" );
}
