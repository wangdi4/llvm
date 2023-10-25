# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2023 Intel Corporation
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

add_llvm_component_library(LLVMInstrumentation
  AddressSanitizer.cpp
  BoundsChecking.cpp
  CGProfile.cpp
  ControlHeightReduction.cpp
  DataFlowSanitizer.cpp
  GCOVProfiling.cpp
  BlockCoverageInference.cpp
  MemProfiler.cpp
  MemorySanitizer.cpp
  IndirectCallPromotion.cpp
  Instrumentation.cpp
  InstrOrderFile.cpp
  InstrProfiling.cpp
# INTEL_CUSTOMIZATION
  Intel_FunctionSplitting.cpp
# INTEL_FEATURE_MARKERCOUNT
  Intel_MarkerCountIntrinsicInserter.cpp
# end INTEL_FEATURE_MARKERCOUNT
  Intel_SPIEmitter.cpp
# end INTEL_CUSTOMIZATION
  KCFI.cpp
  PGOInstrumentation.cpp
  PGOMemOPSizeOpt.cpp
  PoisonChecking.cpp
  SanitizerCoverage.cpp
  SPIRITTAnnotations.cpp
  SanitizerBinaryMetadata.cpp
  ValueProfileCollector.cpp
  ThreadSanitizer.cpp
  HWAddressSanitizer.cpp
# INTEL_CUSTOMIZATION
  Intel_MLPGO/ExtractFeatures.cpp
  Intel_MLPGO/Inference.cpp
# end INTEL_CUSTOMIZATION
  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  Analysis
  Core
  Demangle
  MC
  Support
  TargetParser
  TransformUtils
  ProfileData
# INTEL_CUSTOMIZATION
  MLPGOMLBackend
# end INTEL_CUSTOMIZATION
  )
