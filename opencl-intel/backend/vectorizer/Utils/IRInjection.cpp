/*********************************************************************************************
 * Copyright � 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/

// This pass is a development utility that replaces the IR in an existing module With
// IR that is in some file. This can be helpful if we want to measure the implications
// of some transformation on specific benchmark before actually implementing it.

// This Pass is heavily based on CloneModule utility, however it modifies the cloning
// implementation, to do the cloning from a module written in some file into an 
// exiting module (and not a new one).
// In the way the pass keeps all pointers to function \ global variable that existed in
// module prior to the injection. This means that if a function exits in the moudle 
// before the pass, and exits also in the new module, then the contents of the new
// function is copied into the pre-exitising function, and any reference to the old
// function is still valid. This also implies that if both the old and the new module
// have the same function, it should have the same signature. An exeption is in case
// of opaque pointers (images) that can't have the same type. These are special case
// handled and bitcasts are added as neccesarry.
// Author: Ran Chachick.

#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Type.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include "RuntimeServices.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "VectorizerUtils.h"
#include "llvm/Support/CommandLine.h"
#include <iostream>
#include <fstream>
using namespace llvm;

static cl::opt<std::string>
newModPath("new-mod-path", cl::init(""), cl::Hidden,
  cl::desc("name of new module path"));


// To be used in apple environment - get the module path from specified file.
#define ModulePathFileName "/tmp/vectorizer_replace_IR_module_path.txt"

namespace intel {

class IRInjectModule : public ModulePass {
public:

  static char ID;

  ///@brief holds mapping of values in the new module to inject values.
  ValueToValueMapTy m_VMap;
  
  ///@brief current module
  Module *m_M;

  ///@brief holds path to the new module;
  std::string m_newModulePath;

  /// @brief C'tor
  IRInjectModule(std::string newModulePath=""): ModulePass(ID){
    m_newModulePath = newModulePath;
  };
  /// @brief D'tor
  ~IRInjectModule(){};
  /// @brief Provides name of pass
  virtual const char *getPassName() const {
    return "IRInjectModule";
  }

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
  }

  
  virtual bool runOnModule(Module &M) {
    errs() << "\n init val:@" << newModPath << "@\n";
    

    // Obtain the path for the file with the new module.
    obtainNewModulePath();
    if (m_newModulePath == "") {
      errs() << "no module path - abort\n";
      return false;
    } else {
      errs() << "module path is:  " << m_newModulePath <<"\n";
    }
    
    m_M = &M;
    // Initialize the value map.
    m_VMap.clear(); 

    // Obtain the module from file.
    SMDiagnostic Err;
    Module *newM = ParseIRFile(m_newModulePath, Err, M.getContext());
    if (!newM) {
        errs() << "unable to parse IR from module file\n";
        return false;
    }
    // Map global variables.
    errs() << "will map global vars\n";
    MapGlobalVars(newM);

    // Map declaration of the parsed modules to these of the current module
    errs() << "will map function declarations\n";
    MapFuncDecl(newM);

    // Initalize global variable (big static tables etc...)
    InitializeGlobalVars(newM);

    // Replace function implementation of the current module with these of the new module.
    errs() << "will replace function implementations\n";
    replaceFuncImpl(newM);

    // This problematic - it is the duplicating the kernels metadata - avoid doing it 
    // for the time being.
    //errs() << "will map metadata\n";
    //MapMetaData(newM);
    return true;
  }

  
private:

  void obtainNewModulePath() {
    // Check if path was already set by the constructor.
    if (m_newModulePath != "") {
      errs() << "\nmodule path was set constructor\n";
      return;
    }

    // See if the path was set by constructor \ command line
    if (newModPath != "") {
      errs() << "\nmodule path was by command line\n";
      m_newModulePath = newModPath;
      return;
    }
    
    // See if the path is with environment variable
    char *envVar;
    envVar = getenv("VECT_ModulePathFileName");
    if (envVar) {
      errs() << "\nmodule path was by environment variable\n";
      std::string pathByEnv(envVar);
      m_newModulePath = pathByEnv;
      return;
    }

    // Finally if all options fail try to read path from file. (For Apple)
    std::ifstream modulePathFile(ModulePathFileName);
    if (modulePathFile.is_open()) {
      if (modulePathFile.good()) {
        errs() << "\nmodule path is set by environment variable\n";
        std::string pathByFile;
        std::getline (modulePathFile, pathByFile);
        m_newModulePath = pathByFile;
        return;
      }
    }
  }
  
  void MapGlobalVars(Module *newM) {
    for (Module::const_global_iterator I = newM->global_begin(),
         E = newM->global_end(); I != E; ++I) {
      GlobalVariable *GV = m_M->getGlobalVariable(I->getName(), true);
      if (!GV) {
        GV = new GlobalVariable(*m_M,
                                I->getType()->getElementType(), 
                                I->isConstant(),
                                I->getLinkage(),
                                (Constant*) 0,
                                I->getName(),
                                (GlobalVariable*) 0,
                                I->isThreadLocal(),
                                I->getType()->getAddressSpace());
      }
      GV->copyAttributesFrom(I);
      m_VMap[I] = GV;
    }
  }

  void MapFuncDecl(Module *newM) {
    // Loop over the functions in the module, making external functions as before
    for (Module::const_iterator I = newM->begin(), E = newM->end();
         I != E; ++I) {
      errs() << "  will map decl for: " << I->getNameStr() << "\n";
      Function *curF = m_M->getFunction(I->getName());
      if (!curF) {
        errs() << "    funtion is not in module - will create new one\n";
        curF = Function::Create(cast<FunctionType>(I->getType()->getElementType()),
                       I->getLinkage(), I->getName(), m_M);
      }
      curF->copyAttributesFrom(I);
      m_VMap[I] = curF;
    }
  }

  void InitializeGlobalVars(Module *newM) {
    for (Module::const_global_iterator I = newM->global_begin(),
         E = newM->global_end(); I != E; ++I) {
     GlobalVariable *GV = cast<GlobalVariable>(m_VMap[I]);
     if (I->hasInitializer())
       GV->setInitializer(MapValue(I->getInitializer(), m_VMap));
    }
  }


  void replaceFuncImpl(Module *newM) {
    for (Module::iterator F = m_M->begin(), E = m_M->end(); F != E; ++F) {
      Function *newF = getNewFunc(newM, F);
      if (!newF || newF->isDeclaration()) continue;
      
      errs() << "  will replace implemntation for " << F->getNameStr() << "\n";

      F->deleteBody();
      for (Function::arg_iterator J = F->arg_begin(), I = newF->arg_begin();
           J != F->arg_end();) {
        I->setName(J->getName());
        m_VMap[I++] = J++;
      }
      SmallVector<ReturnInst*, 8> Returns;  // Ignore returns cloned.
      
      CloneFunctionInto(F, newF, m_VMap, /*ModuleLevelChanges=*/true, Returns);
    }

    // Fixing cases of opaque ptr since their type is changes into different opaque ptr.
    // add bitcasts before calling functions and in functions start as neccessary.
    fixOpaquePtrs(newM);
  }
  
 
  void MapMetaData(Module *newM) {
  // And named metadata....
    for (Module::const_named_metadata_iterator I = newM->named_metadata_begin(),
           E = newM->named_metadata_end(); I != E; ++I) {
      const NamedMDNode &NMD = *I;
      NamedMDNode *NewNMD = m_M->getOrInsertNamedMetadata(NMD.getName());
      for (unsigned i = 0, e = NMD.getNumOperands(); i != e; ++i)
        NewNMD->addOperand(MapValue(NMD.getOperand(i), m_VMap));
    }
  }

  void fixOpaquePtrs(Module *newM) {
    for (Module::iterator newF = newM->begin(), E = newM->end(); newF != E; ++newF) {
      Function *F = cast<Function>(m_VMap[newF]);
      
      unsigned argInd = 0;
      for (Function::arg_iterator J = F->arg_begin(), I = newF->arg_begin();
          J != F->arg_end(); ++I, ++J, ++argInd){
        if (I->getType() != J->getType()) {
          std::vector<User *> argUsers (J->use_begin(), J->use_end()); 
          if (argUsers.size()) {
            Value *cast = new BitCastInst(J, I->getType(), "arg_cast", F->getEntryBlock().begin());
            for (unsigned i=0; i<argUsers.size(); ++i) {
              argUsers[i]->replaceUsesOfWith(J, cast);
            }
          }

          std::vector<User *> funcUsers (F->use_begin(), F->use_end()); 
          for (unsigned i=0; i<funcUsers.size(); ++i) {
            if (CallInst *CI = dyn_cast<CallInst>(funcUsers[i])) {
              if (CI->getCalledFunction() == F) {
                Value *cast = new BitCastInst(CI->getArgOperand(argInd), J->getType(), "arg_cast", CI);
                CI->setArgOperand(argInd, cast);
              }
            }
          }
        }
      }
    }
  }
  
  Function *getNewFunc(Module *newM, Function *F) {
    FunctionType *FTy = F->getFunctionType();
    Function *newF = newM->getFunction(F->getName());
    if (!newF) return NULL;
    if (newF->arg_size() != F->arg_size()) return NULL;

    FunctionType *newFTy = newF->getFunctionType();
    for (unsigned j=0; j<FTy->getNumParams(); j++) {
      Type *Ty = FTy->getParamType(j);
      Type *newTy = newFTy->getParamType(j);
      if (!VectorizerUtils::isOpaquePtrPair(Ty, newTy) && Ty != newTy) return NULL;
    }
    return newF;
  }

};

}// namspace intel

char intel::IRInjectModule::ID = 0;
extern "C" {
  
  ModulePass* createIRInjectModulePass() {
    return new intel::IRInjectModule();
  }
}
static RegisterPass<intel::IRInjectModule> IRInjectModule("IRInjectModule", "inject IR from different module");

