//===--------------- Intel_DopeVectorTypeInfo.h -----------------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Provides utility functions for dope vector types using named metadata from
// the Fortran front end.
//

//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_DOPEVECTORTYPEINFO_H
#define LLVM_IR_INTEL_DOPEVECTORTYPEINFO_H

#include "llvm/ADT/MapVector.h"
#include "llvm/IR/Module.h"

namespace llvm {

class DopeVectorTypeInfo {
  using DopeVectorTypeMapType = MapVector<const Type *, const Type *>;

  // A map from a dope vector type to its element type.
  DopeVectorTypeMapType DopeVectorTypeMap;

  // Build the DopeVectorTypeMap from Fortran front end metadata.
  // Here is an example of the Fortran front end metadata used to
  // construct the map:
  // !ifx.types.dv = !{!1, !2, !3, !4}
  // !1 = !{%"QNCA_a0$i32*$rank1$" zeroinitializer, i32 0}
  // !2 = !{%"QNCA_a0$i8*$rank1$" zeroinitializer, ptr null}
  // !3 = !{%"QNCA_a0$double*$rank2$" zeroinitializer, double 0.000000e+00}
  // !4 = !{%"QNCA_a0$DECOMPMODULE$.btLISTS*$rank1$" zeroinitializer,
  //        %"DECOMPMODULE$.btLISTS" zeroinitializer}
  // Type 1: Dope vector type with i32 element type.
  // Type 2: Dope vector type with ptr element type.
  // Type 3: Dope vector type with double element type.
  // Type 4: Dope vector type with struct %"DECOMPMODULE$.btLISTS"
  //         element type.
  void loadDopeVectorTypeMap(Module &M);

  // Is 'true' if after calling initializeDopeVectorTypeMap() a correctly
  // formed 'DopeVectorTypeMap' was created.
  bool MapCorrectlyInitialized;

public:
  DopeVectorTypeInfo(Module &M) { loadDopeVectorTypeMap(M); };
  ~DopeVectorTypeInfo(){};

  // Append dope vector info from the metadata of 'M'.
  void appendToDopeVectorTypeMap(Module &M) { loadDopeVectorTypeMap(M); }

  // Returns 'true' if 'Ty' is a dope vector type.
  bool isDopeVectorType(const Type *Ty);

  // Return the element type associated with 'Ty' if 'Ty' is a dope
  // vector type. Otherwise, return 'nullptr'.
  const Type *getDopeVectorElementType(const Type *Ty);

  // Dump used for debugging and LIT tests.
  void print(void);
};

} // end namespace llvm

#endif // LLVM_IR_INTEL_DOPEVECTORTYPEINFO_H
