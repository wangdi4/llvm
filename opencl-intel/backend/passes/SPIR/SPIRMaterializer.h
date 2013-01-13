///***************************************************************************/
//
//  Copyright (c) Intel Corporation (2012).
//
//  INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
//  LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
//  ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
//  PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
//  DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
//  PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
//  including liability for infringement of any proprietary rights, relating to
//  use of the code. No license, express or implied, by estoppel or otherwise,
//  to any intellectual property rights is granted herein.
//
//  File Name: SPIRMaterializer.h
//
//  Abstract:  Class that translates an LLVM module generated for SPIR to one
//             for another target.
//
//  Notes:     None
//
///***************************************************************************/
#pragma once

#include <set>
#include "llvm/Pass.h"

namespace llvm {
  class Module;
  class Pass;
  class LLVMContext;
  class BasicBlock;
  class BasicBlockPass;
  class MDNode;
  class Type;
  class TargetData;
  class TargetMachine;
  class CallInst;
  class BasicBlock;
}

using namespace llvm;

class MaterializerPass : public BasicBlockPass {
  const TargetMachine *m_pTM;
  const TargetData *m_pTargetData;
  bool m_bInitialized;

  // Opaque types after being defined
  Type *pTypeSizet;

  LLVMContext &m_Context;
  uint64_t m_SpirVersion;
  uint64_t m_OCLVersion;
  StringRef m_CompileOptions;
  StringRef m_ExtCompileOptions;
  StringRef m_OptionalFeatures;
  StringRef m_Extensions;

  enum SPIR_ADDRESS_SPACE {
    SPIR_ADDRESS_SPACE_PRIVATE = 0,
    SPIR_ADDRESS_SPACE_GLOBAL,
    SPIR_ADDRESS_SPACE_CONSTANT,
    SPIR_ADDRESS_SPACE_LOCAL
  };

public:
  static char ID; // Pass identification, replacement for typeid

  // Pass constructors
  explicit MaterializerPass(const char *pTargetTriple = NULL);

  // Scan a basic block and perform instruction transformation
  bool runOnBasicBlock(BasicBlock &BB);

  // Pass initialization. All of the module changes occur here
  bool doInitialization(Module &Module);

  // Translation function that takes a SPIR module or buffer  to return a
  // target module
  Module *MaterializeSPIR(Module *pSpirModule);
  Module *MaterializeSPIR(const char *pSpirBuffer, unsigned long ModuleSize);

  // Access methods
  uint64_t getSPIRVersion() { return m_SpirVersion; }
  uint64_t getCompileOptions() { return m_OCLVersion; }
  StringRef getOCLVersion() { return m_CompileOptions; }
  StringRef getOptionalFeatures() { return m_OptionalFeatures; }
  StringRef getExtensions() { return m_Extensions; }

  // Helper functions
  unsigned int GetTypeSize(Type *pType);

private:
  // Helper function to change the name of a metadata by adding a metadata with
  // the new name, copying the operands and erasing the old metadata name
  void TranslateMetadataName(Module *pModule,
                             const char *pCurName,
                             const char *pNewName);

  // Helper functions to handle special processing for a particular metadata
  // name
  void TranslateAccessQualifier(Function *pKFunction, MDNode *pMDElem);
  void TranslateVecTypeHint(MDNode *pMDElem);

  // Helper function to remove a metadata from a module
  void DeleteMetadata(Module *pModule, const char *pMetadataName);

  // Helper function to extract a string referenced in a named metadata object
  StringRef GetStringObject(Module *pModule, const char *pMetadataName);

  // Helper function to compute the sum of the object referenced in a named
  // metadata object
  uint64_t GetTupleValue(Module *pModule, const char *pMetadataName);

  bool ProcessFunctionMetadata(MDNode* pKernelMD);

  // Allocates a size_t and replaces the function call with a pointer to it.
  // Returns true if it succeeds and the call is replaced
//  bool ProcessSizetAllocation(BasicBlock::iterator *pBBIter, CallInst *pFCall);
  bool ProcessSizetAllocation(void *pVoid, CallInst *pFCall);

  bool ProcessBuiltInfunctions(void *pVoid, CallInst *pFCall);

  // Remove the function decorations and return the unmangled name
  std::string GetUnmangledName(const std::string &MangledName);

  Function* m_pGetGlobalIdFunc;

  static std::set<std::string> m_oclBuiltinsSet;

}; // class MaterializerPass
