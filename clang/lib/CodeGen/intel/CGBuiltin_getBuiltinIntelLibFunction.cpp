#ifdef INTEL_SPECIFIC_IL0_BACKEND
#include "CodeGenFunction.h"
#include "CGObjCRuntime.h"
#include "CodeGenModule.h"
#include "TargetInfo.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/Basic/TargetBuiltins.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/CodeGen/CGFunctionInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Intrinsics.h"

using namespace clang;
using namespace CodeGen;

/// getBuiltinIntelLibFunction - Given a builtin id for a function like
/// "__apply_args", return a Function* for "__apply_args".
llvm::Value *CodeGenModule::getBuiltinIntelLibFunction(const FunctionDecl *FD,
                                                       unsigned BuiltinID) {
  // Get the name, skip over the __builtin_ prefix (if necessary).
  GlobalDecl D(FD);
  StringRef Name = Context.BuiltinInfo.GetName(BuiltinID);

  llvm::FunctionType *Ty =
    cast<llvm::FunctionType>(getTypes().ConvertType(FD->getType()));

  return GetOrCreateLLVMFunction(Name, Ty, D, /*ForVTable=*/false);
}
#endif  // INTEL_SPECIFIC_IL0_BACKEND
