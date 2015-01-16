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
