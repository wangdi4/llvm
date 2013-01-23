#include "llvm/Type.h"
#include "llvm/LLVMContext.h"
#include "name_mangling/Type.h"

namespace intel{
  llvm::Type* reflectionToLLVM(llvm::LLVMContext&, const reflection::Type*);
}
