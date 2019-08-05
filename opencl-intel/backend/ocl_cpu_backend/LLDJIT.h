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

#pragma once

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/ObjectCache.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/RuntimeDyld.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SmallVectorMemoryBuffer.h"
#include "llvm/Support/ToolOutputFile.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/// @brief LLDJIT executes code by building and using shared libraries.
/// LLDJIT links the modules with LLD and loads them into the process with LoadLibrary().
/// Compared to MCJIT, this allows to create debuggable JIT code for Visual
/// Studio. Currently, its only used to debug natively with visual studio and
/// other use-cases are not supported.
/// 1. addModule(module) == Take ownership of module
/// 2. addObjectFile(chkstk) == Add precompiled objects to the linker
/// 3. generateCodeForModule(module) == build OBJ file
/// 4. finalizeObject(module) == build DLL file from OBJ file
/// 5. runStaticConstructorsDestructors(module) == call LoadLibrary()
class LLDJIT : public llvm::ExecutionEngine {
  LLDJIT(std::unique_ptr<llvm::Module> M,
         std::unique_ptr<llvm::TargetMachine> TM);

  typedef llvm::SmallPtrSet<llvm::Module *, 4> ModulePtrSet;

  /// @briefTmpFile will create a temporary file in temp directory that will be
  /// removed automatically when the instance is destrcuted.
  ///
  /// After playing with llvm::sys::fs:TempFile and llvm::ToolOutputFile, I have
  /// decided to create yet another version.
  /// *::fs::TempFile was removing file on close on Windows and can't be opened a
  /// second time, however it has nice interface.
  /// llvm::ToolOutputFile is usable, but it is not giving access to the
  /// filename, it can't generate unique file name and it is not movable. So I'm
  /// wrapping ToolOutputfile to the needs of LLDJIT to make the code more
  /// readable.
  class TmpFile {

	 // unique_ptr is needed to make it movable
    std::unique_ptr<llvm::ToolOutputFile> File;
    std::string Name;

    // Non-copyable, but movable with std::move
    TmpFile::TmpFile(const TmpFile &) = delete;
    TmpFile &TmpFile::operator=(const TmpFile &) = delete;

  public:
    TmpFile &operator=(TmpFile &&other) = default;

    TmpFile(TmpFile &&o) noexcept
        : File(std::move(o.File)), Name(std::move(o.Name)) {}

    TmpFile(const llvm::Twine &Prefix, llvm::StringRef FileExtension);

    const std::string &FileName() { return Name; }
    llvm::raw_fd_ostream &OS() { return File->os(); }
    void keep() { File->keep(); }
  };

private:
  class OwningModuleContainer {
  public:
    OwningModuleContainer() {}
    ~OwningModuleContainer() {
      freeModulePtrSet(AddedModules);
      freeModulePtrSet(LoadedModules);
      freeModulePtrSet(FinalizedModules);
    }

    ModulePtrSet::iterator begin_added() { return AddedModules.begin(); }
    ModulePtrSet::iterator end_added() { return AddedModules.end(); }
    llvm::iterator_range<ModulePtrSet::iterator> added() {
      return make_range(begin_added(), end_added());
    }

    ModulePtrSet::iterator begin_loaded() { return LoadedModules.begin(); }
    ModulePtrSet::iterator end_loaded() { return LoadedModules.end(); }
    llvm::iterator_range<ModulePtrSet::iterator> loaded() {
      return make_range(begin_loaded(), end_loaded());
    }

    ModulePtrSet::iterator begin_finalized() {
      return FinalizedModules.begin();
    }
    ModulePtrSet::iterator end_finalized() { return FinalizedModules.end(); }
    llvm::iterator_range<ModulePtrSet::iterator> finalized() {
      return make_range(begin_finalized(), end_finalized());
    }

    void addModule(std::unique_ptr<llvm::Module> M) {
      AddedModules.insert(M.release());
    }

    bool removeModule(llvm::Module *M) {
      return AddedModules.erase(M) || LoadedModules.erase(M) ||
             FinalizedModules.erase(M);
    }

    bool hasModuleBeenAddedButNotLoaded(llvm::Module *M) {
      return AddedModules.count(M) != 0;
    }

    bool hasModuleBeenLoaded(llvm::Module *M) {
      // If the module is in either the "loaded" or "finalized" sections it
      // has been loaded.
      return (LoadedModules.count(M) != 0) || (FinalizedModules.count(M) != 0);
    }

    bool hasModuleBeenFinalized(llvm::Module *M) {
      return FinalizedModules.count(M) != 0;
    }

    bool ownsModule(llvm::Module *M) {
      return (AddedModules.count(M) != 0) || (LoadedModules.count(M) != 0) ||
             (FinalizedModules.count(M) != 0);
    }

    void markModuleAsLoaded(llvm::Module *M) {
      // This checks against logic errors in the LLDJIT implementation.
      // This function should never be called with either a Module that LLDJIT
      // does not own or a Module that has already been loaded and/or finalized.
      assert(AddedModules.count(M) &&
             "markModuleAsLoaded: Module not found in AddedModules");

      // Remove the module from the "Added" set.
      AddedModules.erase(M);

      // Add the Module to the "Loaded" set.
      LoadedModules.insert(M);
    }

    void markModuleAsFinalized(llvm::Module *M) {
      // This checks against logic errors in the LLDJIT implementation.
      // This function should never be called with either a Module that LLDJIT
      // does not own, a Module that has not been loaded or a Module that has
      // already been finalized.
      assert(LoadedModules.count(M) &&
             "markModuleAsFinalized: Module not found in LoadedModules");

      // Remove the module from the "Loaded" section of the list.
      LoadedModules.erase(M);

      // Add the Module to the "Finalized" section of the list by inserting it
      // before the 'end' iterator.
      FinalizedModules.insert(M);
    }

    void markAllLoadedModulesAsFinalized() {
      for (ModulePtrSet::iterator I = LoadedModules.begin(),
                                  E = LoadedModules.end();
           I != E; ++I) {
        llvm::Module *M = *I;
        FinalizedModules.insert(M);
      }
      LoadedModules.clear();
    }

  private:
    ModulePtrSet AddedModules;
    ModulePtrSet LoadedModules;
    ModulePtrSet FinalizedModules;

    void freeModulePtrSet(ModulePtrSet &MPS) {
      // Go through the module set and delete everything.
      for (ModulePtrSet::iterator I = MPS.begin(), E = MPS.end(); I != E; ++I) {
        llvm::Module *M = *I;
        delete M;
      }
      MPS.clear();
    }
  };

  std::string Triple;
  std::unique_ptr<llvm::TargetMachine> TM;
  llvm::SmallVector<llvm::JITEventListener *, 1> EventListeners;
  llvm::SmallVector<std::string, 2> LoadedObjects;
  llvm::SmallVector<TmpFile, 4> OwnedTempFiles;
  std::string DLLPath;
  void *DLLHandle;
  OwningModuleContainer OwnedModules;
  llvm::SmallVector<std::string, 1> ArgvLLD;

  llvm::Function *FindFunctionNamedInModulePtrSet(llvm::StringRef FnName,
                                                  ModulePtrSet::iterator I,
                                                  ModulePtrSet::iterator E);

  llvm::GlobalVariable *FindGlobalVariableNamedInModulePtrSet(
      llvm::StringRef Name, bool AllowInternal, ModulePtrSet::iterator I,
      ModulePtrSet::iterator E);

  void runStaticConstructorsDestructorsInModulePtrSet(bool isDtors,
                                                      ModulePtrSet::iterator I,
                                                      ModulePtrSet::iterator E);

public:
  ~LLDJIT() override;

  /// @name ExecutionEngine interface implementation
  /// @{
  void addModule(std::unique_ptr<llvm::Module> M) override;
  void addObjectFile(std::unique_ptr<llvm::object::ObjectFile> O) override;
  void addObjectFile(
      llvm::object::OwningBinary<llvm::object::ObjectFile> O) override;
  void addObjectFile(const std::string &FilePath);
  void addArchive(llvm::object::OwningBinary<llvm::object::Archive> O) override;
  bool removeModule(llvm::Module *M) override;

  /// FindFunctionNamed - Search all of the active modules to find the function
  /// that defines FnName.  This is very slow operation and shouldn't be used
  /// for general code.
  llvm::Function *FindFunctionNamed(llvm::StringRef FnName) override;

  /// FindGlobalVariableNamed - Search all of the active modules to find the
  /// global variable that defines Name.  This is very slow operation and
  /// shouldn't be used for general code.
  llvm::GlobalVariable *
  FindGlobalVariableNamed(llvm::StringRef Name,
                          bool AllowInternal = false) override;

  /// Sets the object manager that LLDJIT should use to avoid compilation.
  void setObjectCache(llvm::ObjectCache *Manager) override;

  void setProcessAllSections(bool ProcessAllSections) override {
    assert(!"Not supported");
  }

  void generateCodeForModule(llvm::Module *M) override;

  /// finalizeObject - ensure the module is fully processed and is usable.
  ///
  /// It is the user-level function for completing the process of making the
  /// object usable for execution. It should be called after sections within an
  /// object have been relocated using mapSectionAddress.  When this method is
  /// called the LLDJIT execution engine will reapply relocations for a loaded
  /// object.
  /// Is it OK to finalize a set of modules, add modules and finalize again.
  // FIXME: Do we really need both of these?
  void finalizeObject() override;
  virtual void finalizeModule(llvm::Module *);
  void finalizeLoadedModules();

  /// runStaticConstructorsDestructors - This method is used to execute all of
  /// the static constructors or destructors for a program.
  ///
  /// \param isDtors - Run the destructors instead of constructors.
  void runStaticConstructorsDestructors(bool isDtors) override;

  void *getPointerToFunction(llvm::Function *F) override;

  llvm::GenericValue
  runFunction(llvm::Function *F,
              llvm::ArrayRef<llvm::GenericValue> ArgValues) override;

  /// getPointerToNamedFunction - This method returns the address of the
  /// specified function by using the dlsym function call.  As such it is only
  /// useful for resolving library symbols, not code generated symbols.
  ///
  /// If AbortOnFailure is false and no function with the given name is
  /// found, this function silently returns a null pointer. Otherwise,
  /// it prints a message to stderr and aborts.
  ///
  void *getPointerToNamedFunction(llvm::StringRef Name,
                                  bool AbortOnFailure = true) override;

  /// mapSectionAddress - map a section to its target address space value.
  /// Map the address of a JIT section as returned from the memory manager
  /// to the address in the target process as the running code will see it.
  /// This is the address which will be used for relocation resolution.
  void mapSectionAddress(const void *LocalAddress,
                         uint64_t TargetAddress) override {
    assert(!"Not supported");
  }
  void RegisterJITEventListener(llvm::JITEventListener *L) override;
  void UnregisterJITEventListener(llvm::JITEventListener *L) override;

  // If successful, these function will implicitly finalize all loaded objects.
  // To get a function address within LLDJIT without causing a finalize, use
  // getSymbolAddress.
  uint64_t getGlobalValueAddress(const std::string &Name) override;
  uint64_t getFunctionAddress(const std::string &Name) override;

  llvm::TargetMachine *getTargetMachine() override { return TM.get(); }

  static ExecutionEngine *createJIT(std::unique_ptr<llvm::Module> M,
                                    std::string *ErrorStr,
                                    std::unique_ptr<llvm::TargetMachine> TM);

  // @}

  // Takes a mangled name and returns the corresponding JITSymbol (if a
  // definition of that mangled name has been added to the JIT).
  llvm::JITSymbol findSymbol(const std::string &Name, bool CheckFunctionsOnly);

  // DEPRECATED - Please use findSymbol instead.
  //
  // This is not directly exposed via the ExecutionEngine API, but it is
  // used by the LinkingMemoryManager.
  //
  // getSymbolAddress takes an unmangled name and returns the corresponding
  // JITSymbol if a definition of the name has been added to the JIT.
  uint64_t getSymbolAddress(const std::string &Name, bool CheckFunctionsOnly);

protected:
  /// emitObject -- Generate a JITed object in memory from the specified module
  /// Currently, LLDJIT only supports a single module and the module passed to
  /// this function call is expected to be the contained module.  The module
  /// is passed as a parameter here to prepare for multiple module support in
  /// the future.
  std::string emitObject(llvm::Module *M);

  void notifyObjectLoaded(const llvm::object::ObjectFile &Obj,
                          const llvm::RuntimeDyld::LoadedObjectInfo &L);
  void notifyFreeingObject(const llvm::object::ObjectFile &Obj);

  llvm::JITSymbol findExistingSymbol(const std::string &Name);
  llvm::Module *findModuleForSymbol(const std::string &Name,
                                    bool CheckFunctionsOnly);
  void DeleteTempFiles();
  void DeleteTempFiles(const llvm::SmallVectorImpl<std::string> &FilesToDelete);
  void LoadDLL(llvm::Module *M);
  void mapDllFunctions(llvm::Module *M, void *DllHandle);
  std::string getLastErrorMessage();
  void buildDllFromObjs(const llvm::SmallVector<std::string, 2> &InObjFiles,
                        const std::string &OutDLLPath,
                        const std::string &OutPDBPath);
  int compileSymbolJumpTable(
      const llvm::SmallVector<std::string, 2> &InObjFiles,
      llvm::raw_fd_ostream *OutObjectFileStream);
  std::set<std::string>
  getExternalSymbolsList(const llvm::SmallVector<std::string, 2> &ObjectFiles);
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
