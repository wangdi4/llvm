/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "CompilationUtils.h"
#include "NameMangleAPI.h"

#include "llvm/IR/Metadata.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DebugInfo.h"

#include "llvm/IR/DataLayout.h"
#include "llvm/ADT/SetVector.h"

namespace Intel {

  const std::string CompilationUtils::NAME_GET_GID = "get_global_id";
  const std::string CompilationUtils::NAME_GET_BASE_GID = "get_base_global_id.";
  const std::string CompilationUtils::NAME_GET_GLOBAL_SIZE = "get_global_size";
  const std::string CompilationUtils::NAME_GET_GLOBAL_OFFSET = "get_global_offset";

  static bool isOptionalMangleOf(const std::string& LHS, const std::string& RHS) {
    //LHS should be mangled
    const char* const LC = LHS.c_str();
    if (!isMangledName(LC))
      return LHS == RHS; /* return false; */ /* xmain (until mangling is back) */
    return stripName(LC) == RHS;
  }

  std::string CompilationUtils::mangledGetGID() {
    return NAME_GET_GID.c_str();
  }

  std::string CompilationUtils::mangledGetGlobalSize() {
    return NAME_GET_GLOBAL_SIZE.c_str();
  }

  std::string CompilationUtils::mangledGetGlobalOffset() {
    return NAME_GET_GLOBAL_OFFSET.c_str();
  }

  bool CompilationUtils::isGetGlobalId(const std::string& S) {
    return isOptionalMangleOf(S, NAME_GET_GID);
  }

  bool CompilationUtils::isGetGlobalSize(const std::string& S) {
    return isOptionalMangleOf(S, NAME_GET_GLOBAL_SIZE);
  }

  bool CompilationUtils::isGlobalOffset(const std::string& S) {
    return isOptionalMangleOf(S, NAME_GET_GLOBAL_OFFSET);
  }

}
