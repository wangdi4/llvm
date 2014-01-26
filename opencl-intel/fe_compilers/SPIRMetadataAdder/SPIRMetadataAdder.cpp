//===- SPIRMetadataAdder.cpp - Add SPIR related module scope metadata -----===//
//
// TODO: add copyright info
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//


#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/TypeFinder.h"
#include <set>

using namespace llvm;

namespace {
  static const char *ImageTypeNames[] = {
    "opencl.image1d_t", "opencl.image1d_array_t", "opencl.image1d_buffer_t",
    "opencl.image2d_t", "opencl.image2d_array_t",
    "opencl.image2d_depth_t", "opencl.image2d_array_depth_t",
    "opencl.image2d_msaa_t", "opencl.image2d_array_msaa_t",
    "opencl.image2d_msaa_depth_t", "opencl.image2d_array_msaa_depth_t",
    "opencl.image3d_t"
  };

  static const char *ImageDepthTypeNames[] = {
    "opencl.image2d_depth_t", "opencl.image2d_array_depth_t"
  };

  static const char *ImageMSAATypeNames[] = {
    "opencl.image2d_msaa_t", "opencl.image2d_array_msaa_t",
    "opencl.image2d_msaa_depth_t", "opencl.image2d_array_msaa_depth_t"
  };
  struct OCLExtensionsTy {
#define OPENCLEXT(nm)  unsigned _##nm : 1;
#include "clang/Basic/OpenCLExtensions.def"

    OCLExtensionsTy() {
#define OPENCLEXT(nm)   _##nm = 0;
#include "clang/Basic/OpenCLExtensions.def"
    }
  };

  typedef void (*func_call_handler)(CallInst *callInstr, OCLExtensionsTy &exts);

  void baseAtomics64(CallInst *callInstr, OCLExtensionsTy &exts) {
    PointerType *firstArgType = dyn_cast<PointerType>(callInstr->getArgOperand(0)->getType());

    if (firstArgType && 
        firstArgType->getPointerElementType()->isIntegerTy() &&
        firstArgType->getPointerElementType()->getScalarSizeInBits() == 64)
      exts._cl_khr_int64_base_atomics = 1;
  }

  void extAtomics64(CallInst *callInstr, OCLExtensionsTy &exts) {
    PointerType *firstArgType = dyn_cast<PointerType>(callInstr->getArgOperand(0)->getType());

    if (firstArgType && 
        firstArgType->getPointerElementType()->isIntegerTy() &&
        firstArgType->getPointerElementType()->getScalarSizeInBits() == 64)
        exts._cl_khr_int64_extended_atomics = 1;
  }

  void image3DWrite(CallInst *callInstr, OCLExtensionsTy &exts) {
    PointerType *firstArgType = dyn_cast<PointerType>(callInstr->getArgOperand(0)->getType());

    if (firstArgType && 
        firstArgType->getPointerElementType()->isStructTy() &&
        !firstArgType->getPointerElementType()->getStructName().compare("opencl.image3d_t"))
        exts._cl_khr_3d_image_writes = 1;
  }

  typedef struct {
    const char *funcName;
    func_call_handler handler;
  } funcCallHandlersTy;

  static const funcCallHandlersTy funcCallHandlers[] = {
    {"_Z8atom_add", baseAtomics64},
    {"_Z8atom_sub", baseAtomics64},
    {"_Z9atom_xchg", baseAtomics64},
    {"_Z8atom_inc", baseAtomics64},
    {"_Z8atom_dec", baseAtomics64},
    {"_Z12atom_cmpxchg", baseAtomics64},
    {"_Z8atom_min", extAtomics64},
    {"_Z8atom_max", extAtomics64},
    {"_Z8atom_and", extAtomics64},
    {"_Z7atom_or", extAtomics64},
    {"_Z8atom_xor", extAtomics64},
    {"_Z12write_imagef", image3DWrite},
    {"_Z12write_imagei", image3DWrite},
    {"_Z13write_imageui", image3DWrite}
  };

  struct SPIRMetadataAdder : public ModulePass {
    static char ID; // Pass identification, replacement for typeid

    int m_iOCLVersion;
    std::list<std::string> m_sBuildOptions;

    Type *m_pDoubleType;
    Type *m_pHalfType;

    // Optional core features
    bool m_bUseDoubles;
    bool m_bUseImages;

    OCLExtensionsTy m_sUsedExts;

    SPIRMetadataAdder(const std::list<std::string> optList = std::list<std::string>(), int OCLVer = 120 ) : 
      ModulePass(ID), m_iOCLVersion(OCLVer){
      m_sBuildOptions = optList;
      initializeSPIRMetadataAdderPass(*PassRegistry::getPassRegistry());
    }

    bool searchTypeInType (llvm::Type *ty1, llvm::Type *ty2, bool ignorePtrs, std::set<llvm::Type*> &typesList) {
      if (ty1 == ty2)
        return true;

      if (ty1->isVectorTy())
        return searchTypeInType(ty1->getVectorElementType(), ty2, ignorePtrs, typesList);

      if (ty1->isArrayTy())
        return searchTypeInType(ty1->getArrayElementType(), ty2, ignorePtrs, typesList);

      if (!ignorePtrs && ty1->isPointerTy()) {
        // prevent infinte loop (such a struct that conatinc pointer to itself)
        std::set<llvm::Type*>::iterator itr = typesList.find(ty1->getPointerElementType());
        if ( itr != typesList.end() ) {
          return false;
        }
        return searchTypeInType(ty1->getPointerElementType(), ty2, ignorePtrs, typesList);
      }

      if (ty1->isStructTy()) {
        typesList.insert( ty1 );
        llvm::StructType *strTy = dyn_cast<llvm::StructType>(ty1);

        for (StructType::element_iterator EI = strTy->element_begin(),
             EE = strTy->element_end(); EI != EE; ++EI)
          if (searchTypeInType((*EI), ty2, ignorePtrs, typesList))
            return true;
      }

      if (ty1->isFunctionTy()) {
        typesList.insert( ty1 );
        FunctionType *FuncTy = dyn_cast<llvm::FunctionType>(ty1);

        if (searchTypeInType(FuncTy->getReturnType(), ty2, ignorePtrs))
          return true;

        for (FunctionType::param_iterator PI = FuncTy->param_begin(),
             PE = FuncTy->param_end(); PI != PE; ++PI)
          if (searchTypeInType((*PI), ty2, ignorePtrs))
            return true;
      }

      return false;
    }

    bool searchTypeInType (llvm::Type *ty1, llvm::Type *ty2, bool ignorePtrs) {
      std::set<llvm::Type*> typesList;
      return searchTypeInType( ty1, ty2, ignorePtrs, typesList);
    }

    bool runOnModule(Module &M) {
      m_pDoubleType = Type::getDoubleTy(M.getContext());
      m_pHalfType = Type::getHalfTy(M.getContext());

      m_bUseDoubles = false;
      m_bUseImages  = false;

      for (Module::global_iterator GI = M.global_begin(), GE = M.global_end();
           GI != GE; ++GI) {
        if (searchTypeInType(GI->getType(), m_pDoubleType, false))
          m_bUseDoubles = true;
        if (searchTypeInType(GI->getType(), m_pHalfType, true))
          m_sUsedExts._cl_khr_fp16 = true;
      }

      //check if image types are defined
      for (size_t i = 0; i < sizeof(ImageTypeNames)/sizeof(ImageTypeNames[0]); i++) {
        if (M.getTypeByName(ImageTypeNames[i])) {
          m_bUseImages = true;
          break;
        }
      }


      //check if depth image types are defined
      for (size_t i = 0; i < sizeof(ImageDepthTypeNames)/sizeof(ImageDepthTypeNames[0]); i++) {
        if (M.getTypeByName(ImageDepthTypeNames[i])) {
          m_sUsedExts._cl_khr_depth_images = true;
          break;
        }
      }

      //check if msaa image types are defined
      for (size_t i = 0; i < sizeof(ImageMSAATypeNames)/sizeof(ImageMSAATypeNames[0]); i++) {
        if (M.getTypeByName(ImageMSAATypeNames[i])) {
          m_sUsedExts._cl_khr_gl_msaa_sharing = true;
          break;
        }
      }

      // scan all functions
      for (Module::iterator FI = M.begin(), FE = M.end();
           FI != FE; ++FI) {
        runOnFunction(*FI);
      }

      // Add SPIR version (1.0)
      Value *SPIRVerElts[] = {
        ConstantInt::get(Type::getInt32Ty(M.getContext()), 1),
        ConstantInt::get(Type::getInt32Ty(M.getContext()), 0)
      };
      llvm::NamedMDNode *SPIRVerMD =
        M.getOrInsertNamedMetadata("opencl.spir.version");
      SPIRVerMD->addOperand(llvm::MDNode::get(M.getContext(), SPIRVerElts));

      // Add OpenCL version
      Value *OCLVerElts[] = {
        ConstantInt::get(Type::getInt32Ty(M.getContext()), m_iOCLVersion / 100),
        ConstantInt::get(Type::getInt32Ty(M.getContext()), (m_iOCLVersion % 100) / 10)
      };
      llvm::NamedMDNode *OCLVerMD =
        M.getOrInsertNamedMetadata("opencl.ocl.version");
      OCLVerMD->addOperand(llvm::MDNode::get(M.getContext(), OCLVerElts));

      // Add used extensions
      llvm::SmallVector< llvm::Value*, 5> OCLExtElts;

#define OPENCLEXT(nm)  if (m_sUsedExts._##nm) \
  OCLExtElts.push_back(llvm::MDString::get(M.getContext(), #nm));
#include "clang/Basic/OpenCLExtensions.def"

      llvm::NamedMDNode *OCLExtMD =
        M.getOrInsertNamedMetadata("opencl.used.extensions");

      OCLExtMD->addOperand(llvm::MDNode::get(M.getContext(), OCLExtElts));

      // Add used optional core features
      llvm::SmallVector< llvm::Value*, 5> OCLOptCoreElts;

      if (m_bUseDoubles)
        OCLOptCoreElts.push_back(llvm::MDString::get(M.getContext(), "cl_doubles"));

      if (m_bUseImages)
        OCLOptCoreElts.push_back(llvm::MDString::get(M.getContext(), "cl_images"));
      llvm::NamedMDNode *OptCoreMD =
        M.getOrInsertNamedMetadata("opencl.used.optional.core.features");
      OptCoreMD->addOperand(llvm::MDNode::get(M.getContext(), OCLOptCoreElts));

      // Add build options
      llvm::NamedMDNode *OCLCompOptsMD =
        M.getOrInsertNamedMetadata("opencl.compiler.options");
      llvm::SmallVector<llvm::Value*,5> OCLBuildOptions;
      for (std::list<std::string>::const_iterator it = m_sBuildOptions.begin(),
           e = m_sBuildOptions.end(); it != e ; ++it){
        OCLBuildOptions.push_back(llvm::MDString::get(M.getContext(), *it));
      }
      OCLCompOptsMD->addOperand(llvm::MDNode::get(M.getContext(), OCLBuildOptions));
      
      return true;
    }

    bool runOnFunction(Function &F) {
      for (Function::arg_iterator AI = F.arg_begin(), AE = F.arg_end();
           AI != AE; ++AI) {
        if (searchTypeInType(AI->getType(), m_pDoubleType, false))
          m_bUseDoubles = true;
        if (searchTypeInType(AI->getType(), m_pHalfType, true))
          m_sUsedExts._cl_khr_fp16 = true;
      }

      for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
        for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {
          if (searchTypeInType(I->getType(), m_pDoubleType, false))
            if (!(dyn_cast<FPExtInst>(I)))
              m_bUseDoubles = true;
          if (searchTypeInType(I->getType(), m_pHalfType, true))
            m_sUsedExts._cl_khr_fp16 = true;

          for (Instruction::op_iterator OI = (*I).op_begin(), OE = (*I).op_end();
               OI != OE; ++OI) {
            if (searchTypeInType((*OI)->getType(), m_pDoubleType, false))
              if (!(dyn_cast<CallInst>(I) &&
                    dyn_cast<CallInst>(I)->getCalledFunction()->isVarArg()))
                m_bUseDoubles = true;
            if (searchTypeInType((*OI)->getType(), m_pHalfType, true))
              m_sUsedExts._cl_khr_fp16 = true;
          }

          CallInst* pCallInst = dyn_cast<CallInst>(I);
          if (pCallInst && pCallInst->getCalledFunction()) {
            std::string funcName = pCallInst->getCalledFunction()->getName().str();

            for (size_t i = 0; i < sizeof(funcCallHandlers)/sizeof(funcCallHandlers[0]); i++) {
              if (funcName.find(funcCallHandlers[i].funcName) == 0)
                funcCallHandlers[i].handler(pCallInst, m_sUsedExts);
            }
          }
        }
      return true;
    }
  };
}

char SPIRMetadataAdder::ID = 0;

INITIALIZE_PASS(SPIRMetadataAdder, "spirmetadataadder", 
                "Add SPIR related module scope metadata", false, false)

namespace llvm {
  //===----------------------------------------------------------------------===//
  //
  // SPIRMetadataAdder - Add SPIR related module scope metadata.
  //
  ModulePass *createSPIRMetadataAdderPass(const std::list<std::string> BuildOpt, int OCLVer) {
    return new SPIRMetadataAdder(BuildOpt, OCLVer);
  }
}