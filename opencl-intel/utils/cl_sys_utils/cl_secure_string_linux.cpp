// INTEL CONFIDENTIAL
//
// Copyright 2007 Intel Corporation.
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

#include "cl_secure_string_linux.h"

using namespace Intel::OpenCL::Utils;

errno_t Intel::OpenCL::Utils::safeStrCpy(char *dst, size_t len,
                                         const char *src) {
  errno = 0;
  if ((src == nullptr) || (dst == nullptr)) {
    errno = EINVAL;
    return errno;
  }
  if ((len <= 0) || (strnlen(src, len) >= len)) {
    errno = ERANGE;
    return errno;
  }
  strncpy(dst, src, len);
  return 0;
}

errno_t Intel::OpenCL::Utils::safeStrNCpy(char *dst, size_t len,
                                          const char *src, size_t src_len) {
  errno = 0;
  if ((src == nullptr) || (dst == nullptr)) {
    errno = EINVAL;
    return errno;
  }
  if ((len <= 0) || (strnlen(src, src_len) >= len)) {
    errno = ERANGE;
    return errno;
  }
  strncpy(dst, src, src_len);
  return 0;
}

errno_t Intel::OpenCL::Utils::safeStrCat(char *dst, size_t len,
                                         const char *src) {
  errno = 0;
  if ((src == nullptr) || (dst == nullptr)) {
    errno = EINVAL;
    return errno;
  }
  if (len <= 0) {
    errno = ERANGE;
    return errno;
  }
  size_t tRemain = len - strnlen(dst, len);
  if (strnlen(src, len) >= tRemain) {
    errno = ERANGE;
    return errno;
  }
  strncat(dst, src, tRemain - 1);
  return 0;
}

int Intel::OpenCL::Utils::safeStrPrintf(char *dst, size_t len,
                                        const char *format, ...) {
  errno = 0;
  if ((format == nullptr) || (dst == nullptr)) {
    errno = EINVAL;
    return -1;
  }
  if (len <= 0) {
    errno = ERANGE;
    return -1;
  }
  va_list src;
  va_start(src, format);
  size_t tSrcOrgSize = vsnprintf(dst, len, format, src);
  va_end(src);
  if (tSrcOrgSize >= len) {
    dst[0] = '\0';
    errno = ERANGE;
    return -1;
  }
  return tSrcOrgSize;
}

int Intel::OpenCL::Utils::safeVStrPrintf(char *dst, size_t len,
                                         const char *format, va_list args) {
  errno = 0;
  if ((format == nullptr) || (dst == nullptr)) {
    errno = EINVAL;
    return -1;
  }
  if (len <= 0) {
    errno = ERANGE;
    return -1;
  }
  return vsnprintf(dst, len, format, args);
}

errno_t Intel::OpenCL::Utils::safeMemCpy(void *dst, size_t numOfElements,
                                         const void *src, size_t count) {
  errno = 0;
  if ((src == nullptr) || (dst == nullptr)) {
    errno = EINVAL;
    return errno;
  }
  if ((count <= 0) || (numOfElements < count)) {
    errno = ERANGE;
    return errno;
  }
  if (dst == src) {
    return 0;
  }
  memcpy(dst, src, count);
  return 0;
}

char *Intel::OpenCL::Utils::safe_strtok(char *strToken, const char *strDelimit,
                                        char **context) {
  errno = 0;
  if (context == nullptr) {
    errno = EINVAL;
    return nullptr;
  }
  if (strDelimit == nullptr) {
    errno = EINVAL;
    return nullptr;
  }
  if ((strToken == nullptr) && (*context == nullptr)) {
    errno = EINVAL;
    return nullptr;
  }
  return strtok_r(strToken, strDelimit, context);
}
