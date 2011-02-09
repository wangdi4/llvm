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

#pragma once

#include <string.h>
#include <wchar.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include "cl_sys_defines.h"

namespace Intel { namespace OpenCL { namespace Utils {

	/* Safe copy of source string into destination string.
	   dst - location of destination string.
	   src - location of source string.
	   len - size of destination string buffer.
	   If src or dst are NULL pointer than EINVAL return.
	   If len is less or equal to 0 than ERANGE return.
	   If src size is greater than or equal to len, than dst doesn't modify and ERANGE return.
	   If src size is smaller than len, this function pads the remainder of dst with null bytes ("\0").
	   On success 0 is returned.
	*/
	errno_t safeStrCpy(char* dst, size_t len, const char* src );

	/* Safe copy of source wide character string into destination wide character string.
	   dst - location of destination string.
	   src - location of source string.
	   len - size of destination string buffer.
	   If src or dst are NULL pointer than EINVAL return.
	   If len is less or equal to 0 than ERANGE return.
	   If src size is greater than or equal to len, than dst doesn't modify and ERANGE return.
	   If src size is smaller than len, this function pads the remainder of dst with null bytes (L"\0").
	   On success 0 is returned.
	*/
	errno_t safeWStrCpy(wchar_t* dst, size_t len, const wchar_t* src );

	/* Safe append of source string into destination string.
	   dst - location of destination string.
	   src - location of source string.
	   len - the total size of dst buffer, not the remaining size.
	   If src or dst are NULL pointer than EINVAL return.
	   If len is less or equal to 0 than ERANGE return.
	   If src size is greater than or equal to remaining place in dst buffer, than dst doesn't modify and ERANGE return.
	   If src size is smaller than remaining place in dst buffer, this function pads the remainder of dst with null bytes ("\0").
	   On success 0 is returned.
	*/
	errno_t safeStrCat(char* dst, size_t len, const char* src );

	/* Safe append of source wide character string into destination wide character string.
	   dst - location of destination string.
	   src - location of source string.
	   len - the total size of dst buffer, not the remaining size.
	   If src or dst are NULL pointer than EINVAL return.
	   If len is less or equal to 0 than ERANGE return.
	   If src size is greater than or equal to remaining place in dst buffer, than dst doesn't modify and -ERANGE return.
	   If src size is smaller than remaining place in dst buffer, this function pads the remainder of dst with null bytes (L"\0").
	   On success 0 is returned.
	*/
	errno_t safeWStrCat(wchar_t* dst, size_t len, const wchar_t* src );

	/* Write formated data to a string.
	   dst - location of destination string.
	   len - size of destination string buffer.
	   format - format control string.
	   ... - optional arguments.
	   If src or dst are NULL pointer than -1 return and errno sets to EINVAL.
	   If len is less or equal to 0 than -1 return and errno sets to ERANGE.
	   If the buffer is too small for the text being printed then the buffer is set to an empty string and -1 returned and errno sets to ERANGE.
	   Return the number of characters written, or –1 if an error occurred.
	*/
	int safeStrPrintf(char* dst, size_t len, const char* format, ...);

	/* Write formated data to a wide character string.
	   dst - location of destination string.
	   len - size of destination string buffer.
	   format - format control string.
	   ... - optional arguments.
	   If src or dst are NULL pointer than -1 return and errno sets to EINVAL.
	   If len is less or equal to 0 than -1 return and errno sets to ERANGE.
	   If the buffer is too small for the text being printed then the buffer is set to an empty string and -1 returned and errno sets to ERANGE.
	   Return the number of characters written, or –1 if an error occurred.
	*/
	int safeWStrPrintf(wchar_t* dst, size_t len, const wchar_t* format, ...);

	/* Same behaviour as vsnprintf. Also check that dst, format are not NULL pointer and that len is greater than 0.
	   On error return negative integer and errno set accordingly.
	   If success return vsnprintf returned value.
	*/
	int safeVStrPrintf(char* dst, size_t len, const char* format, va_list args);

	/* Copies bytes between buffers.
	   dst - location of destination buffer.
	   numOfElements - Size of the destination buffer.
	   src - Buffer to copy from.
	   count - Number of characters to copy.
	   If src or dst are NULL pointer than EINVAL return (dst doesn't modify).
	   If numOfElements < count than ERANGE return (dst doesn't modify).
	   On success 0 is returned.
	*/
	errno_t safeMemCpy(void* dst, size_t numOfElements, const void* src, size_t count);

	/* Converts a sequence of multibyte characters to a corresponding sequence of wide characters.
	   pReturnValue - The number of characters converted.
	   wcstr - Address of buffer for the resulting converted wide character string.
	   sizeInWords - The size of the wcstr buffer in words.
	   mbstr - The address of a sequence of null terminated multibyte characters.
	   count - The maximum number of wide characters to store in the wcstr buffer, not including the terminating null.
	   If wcstr is NULL and sizeInWords > 0, return ERANGE.
	   If mbstr is NULL, return EINVAL.
	   If the destination buffer is too small to contain the converted string, return ERANGE.
	   The mbstowcs_s function converts a string of multibyte characters pointed to by mbstr into wide characters stored in the buffer pointed to by wcstr. The conversion will continue for each character until one of these conditions is met:
	   * A multibyte null character is encountered.
	   * An invalid multibyte character is encountered.
	   * The number of wide characters stored in the wcstr buffer equals count.
	   The destination string is always null-terminated (even in the case of an error).
	   If safeMbToWc successfully converts the source string, it puts the size in wide characters of the converted string, including the null terminator, into *pReturnValue (provided pReturnValue is not NULL). This occurs even if the wcstr argument is NULL and provides a way to determine the required buffer size. Note that if wcstr is NULL, count is ignored.
	   If safeMbToWc encounters an invalid multibyte character, it puts 0 in *pReturnValue, sets the destination buffer to an empty string and return EILSEQ.
	*/
	errno_t safeMbToWc(size_t* pReturnValue, wchar_t* wcstr, size_t sizeInWords, const char* mbstr, size_t count);

	/* The safe_strtok() function parses a string into a sequence of tokens.  On the first call to safe_strtok() the string to be parsed should be specified in strToken.
           In each subsequent call that should parse the same string, strToken should be NULL.
           The strDelimit argument specifies a set of characters that delimit the tokens in the parsed string.  The caller may specify different strings in strDelimit in
           successive calls that parse the same string.
	   Each call to safe_strtok() returns a pointer to a null-terminated string containing the next token.  This string does not include the delimiting character.
	   If no more tokens are found, safe_strtok() returns NULL.
	   The context argument is a pointer to a char * variable that is used internally by strtok_r() in order to maintain context between successive calls that parse the same string.
          On the first call to safe_strtok(), strToken should point to the string to be parsed, and the value of context is ignored.  In subsequent calls,  strToken  should
          be NULL, and context should be unchanged since the previous call.
          Different strings may be parsed concurrently using sequences of calls to asfe_strtok() that specify different context arguments.
	  If context is NULL, errno sets to EINVAL and NULL pointer is return.
	  If strDelimit is NULL, errno sets to EINVAL and NULL pointer is return.
	  If strToken is NULL and context point to NULL pointer, errno sets to EINVAL and NULL pointer is return.
          On successfull, safe_strtok() return a pointer to the next token, or NULL if there are no more tokens.
	*/
	char* safe_strtok(char* strToken, const char* strDelimit, char** context);

}}}

