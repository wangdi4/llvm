/////////////////////////////////////////////////////////////////////////
// cl_secure_string_linux.h:
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
// or disclosed in any way without Intels prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intels suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////

#include "cl_secure_string_linux.h"

using namespace Intel::OpenCL::Utils;

errno_t Intel::OpenCL::Utils::safeStrCpy(char* dst, size_t len, const char* src )
{
	errno = 0;
	if ((src == nullptr) || (dst == nullptr))
	{
		errno = EINVAL;
		return errno;
	}
	if ((len <= 0) || (strnlen(src, len) >= len))
	{
		errno = ERANGE;
		return errno;
	}
	strncpy(dst, src, len);
	return 0;
}

errno_t Intel::OpenCL::Utils::safeStrNCpy(char* dst, size_t len, const char* src, size_t src_len )
{
	errno = 0;
	if ((src == nullptr) || (dst == nullptr))
	{
		errno = EINVAL;
		return errno;
	}
	if ((len <= 0) || (strnlen(src, src_len) >= len))
	{
		errno = ERANGE;
		return errno;
	}
	strncpy(dst, src, src_len );
	return 0;
}

errno_t Intel::OpenCL::Utils::safeStrCat(char* dst, size_t len, const char* src )
{
	errno = 0;
	if ((src == nullptr) || (dst == nullptr))
	{
		errno = EINVAL;
		return errno;
	}
	if (len <= 0)
	{
		errno = ERANGE;
		return errno;
	}
	size_t tRemain = len - strnlen(dst, len);
	if (strnlen(src, len) >= tRemain)
	{
		errno = ERANGE;
		return errno;
	}
	strncat(dst, src, tRemain -1);
	return 0;
}

int Intel::OpenCL::Utils::safeStrPrintf(char* dst, size_t len, const char* format, ...)
{
	errno = 0;
	if ((format == nullptr) || (dst == nullptr))
	{
		errno = EINVAL;
		return -1;
	}
	if (len <= 0)
	{
		errno = ERANGE;
		return -1;
	}
	va_list src;
	va_start(src, format);
	size_t tSrcOrgSize = vsnprintf(dst, len, format, src);
	va_end(src);
	if (tSrcOrgSize >= len)
	{
		dst[0] = '\0';
		errno = ERANGE;
		return -1;
	}
	return tSrcOrgSize;
}

int Intel::OpenCL::Utils::safeVStrPrintf(char* dst, size_t len, const char* format, va_list args)
{
	errno = 0;
	if ((format == nullptr) || (dst == nullptr))
	{
		errno = EINVAL;
		return -1;
	}
	if (len <= 0)
	{
		errno = ERANGE;
		return -1;
	}
	return vsnprintf(dst, len, format, args);
}

errno_t Intel::OpenCL::Utils::safeMemCpy(void* dst, size_t numOfElements, const void* src, size_t count)
{
	errno = 0;
	if ((src == nullptr) || (dst == nullptr))
	{
		errno = EINVAL;
		return errno;
	}
	if ((count <= 0) || (numOfElements < count))
	{
		errno = ERANGE;
		return errno;
	}
	memcpy(dst, src, count);
	return 0;
}

char* Intel::OpenCL::Utils::safe_strtok(char* strToken, const char* strDelimit, char** context)
{
	errno = 0;
	if (context == nullptr)
	{
		errno = EINVAL;
		return nullptr;
	}
	if (strDelimit == nullptr)
	{
		errno = EINVAL;
		return nullptr;
	}
	if ((strToken == nullptr) && (*context == nullptr))
	{
		errno = EINVAL;
		return nullptr;
	}
	return strtok_r(strToken, strDelimit, context);
}

