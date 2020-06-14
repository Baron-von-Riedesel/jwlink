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
* Description:  Open Watcom banner strings and version defines.
*
****************************************************************************/


//#include "bancfg.h"

//#define DOBANSTR( p )   #p
//#define BANSTR( p )     DOBANSTR( p )

//#ifndef _BETASTR_
//#define _BETASTR_ "beta"
//#endif

//#ifdef _BETAVER
//#define _BETA_                  _BETASTR_ BANSTR( _BETAVER )  _BANEXTRA
//#else
//#define _BETA_                  _BANEXTRA
//#endif

#define CURR_YEAR       "2012"

#define banner1p1(p)  p
#define banner1p2(v)  "Version " v
#define banner1(p,v) banner1p1(p) " " banner1p2(v)
#define banner1w(p,v) "JWlib " banner1p1(p) " " banner1p2(v)

#define banner2p1(year) "Portions Copyright (c) " year "-2002 Sybase, Inc. All Rights Reserved"
//#define banner2p2() "All rights reserved"
//#define banner2(year) banner2p1(year) ". " banner2p2() "."
#define banner2p2() ""
#define banner2(year) banner2p1(year) "."

#define banner2a() "Portions Copyright (c) 1984-2002 Sybase, Inc. All Rights Reserved."

#define banner3       "Source code is available under the Sybase Open Watcom Public License."
//define banner3a      "See http://www.openwatcom.org/ for details."

#define banner1ps(p,v)  "Powersoft " banner1p1(p) " " banner1p2(v)
#define banner2ps       banner2a()
#define banner3ps       "All rights reserved.  Powersoft is a trademark of Sybase, Inc."

/* Used by setupgui/c/guiinit.c */
//#define banner4gui()    "Copyright � 2002-%s Open Watcom Contributors. All Rights Reserved."
//#define banner2agui() "Portions Copyright � 1984-2002 Sybase, Inc. All Rights Reserved."

// the following macros define the delimeters used by the resource
// compiler when concatenating strings
#define _RC_DELIM_LEFT_         [
#define _RC_DELIM_RIGHT_        ]

#define BAN_VER_STR "1.9" "beta 8"

/* these should all be _BANVER/100.0 */
#define _I86WCGL_VERSION_       BAN_VER_STR
#define _386WCGL_VERSION_       BAN_VER_STR
#define _WCC_VERSION_           BAN_VER_STR
#define _WPP_VERSION_           BAN_VER_STR
#define _WCL_VERSION_           BAN_VER_STR
#define _WFC_VERSION_           BAN_VER_STR
#define _WFL_VERSION_           BAN_VER_STR
#define _WLINK_VERSION_         BAN_VER_STR
/* these can be what ever they want to be */
#define _BPATCH_VERSION_        BAN_VER_STR
#define _MOUSEFIX_VERSION_      BAN_VER_STR
#define _XXXSERV_VERSION_       BAN_VER_STR
#define _RFX_VERSION_           BAN_VER_STR
#define _WVIDEO_VERSION_        BAN_VER_STR
#define _WD_VERSION_            BAN_VER_STR
#define _WBED_VERSION_          BAN_VER_STR
#define _WCEXP_VERSION_         BAN_VER_STR
#define _WATFOR77_VERSION_      BAN_VER_STR
#define _WHELP_VERSION_         BAN_VER_STR
#define _WDISASM_VERSION_       BAN_VER_STR
#define _FCENABLE_VERSION_      BAN_VER_STR
#define _MS2WLINK_VERSION_      BAN_VER_STR
#define _EXE2BIN_VERSION_       BAN_VER_STR
#define _WLIB_VERSION_          BAN_VER_STR
#define _WMAKE_VERSION_         BAN_VER_STR
#define _WOMP_VERSION_          BAN_VER_STR
#define _WPROF_VERSION_         BAN_VER_STR
#define _WSAMP_VERSION_         BAN_VER_STR
#define _WSTRIP_VERSION_        BAN_VER_STR
#define _WTOUCH_VERSION_        BAN_VER_STR
#define _WBIND_VERSION_         BAN_VER_STR
#define _PERES_VERSION_         BAN_VER_STR
#define _EDBIND_VERSION_        BAN_VER_STR
#define _WASM_VERSION_          BAN_VER_STR
#define _WBRW_VERSION_          BAN_VER_STR
#define _WBRG_VERSION_          BAN_VER_STR
#define _VIPER_VERSION_         BAN_VER_STR
#define _RESEDIT_VERSION_       BAN_VER_STR
#define _WRC_VERSION_           BAN_VER_STR
#define _SPY_VERSION_           BAN_VER_STR
#define _HEAPWALKER_VERSION_    BAN_VER_STR
#define _DDESPY_VERSION_        BAN_VER_STR
#define _DRWATCOM_VERSION_      BAN_VER_STR
#define _DRNT_VERSION_          BAN_VER_STR
#define _ZOOM_VERSION_          BAN_VER_STR
#define _VI_VERSION_            BAN_VER_STR
#define _ASAXP_CLONE_VERSION_   BAN_VER_STR
#define _CL_CLONE_VERSION_      BAN_VER_STR
#define _CVTRES_CLONE_VERSION_  BAN_VER_STR
#define _LIB_CLONE_VERSION_     BAN_VER_STR
#define _LINK_CLONE_VERSION_    BAN_VER_STR
#define _NMAKE_CLONE_VERSION_   BAN_VER_STR
#define _RC_CLONE_VERSION_      BAN_VER_STR
#define _WIC_VERSION_           BAN_VER_STR
#define _WGML_VERSION_          BAN_VER_STR

/*
 * Java Tools
 */
//#define JAVA_BAN_VER_STR        "1.0" _BETA_

//#define _WJDUMP_VERSION_        JAVA_BAN_VER_STR
//#define _JLIB_VERSION_          JAVA_BAN_VER_STR
//#define _JAVAC_VERSION_         JAVA_BAN_VER_STR

/*
 * Versions of Microsoft tools with OW clones are compatible
 */
#define _MS_CL_VERSION_         "13.0.0"
#define _MS_LINK_VERSION_       "7.0.0"
