# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2021 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may not
# use, modify, copy, publish, distribute, disclose or transmit this software or
# the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.
#
# end INTEL_CUSTOMIZATION
set(LLVM_LINK_COMPONENTS
  AllTargetsAsmParsers
  AllTargetsCodeGens
  AllTargetsDescs
  AllTargetsInfos
  Analysis
  BitReader
  BitWriter
  CodeGen
  Core
  Coroutines
  IPO
  IRReader
  AggressiveInstCombine
  InstCombine
  Instrumentation
  FuzzMutate
  MC
  ObjCARCOpts
  ScalarOpts
  Support
  Target
  TransformUtils
  Vectorize
  Passes
  SYCLLowerIR
)

add_llvm_fuzzer(llvm-opt-fuzzer
  llvm-opt-fuzzer.cpp
  DUMMY_MAIN DummyOptFuzzer.cpp
  )
