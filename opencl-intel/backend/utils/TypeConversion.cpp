#include "TypeConversion.h"
#include "llvm/DerivedTypes.h"
#include "llvm/ADT/StringRef.h"

namespace intel{

const std::string PRIVATE  = "__private";
const std::string GLOBAL   = "__global";
const std::string CONSTANT = "__constant";
const std::string LOCAL    = "__local";

class ConversionVisitor : public reflection::TypeVisitor{
  llvm::Type *llvmTy;
  llvm::LLVMContext &Ctx;

  bool isAddressSpace(const std::string& S){
    return (S == PRIVATE || S == GLOBAL || S == CONSTANT || S == LOCAL);
  }

  unsigned convertAddressSpace(const std::string& S){
    if(S == PRIVATE)
      return 0U;
    if(S == GLOBAL)
      return 1U;
    if(S == CONSTANT)
      return 2U;
    if(S == LOCAL)
      return 3U;
    assert(false && "unreachable");
    return 42U;
  }
public:
  ConversionVisitor(llvm::LLVMContext &ctx): Ctx(ctx){
  }

  virtual void visit(const reflection::Type *Ty){
    switch (Ty->getPrimitive()){
    case reflection::primitives::BOOL:
      llvmTy = llvm::IntegerType::get(Ctx, 1U);
      break;
    case reflection::primitives::UCHAR:
    case reflection::primitives::CHAR:
      llvmTy = llvm::IntegerType::get(Ctx, 8U);
      break;
    case reflection::primitives::USHORT:
    case reflection::primitives::SHORT:
      llvmTy = llvm::IntegerType::get(Ctx, 16U);
      break;
    case reflection::primitives::UINT:
    case reflection::primitives::INT:
    case reflection::primitives::SAMPLER_T:
      llvmTy = llvm::IntegerType::get(Ctx, 32U);
      break;
    case reflection::primitives::ULONG:
    case reflection::primitives::LONG:
      llvmTy = llvm::IntegerType::get(Ctx, 64U);
      break;
    case reflection::primitives::HALF:
      llvmTy = llvm::Type::getHalfTy(Ctx);
    break;
    case reflection::primitives::FLOAT:
      llvmTy = llvm::Type::getFloatTy(Ctx);
      break;
    case reflection::primitives::DOUBLE:
      llvmTy = llvm::Type::getDoubleTy(Ctx);
    break;
    case reflection::primitives::VOID:
      llvmTy = llvm::Type::getVoidTy(Ctx);
      break;
    default:
      assert(false && "unexpected type");
      break;
    }
  }

  virtual void visit(const reflection::Vector *VTy){
    reflection::Type pTy(VTy->getPrimitive()); 
    pTy.accept(this);
    llvmTy = llvm::VectorType::get(llvmTy, VTy->getLen());
  }

  virtual void visit(const reflection::Pointer *PTy){
    const reflection::Type *Ty = PTy->getPointee();
    Ty->accept(this);
    unsigned AS = 0U;
    std::vector<std::string>::const_iterator b = PTy->beginAttributes(),
      e = PTy->endAttributes();
    while (b != e){
      const std::string Attrib = *b++;
      if ( isAddressSpace(Attrib) ){
        AS = convertAddressSpace(Attrib);
        break;
      }
    }
    llvmTy = llvm::PointerType::get(llvmTy, AS);
  }

  virtual void visit(const reflection::UserDefinedTy *UdTy){
    llvm::StringRef Name = UdTy->toString();
    llvmTy = llvm::StructType::create(Ctx, Name);
  }

  const llvm::Type* getType()const{ return llvmTy;}

  llvm::Type* getType(){ return llvmTy; }
};


llvm::Type* reflectionToLLVM(llvm::LLVMContext &Ctx, const reflection::Type *Ty){
  ConversionVisitor V(Ctx);
  Ty->accept(&V);
  return V.getType();
}

}
