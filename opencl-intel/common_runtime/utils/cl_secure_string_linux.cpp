/////////////////////////////////////////////////////////////////////////
// cl_secure_string_linux.cpp
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its
// suppliers and licensors, and is protected by worldwide copyright and trade
// secret laws and treaty provisions. No part of the Material may be used, copied,
// reproduced, modified, published, uploaded, posted, transmitted, distributed,
// or disclosed in any way without Intel’s prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////

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
