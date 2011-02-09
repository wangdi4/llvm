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
	if ((src == NULL) || (dst == NULL))
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

errno_t Intel::OpenCL::Utils::safeWStrCpy(wchar_t* dst, size_t len, const wchar_t* src )
{
	errno = 0;
	if ((src == NULL) || (dst == NULL))
	{
		errno = EINVAL;
		return errno;
	}
	if ((len <= 0) || (wcsnlen(src, len) >= len))
	{
		errno = ERANGE;
		return errno;
	}
	wcsncpy(dst, src, len);
	return 0;
}

errno_t Intel::OpenCL::Utils::safeStrCat(char* dst, size_t len, const char* src )
{
	errno = 0;
	if ((src == NULL) || (dst == NULL))
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

errno_t Intel::OpenCL::Utils::safeWStrCat(wchar_t* dst, size_t len, const wchar_t* src )
{
	errno = 0;
	if ((src == NULL) || (dst == NULL))
	{
		errno = EINVAL;
		return errno;
	}
	if (len <= 0)
	{
		errno = ERANGE;
		return errno;
	}
	size_t tRemain = len - wcsnlen(dst, len);
	if (wcsnlen(src, len) >= tRemain)
	{
		errno = ERANGE;
		return errno;
	}
	wcsncat(dst, src, tRemain -1);
	return 0;
}


int Intel::OpenCL::Utils::safeStrPrintf(char* dst, size_t len, const char* format, ...)
{
	errno = 0;
	if ((format == NULL) || (dst == NULL))
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

int Intel::OpenCL::Utils::safeWStrPrintf(wchar_t* dst, size_t len, const wchar_t* format, ...)
{
	errno = 0;
	if ((format == NULL) || (dst == NULL))
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
	size_t tSrcOrgSize = vswprintf(dst, len, format, src);
	va_end(src);
	if ((tSrcOrgSize == (size_t)-1) || (tSrcOrgSize >= len))
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
	if ((format == NULL) || (dst == NULL))
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
	if ((src == NULL) || (dst == NULL))
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

errno_t Intel::OpenCL::Utils::safeMbToWc(size_t* pReturnValue, wchar_t* wcstr, size_t sizeInWords, const char* mbstr, size_t count)
{
	errno = 0;
	if ((wcstr == NULL) && ((sizeInWords > 0) || (mbstr == NULL)))
	{
		errno = EINVAL;
		return errno;
	}
	if ((wcstr != NULL) && ((count >= sizeInWords) || (mbstr == NULL)))
	{
		wcstr[0] = 0;
		errno = ERANGE;
		return errno;
	}
	size_t counter = 0;
	mbstate_t state;
	memset (&state, '\0', sizeof(state));
	size_t len = strlen(mbstr);
	wchar_t tmp[1];
	size_t alreadyRead = 0;
	size_t nBytes = 0;
	while ((nBytes = mbrtowc (tmp, mbstr + alreadyRead, len - alreadyRead, &state)) > 0)
	{
		// Invalid input string.
		if (nBytes >= (size_t)-2)
		{
			if (wcstr != NULL)
			{
				wcstr[0] = 0;
			}
			if (pReturnValue != NULL)
			{
				*pReturnValue = 0;
			}
			errno = EILSEQ;
			return errno;
		}
		if (wcstr != NULL)
		{
			*(wcstr + counter) = tmp[0];
		}
		counter ++;
		alreadyRead = alreadyRead + nBytes;
		if ((wcstr != NULL) && (counter >= count))
		{
			break;
		}
	}
	if (wcstr != NULL)
	{
		wcstr[counter] = 0;
	}
	if (pReturnValue != NULL)
	{
		*pReturnValue = counter + 1;
	}

	return 0;
}

char* Intel::OpenCL::Utils::safe_strtok(char* strToken, const char* strDelimit, char** context)
{
	errno = 0;
	if (context == NULL)
	{
		errno = EINVAL;
		return NULL;
	}
	if (strDelimit == NULL)
	{
		errno = EINVAL;
		return NULL;
	}
	if ((strToken == NULL) && (*context == NULL))
	{
		errno = EINVAL;
		return NULL;
	}
	return strtok_r(strToken, strDelimit, context);
}

