//===-- LPUMCAsmInfo.h - LPU asm properties --------------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source 
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the LPUMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LPU_MCTARGETDESC_LPUMCASMINFO_H
#define LLVM_LIB_TARGET_LPU_MCTARGETDESC_LPUMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
  class Triple;

  class LPUMCAsmInfo : public MCAsmInfoELF {
    void anchor() override;
  public:
    explicit LPUMCAsmInfo(const Triple &T);
  };

} // namespace llvm

#endif
