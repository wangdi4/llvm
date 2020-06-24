//===----------------------- IntelVPlanVectorShape.h ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the VectorShape class, which is used to provide
/// information about how different VPValues vary within a vector, namely
/// uniform, strided, or random.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANVECTORSHAPE_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANVECTORSHAPE_H

#include "IntelVPlanValue.h"
#include "llvm/ADT/StringRef.h"

namespace llvm {
namespace vpo {

class VPVectorShape {

public:
  // Used for transfer function table lookup
  enum VPShapeDescriptor {
    Uni      = 0, // All elements in a vector are the same
    Seq      = 1, // Elements are consecutive
    Str      = 2, // Elements are in strides
    Rnd      = 3, // Unknown or non-consecutive order
    SOASeq   = 4, // Elements are consecutive under SOA-layout.
    SOAStr   = 5, // Elements are in strides under SOA-layout.
    SOARnd   = 6, // Unknown or non-consecutive stride under SOA-layout.
    Undef    = 7, // Undefined shape
    NumDescs = 8
  };

  // describes how the contents of a vector vary with the vectorized dimension
  VPVectorShape(VPShapeDescriptor Desc, VPValue *Stride = nullptr) :
                Desc(Desc), Stride(Stride) {
    if (Desc == VPShapeDescriptor::Rnd && Stride)
      llvm_unreachable("Stride should not be set for random shapes");
    if (Desc == VPShapeDescriptor::Uni && getStrideVal() != 0)
      llvm_unreachable("Stride for uniform shapes should be 0");
    //TODO: add assert to enforce stride as integer
  };

  VPVectorShape() : VPVectorShape(VPShapeDescriptor::Undef) {}

  VPValue *getStride() const { return Stride; }
  void setStride(VPValue *S) { Stride = S; }

  // The stride-value can be negative.
  int64_t getStrideVal() const {
    assert(hasKnownStride() && "Stride val is not known");
    ConstantInt *CInt = cast<ConstantInt>(Stride->getUnderlyingValue());
    return CInt->getSExtValue();
  }

  /// Stride is a known constant.
  bool hasKnownStride() const {
    VPConstant *C = dyn_cast_or_null<VPConstant>(Stride);
    return C && isa<ConstantInt>(C->getUnderlyingValue());
  }

  bool isUniform() const {
    return (Desc == VPShapeDescriptor::Uni);
  }

  bool isUnitStride() const {
    return (Desc == VPShapeDescriptor::Seq ||
            Desc == VPShapeDescriptor::SOASeq);
  }

  bool isStrided() const {
    return (Desc == VPShapeDescriptor::Str);
  }

  bool isAnyStridedNonPtr() const {
    return (Desc == VPShapeDescriptor::Seq ||
            Desc == VPShapeDescriptor::Str);
  }

  bool isAnyStrided() const { return isAnyStrided(Desc); }

  static bool isAnyStrided(VPShapeDescriptor Desc) {
    return (Desc == VPShapeDescriptor::Seq || Desc == VPShapeDescriptor::Str ||
            Desc == VPShapeDescriptor::SOASeq ||
            Desc == VPShapeDescriptor::SOAStr);
  }

  bool isRandom() const {
    return (Desc == VPShapeDescriptor::Rnd);
  }

  bool isSOAUnitStride() const {
    return (Desc == VPShapeDescriptor::SOASeq);
  }

  bool isSOAStrided() const {
    return (Desc == VPShapeDescriptor::SOAStr);
  }

  bool isSOARandom() const {
    return (Desc == VPShapeDescriptor::SOARnd);
  }

  bool isUndefined() const {
    return (Desc == VPShapeDescriptor::Undef);
  }

  /// Create a new shape identical to the existing one.
  VPVectorShape *clone() {
    return new VPVectorShape(getShapeDescriptor(), getStride());
  }

  VPShapeDescriptor getShapeDescriptor() const { return Desc; }

  StringRef getShapeDescriptorStr() const {
    return getShapeDescriptorStr(Desc);
  }

  static StringRef getShapeDescriptorStr(VPShapeDescriptor Desc) {
    switch (Desc) {
      case VPShapeDescriptor::Uni:
        return "Uniform";
      case VPShapeDescriptor::Seq:
        return "Unit Stride";
      case VPShapeDescriptor::Str:
        return "Strided";
      case VPShapeDescriptor::Rnd:
        return "Random";
      case VPShapeDescriptor::SOASeq:
        return "SOA Unit Stride";
      case VPShapeDescriptor::SOAStr:
        return "SOA Strided";
      case VPShapeDescriptor::SOARnd:
        return "SOA Random";
      case VPShapeDescriptor::Undef:
        return "Undef";
      default:
        llvm_unreachable("Descriptor not supported");
    }
  }

  static VPVectorShape joinShapes(VPVectorShape Shape1, VPVectorShape Shape2);
  static bool shapesHaveSameStride(VPVectorShape Shape1, VPVectorShape Shape2);

  static VPVectorShape getUndef() { return {VPShapeDescriptor::Undef}; }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#if INTEL_CUSTOMIZATION
  void print(raw_ostream &OS) const {
    OS << "[Shape: " << getShapeDescriptorStr();
    if (isAnyStrided()) {
      OS << ", Stride: ";
      if (hasKnownStride()) {
        if (isSOAStrided())
          OS << "VF x ";
        Stride->dump(OS);
      } else
        OS << "?";
    }
    OS << ']';
  }
#endif
#endif

private:
  VPShapeDescriptor Desc;
  // Set when known integer or variable constant for strided shapes.
  // Otherwise, nullptr.
  VPValue *Stride;
};
} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANVECTORSHAPE_H
