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

#pragma once

#include "llvm/ADT/StringExtras.h"
#include <cassert>
#include <cl_device_api.h>
#include <cl_monitor.h>
#include <cl_types.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <locale>
#include <map>
#include <math.h>
#include <sstream>
#include <string>
#include <vector>

#define MAKE_HEX_FLOAT(x, y, z) ((float)ldexp((float)(y), z))
#define IS_64_BIT (sizeof(void *) == 8)

// assert macroes:

#define ASSERT_RET(exp, msg)                                                   \
  assert((exp) && msg);                                                        \
  if (!(exp))                                                                  \
    return;

#define ASSERT_RET_VAL(exp, msg, retVal)                                       \
  assert((exp) && msg);                                                        \
  if (!(exp))                                                                  \
    return retVal;

#ifdef WIN32
typedef int threadid_t;
#define INVALID_THREAD_HANDLE ((threadid_t)0)
#else
#include <sched.h>
typedef pid_t threadid_t;
#define INVALID_THREAD_HANDLE ((threadid_t)-1)
#endif

typedef enum { OCL_BUFFER, OCL_IMAGE, OCL_PIPE, OCL_UNKNOWN } OclMemObjectType;

/*******************************************************************************
 * Function:     ClErrTxt
 * Description:    returns a wide-character string of the error code
 * Arguments:    error_code [in] -    error code
 * Return value:    wide-character string - representing the error code
 ******************************************************************************/
const char *ClErrTxt(cl_err_code error_code);

/*******************************************************************************
 * Function:     stringify
 * Description: Turn any type supporting the ostream&<< operator into a string
 * Arguments:    a variable to convert into string
 * Return value:    string
 ******************************************************************************/
template <typename T> inline std::string stringify(const T &x) {
  std::ostringstream out;
  out.precision(std::numeric_limits<T>::digits10);
  out << x;
  return out.str();
}

// Specialization for booleans
//
template <> inline std::string stringify(const bool &b) {
  return b ? "true" : "false";
}

// Specialization for signed and unsigned chars: we want them to be just
// displayed as numbers
//
template <> inline std::string stringify(const unsigned char &c) {
  return stringify(static_cast<unsigned int>(c));
}

template <> inline std::string stringify(const signed char &c) {
  return stringify(static_cast<signed int>(c));
}

template <> inline std::string stringify(const char &c) {
  return stringify(static_cast<signed int>(c));
}

/////////////////////////////////////////////////////////////////////////
// ToNarrow
//  Change any unicode (wchar) string to "regular" string
/////////////////////////////////////////////////////////////////////////
std::string ToNarrow(const wchar_t *s, char dfault = '?',
                     const std::locale &loc = std::locale());

/////////////////////////////////////////////////////////////////////////
// SplitString
//  Split the string using given delimiter
//  Input:
//    string s - the string to split
//    char dlim - the char to use as a delimiter
/////////////////////////////////////////////////////////////////////////
std::vector<std::string> &SplitString(const std::string &s, char delim,
                                      std::vector<std::string> &elems);
std::vector<std::string> &
SplitStringAllowEmpty(const std::string &s, char delim,
                      std::vector<std::string> &elems);
std::vector<std::string> SplitString(const std::string &s, char delim);

/// Split the string using given delimiter and all elements are integer.
/// Convert elements to a vector to integers.
/// Return true if split is successful, false if there is no integer in the
/// string.
template <typename T>
bool SplitStringInteger(const std::string &s, char delim,
                        std::vector<T> &elems) {
  std::vector<std::string> strs = SplitString(s, delim);
  size_t elemsCount = strs.size();
  if (0 == elemsCount)
    return false;
  elems.resize(elemsCount);
  for (size_t i = 0; i < elemsCount; ++i) {
    if (!llvm::to_integer(strs[i], elems[i])) {
      elems.clear();
      return false;
    }
  }
  return true;
}

/*******************************************************************************
* Function:     XXXtoString
* Description:    Turn some kind of OpenCL objects defines into strings
                                Note: can not overload stringfy because most of
the object are merely a typedefs
* Arguments:    a variable to convert into string
* Return value:    string
*******************************************************************************/
const string channelOrderToString(const cl_channel_order &co);
const string channelTypeToString(const cl_channel_type &ct);

/*******************************************************************************
 * Function:     clIsNumaAvailable
 * Description:    Checks if machine supports NUMA
 * Return value:    bool
 ******************************************************************************/
bool clIsNumaAvailable();

/*******************************************************************************
 * Function:     clNUMASetLocalNodeAlloc
 * Description:  Set prefered node for memory allocation for the calling thread
 * Return value: void
 ******************************************************************************/
void clNUMASetLocalNodeAlloc();

/*******************************************************************************
 * Function:     clSleep
 * Description:    put the calling thread on sleep for time 'milliseconds'
 * Arguments:    milliseconds [ int ] - duration of required thread sleep
 * Return value:    void
 ******************************************************************************/
void clSleep(int milliseconds);

/*******************************************************************************
 * Function:     clSetThreadAffinityMask
 * Description:    sets the given affinity mask
 * Arguments:    mask [ affinityMask_t* ] - a pointer to the mask to use
 * Return value:    void
 ******************************************************************************/
void clSetThreadAffinityMask(affinityMask_t *mask, threadid_t tid = 0);

/*******************************************************************************
 * Function:     clGetThreadAffinityMask
 * Description:    retrieves the given thread affinity mask
 * Arguments:    mask [ affinityMask_t* ] - a pointer to the mask to be set
 * Return value:    void
 ******************************************************************************/
void clGetThreadAffinityMask(affinityMask_t *mask, threadid_t tid = 0);

/*******************************************************************************
 * Function:     clTranslateAffinityMask
 * Description:    fills the given array of unsigned ints with the values
 *corresponding to set bits in the mask Arguments:    mask [ affinityMask_t*
 *] - a pointer to the mask to use IDs  [ unsigned int*   ] - the array to fill
 *               len  [ size_t          ] - the size of the array
 * Return value:    bool
 ******************************************************************************/
bool clTranslateAffinityMask(affinityMask_t *mask, unsigned int *arr,
                             size_t len);

/*******************************************************************************
 * Function:     clSetThreadAffinityToCore
 * Description:    affinitizes the calling thread to the requested core
 * Arguments:    core [ unsigned int ] - the index of the core
 * Return value:    void
 ******************************************************************************/
void clSetThreadAffinityToCore(unsigned int core, threadid_t tid = 0);

/*******************************************************************************
 * Function:     clResetThreadAffinityMask
 * Description:    allows the thread to run on any CPU in the process
 * Return value:    void
 ******************************************************************************/
void clResetThreadAffinityMask(threadid_t tid = 0);

/*******************************************************************************
 * Function:     clMyThreadId
 * Description:    returns the caller's OS-specific tid
 * Return value:    threadid_t
 ******************************************************************************/
threadid_t clMyThreadId();

/*******************************************************************************
 * Function:     clMyParentThreadId
 * Description:    returns the caller's parent thread's OS-specific tid
 * Return value:    threadid_t
 ******************************************************************************/
threadid_t clMyParentThreadId();

/*******************************************************************************
 * Function:     clCopyMemoryRegion
 * Description:    Copies memory region defined in SMemCpyParams
 * Return value:    void
 ******************************************************************************/
struct SMemCpyParams {
  cl_uint uiDimCount;
  const cl_char *pSrc;
  size_t vSrcPitch[MAX_WORK_DIM - 1];
  cl_char *pDst;
  size_t vDstPitch[MAX_WORK_DIM - 1];
  size_t vRegion[MAX_WORK_DIM];
};

void clCopyMemoryRegion(SMemCpyParams *pCopyCmd);

/************************************************************************
 * Function:        clGetPixelBytesCount
 * Parameters:
 *  pclImageFormat  a cl_image_format
 * Return value:    the size in bytes of each pixel in this image format
 ************************************************************************/
size_t clGetPixelBytesCount(const cl_image_format *pclImageFormat);

/************************************************************************
 * Function:        clGetPixelElementsCount
 * Parameters:
 *  pclImageFormat  a cl_image_format
 * Return value:    the element number in a pixel of the specified order
 ************************************************************************/
size_t clGetPixelElementsCount(const cl_image_format *pclImageFormat);

/************************************************************************
 * Function:        clGetChannelTypeBytesCount
 * Parameters:
 *  pclImageChannelType  a cl_channel_type
 * Return value:    the size in bytes of each element in an image of the
 *                  specified channel type
 ************************************************************************/
size_t clGetChannelTypeBytesCount(cl_channel_type pclImageChannelType);

/////////////////////////////////////////////////////////////////////////
// float2half_rte
// Parameters: float
// Output: half
//
/////////////////////////////////////////////////////////////////////////
cl_ushort float2half_rte(float f);

/////////////////////////////////////////////////////////////////////////
// half2float
// Parameters: half
// Output: float
//
/////////////////////////////////////////////////////////////////////////
float half2float(cl_ushort us);

// Returns the number of leading 0-bits in x,
// starting at the most significant bit position.
// If x is 0, the result is undefined.
//
int internal_clz(unsigned int pattern);

/**
 * Copy a pattern repeatedly to a buffer
 * @param pPattern        the pattern
 * @param szPatternSize the pattern's size
 * @param pBuffer        a pointer to the buffer
 * @param szBufferSize  the buffer's size
 */
void CopyPattern(const void *pPattern, size_t szPatternSize, void *pBuffer,
                 size_t szBufferSize);

/**
 * @return the file path of the configuration file
 */
std::string GetConfigFilePath();

/**
 * @fetch a registry entry 'valueName' into 'retValue' buffer of size 'size'
 */
#ifdef WIN32
bool GetStringValueFromRegistryOrETC(HKEY top_hkey, const char *keyPath,
                                     const char *valueName, char *retValue,
                                     DWORD size);
#endif

/**
 * @return the text content of filePath
 */
string ReadFileContents(const string &filePath);

// Identify the node on which the calling thread is current running.
// The return value should lie in the range [0, max_node_count).
unsigned getCpuNodeId();

// Identify the processor on which the calling thread is current running.
// The return value should lie in the range [0, max_processor_count).
unsigned getHWThreadId();
