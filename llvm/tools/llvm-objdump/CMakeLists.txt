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
  AllTargetsDescs
  AllTargetsDisassemblers
  AllTargetsInfos
  BinaryFormat
  DebugInfoDWARF
  DebugInfoTraceBack # INTEL
  DebugInfoPDB
  Demangle
  MC
  MCDisassembler
  Object
  Option
  Support
  Symbolize
  )

set(LLVM_TARGET_DEFINITIONS ObjdumpOpts.td)
tablegen(LLVM ObjdumpOpts.inc -gen-opt-parser-defs)
add_public_tablegen_target(ObjdumpOptsTableGen)

set(LLVM_TARGET_DEFINITIONS OtoolOpts.td)
tablegen(LLVM OtoolOpts.inc -gen-opt-parser-defs)
add_public_tablegen_target(OtoolOptsTableGen)

add_llvm_tool(llvm-objdump
  llvm-objdump.cpp
  SourcePrinter.cpp
  COFFDump.cpp
  ELFDump.cpp
  MachODump.cpp
  OffloadDump.cpp
  WasmDump.cpp
  XCOFFDump.cpp
  DEPENDS
  ObjdumpOptsTableGen
  OtoolOptsTableGen
  )

target_link_libraries(llvm-objdump PRIVATE LLVMDebuginfod)

if(LLVM_HAVE_LIBXAR)
  target_link_libraries(llvm-objdump PRIVATE ${XAR_LIB})
endif()

add_llvm_tool_symlink(llvm-otool llvm-objdump)

if(LLVM_INSTALL_BINUTILS_SYMLINKS)
  add_llvm_tool_symlink(objdump llvm-objdump)
endif()

if(LLVM_INSTALL_CCTOOLS_SYMLINKS)
  add_llvm_tool_symlink(otool llvm-otool)
endif()
