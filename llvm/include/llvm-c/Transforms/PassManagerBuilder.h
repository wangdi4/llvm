/*===-- llvm-c/Transform/PassManagerBuilder.h - PMB C Interface ---*- C -*-===*\ */
/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Modifications, Copyright (C) 2022 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may not
 * use, modify, copy, publish, distribute, disclose or transmit this software or
 * the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */
/* end INTEL_CUSTOMIZATION */
/*                                                                            *|
|* Part of the LLVM Project, under the Apache License v2.0 with LLVM          *|
|* Exceptions.                                                                *|
|* See https://llvm.org/LICENSE.txt for license information.                  *|
|* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception                    *|
|*                                                                            *|
|*===----------------------------------------------------------------------===*|
|*                                                                            *|
|* This header declares the C interface to the PassManagerBuilder class.      *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifndef LLVM_C_TRANSFORMS_PASSMANAGERBUILDER_H
#define LLVM_C_TRANSFORMS_PASSMANAGERBUILDER_H

#include "llvm-c/ExternC.h"
#include "llvm-c/Types.h"

typedef struct LLVMOpaquePassManagerBuilder *LLVMPassManagerBuilderRef;

LLVM_C_EXTERN_C_BEGIN

/**
 * @defgroup LLVMCTransformsPassManagerBuilder Pass manager builder
 * @ingroup LLVMCTransforms
 *
 * @{
 */

/** See llvm::PassManagerBuilder. */
LLVMPassManagerBuilderRef LLVMPassManagerBuilderCreate(void);
void LLVMPassManagerBuilderDispose(LLVMPassManagerBuilderRef PMB);

/** See llvm::PassManagerBuilder::OptLevel. */
void
LLVMPassManagerBuilderSetOptLevel(LLVMPassManagerBuilderRef PMB,
                                  unsigned OptLevel);

/** See llvm::PassManagerBuilder::SizeLevel. */
void
LLVMPassManagerBuilderSetSizeLevel(LLVMPassManagerBuilderRef PMB,
                                   unsigned SizeLevel);

/** See llvm::PassManagerBuilder::DisableUnitAtATime. */
void
LLVMPassManagerBuilderSetDisableUnitAtATime(LLVMPassManagerBuilderRef PMB,
                                            LLVMBool Value);

/** See llvm::PassManagerBuilder::DisableUnrollLoops. */
void
LLVMPassManagerBuilderSetDisableUnrollLoops(LLVMPassManagerBuilderRef PMB,
                                            LLVMBool Value);

/** See llvm::PassManagerBuilder::DisableSimplifyLibCalls */
void
LLVMPassManagerBuilderSetDisableSimplifyLibCalls(LLVMPassManagerBuilderRef PMB,
                                                 LLVMBool Value);

/** See llvm::PassManagerBuilder::Inliner. */
void
LLVMPassManagerBuilderUseInlinerWithThreshold(LLVMPassManagerBuilderRef PMB,
                                              unsigned Threshold);

/** See llvm::PassManagerBuilder::populateFunctionPassManager. */
void
LLVMPassManagerBuilderPopulateFunctionPassManager(LLVMPassManagerBuilderRef PMB,
                                                  LLVMPassManagerRef PM);

/** See llvm::PassManagerBuilder::populateModulePassManager. */
void
LLVMPassManagerBuilderPopulateModulePassManager(LLVMPassManagerBuilderRef PMB,
                                                LLVMPassManagerRef PM);

#if INTEL_CUSTOMIZATION
/** See llvm::PassManagerBuilder::populateLTOPassManager. */
void LLVMPassManagerBuilderPopulateLTOPassManager(LLVMPassManagerBuilderRef PMB,
                                                  LLVMPassManagerRef PM,
                                                  LLVMBool Internalize,
                                                  LLVMBool RunInliner);
#endif // INTEL_CUSTOMIZATION

/**
 * @}
 */

LLVM_C_EXTERN_C_END

#endif
