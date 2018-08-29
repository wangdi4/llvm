// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "cl_secure_string.h"

errno_t strcpy_s( char *dst, size_t numberOfElements, const char *src )
{
    if( ( dst == NULL ) ||
        ( src == NULL ) )
    {
        return EINVAL;
    }
    size_t length = strlen( src );
    if( numberOfElements < length )
    {
        return ERANGE;
    }
    strcpy( dst, src );
    return 0;
}

errno_t strncpy_s( char *dst, size_t numberOfElements, const char *src, size_t count )
{
    if( ( dst == NULL ) ||
        ( src == NULL ) )
    {
        return EINVAL;
    }
    if( numberOfElements < count )
    {
        return ERANGE;
    }
    size_t length = strlen( src );
    if( length > count )
    {
        length = count;
    }
    strncpy( dst, src, length );
    if( length < numberOfElements )
    {
        numberOfElements = length;
    }
    dst[ numberOfElements ] = '\0';
    return 0;
}

size_t strnlen_s( const char *str, size_t count )
{
    if( str == NULL )
    {
        return 0;
    }
    size_t length = strlen( str );
    return ( length > count ) ? count : length;
}

errno_t memcpy_s( void *dst, size_t numberOfElements, const void *src, size_t count )
{
    if( ( dst == NULL ) || ( src == NULL ) )
    {
        return EINVAL;
    }
    if( numberOfElements < count )
    {
        return ERANGE;
    }
    memcpy( dst, src, count );
    return 0;
}
