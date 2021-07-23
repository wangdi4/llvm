//===---- Intel_VTableFixup.h -----------------------------------*- C++ -*-===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass removes references to unresolved symbols from the initializers
// of typeinfo and vtable data structures.
//
// This pass is used only for OpenMP SPIR-V offload right now.
// OpenMP offload allows not compiling/defining some functions from the user
// code for the device offload. It is illegal to call such functions from
// the device code, so users may not introduce references to such symbols.
// It is a different story with virtual methods of C++ classes.
// C++ FE may need to generate an abstract C++ class' vtable in the device
// code implicitly (for example, vtable will be generated if user calls
// a constructor of this class in the device code). This vtable is a global
// constant variable with an initializer that references all virtual methods
// of this class, e.g.:
//   @_ZTV4Base = linkonce_odr hidden unnamed_addr addrspace(1) constant
//       { [4 x i8 addrspace(4)*] } {
//           [4 x i8 addrspace(4)*] [
//               i8 addrspace(4)* null,
//               i8 addrspace(4)* addrspacecast (
//                   i8 addrspace(1)* bitcast (
//                       { i8 addrspace(4)*, i8 addrspace(4)* } addrspace(1)*
//                           @_ZTI4Base
//                       to i8 addrspace(1)*)
//                   to i8 addrspace(4)*),
//               i8 addrspace(4)* addrspacecast (
//                   i8* bitcast (
//                       void (%struct.Base addrspace(4)*)* @_ZN4BaseD1Ev
//                       to i8*)
//                   to i8 addrspace(4)*),
//               i8 addrspace(4)* addrspacecast (
//                   i8* bitcast (
//                       void (%struct.Base addrspace(4)*)* @_ZN4BaseD0Ev
//                       to i8*)
//                   to i8 addrspace(4)*)
//           ]
//       }, comdat, align 8
//
// So C++ FE may implicitly introduce references to virtual methods that user
// does not want to define or cannot define in the device code (e.g. these
// methods cannot be compiled for the device environment).
//
// In order to still allow using vtables for dispatching virtual function calls,
// the compiler has to make sure that there are no unresolved references
// in vtable initializers. This pass goes through initializers of vtable
// global variables and replaces references to symbols not defined in this
// module with null values.
//
// This pass can only be run on the whole program, and here is why.
// Consider a case, where two C++ modules declare a vtable for the same
// abstract class. The two initializers will reference the same virtual method.
// Next, this method is defined for the device compilation in one of these
// modules. If we run this pass on the two modules separately, then
// in one of the modules the vtable initializer will reference the virtual
// method, and in another module the reference will be replaced with null value
// (since there is no definition of the method there). This will cause the same
// linkonce_odr variable to be defined with different initializers,
// which is illegal.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_VTABLEFIXUP_H
#define LLVM_TRANSFORMS_IPO_INTEL_VTABLEFIXUP_H

#include "llvm/IR/PassManager.h"

namespace llvm {
class IntelVTableFixupPass : public PassInfoMixin<IntelVTableFixupPass> {
public:
  PreservedAnalyses run(Module &, ModuleAnalysisManager &);
};
} // namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_VTABLEFIXUP_H
