// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
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

#include "LLDJIT.h"

#include "AsmCompiler.h"
#include "CompilationUtils.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/Module.h"
#include "llvm/Object/Archive.h"
#include "llvm/Object/Binary.h"
#include "llvm/Object/COFF.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include <mutex>

#include "lld/Common/Driver.h"

#include <fstream>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN // Reduce windows.h internal includes
#define NOMINMAX            // Don't add min() macro, that breaks std::min()
#include <windows.h>

using namespace llvm;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

ExecutionEngine *LLDJIT::createJIT(std::unique_ptr<Module> M,
                                   std::string *ErrorStr,
                                   std::unique_ptr<TargetMachine> TM) {

  // Try to register the program as a source of symbols to resolve against.
  sys::DynamicLibrary::LoadLibraryPermanently(nullptr, nullptr);

  return new LLDJIT(std::move(M), std::move(TM));
}

LLDJIT::LLDJIT(std::unique_ptr<Module> M, std::unique_ptr<TargetMachine> TM)
    : ExecutionEngine(TM->createDataLayout(), std::move(M)), TM(std::move(TM)),
      DLLHandle(nullptr) {
  // FIXME: We are managing our modules, so we do not want the base class
  // ExecutionEngine to manage them as well. To avoid double destruction
  // of the first (and only) module added in ExecutionEngine constructor
  // we remove it from EE and will destruct it ourselves.
  //
  // It may make sense to move our module manager (based on SmallStPtr) back
  // into EE if the JIT and Interpreter can live with it.
  // If so, additional functions: addModule, removeModule, FindFunctionNamed,
  // runStaticConstructorsDestructors could be moved back to EE as well.
  //
  std::unique_ptr<Module> First = std::move(Modules[0]);
  Modules.clear();

  if (First->getDataLayout().isDefault())
    First->setDataLayout(getDataLayout());

  Triple = First->getTargetTriple();
  OwnedModules.addModule(std::move(First));

  // Build debug information by default, since LLDJIT's purpose is to allow
  // native VS debugging.
  ArgvLLD.push_back("-debug");
}

LLDJIT::~LLDJIT() {
  std::lock_guard<sys::Mutex> locked(lock);

  FreeLibrary((HMODULE)DLLHandle);
  DeleteTempFiles();
}

void LLDJIT::DeleteTempFiles() { OwnedTempFiles.clear(); }

void LLDJIT::DeleteTempFiles(
    const llvm::SmallVectorImpl<std::string> &FilesToDelete) {
  const std::string *B = FilesToDelete.begin();
  const std::string *E = FilesToDelete.end();
  for (int i = 0; i < OwnedTempFiles.size(); i++) {
    TmpFile &TempFile = OwnedTempFiles[i];
    if (std::find(B, E, TempFile.FileName()) != E) {
      std::swap<TmpFile, TmpFile>(TempFile, OwnedTempFiles.back());
      OwnedTempFiles.pop_back();
      i--;
    }
  }
}

void LLDJIT::addModule(std::unique_ptr<Module> M) {
  std::lock_guard<sys::Mutex> locked(lock);

  if (M->getDataLayout().isDefault())
    M->setDataLayout(getDataLayout());

  OwnedModules.addModule(std::move(M));
}

bool LLDJIT::removeModule(Module *M) {
  std::lock_guard<sys::Mutex> locked(lock);
  return OwnedModules.removeModule(M);
}

void LLDJIT::addObjectFile(std::unique_ptr<object::ObjectFile> Obj) {
  llvm_unreachable("Not supported");
}

void LLDJIT::addObjectFile(object::OwningBinary<object::ObjectFile> Obj) {
  llvm_unreachable("Not supported");
}
void LLDJIT::addObjectFile(const std::string &FilePath) {
  LoadedObjects.push_back(FilePath);
}
void LLDJIT::addArchive(object::OwningBinary<object::Archive> A) {
  llvm_unreachable("Not supported");
}

void LLDJIT::setObjectCache(ObjectCache *NewCache) {
  // This function can be called, but caching will not work.
  /*MutexGuard Locked(lock);
  ObjCache = NewCache;*/
}

std::string LLDJIT::getLastErrorMessage() {
  DWORD EC = GetLastError();
  char Buf[256];
  size_t Size = FormatMessageA(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, EC,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &Buf[0], sizeof(Buf), NULL);
  return std::string(Buf, Size);
}

void LLDJIT::mapDllFunctions(llvm::Module *M, void *DLLHandle) {
  HMODULE HMod = reinterpret_cast<HMODULE>(DLLHandle);
  for (llvm::Function &F : M->functions()) {
    if (F.getDLLStorageClass() ==
        llvm::GlobalValue::DLLStorageClassTypes::DLLExportStorageClass) {
      StringRef SymbolName = F.getName();
      void *Function = GetProcAddress(HMod, SymbolName.data());
      if (Function) {
        updateGlobalMapping(SymbolName, (uint64_t)Function);
      }
    }
  }
}

std::string LLDJIT::emitObject(Module *M) {
  assert(M && "Can not emit a null module");

  std::lock_guard<sys::Mutex> locked(lock);

  // Materialize all globals in the module if they have not been
  // materialized already.
  cantFail(M->materializeAll());

  TmpFile ObjFile = TmpFile("OpenCLKernel", "obj");

  M->addModuleFlag(llvm::Module::Warning, "ObjFilePath",
                   llvm::ConstantDataArray::getString(
                       M->getContext(), ObjFile.FileName().c_str(), true));

  llvm::legacy::PassManager PM;
  TM->addPassesToEmitFile(PM, ObjFile.OS(),
                          /*raw_pwrite_stream*/ nullptr,
                          TargetMachine::CGFT_ObjectFile,
                          /*DisableVerify*/ true);

  // Initialize passes.
  PM.run(*M);
  ObjFile.OS().flush();

  OwnedTempFiles.emplace_back(std::move(ObjFile));

  return OwnedTempFiles.back().FileName();
}

void LLDJIT::generateCodeForModule(Module *M) {
  // This function will generate OBJ file from module

  // Get a thread lock to make sure we aren't trying to load multiple times
  std::lock_guard<sys::Mutex> locked(lock);

  // This must be a module which has already been added to this LLDJIT instance.
  assert(OwnedModules.ownsModule(M) &&
         "LLDJIT::generateCodeForModule: Unknown module.");

  // Re-compilation is not supported
  if (OwnedModules.hasModuleBeenLoaded(M))
    return;

  std::string ObjectToLoad;

  assert(TM->isCompatibleDataLayout(M->getDataLayout()) &&
         "DataLayout Mismatch");

  // If the cache did not contain a suitable object, compile the object
  ObjectToLoad = emitObject(M);

  LoadedObjects.push_back(ObjectToLoad);
  LoadDLL(M);

  OwnedModules.markModuleAsLoaded(M);
}

void LLDJIT::LoadDLL(Module *M) {
  TmpFile DLLFile = TmpFile("OpenCLKernel", "dll");
  TmpFile PDBFile = TmpFile("OpenCLKernel", "pdb");
  buildDllFromObjs(LoadedObjects, DLLFile.FileName(), PDBFile.FileName());
  DLLPath = DLLFile.FileName();
  this->DeleteTempFiles(LoadedObjects);
  OwnedTempFiles.emplace_back(std::move(DLLFile));
  OwnedTempFiles.emplace_back(std::move(PDBFile));

  assert(!DLLHandle);
  HMODULE dllHandle = LoadLibraryA(DLLPath.c_str());
  if (!dllHandle)
    throw Exceptions::CompilerException("LoadLibrary(" + DLLPath +
                                        ") failed: " + getLastErrorMessage());
  DLLHandle = dllHandle;
  mapDllFunctions(M, DLLHandle);
}

void LLDJIT::finalizeLoadedModules() {
  std::lock_guard<sys::Mutex> locked(lock);
  OwnedModules.markAllLoadedModulesAsFinalized();
}

std::set<std::string> LLDJIT::getExternalSymbolsList(
    const llvm::SmallVector<std::string, 2> &InObjFiles) {
  std::set<std::string> ExternalSymbols;
  for (const std::string &ObjFile : InObjFiles) {
    Expected<llvm::object::OwningBinary<llvm::object::Binary>> BinaryOrErr =
        llvm::object::createBinary(ObjFile);
    assert(BinaryOrErr);
    llvm::object::Binary &Binary = *BinaryOrErr.get().getBinary();
    llvm::object::ObjectFile *O =
        llvm::dyn_cast<llvm::object::ObjectFile>(&Binary);
    assert(O); // Not an ObjectFile
    const llvm::object::COFFObjectFile *Coff =
        dyn_cast<const llvm::object::COFFObjectFile>(O);
    assert(Coff); // Not a COFFObjectFile

    for (unsigned SI = 0, SE = Coff->getNumberOfSymbols(); SI != SE; ++SI) {
      Expected<llvm::object::COFFSymbolRef> Symbol = Coff->getSymbol(SI);
      StringRef Name;
      assert(!errorToErrorCode(Symbol.takeError()));
      Coff->getSymbolName(*Symbol, Name);
      if (Symbol->getSectionNumber() == 0)
        ExternalSymbols.insert(Name);
    }
  }
  return ExternalSymbols;
}

int LLDJIT::compileSymbolJumpTable(
    const llvm::SmallVector<std::string, 2> &InObjFiles,
    raw_fd_ostream *OutJumpTableObj) {
  std::set<std::string> ExternalSymbols = getExternalSymbolsList(InObjFiles);
  std::stringstream AsmCode;

  // Write text section header
  AsmCode << ".text\n"
             ".balign 4\n";

  for (const std::string &Sym : ExternalSymbols) {
    std::string Name = Sym;
    assert(!Name.empty());
    void *F;

#ifdef _WIN64
    F = sys::DynamicLibrary::SearchForAddressOfSymbol(Name);

#else // 32 bit symbols should always get an "_" prefix by the compiler
    if (Name[0] == '_')
      F = sys::DynamicLibrary::SearchForAddressOfSymbol(Name.substr(1));
    else
      F = sys::DynamicLibrary::SearchForAddressOfSymbol(Name);
#endif

    if (F) {
      // Define the symbol as global
      AsmCode << ".globl " << Name
              << "\n"
                 ".def "
              << Name
              << "\n"
                 ".scl 2\n"
                 ".type 32\n"
                 ".endef\n";

      // Define the jump table function
#ifdef _WIN64
      AsmCode << Name
              << ":\n"
                 "sub $8, %rsp\n"
                 "movl $0x"
              << std::hex << reinterpret_cast<uint32_t>(F)
              << ", 0(%rsp)\n"
                 "movl $0x"
              << std::hex << (reinterpret_cast<size_t>(F) >> 32)
              << ", 4(%rsp)\n"
                 "ret\n";
#else // 32 bit:
      AsmCode << Name
              << ":\n"
                 "pushl $0x"
              << std::hex << (((size_t)F))
              << "\n"
                 "ret\n";
#endif
    }
  }

  std::string AsmCodeStdStr(AsmCode.str());

#if 0 // Enable this to dump the asm code to disk for debugging.
  std::ofstream jumptableDump("jumptable.S");
  jumptableDump << asmCodeStdStr;
  jumptableDump.close();
#endif

  StringRef AsmCodeStrRef(AsmCodeStdStr);
  std::unique_ptr<llvm::MemoryBuffer> MemBuffer =
      llvm::MemoryBuffer::getMemBuffer(AsmCodeStrRef, "jumptable.S", false);

  return AsmCompiler::compileAsmToObjectFile(std::move(MemBuffer),
                                             OutJumpTableObj, Triple);
}

void LLDJIT::buildDllFromObjs(
    const llvm::SmallVector<std::string, 2> &InObjFiles,
    const std::string &OutDLLPath, const std::string &OutPDBPath) {
  std::lock_guard<sys::Mutex> locked(lock);

  assert(!InObjFiles.empty());

  std::string DLLPathArg("-out:" + OutDLLPath);
  std::string PDBPathArg("-pdb:" + OutPDBPath);
  TmpFile ImpLibFile("OpenCLKernel-implib", "lib");
  std::string LibPathArg("-implib:" + ImpLibFile.FileName());
  std::vector<const char *> Args;
  Args.push_back("dummy.exe");
  Args.push_back("-dll");
  Args.push_back("-intel-embedded-linker");
  Args.push_back("-nodefaultlib");
  Args.push_back(LibPathArg.c_str());
  Args.push_back(PDBPathArg.c_str());
  Args.push_back(DLLPathArg.c_str());

  for (std::string &Arg : ArgvLLD)
    Args.push_back(Arg.c_str());
  for (const std::string &ObjFile : InObjFiles)
    Args.push_back(ObjFile.c_str());

  TmpFile JumpTableObjectFile = TmpFile("OpenCLKernel-jumptable", "obj");

  int Ret = compileSymbolJumpTable(InObjFiles, &JumpTableObjectFile.OS());
  if (Ret)
    throw Exceptions::CompilerException("Failed to compile jump table");

  JumpTableObjectFile.OS().flush();

  Args.push_back(JumpTableObjectFile.FileName().c_str());

  bool Success = lld::coff::link(Args, false);
  if (!Success) {
    // TODO: Currently, error message will be written to stdout.
    throw Exceptions::CompilerException("Linker failed");
  }
}

// FIXME: Rename this.
void LLDJIT::finalizeObject() {
  std::lock_guard<sys::Mutex> locked(lock);

  // Generate code for module is going to move objects out of the 'added' list,
  // so we need to copy that out before using it:
  SmallVector<Module *, 16> ModsToAdd;
  for (auto M : OwnedModules.added())
    ModsToAdd.push_back(M);

  for (auto M : ModsToAdd)
    generateCodeForModule(M);

  finalizeLoadedModules();
}

void LLDJIT::finalizeModule(Module *M) {
  std::lock_guard<sys::Mutex> locked(lock);

  // This must be a module which has already been added to this LLDJIT instance.
  assert(OwnedModules.ownsModule(M) &&
         "LLDJIT::finalizeModule: Unknown module.");

  // If the module hasn't been compiled, just do that.
  if (!OwnedModules.hasModuleBeenLoaded(M))
    generateCodeForModule(M);

  finalizeLoadedModules();
}

JITSymbol LLDJIT::findExistingSymbol(const std::string &Name) {
  if (void *Addr = getPointerToGlobalIfAvailable(Name))
    return JITSymbol(static_cast<uint64_t>(reinterpret_cast<uintptr_t>(Addr)),
                     JITSymbolFlags::Exported);
  return JITSymbol(nullptr);
}

Module *LLDJIT::findModuleForSymbol(const std::string &Name,
                                    bool CheckFunctionsOnly) {
  StringRef DemangledName = Name;
  if (DemangledName[0] == getDataLayout().getGlobalPrefix())
    DemangledName = DemangledName.substr(1);

  std::lock_guard<sys::Mutex> locked(lock);

  // If it hasn't already been generated, see if it's in one of our modules.
  for (ModulePtrSet::iterator I = OwnedModules.begin_added(),
                              E = OwnedModules.end_added();
       I != E; ++I) {
    Module *M = *I;
    Function *F = M->getFunction(DemangledName);
    if (F && !F->isDeclaration())
      return M;
    if (!CheckFunctionsOnly) {
      GlobalVariable *G = M->getGlobalVariable(DemangledName);
      if (G && !G->isDeclaration())
        return M;
      // FIXME: Do we need to worry about global aliases?
    }
  }
  // We didn't find the symbol in any of our modules.
  return nullptr;
}

uint64_t LLDJIT::getSymbolAddress(const std::string &Name,
                                  bool CheckFunctionsOnly) {
  std::string MangledName = Name;
  if (auto Sym = findSymbol(MangledName, CheckFunctionsOnly)) {
    if (auto AddrOrErr = Sym.getAddress())
      return *AddrOrErr;
    else
      report_fatal_error(AddrOrErr.takeError());
  } else if (auto Err = Sym.takeError())
    report_fatal_error(Sym.takeError());
  return 0;
}

JITSymbol LLDJIT::findSymbol(const std::string &Name, bool CheckFunctionsOnly) {
  std::lock_guard<sys::Mutex> locked(lock);

  // First, check to see if we already have this symbol.
  if (auto Sym = findExistingSymbol(Name))
    return Sym;

  // If it hasn't already been generated, see if it's in one of our modules.
  Module *M = findModuleForSymbol(Name, CheckFunctionsOnly);
  if (M) {
    generateCodeForModule(M);

    // Check the RuntimeDyld table again, it should be there now.
    return findExistingSymbol(Name);
  }

  // If a LazyFunctionCreator is installed, use it to get/create the function.
  // FIXME: Should we instead have a LazySymbolCreator callback?
  if (LazyFunctionCreator) {
    auto Addr = static_cast<uint64_t>(
        reinterpret_cast<uintptr_t>(LazyFunctionCreator(Name)));
    return JITSymbol(Addr, JITSymbolFlags::Exported);
  }

  return nullptr;
}

uint64_t LLDJIT::getGlobalValueAddress(const std::string &Name) {
  std::lock_guard<sys::Mutex> locked(lock);
  uint64_t Result = getSymbolAddress(Name, false);
  if (Result != 0)
    finalizeLoadedModules();
  return Result;
}

uint64_t LLDJIT::getFunctionAddress(const std::string &Name) {
  std::lock_guard<sys::Mutex> locked(lock);
  uint64_t Result = getSymbolAddress(Name, true);
  return Result;
}

// Deprecated.  Use getFunctionAddress instead.
void *LLDJIT::getPointerToFunction(Function *F) {
  std::lock_guard<sys::Mutex> locked(lock);

  Mangler Mang;
  SmallString<128> Name;
  TM->getNameWithPrefix(Name, F, Mang);

  if (F->isDeclaration() || F->hasAvailableExternallyLinkage()) {
    bool AbortOnFailure = !F->hasExternalWeakLinkage();
    void *Addr = getPointerToNamedFunction(Name, AbortOnFailure);
    updateGlobalMapping(F, Addr);
    return Addr;
  }

  Module *M = F->getParent();
  bool HasBeenAddedButNotLoaded =
      OwnedModules.hasModuleBeenAddedButNotLoaded(M);

  // Make sure the relevant module has been compiled and loaded.
  if (HasBeenAddedButNotLoaded)
    generateCodeForModule(M);
  else if (!OwnedModules.hasModuleBeenLoaded(M)) {
    // If this function doesn't belong to one of our modules, we're done.
    // FIXME: Asking for the pointer to a function that hasn't been registered,
    //        and isn't a declaration (which is handled above) should probably
    //        be an assertion.
    return nullptr;
  }

  return nullptr;
}

void LLDJIT::runStaticConstructorsDestructorsInModulePtrSet(
    bool isDtors, ModulePtrSet::iterator I, ModulePtrSet::iterator E) {
  llvm_unreachable("Not supported");
}

void LLDJIT::runStaticConstructorsDestructors(bool isDtors) {
  if (isDtors) {
    llvm_unreachable("Calling destructors explicitly is not supported");
  }
}

Function *LLDJIT::FindFunctionNamedInModulePtrSet(StringRef FnName,
                                                  ModulePtrSet::iterator I,
                                                  ModulePtrSet::iterator E) {
  for (; I != E; ++I) {
    Function *F = (*I)->getFunction(FnName);
    if (F && !F->isDeclaration())
      return F;
  }
  return nullptr;
}

GlobalVariable *LLDJIT::FindGlobalVariableNamedInModulePtrSet(
    StringRef Name, bool AllowInternal, ModulePtrSet::iterator I,
    ModulePtrSet::iterator E) {
  for (; I != E; ++I) {
    GlobalVariable *GV = (*I)->getGlobalVariable(Name, AllowInternal);
    if (GV && !GV->isDeclaration())
      return GV;
  }
  return nullptr;
}

Function *LLDJIT::FindFunctionNamed(StringRef FnName) {
  Function *F = FindFunctionNamedInModulePtrSet(
      FnName, OwnedModules.begin_added(), OwnedModules.end_added());
  if (!F)
    F = FindFunctionNamedInModulePtrSet(FnName, OwnedModules.begin_loaded(),
                                        OwnedModules.end_loaded());
  if (!F)
    F = FindFunctionNamedInModulePtrSet(FnName, OwnedModules.begin_finalized(),
                                        OwnedModules.end_finalized());
  return F;
}

GlobalVariable *LLDJIT::FindGlobalVariableNamed(StringRef Name,
                                                bool AllowInternal) {
  GlobalVariable *GV = FindGlobalVariableNamedInModulePtrSet(
      Name, AllowInternal, OwnedModules.begin_added(),
      OwnedModules.end_added());
  if (!GV)
    GV = FindGlobalVariableNamedInModulePtrSet(Name, AllowInternal,
                                               OwnedModules.begin_loaded(),
                                               OwnedModules.end_loaded());
  if (!GV)
    GV = FindGlobalVariableNamedInModulePtrSet(Name, AllowInternal,
                                               OwnedModules.begin_finalized(),
                                               OwnedModules.end_finalized());
  return GV;
}

GenericValue LLDJIT::runFunction(Function *F,
                                 ArrayRef<GenericValue> ArgValues) {
  assert(F && "Function *F was null at entry to run()");

  void *FPtr = getPointerToFunction(F);
  finalizeModule(F->getParent());
  assert(FPtr && "Pointer to fn's code was null after getPointerToFunction");
  FunctionType *FTy = F->getFunctionType();
  Type *RetTy = FTy->getReturnType();

  assert((FTy->getNumParams() == ArgValues.size() ||
          (FTy->isVarArg() && FTy->getNumParams() <= ArgValues.size())) &&
         "Wrong number of arguments passed into function!");
  assert(FTy->getNumParams() == ArgValues.size() &&
         "This doesn't support passing arguments through varargs (yet)!");

  // Handle some common cases first.  These cases correspond to common `main'
  // prototypes.
  if (RetTy->isIntegerTy(32) || RetTy->isVoidTy()) {
    switch (ArgValues.size()) {
    case 3:
      if (FTy->getParamType(0)->isIntegerTy(32) &&
          FTy->getParamType(1)->isPointerTy() &&
          FTy->getParamType(2)->isPointerTy()) {
        int (*PF)(int, char **, const char **) =
            (int (*)(int, char **, const char **))(intptr_t)FPtr;

        // Call the function.
        GenericValue rv;
        rv.IntVal = APInt(32, PF(ArgValues[0].IntVal.getZExtValue(),
                                 (char **)GVTOP(ArgValues[1]),
                                 (const char **)GVTOP(ArgValues[2])));
        return rv;
      }
      break;
    case 2:
      if (FTy->getParamType(0)->isIntegerTy(32) &&
          FTy->getParamType(1)->isPointerTy()) {
        int (*PF)(int, char **) = (int (*)(int, char **))(intptr_t)FPtr;

        // Call the function.
        GenericValue rv;
        rv.IntVal = APInt(32, PF(ArgValues[0].IntVal.getZExtValue(),
                                 (char **)GVTOP(ArgValues[1])));
        return rv;
      }
      break;
    case 1:
      if (FTy->getNumParams() == 1 && FTy->getParamType(0)->isIntegerTy(32)) {
        GenericValue rv;
        int (*PF)(int) = (int (*)(int))(intptr_t)FPtr;
        rv.IntVal = APInt(32, PF(ArgValues[0].IntVal.getZExtValue()));
        return rv;
      }
      break;
    }
  }

  // Handle cases where no arguments are passed first.
  if (ArgValues.empty()) {
    GenericValue rv;
    switch (RetTy->getTypeID()) {
    default:
      llvm_unreachable("Unknown return type for function call!");
    case Type::IntegerTyID: {
      unsigned BitWidth = cast<IntegerType>(RetTy)->getBitWidth();
      if (BitWidth == 1)
        rv.IntVal = APInt(BitWidth, ((bool (*)())(intptr_t)FPtr)());
      else if (BitWidth <= 8)
        rv.IntVal = APInt(BitWidth, ((char (*)())(intptr_t)FPtr)());
      else if (BitWidth <= 16)
        rv.IntVal = APInt(BitWidth, ((short (*)())(intptr_t)FPtr)());
      else if (BitWidth <= 32)
        rv.IntVal = APInt(BitWidth, ((int (*)())(intptr_t)FPtr)());
      else if (BitWidth <= 64)
        rv.IntVal = APInt(BitWidth, ((int64_t(*)())(intptr_t)FPtr)());
      else
        llvm_unreachable("Integer types > 64 bits not supported");
      return rv;
    }
    case Type::VoidTyID:
      rv.IntVal = APInt(32, ((int (*)())(intptr_t)FPtr)());
      return rv;
    case Type::FloatTyID:
      rv.FloatVal = ((float (*)())(intptr_t)FPtr)();
      return rv;
    case Type::DoubleTyID:
      rv.DoubleVal = ((double (*)())(intptr_t)FPtr)();
      return rv;
    case Type::X86_FP80TyID:
    case Type::FP128TyID:
    case Type::PPC_FP128TyID:
      llvm_unreachable("long double not supported yet");
    case Type::PointerTyID:
      return PTOGV(((void *(*)())(intptr_t)FPtr)());
    }
  }

  report_fatal_error("LLDJIT::runFunction does not support full-featured "
                     "argument passing. Please use "
                     "ExecutionEngine::getFunctionAddress and cast the result "
                     "to the desired function pointer type.");
}

void *LLDJIT::getPointerToNamedFunction(StringRef Name, bool AbortOnFailure) {
  llvm_unreachable("Not supported");
  if (AbortOnFailure) {
    report_fatal_error("Program used external function '" + Name +
                       "' which could not be resolved!");
  }
  return nullptr;
}

void LLDJIT::RegisterJITEventListener(JITEventListener *L) {
  if (!L)
    return;
  std::lock_guard<sys::Mutex> locked(lock);
  EventListeners.push_back(L);
}

void LLDJIT::UnregisterJITEventListener(JITEventListener *L) {
  if (!L)
    return;
  std::lock_guard<sys::Mutex> locked(lock);
  auto I = find(reverse(EventListeners), L);
  if (I != EventListeners.rend()) {
    std::swap(*I, EventListeners.back());
    EventListeners.pop_back();
  }
}

LLDJIT::TmpFile::TmpFile(const llvm::Twine &Prefix,
                         llvm::StringRef FileExtension) {
  int FD;
  SmallString<128> ResultPath;
  // const Twine &Model, int &ResultFd,SmallVectorImpl<char>
  // &ResultPath,unsigned Mode, OpenFlags Flags
  if (std::error_code EC = llvm::sys::fs::createTemporaryFile(
          Prefix, FileExtension, FD, ResultPath))
    report_fatal_error("Failed to create a temporary file on the system!");
  File = std::make_unique<llvm::ToolOutputFile>(ResultPath, FD);
  Name = ResultPath.str();
}

void LLDJIT::notifyObjectLoaded(const object::ObjectFile &Obj,
                                const RuntimeDyld::LoadedObjectInfo &L) {
  uint64_t Key =
      static_cast<uint64_t>(reinterpret_cast<uintptr_t>(Obj.getData().data()));
  std::lock_guard<sys::Mutex> locked(lock);
  // MemMgr->notifyObjectLoaded(this, Obj);
  for (unsigned I = 0, S = EventListeners.size(); I < S; ++I) {
    EventListeners[I]->notifyObjectLoaded(Key, Obj, L);
  }
}

void LLDJIT::notifyFreeingObject(const object::ObjectFile &Obj) {
  uint64_t Key =
      static_cast<uint64_t>(reinterpret_cast<uintptr_t>(Obj.getData().data()));
  std::lock_guard<sys::Mutex> locked(lock);
  for (JITEventListener *L : EventListeners)
    L->notifyFreeingObject(Key);
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
