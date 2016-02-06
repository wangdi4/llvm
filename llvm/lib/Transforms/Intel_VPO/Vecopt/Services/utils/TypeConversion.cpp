/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "TypeConversion.h"
#include "Utils.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/ADT/StringRef.h"
#include <string>

namespace intel{

class ConversionVisitor : public reflection::TypeVisitor{
  llvm::Type *m_llvmTy;
  llvm::LLVMContext &m_Ctx;

public:
  ConversionVisitor(llvm::LLVMContext &ctx): m_Ctx(ctx){
  }

  virtual void visit(const reflection::PrimitiveType *Ty){
    switch (Ty->getPrimitive()){
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
    default:
      assert(false && "unexpected type");
      break;
    }
  }

  virtual void visit(const reflection::VectorType *VTy){
    VTy->getScalarType()->accept(this);
    m_llvmTy = llvm::VectorType::get(m_llvmTy, VTy->getLength());
  }

  virtual void visit(const reflection::PointerType *PTy){
    PTy->getPointee()->accept(this);
    unsigned AS = 0U;
    m_llvmTy = llvm::PointerType::get(m_llvmTy, AS);
  }

  void visit(const reflection::BlockType *BTy) {
    assert(false && "need to support Block Parameter type");
  }

  virtual void visit(const reflection::UserDefinedType *UdTy){
    std::string Name = UdTy->toString();
    m_llvmTy = llvm::StructType::create(m_Ctx, Name);
  }

  const llvm::Type* getType()const{ return m_llvmTy;}

  llvm::Type* getType(){ return m_llvmTy; }
};


llvm::Type* reflectionToLLVM(llvm::LLVMContext &Ctx, const reflection::RefParamType& Ty){
  ConversionVisitor V(Ctx);
  Ty->accept(&V);
  return V.getType();
}

}
