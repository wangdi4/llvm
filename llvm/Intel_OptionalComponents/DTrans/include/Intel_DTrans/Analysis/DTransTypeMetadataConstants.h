//===--DTransTypeMetadataConstants.h --Constants for DTrans type metadata--===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file provides constants used in DTrans type metadata.
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error DTransTypeMetadataConstants.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSTYPEMETADATACONSTANTS_H
#define INTEL_DTRANS_ANALYSIS_DTRANSTYPEMETADATACONSTANTS_H

namespace llvm {
namespace dtransOP {

// The tag name for the named metadata nodes that contains the list of structure
// types. This node is used to identify all the nodes that describe the fields
// of the structure so that we will know what all the original pointer type
// fields were.
static const char *MDStructTypesTag = "intel.dtrans.types";

// The tag name used for variables and instructions marked with DTrans type
// information for pointer type recovery.
static const char *MDDTransTypeTag = "intel_dtrans_type";

// Tag used for metadata on a Function declaration/definition to map a set
// of metadata nodes of encoded types to attributes used on the return type
// and parameters.
static const char *DTransFuncTypeMDTag = "intel.dtrans.func.type";

// String attribute name that is set on return type and parameters to provide
// indexing into the DTransFuncTypeMD metadata.
static const char *DTransFuncIndexTag = "intel_dtrans_func_index";

// Constants for meanings of the operands for the record that describes a
// structure type in the DTrans metadata.
//
//   Named opaque struct type: !{!"S", <type> zeroinitializer, i32 -1}
//
//   Named struct type       : !{!"S", <type> zeroinitializer, i32<numElem>,
//                               !MDNodefield1, !MDNodefield2, ...}
//
struct DTransStructMDConstants {
  static const unsigned RecTypeOffset = 0;
  static const unsigned StructTypeOffset = 1;
  static const unsigned FieldCountOffset = 2;
  static const unsigned FieldNodeOffset = 3;

  // Minimum operand count for a structure record
  static const unsigned MinOperandCount = 3;
};

} // end namespace dtransOP
} // end namespace llvm

#endif // INTEL_DTRANS_ANALYSIS_DTRANSTYPEMETADATACONSTANTS_H
