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

#include <string.h>

#include "linkstd.h"
#include "reloc.h"

#define export
#include "globals.h"
#include "specials.h"

#ifdef __UNIX__

char *_strupr( char *str )
/************************/
{
    char    *p;
    unsigned char   c;

    p = str;
    while( (c = *p) ) {
        c -= 'a';
        if( c <= 'z' - 'a' ) {
            c += 'A';
            *p = c;
        }
        ++p;
    }
    return( str );
}

char *ultoa( unsigned long val, char *s, int radix )
{
    switch (radix) {
    case 10:  sprintf( s, "%lu", val ); break;
    case 16:  sprintf( s, "%lx", val ); break;
    default: printf("ultoa: unsupported radix %u\n", radix );
    }
    return s;
}

char *utoa( unsigned int val, char *s, int radix )
{
    switch (radix) {
    case 10:  sprintf( s, "%u", val ); break;
    case 16:  sprintf( s, "%x", val ); break;
    default: printf("utoa: unsupported radix %u\n", radix );
    }
    return s;
}

char *itoa( int val, char *s, int radix )
{
    switch (radix) {
    case 10:  sprintf( s, "%d", val ); break;
    case 16:  sprintf( s, "%x", val ); break;
    default: printf("itoa: unsupported radix %u\n", radix );
    }
    return s;
}

/* emulate getcmd() in Linux;
 * requires external _argv[]
 */

char **_argv = NULL;

int _bgetcmd( char *buffer, int len )
{
    int     total;
    int     i;
    char    *word;
    char    *p     = NULL;
    char    **argv = &_argv[1];

    --len; // reserve space for NULL byte

    if( buffer && (len > 0) ) {
        p  = buffer;
        *p = '\0';
    }

    /* create approximation of original command line */
    for( word = *argv++, i = 0, total = 0; word; word = *argv++ ) {
        i      = strlen( word );
        total += i;

        if( p ) {
            if( i >= len ) {
                strncpy( p, word, len );
                p[len] = '\0';
                p      = NULL;
                len    = 0;
            } else {
                strcpy( p, word );
                p   += i;
                len -= i;
            }
        }

        /* account for at least one space separating arguments */
        if( *argv ) {
            if( p ) {
                *p++ = ' ';
                --len;
            }
            ++total;
        }
    }

    return( total );
}


char * getcmd( char *buffer )
{
    _bgetcmd( buffer, 1024 );
    return( buffer );
}

#endif
