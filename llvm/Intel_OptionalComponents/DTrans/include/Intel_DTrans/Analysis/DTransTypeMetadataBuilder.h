//===---------DTransTypeMetadataBuilder.h --Builder for DTrans metadata ---===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the DTransTypeMetadataBuilder class, which is used as a
// convenient way to set DTrans type metadata
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error DTransTypeMetadataBuilder.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSTYPEMETADATABUILDER_H
#define INTEL_DTRANS_ANALYSIS_DTRANSTYPEMETADATABUILDER_H

#include "llvm/ADT/ArrayRef.h"

namespace llvm {
class AttributeSet;
class Function;
class LLVMContext;
class Metadata;
class MDNode;
class MDTuple;
class Value;

namespace dtransOP {

class DTransFunctionType;

//
// This class defines some methods that help with creating & setting DTrans type
// metadata.
//
class DTransTypeMetadataBuilder {
public:
  // Set the metadata on the Value using an appropriate tag based on the Value
  // type. If MD is nullptr, clear any existing DTrans metadata from the object.
  static void addDTransMDNode(Value &V, MDNode *MD);

  // Remove any existing DTransFuncIndex metadata on 'F'. And set the return and
  // argument attributes for the DTrans type metadata based on the function type
  // defined by 'FnType'
  static void setDTransFuncMetadata(Function *F, DTransFunctionType *FnType);

  // Copy any DTrans type attributes and DTrans type metadata that is on 'SrcF'
  // to 'DstF'. This function should only be used when the destination function
  // has the same signature as the source function.
  static void copyDTransFuncMetadata(Function* SrcF, Function *DstF);

  // Create metadata to represent a literal structure based on a list of
  // metadata nodes contained in 'MDTypeList'
  //
  // Metadata for a literal structure looks like:
  //     !{!"L", i32 <numElem>, !MDNodefield1, !MDNodefield2, ...}
  static MDTuple *createLiteralStructMetadata(LLVMContext &Ctx,
                                              ArrayRef<Metadata *> MDTypeList);
};

//
// This class defines some convenience methods for working with the
// "intel_dtrans_func_index" attribute on function parameters.
//
class DTransTypeAttributeUtil {
public:
  // Extract the index value from the "intel_dtrans_func_index" attribute.
  // Return 0, if the value attribute is not present.
  static uint64_t GetMetadataIndex(AttributeSet &Attrs);

  // Removes the DTrans type attribute from a function. 'Index' corresponds to
  // the way Function attributes are numbered, i.e. 0 = return value, 1..n for
  // the parameters.
  static void RemoveDTransFuncIndexAttribute(Function *F, unsigned Index);

  // Add a DTrans function index attribute to 'F' and update the MDTypeList
  // with the metadata reference to add to the function. 'Index' corresponds to
  // the way Function attributes are numbered, i.e. 0 = return value, 1..n for
  // the parameters.
  static void
  AddDTransFuncIndexAttribute(Function *F, Metadata *MD, unsigned Index,
                              SmallVectorImpl<Metadata *> &MDTypeList);
};

} // end namespace dtransOP
} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSTYPEMETADATABUILDER_H
