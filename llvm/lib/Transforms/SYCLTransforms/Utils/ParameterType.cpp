//===- ParameterType.cpp - Parameter types --------------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/Utils/ParameterType.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {
namespace reflection {
//
// Primitive Type
//

PrimitiveType::PrimitiveType(TypePrimitiveEnum primitive)
    : ParamType(TYPE_ID_PRIMITIVE), m_primitive(primitive) {}

void PrimitiveType::accept(TypeVisitor *visitor) const { visitor->visit(this); }

std::string PrimitiveType::toString() const {
  assert((m_primitive >= PRIMITIVE_FIRST && m_primitive <= PRIMITIVE_LAST) &&
         "illegal primitive");
  return readablePrimitiveString(m_primitive).str();
}

bool PrimitiveType::equals(const ParamType *type) const {
  if (!type)
    return false;
  const PrimitiveType *p = dyn_cast<PrimitiveType>(type);
  return p && (m_primitive == p->m_primitive);
}

void PrimitiveType::setPrimitive(TypePrimitiveEnum primitive) {
  assert((primitive >= PRIMITIVE_FIRST && primitive <= PRIMITIVE_LAST) &&
         "illegal primitive");
  m_primitive = primitive;
}

//
// Pointer Type
//

PointerType::PointerType(const RefParamType type)
    : ParamType(TYPE_ID_POINTER), m_pType(type) {}

PointerType::PointerType(const RefParamType type,
                         std::vector<TypeAttributeEnum> &&attrs)
    : ParamType(TYPE_ID_POINTER), m_pType(type),
      m_attributes(std::move(attrs)) {}

void PointerType::accept(TypeVisitor *visitor) const { visitor->visit(this); }

std::string PointerType::toString() const {
  std::string Name;
  raw_string_ostream S(Name);
  for (unsigned int i = m_attributes.size(); i > 0; --i) {
    S << getReadableAttribute(m_attributes[i - 1]) << " ";
  }
  S << getPointee()->toString() << " *";
  return Name;
}

bool PointerType::equals(const ParamType *type) const {
  if (!type)
    return false;
  const PointerType *p = dyn_cast<PointerType>(type);
  if (!p || p->getAttributes().size() != getAttributes().size()) {
    return false;
  }
  for (unsigned int i = 0; i < getAttributes().size(); ++i) {
    if (getAttributes()[i] != p->getAttributes()[i]) {
      return false;
    }
  }
  return (*getPointee()).equals(&*(p->getPointee()));
}

bool PointerType::convertAddrSpaceAttribute(TypeAttributeEnum from,
                                            TypeAttributeEnum to) {
  assert((from >= ATTR_ADDR_SPACE_FIRST && from <= ATTR_ADDR_SPACE_LAST) &&
         "Only addr space attribute can be replaced!");
  assert((to >= ATTR_ADDR_SPACE_FIRST && to <= ATTR_ADDR_SPACE_LAST) &&
         "Only addr space attribute can be applied!");
  for (std::vector<TypeAttributeEnum>::iterator attr_it = m_attributes.begin(),
                                                attr_end = m_attributes.end();
       attr_it != attr_end; attr_it++) {
    if (*attr_it == from) {
      *attr_it = to;
      return true;
    }
  }
  return false;
}

//
// Vector Type
//

VectorType::VectorType(const RefParamType type, int len)
    : ParamType(TYPE_ID_VECTOR), m_pType(type), m_len(len) {}

void VectorType::accept(TypeVisitor *visitor) const { visitor->visit(this); }

std::string VectorType::toString() const {
  return (Twine(getScalarType()->toString()) + Twine(m_len)).str();
}

bool VectorType::equals(const ParamType *type) const {
  if (!type)
    return false;
  const VectorType *pVec = dyn_cast<VectorType>(type);
  return pVec && (m_len == pVec->m_len) &&
         (*getScalarType()).equals(&*(pVec->getScalarType()));
}

//
// Atomic Type
//

AtomicType::AtomicType(const RefParamType type)
    : ParamType(TYPE_ID_ATOMIC), m_pType(type) {}

void AtomicType::accept(TypeVisitor *visitor) const { visitor->visit(this); }

std::string AtomicType::toString() const {
  return (Twine("atomic_") + Twine(getBaseType()->toString())).str();
}

bool AtomicType::equals(const ParamType *type) const {
  if (!type)
    return false;
  const AtomicType *a = dyn_cast<AtomicType>(type);
  return (a && (*getBaseType()).equals(&*(a->getBaseType())));
}

//
// Block Type
//

BlockType::BlockType() : ParamType(TYPE_ID_BLOCK) {}

void BlockType::accept(TypeVisitor *visitor) const { visitor->visit(this); }

std::string BlockType::toString() const {
  std::string Name;
  raw_string_ostream S(Name);
  S << "void (";
  for (unsigned int i = 0; i < getNumOfParams(); ++i) {
    if (i > 0)
      S << ", ";
    S << m_params[i]->toString();
  }
  S << ")*";
  return Name;
}

bool BlockType::equals(const ParamType *type) const {
  if (!type)
    return false;
  const BlockType *pBlock = dyn_cast<BlockType>(type);
  if (!pBlock || getNumOfParams() != pBlock->getNumOfParams()) {
    return false;
  }
  for (unsigned int i = 0; i < getNumOfParams(); ++i) {
    if (!getParam(i)->equals(&*pBlock->getParam(i))) {
      return false;
    }
  }
  return true;
}

//
// User Defined Type
//
//
UserDefinedType::UserDefinedType(StringRef name)
    : ParamType(TYPE_ID_STRUCTURE), m_name(name) {}

void UserDefinedType::accept(TypeVisitor *visitor) const {
  visitor->visit(this);
}

std::string UserDefinedType::toString() const { return m_name.str(); }

bool UserDefinedType::equals(const ParamType *pType) const {
  if (!pType)
    return false;
  const UserDefinedType *pTy = dyn_cast<UserDefinedType>(pType);
  return pTy && (m_name == pTy->m_name);
}

//
// static enums
//
const TypeEnum PrimitiveType::enumTy = TYPE_ID_PRIMITIVE;
const TypeEnum PointerType::enumTy = TYPE_ID_POINTER;
const TypeEnum VectorType::enumTy = TYPE_ID_VECTOR;
const TypeEnum AtomicType::enumTy = TYPE_ID_ATOMIC;
const TypeEnum BlockType::enumTy = TYPE_ID_BLOCK;
const TypeEnum UserDefinedType::enumTy = TYPE_ID_STRUCTURE;

namespace {

// string represenration for the primitive types
StringRef PrimitiveNames[PRIMITIVE_NUM] = {"bool",
                                           "uchar",
                                           "char",
                                           "ushort",
                                           "short",
                                           "uint",
                                           "int",
                                           "ulong",
                                           "long",
                                           "half",
                                           "float",
                                           "double",
                                           "void",
                                           "...",
                                           "image1d_t",
                                           "image1d_ro_t",
                                           "image1d_wo_t",
                                           "image1d_rw_t",
                                           "image2d_t",
                                           "image2d_ro_t",
                                           "image2d_wo_t",
                                           "image2d_rw_t",
                                           "image2d_depth_t",
                                           "image2d_depth_ro_t",
                                           "image2d_depth_wo_t",
                                           "image2d_depth_rw_t",
                                           "image3d_t",
                                           "image3d_ro_t",
                                           "image3d_wo_t",
                                           "image3d_rw_t",
                                           "image1d_buffer_t",
                                           "image1d_buffer_ro_t",
                                           "image1d_buffer_wo_t",
                                           "image1d_buffer_rw_t",
                                           "image1d_array_t",
                                           "image1d_array_ro_t",
                                           "image1d_array_wo_t",
                                           "image1d_array_rw_t",
                                           "image2d_array_t",
                                           "image2d_array_ro_t",
                                           "image2d_array_wo_t",
                                           "image2d_array_rw_t",
                                           "image2d_array_depth_t",
                                           "image2d_array_depth_ro_t",
                                           "image2d_array_depth_wo_t",
                                           "image2d_array_depth_rw_t",
                                           "event_t",
                                           "clk_event_t",
                                           "queue_t",
                                           "pipe_t",
                                           "pipe_ro_t",
                                           "pipe_wo_t",
                                           "memory_order",
                                           "memory_scope",
                                           "sampler_t"};

StringRef mangledTypes[PRIMITIVE_NUM] = {
    "b",                            // BOOL
    "h",                            // UCHAR
    "c",                            // CHAR
    "t",                            // USHORT
    "s",                            // SHORT
    "j",                            // UINT
    "i",                            // INT
    "m",                            // ULONG
    "l",                            // LONG
    "Dh",                           // HALF
    "f",                            // FLOAT
    "d",                            // DOUBLE
    "v",                            // VOID
    "z",                            // VarArg
    "11ocl_image1d",                // PRIMITIVE_IMAGE_1D_T
    "14ocl_image1d_ro",             // PRIMITIVE_IMAGE_1D_T
    "14ocl_image1d_wo",             // PRIMITIVE_IMAGE_1D_T
    "14ocl_image1d_rw",             // PRIMITIVE_IMAGE_1D_T
    "11ocl_image2d",                // PRIMITIVE_IMAGE_2D_T
    "14ocl_image2d_ro",             // PRIMITIVE_IMAGE_2D_T
    "14ocl_image2d_wo",             // PRIMITIVE_IMAGE_2D_T
    "14ocl_image2d_rw",             // PRIMITIVE_IMAGE_2D_T
    "16ocl_image2ddepth",           // PRIMITIVE_IMAGE_2D_DEPTH_T
    "20ocl_image2d_depth_ro",       // PRIMITIVE_IMAGE_2D_DEPTH_T
    "20ocl_image2d_depth_wo",       // PRIMITIVE_IMAGE_2D_DEPTH_T
    "20ocl_image2d_depth_rw",       // PRIMITIVE_IMAGE_2D_DEPTH_T
    "11ocl_image3d",                // PRIMITIVE_IMAGE_3D_T
    "14ocl_image3d_ro",             // PRIMITIVE_IMAGE_3D_T
    "14ocl_image3d_wo",             // PRIMITIVE_IMAGE_3D_T
    "14ocl_image3d_rw",             // PRIMITIVE_IMAGE_3D_T
    "17ocl_image1dbuffer",          // PRIMITIVE_IMAGE_1D_BUFFER_T
    "21ocl_image1d_buffer_ro",      // PRIMITIVE_IMAGE_1D_BUFFER_T
    "21ocl_image1d_buffer_wo",      // PRIMITIVE_IMAGE_1D_BUFFER_T
    "21ocl_image1d_buffer_rw",      // PRIMITIVE_IMAGE_1D_BUFFER_T
    "16ocl_image1darray",           // PRIMITIVE_IMAGE_1D_ARRAY_T
    "20ocl_image1d_array_ro",       // PRIMITIVE_IMAGE_1D_ARRAY_T
    "20ocl_image1d_array_wo",       // PRIMITIVE_IMAGE_1D_ARRAY_T
    "20ocl_image1d_array_rw",       // PRIMITIVE_IMAGE_1D_ARRAY_T
    "16ocl_image2darray",           // PRIMITIVE_IMAGE_2D_ARRAY_T
    "20ocl_image2d_array_ro",       // PRIMITIVE_IMAGE_2D_ARRAY_T
    "20ocl_image2d_array_wo",       // PRIMITIVE_IMAGE_2D_ARRAY_T
    "20ocl_image2d_array_rw",       // PRIMITIVE_IMAGE_2D_ARRAY_T
    "21ocl_image2darraydepth",      // PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_T
    "26ocl_image2d_array_depth_ro", // PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_T
    "26ocl_image2d_array_depth_wo", // PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_T
    "26ocl_image2d_array_depth_rw", // PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_T
    "9ocl_event",                   // PRIMITIVE_EVENT_T
    "12ocl_clkevent",               // PRIMITIVE_CLK_EVENT_T
    "9ocl_queue",                   // PRIMITIVE_QUEUE_T
    "8ocl_pipe",                    // PRIMITIVE_PIPE_T
    "11ocl_pipe_ro",                // PRIMITIVE_PIPE_RO_T
    "11ocl_pipe_wo",                // PRIMITIVE_PIPE_WO_T
    "12memory_order",               // PRIMITIVE_MEMORY_ORDER
    "12memory_scope",               // PRIMITIVE_MEMORY_SCOPE
    "11ocl_sampler"                 // PRIMITIVE_SAMPLER_T
};

StringRef readableAttribute[ATTR_NUM] = {"__private", "__global",  "__constant",
                                         "__local",   "__generic", "restrict",
                                         "volatile",  "const"};

StringRef mangledAttribute[ATTR_NUM] = {"",      "U3AS1", "U3AS2", "U3AS3",
                                        "U3AS4", "r",     "V",     "K"};

} // namespace

StringRef mangledPrimitiveString(TypePrimitiveEnum t) {
  return mangledTypes[t];
}

StringRef readablePrimitiveString(TypePrimitiveEnum t) {
  return PrimitiveNames[t];
}

std::string llvmPrimitiveString(TypePrimitiveEnum t) {
  assert(t >= PRIMITIVE_STRUCT_FIRST && t <= PRIMITIVE_STRUCT_LAST &&
         "assuming struct primitive type only!");
  return (Twine("opencl.") + Twine(PrimitiveNames[t])).str();
}

StringRef getMangledAttribute(TypeAttributeEnum attribute) {
  return mangledAttribute[attribute];
}

StringRef getReadableAttribute(TypeAttributeEnum attribute) {
  return readableAttribute[attribute];
}

std::string getDuplicateString(int index) {
  assert(index >= 0 && "illegal index");
  if (0 == index)
    return "S_";
  return (Twine("S") + Twine(index - 1) + Twine("_")).str();
}

} // namespace reflection
} // namespace llvm
