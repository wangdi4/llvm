// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "TypeConversion.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/DerivedTypes.h"
#include <string>

using namespace llvm;

namespace intel {

class ConversionVisitor : public reflection::TypeVisitor {
  llvm::Type *m_llvmTy{nullptr};
  llvm::LLVMContext &m_Ctx;

  bool isAddressSpace(reflection::TypeAttributeEnum attr) {
    return (attr >= reflection::ATTR_ADDR_SPACE_FIRST &&
            attr <= reflection::ATTR_ADDR_SPACE_LAST);
  }

  unsigned convertAddressSpace(reflection::TypeAttributeEnum attr) {
    switch (attr) {
    case reflection::ATTR_PRIVATE:
      return 0U;
    case reflection::ATTR_GLOBAL:
      return 1U;
    case reflection::ATTR_CONSTANT:
      return 2U;
    case reflection::ATTR_LOCAL:
      return 3U;
    case reflection::ATTR_GENERIC:
      return 4U;
    default:
      assert(false && "unreachable");
      return 42U;
    }
  }

public:
  ConversionVisitor(llvm::LLVMContext &ctx) : m_Ctx(ctx) {}

  virtual void visit(const reflection::PrimitiveType *Ty) override {
    switch (Ty->getPrimitive()) {
    case reflection::PRIMITIVE_BOOL:
      m_llvmTy = llvm::IntegerType::get(m_Ctx, 1U);
      break;
    case reflection::PRIMITIVE_UCHAR:
    case reflection::PRIMITIVE_CHAR:
      m_llvmTy = llvm::IntegerType::get(m_Ctx, 8U);
      break;
    case reflection::PRIMITIVE_USHORT:
    case reflection::PRIMITIVE_SHORT:
      m_llvmTy = llvm::IntegerType::get(m_Ctx, 16U);
      break;
    case reflection::PRIMITIVE_UINT:
    case reflection::PRIMITIVE_INT:
      m_llvmTy = llvm::IntegerType::get(m_Ctx, 32U);
      break;
    case reflection::PRIMITIVE_ULONG:
    case reflection::PRIMITIVE_LONG:
      m_llvmTy = llvm::IntegerType::get(m_Ctx, 64U);
      break;
    case reflection::PRIMITIVE_HALF:
      m_llvmTy = llvm::Type::getHalfTy(m_Ctx);
      break;
    case reflection::PRIMITIVE_FLOAT:
      m_llvmTy = llvm::Type::getFloatTy(m_Ctx);
      break;
    case reflection::PRIMITIVE_DOUBLE:
      m_llvmTy = llvm::Type::getDoubleTy(m_Ctx);
      break;
    case reflection::PRIMITIVE_VOID:
      m_llvmTy = llvm::Type::getVoidTy(m_Ctx);
      break;
    case reflection::PRIMITIVE_IMAGE_1D_T:
    case reflection::PRIMITIVE_IMAGE_2D_T:
    case reflection::PRIMITIVE_IMAGE_2D_DEPTH_T:
    case reflection::PRIMITIVE_IMAGE_3D_T:
    case reflection::PRIMITIVE_IMAGE_1D_BUFFER_T:
    case reflection::PRIMITIVE_IMAGE_1D_ARRAY_T:
    case reflection::PRIMITIVE_IMAGE_2D_ARRAY_T:
    case reflection::PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_T:
    case reflection::PRIMITIVE_EVENT_T:
    case reflection::PRIMITIVE_CLK_EVENT_T:
    case reflection::PRIMITIVE_QUEUE_T:
    case reflection::PRIMITIVE_PIPE_RO_T:
    case reflection::PRIMITIVE_PIPE_WO_T: {
      std::string Name = reflection::llvmPrimitiveString(Ty->getPrimitive());
      m_llvmTy = llvm::StructType::create(m_Ctx, Name);
    } break;
    case reflection::PRIMITIVE_SAMPLER_T:
      m_llvmTy = llvm::IntegerType::get(m_Ctx, 32U);
      break;
    default:
      assert(false && "unexpected type");
      break;
    }
  }

  virtual void visit(const reflection::VectorType *VTy) override {
    VTy->getScalarType()->accept(this);
    m_llvmTy = llvm::FixedVectorType::get(m_llvmTy, VTy->getLength());
  }

  virtual void visit(const reflection::PointerType *PTy) override {
    PTy->getPointee()->accept(this);
    unsigned AS = 0U;
    for (unsigned int i = 0; i < PTy->getAttributes().size(); ++i) {
      reflection::TypeAttributeEnum attr = PTy->getAttributes()[i];
      if (isAddressSpace(attr)) {
        AS = convertAddressSpace(attr);
        break;
      }
    }
    m_llvmTy = llvm::PointerType::get(m_llvmTy, AS);
  }

  void visit(const reflection::AtomicType * /*ATy*/) override {
    assert(false && "need to support Atomic Parameter type");
  }

  void visit(const reflection::BlockType * /*BTy*/) override {
    assert(false && "need to support Block Parameter type");
  }

  virtual void visit(const reflection::UserDefinedType *UdTy) override {
    std::string Name = UdTy->toString();
    m_llvmTy = llvm::StructType::create(m_Ctx, Name);
  }

  const llvm::Type *getType() const { return m_llvmTy; }

  llvm::Type *getType() { return m_llvmTy; }
};

llvm::Type *reflectionToLLVM(llvm::LLVMContext &Ctx,
                             const reflection::RefParamType &Ty) {
  ConversionVisitor V(Ctx);
  Ty->accept(&V);
  return V.getType();
}

} // namespace intel
