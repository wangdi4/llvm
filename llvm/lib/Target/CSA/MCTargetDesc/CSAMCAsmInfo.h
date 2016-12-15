//===-- CSAMCAsmInfo.h - CSA asm properties --------------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source 
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the CSAMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_MCTARGETDESC_CSAMCASMINFO_H
#define LLVM_LIB_TARGET_CSA_MCTARGETDESC_CSAMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
  class Triple;

  class CSAMCAsmInfo : public MCAsmInfoELF {
    void anchor() override;
  public:
    explicit CSAMCAsmInfo(const Triple &T);
  };

} // namespace llvm

#endif
