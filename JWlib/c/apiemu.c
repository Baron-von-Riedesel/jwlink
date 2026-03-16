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
*  Description: API emulations:
*               gcc:     _makepath(), _splitpath(), _fullpath(), strupr()
*               OW:      CharUpperA()
*               PellesC: _makepath()
*
****************************************************************************/

#ifdef __UNIX__

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define __set_errno( err ) errno = (err)

/****************************************************************************
*
* Description:  Platform independent _splitpath() implementation.
*
****************************************************************************/

#define PC '/'
#define ISPC(x) (x == PC)

static void copypart( char *buf, const char *p, int len, int maxlen )
/*******************************************************************/
{
    if( buf != NULL ) {
        if( len > maxlen ) len = maxlen;
        memcpy( buf, p, len );
        buf[len] = '\0';
    }
}

#define _MAX_DRIVE  256
#define _MAX_DIR    256
#define _MAX_FNAME  256
#define _MAX_EXT    256
#define _MAX_PATH  1024

/* split full path name into its components */

void _splitpath( const char *path, char *drive, char *dir, char *fname, char *ext )
/*********************************************************************************/
{
    const char *dotp;
    const char *fnamep;
    const char *startp;
    unsigned    ch;

    /* process node/drive specification */
    startp = path;
    if( path[0] == PC  &&  path[1] == PC ) {
        path += 2;
        for( ;; ) {
            if( *path == '\0' ) break;
            if( *path == PC ) break;
            if( *path == '.' ) break;
            path++;
        }
    }
    copypart( drive, startp, path - startp, _MAX_DRIVE );

    /* process /user/fred/filename.ext for QNX */
    /* process /fred/filename.ext for DOS, OS/2 */
    dotp = NULL;
    fnamep = path;
    startp = path;

    for(;;) {           /* 07-jul-91 DJG -- save *path in ch for speed */
        if( *path == '\0' )  break;
        ch = *path;
        if( ch == '.' ) {
            dotp = path;
            ++path;
            continue;
        }
        path++;
        if( ISPC(ch) ) {
            fnamep = path;
            dotp = NULL;
        }
    }
    copypart( dir, startp, fnamep - startp, _MAX_DIR - 1 );
    if( dotp == NULL ) dotp = path;
    copypart( fname, fnamep, dotp - fnamep, _MAX_FNAME - 1 );
    copypart( ext,   dotp,   path - dotp,   _MAX_EXT - 1);
}

/* create full Unix style path name from the components */

void _makepath( char *path, const char *node, const char *dir, const char *fname, const char *ext )
/*************************************************************************************************/
{
    *path = '\0';

    if( node != NULL ) {
        if( *node != '\0' ) {
            strcpy( path, node );
            path = strchr( path, '\0' );

            /* if node did not end in '/' then put in a provisional one */
            if( path[-1] == PC )
                path--;
            else
                *path = PC;
        }
    }
    if( dir != NULL ) {
        if( *dir != '\0' ) {
            /*  if dir does not start with a '/' and we had a node then
                    stick in a separator
            */
            if( ( ! ISPC(*dir) ) && ( ISPC(*path) ) ) path++;

            strcpy( path, dir );
            path = strchr( path, '\0' );

            /* if dir did not end in '/' then put in a provisional one */
            if( path[-1] == PC )
                path--;
            else
                *path = PC;
        }
    }

    if( fname != NULL ) {
        if( ( !ISPC(*fname) ) && ( ISPC(*path) ) ) path++;

        strcpy( path, fname );
        path = strchr( path, '\0' );

    } else {
        if( ISPC(*path) ) path++;
    }
    if( ext != NULL ) {
        if( *ext != '\0' ) {
            if( *ext != '.' )  *path++ = '.';
            strcpy( path, ext );
            path = strchr( path, '\0' );
        }
    }
    *path = '\0';
}

#define _WILL_FIT( c )  if(( (c) + 1 ) > size ) {       \
                            __set_errno( ERANGE );      \
                            return( NULL );             \
                        }                               \
                        size -= (c);

static char *_sys_fullpath( char *buff, const char *path, size_t size )
/*********************************************************************/
{

    const char  *p;
    char        *q;
    size_t      len;
    char        curr_dir[_MAX_PATH];

    p = path;
    q = buff;
    if( ! ISPC( p[0] ) ) {
        if( getcwd( curr_dir, sizeof(curr_dir) ) == NULL ) {
            __set_errno( ENOENT );
            return( NULL );
        }
        len = strlen( curr_dir );
        _WILL_FIT( len );
        strcpy( q, curr_dir );
        q += len;
        if( q[-1] != '/' ) {
            _WILL_FIT( 1 );
            *(q++) = '/';
        }
        for(;;) {
            if( p[0] == '\0' ) break;
            if( p[0] != '.' ) {
                _WILL_FIT( 1 );
                *(q++) = *(p++);
                continue;
            }
            ++p;
            if( ISPC( p[0] ) ) {
                /* ignore "./" in directory specs */
                if( ! ISPC( q[-1] ) ) {
                    *q++ = '/';
                }
                ++p;
                continue;
            }
            if( p[0] == '\0' ) break;
            if( p[0] == '.' && ISPC( p[1] ) ) {
                /* go up a directory for a "../" */
                p += 2;
                if( ! ISPC( q[-1] ) ) {
                    return( NULL );
                }
                q -= 2;
                for(;;) {
                    if( q < buff ) {
                        return( NULL );
                    }
                    if( ISPC( *q ) ) break;
                    --q;
                }
                ++q;
                *q = '\0';
                continue;
            }
            _WILL_FIT( 1 );
            *(q++) = '.';
        }
        *q = '\0';
    } else {
        len = strlen( p );
        _WILL_FIT( len );
        strcpy( q, p );
    }
    return( buff );
}

char *_fullpath( char *buff, const char *path, size_t size )
/**********************************************************/
{
    char *ptr = NULL;

    if( buff == NULL ) {
        size = _MAX_PATH;
        ptr = malloc( size );
        if( ptr == NULL ) __set_errno( ENOMEM );
        buff = ptr;
    }
    if( buff != NULL ) {
        buff[0] = '\0';
        if( path == NULL || path[0] == '\0' ) {
            buff = getcwd( buff, size );
        } else {
            buff = _sys_fullpath( buff, path, size );
        }
        if( buff == NULL ) {
            if( ptr != NULL ) free( ptr );
        }
    }
    return buff;
}

#if 0
int _memicmp( const void *in_s1, const void *in_s2, size_t len )
/**************************************************************/
{
    const unsigned char *   s1 = (const unsigned char *)in_s1;
    const unsigned char *   s2 = (const unsigned char *)in_s2;
    unsigned char           c1;
    unsigned char           c2;

    for( ; len; --len )  {
        c1 = *s1;
        c2 = *s2;
        if( c1 >= 'A'  &&  c1 <= 'Z' )  c1 += 'a' - 'A';
        if( c2 >= 'A'  &&  c2 <= 'Z' )  c2 += 'a' - 'A';
        if( c1 != c2 ) return( c1 - c2 );
        ++s1;
        ++s2;
    }
    return( 0 );    /* both operands are equal */
}
#endif

char *strupr( char *str )
/***********************/
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
#endif


