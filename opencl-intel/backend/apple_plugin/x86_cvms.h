/*******************************************************************************
 * Copyright:  (c) 2007-2011 by Apple, Inc., All Rights Reserved.
 ******************************************************************************/

#ifndef __X86_CVMS_H
#define __X86_CVMS_H

#include <CoreFoundation/CoreFoundation.h>
#include <OpenGL/cl_driver_types.h>
#include <stdint.h>

#include <cvms/client_server.h> // for cvms_architecture_t, below.  This field is not used and should be removed.

#define CLD_CVMS_CPU_VERSION 0x0102
#define DEBUG_CVMS 0

typedef struct _cvmsCPUReturnData {
  int32_t err;
  unsigned offsets[3];
  unsigned char data[0];
} cvmsCPUReturnData;

typedef struct _cvmsKeys {
  cld_comp_opt opt;
  cvms_architecture_t arch;
} cvmsKeys;

enum {
  cvmsSrcIdxKeys,
  cvmsSrcIdxData,
  cvmsSrcIdxCount
};

enum {
  cvmsStringNumSource,
  cvmsStringOffsetCacheSalt,
  cvmsStringOffsetSource,
  cvmsStringOffsetOptions,
  cvmsStringOffsetNum
};

typedef struct _cvmsFunctionBuildData {
  unsigned int offsets[cvmsStringOffsetNum];
  char data[0];
} cvmsStrings;

#endif  // __X86_CVMS_H
