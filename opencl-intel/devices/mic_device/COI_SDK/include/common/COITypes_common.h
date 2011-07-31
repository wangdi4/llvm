/* ************************************************************************* *\
                  INTEL CORPORATION PROPRIETARY INFORMATION
      This software is supplied under the terms of a license agreement or
      nondisclosure agreement with Intel Corporation and may not be copied
      or disclosed except in accordance with the terms of that agreement.
          Copyright (C) 2010-2011 Intel Corporation. All Rights Reserved.
\* ************************************************************************* */
#ifndef COITYPES_COMMON_H
#define COITYPES_COMMON_H

/** @ingroup COITypes
 *  @addtogroup COITypesSource
@{

* @file common/COITypes_common.h
*/
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include <stdint.h>
#endif // DOXYGEN_SHOULD_SKIP_THIS

#ifdef __cplusplus
extern "C" {
#endif


struct coibarrier  { uint64_t opaque[2]; };

typedef struct coiprocess  * COIPROCESS;
typedef struct coipipeline * COIPIPELINE;
typedef struct coifunction * COIFUNCTION;
typedef struct coiengine   * COIENGINE;
typedef struct coibarrier    COIBARRIER;
typedef struct coibuffer   * COIBUFFER;
typedef struct coilibrary  * COILIBRARY;
typedef struct coimapinst  * COIMAPINSTANCE;

typedef uint32_t COI_CPU_MASK[8];

#ifdef __cplusplus
}
#endif

#endif /* COITYPES_COMMON_H */
