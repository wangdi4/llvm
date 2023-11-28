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
set(LLVM_LINK_COMPONENTS
  Core
  Demangle # INTEL
  Support
  Object
  Coverage
  ProfileData
  TargetParser
  )

add_llvm_tool(llvm-cov
  llvm-cov.cpp
  gcov.cpp
  CodeCoverage.cpp
  CoverageExporterJson.cpp
  CoverageExporterLcov.cpp
  CoverageFilters.cpp
  CoverageReport.cpp
  CoverageSummaryInfo.cpp
#INTEL_CUSTOMIZATION
  Intel_CoverageReportXML.cpp
#end INTEL_CUSTOMIZATION
  SourceCoverageView.cpp
  SourceCoverageViewHTML.cpp
  SourceCoverageViewText.cpp
  TestingSupport.cpp
  )

target_link_libraries(llvm-cov PRIVATE LLVMDebuginfod)
