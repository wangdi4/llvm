//===-- SPIRMaterializer.cpp - Implement SPIR materializer ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
//
//===----------------------------------------------------------------------===//
//
// This file implements the Type class for the VMCore library.
//
//===----------------------------------------------------------------------===//
#include "SPIRMaterializer.h"

#include "llvm/Support/IRReader.h"
#include "llvm/Config/config.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/CallingConv.h"

#include "llvm-c/BitWriter.h"
#include "llvm/Target/TargetData.h"
#include "llvm/PassManager.h"
#include "llvm/Analysis/Verifier.h"

#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetMachine.h"

std::set<std::string> MaterializerPass::m_oclBuiltinsSet;

// Helper function that returns the size for a particular type
unsigned int MaterializerPass::GetTypeSize(Type *pType)
{
  unsigned int ElementSizeSum = 0;
  unsigned int NumContainedTypes = pType->getNumContainedTypes();

  // Scalar and vector types
  if (NumContainedTypes == 0)
  {
    ElementSizeSum += pType->getPrimitiveSizeInBits() / 8;
  }
  else
  {
    switch (pType->getTypeID())
    {
      case Type::ArrayTyID: // Arrays
      {
        Type *pElemType = pType->getContainedType(0);
        ArrayType *ATy = cast<ArrayType>(pType);
        assert (ATy != NULL);
        ElementSizeSum += GetTypeSize(pElemType) * (unsigned int)ATy->getNumElements();
        break;
      }
      case Type::StructTyID: // Structures
        for (unsigned int i = 0; i < NumContainedTypes; i++)
        {
          Type *pElemType = pType->getContainedType(i);
          ElementSizeSum += GetTypeSize(pElemType) * pElemType->getNumElements();
        }
        break;
      case Type::PointerTyID:
        ElementSizeSum += m_pTargetData->getPointerPrefAlignment() / 8;
        break;
      default:
        assert(0 && "Unexpected type");
        break;
    }
  }

  return ElementSizeSum; //return size in bytes
}

// Pass constructor
MaterializerPass::MaterializerPass(const char *pTargetTriple) :
  BasicBlockPass(ID),
  m_Context(getGlobalContext())
{
  std::string Error;
  std::string Triple = pTargetTriple;

  m_bInitialized = false;

  const Target *TheTarget = TargetRegistry::lookupTarget(pTargetTriple, Error);
  assert(TheTarget != NULL);

  m_pTM = TheTarget->createTargetMachine(pTargetTriple, "", "");
  assert(m_pTM != NULL);

  if (m_pTM != NULL)
  {
    m_pTargetData = m_pTM->getTargetData();
    assert(m_pTargetData != NULL);
  }

  if ( m_oclBuiltinsSet.empty() ) {
    m_oclBuiltinsSet.insert( "get_work_dim" );
    m_oclBuiltinsSet.insert( "get_global_size" );
    m_oclBuiltinsSet.insert( "get_global_id" );
    m_oclBuiltinsSet.insert( "get_local_size" );
    m_oclBuiltinsSet.insert( "get_local_id" );
    m_oclBuiltinsSet.insert( "get_num_group" );
    m_oclBuiltinsSet.insert( "get_group_id" );
    m_oclBuiltinsSet.insert( "get_global_offset" );
  }
  m_bInitialized = true;
}


// Pass initialization. All of the module changes occur here
bool MaterializerPass::doInitialization(Module &Module) {
  if (!m_bInitialized)
  {
    assert(false && "MaterializerPass was not initialized correctly");
    return false;
  }

  llvm::Module *pModule = &Module;

  // Set triple and data layout based on the target information
  Module.setDataLayout(m_pTargetData->getStringRepresentation());
  Module.setTargetTriple(m_pTM->getTargetTriple());

  // Process kernel functions
  NamedMDNode *pSPIRFcns = pModule->getNamedMetadata("spir.functions");
  NamedMDNode *pOCLKernels = pModule->getOrInsertNamedMetadata("opencl.kernels");
  llvm::SmallVector <MDNode*, 5> KFunctions;
  bool bNoKernels = true;
  if ( pSPIRFcns != NULL ) {
	for (unsigned i = 0, e = pSPIRFcns->getNumOperands(); i < e; i++)
	{
	  MDNode *pKernelMD = pSPIRFcns->getOperand(i);
	  if (ProcessFunctionMetadata(pKernelMD))
	  {
	    pOCLKernels->addOperand(pKernelMD);
		bNoKernels = false;
	  }
	}
    pModule->eraseNamedMetadata(pSPIRFcns);
  }
  if (bNoKernels)
  {
    pModule->eraseNamedMetadata(pOCLKernels);
  }

  // Get string values
  m_CompileOptions = GetStringObject(pModule, "spir.compiler.options");
  m_ExtCompileOptions = GetStringObject(pModule,
                                       "spir.compiler.ext.options");
  m_OptionalFeatures = GetStringObject(pModule,
                                       "spir.used.optional.core.features");
  m_Extensions = GetStringObject(pModule, "spir.used.extensions");

  // Get version values
  m_SpirVersion = GetTupleValue(pModule, "spir.version");
  m_OCLVersion = GetTupleValue(pModule, "spir.ocl.version");

  // Translate metadata elements
  TranslateMetadataName(pModule,
                        "spir.compiler.options",
                        "opencl.build.options");

  // Remove additional metadata elements
  DeleteMetadata(pModule, "spir.used.optional.core.features");
  DeleteMetadata(pModule, "spir.used.extensions");
  DeleteMetadata(pModule, "spir.version");
  DeleteMetadata(pModule, "spir.ocl.version");

  // Replace opaque structures for actual definitions
  std::vector<StructType *> StructVec;
  Module.findUsedStructTypes(StructVec);
  for (int i = 0, size = StructVec.size(); i < size; i++) {
    StructType *pSType = StructVec[i];
    std::string SName = pSType->getName();
    if (SName.find("spir.") == std::string::npos) {
      continue;
    }

    // Remove the "spir." prefix
//    pSType->setName(pSType->getName().substr(strlen("spir.")));
    // Replace '.' with '_'
    size_t Pos = SName.find_first_of(".");
    while (Pos != std::string::npos) {
      SName.replace(Pos, 1, "_");
      Pos = SName.find_first_of(".");
    }
    pSType->setName(SName);

  }

  return true;
}


// Scan a basic block and perform instruction transformation
bool MaterializerPass::runOnBasicBlock(BasicBlock &BB) {
  bool bModified = false;

  if (!m_bInitialized)
  {
    assert(false && "MaterializerPass was not initialized correctly");
    return bModified;
  }

  Function* parentFunc = BB.getParent();
  if ( parentFunc->getCallingConv() == CallingConv::SPIR_KERNEL ) {
	  std::string demangleFuncNMame = GetUnmangledName( parentFunc->getName() );
	  parentFunc->setName( demangleFuncNMame );
  }

  BasicBlock::iterator BBIter, BBEnd;
  for (BBIter = BB.begin(), BBEnd = BB.end(); BBIter != BBEnd; BBIter++) {
    Instruction *pGenericInst = BBIter;
    switch (pGenericInst->getOpcode()) {
    case Instruction::Call:
      {
        CallInst *pSpecificInst = dyn_cast_or_null<CallInst>(pGenericInst);
        Function *pCalledFunc = pSpecificInst->getCalledFunction();
        std::string FName = GetUnmangledName( pCalledFunc->getNameStr() );

		if ( m_oclBuiltinsSet.find( FName ) != m_oclBuiltinsSet.end() ) {
			pCalledFunc->setName( FName );
		}

        const char *pSpirPrefix = "__spir";
        size_t Pos = FName.find(pSpirPrefix);
        if (Pos == std::string::npos) {
          // Skip non-SPIR intrinsics
          break;
        }

        // Build new function name and update the call
        //FName.erase(0, FName.find_first_of(".") + 1);
        //Pos = FName.find_first_of(".");
        //while (Pos != std::string::npos) {
        //  FName.replace(Pos, 1, "_");
        //  Pos = FName.find_first_of(".");
        //}
        //pCalledFunc->setName(FName);
        break;
      }
    case Instruction::Alloca:
      {
        AllocaInst *pSpecificInst = dyn_cast_or_null<AllocaInst>(pGenericInst);
        Type *pType = pSpecificInst->getAllocatedType();
        pSpecificInst->setAlignment(GetTypeSize(pType));
        bModified = true;
        break;
      }
    case Instruction::Load:
      {
        LoadInst *pSpecificInst = dyn_cast_or_null<LoadInst>(pGenericInst);
        if (pSpecificInst->getAlignment() > 0) {
          Type *pType = pSpecificInst->getPointerOperand()->getType();
          pSpecificInst->setAlignment(GetTypeSize(pType));
          bModified = true;
        }
        break;
      }
    case Instruction::Store:
      {
        StoreInst *pSpecificInst = dyn_cast_or_null<StoreInst>(pGenericInst);
        if (pSpecificInst->getAlignment() > 0) {
          Type *pType = pSpecificInst->getValueOperand()->getType();
          pSpecificInst->setAlignment(GetTypeSize(pType));
          bModified = true;
        }
        break;
      }
    }
  }
  return bModified;
}


// Helper function to change the name of a metadata by adding a metadata with
// the new name, copying the operands and erasing the old metadata name
void MaterializerPass::TranslateMetadataName(Module *pModule,
                                             const char *pCurName,
                                             const char *pNewName)
{
  NamedMDNode *pCurTuple = pModule->getNamedMetadata(pCurName);
  NamedMDNode *pNewTuple = pModule->getOrInsertNamedMetadata(pNewName);
  if ((pCurTuple == NULL) || (pNewTuple == NULL)) {
    assert(0);
    return;
  }

  // Add operands to destination
  for (unsigned i = 0, e = pCurTuple->getNumOperands(); i != e; ++i) {
    MDNode *pKernel = pCurTuple->getOperand(i);
    pNewTuple->addOperand(pKernel);
  }

  // Remove metadata from module
  pModule->eraseNamedMetadata(pCurTuple);
}


void MaterializerPass::TranslateAccessQualifier(Function *pKFunction,
                                                MDNode *pMDElem)
{
  llvm::SmallVector <llvm::Value*, 5> accQuals;

  MDString *pName = llvm::MDString::get(m_Context, "image_access_qualifier");
  accQuals.push_back(pName);
//  accQuals.push_back(pMDElem->getOperand(0));

  // Find out if one of the argumets is a sampler
  unsigned i = 1; // Skip the metadata name and start at the first operand
  Function::const_arg_iterator ArgIter, ArgEnd;
  for (ArgIter = pKFunction->arg_begin(), ArgEnd = pKFunction->arg_end();
       ArgIter != ArgEnd; ArgIter++, i++)
  {

    Value *pMDValue = pMDElem->getOperand(i);

    StructType *STy = dyn_cast<StructType>(ArgIter->getType());
    if (STy != NULL)
    {
      StringRef ArgName = STy->getName();
      if (ArgName == "spir.sampler_t")
      {
        // Replace the constant value in the metadata element for the constant -1
        ConstantInt *accessQualVal = cast<ConstantInt>(pMDElem->getOperand(i));
        Type *pType = accessQualVal->getType();
        unsigned BitWidth = accessQualVal->getBitWidth();
        Constant *pMinusOne = llvm::ConstantInt::get(pType, llvm::APInt(BitWidth, -1));
        pMDValue = pMinusOne;
      }
    }

    accQuals.push_back(pMDValue);
  }

  pMDElem->replaceAllUsesWith(llvm::MDNode::get(m_Context, accQuals));
}

// The target version of vec_type_hint doesn't have the integer at the end to
// indicate a signed type. This function drops it.
void MaterializerPass::TranslateVecTypeHint(MDNode *pMDElem)
{
  llvm::SmallVector <llvm::Value*, 5> vecTypeHint;

  // Copy only the name and type
  vecTypeHint.push_back(pMDElem->getOperand(0));
  vecTypeHint.push_back(pMDElem->getOperand(1));

  // Replace the node with the new one
  pMDElem->replaceAllUsesWith(llvm::MDNode::get(m_Context, vecTypeHint));
}


// Helper function to remove a metadata from a module
void MaterializerPass::DeleteMetadata(Module *pModule,
                                      const char *pMetadataName)
{
  NamedMDNode *pMetadata = pModule->getNamedMetadata(pMetadataName);
  if (pMetadata == NULL) {
    return;
  }

  // Remove metadata from module
  pModule->eraseNamedMetadata(pMetadata);
}


// Extract the string from the first element in a metadata object
StringRef MaterializerPass::GetStringObject(Module *pModule,
                                            const char *pMetadataName)
{
  NamedMDNode *pNamedMDNode = pModule->getNamedMetadata(pMetadataName);
  if ((pNamedMDNode == NULL) || (pNamedMDNode->getNumOperands() == 0))
  {
    // The metadata object wasn't found
//    assert(0);
    return "";
  }

  MDNode * pMDElement = (MDNode *)pNamedMDNode->getOperand(0);
  if (pMDElement == NULL)
  {
    assert(0);
    return "";
  }

  // Not all nodes have elements. Return an empty string in this case
  if (pMDElement->getNumOperands() == 0)
  {
    return "";
  }

  // Get reference to the object
  MDString *pMDString = (MDString *)pMDElement->getOperand(0);
  if (pMDString == NULL)
  {
    assert(0);
    return "";
  }
  return pMDString->getString();
}


// Extract the string from the first element in a metadata object
uint64_t MaterializerPass::GetTupleValue(Module *pModule,
                                         const char *pMetadataName)
{
  NamedMDNode *pMDNode = pModule->getNamedMetadata(pMetadataName);
  if ((pMDNode == NULL) || (pMDNode->getNumOperands() == 0))
  {
    assert(0);
    return -1;
  }

  // Get reference to the object
  MDNode *pMDElement = (MDNode *)pMDNode->getOperand(0);

  // Calculate value. For example { 1, 2 } gives 12
  unsigned i = pMDElement->getNumOperands();
  uint64_t Value = 0, Decades = 1;
  do
  {
    ConstantInt *pVal = cast<ConstantInt>(pMDElement->getOperand(i - 1));
    Value += Decades * pVal->getZExtValue();
    Decades *= 10;
    i--;
  } while (i > 0);

  return Value;
}

// Process a kernel metadata and extract objects that differ between SPIR and
// LLVM
bool MaterializerPass::ProcessFunctionMetadata(MDNode *pKernelMD)
{
  unsigned KernelMDOperands = pKernelMD->getNumOperands();
  if (KernelMDOperands == 0)
  {
    // This shouldn't happen. There must be at least the pointer to the
    // function declaration
    assert(0);
    return false;
  }

  Function *pKFunction = (Function *)pKernelMD->getOperand(0);

  llvm::CallingConv::ID CC = pKFunction->getCallingConv();
  pKFunction->setCallingConv(llvm::CallingConv::C);

  // Skip SPIR user defined functions
  if (CC == llvm::CallingConv::SPIR_FUNC) {
    return false;
  }

  if ( CC == CallingConv::SPIR_KERNEL ) {
	  std::string demangleFuncNMame = GetUnmangledName( pKFunction->getName() );
	  pKFunction->setName( demangleFuncNMame );
  }

  for (unsigned i = 1, e = KernelMDOperands; i < e; i++)
  {
    MDNode *pMDElement = (MDNode *)pKernelMD->getOperand(i);
    if (pMDElement == NULL) {
      assert(0);
      continue;
    }

    // Get the element name
    MDString *pMDElemName = (MDString *)pMDElement->getOperand(0);
    llvm::StringRef ElemName = pMDElemName->getString();

    // Process values that are different in SPIR and LLVM
    if (ElemName == "access_qualifier")
    {
      TranslateAccessQualifier(pKFunction, pMDElement);
    }
    else if (ElemName == "vec_type_hint")
    {
      TranslateVecTypeHint(pMDElement);
    }
  }

  return true;
}

// Remove the function decorations and return the unmangled name
std::string MaterializerPass::GetUnmangledName(const std::string& MangledName)
{
  // Return input if not mangled
  if (MangledName.find("_Z") == std::string::npos)
  {
    return MangledName;
  }

  // Find the length of the undecorated function name. We start from the
  // character after the '_Z' prefix.
  unsigned Digit = 0;
  unsigned DigitPosition = 1;
  do
  {
    DigitPosition++;
    Digit = MangledName[DigitPosition];
  } while (isdigit(Digit) != 0);
  assert(DigitPosition > 2);
  unsigned Length = atoi(MangledName.substr(2, DigitPosition - 2).c_str());

  // Return the substring corresponding with the function name
  return MangledName.substr(DigitPosition, Length);
}


Module *MaterializerPass::MaterializeSPIR(Module *pSpirModule)
{
  // Translate module content
  // Create PassManager and add passes. The passes are the same to what opt.exe
  // uses
  PassManager PM;
  PM.add(new TargetData(pSpirModule));
  PM.add(this);
  PM.add(createVerifierPass());
  PM.run(*pSpirModule);

// Save the module to a file (debug)
if (0) {
  LLVMWriteBitcodeToFile(wrap(pSpirModule), "ModifiedSpir.bc");
}

  return pSpirModule;
}


Module *MaterializerPass::MaterializeSPIR(const char *pSpirBuffer,
                                          unsigned long BufferSize)
{
  // Read input module from memory
  StringRef buf(pSpirBuffer, BufferSize);
  MemoryBuffer *pMemBuffer = llvm::MemoryBuffer::getMemBufferCopy(buf);
  SMDiagnostic Err;
  Module *pModule = ParseIR(pMemBuffer, Err, getGlobalContext()); // This deletes pMemBuffer too

  return MaterializeSPIR(pModule);
}

char MaterializerPass::ID = 0;
static RegisterPass<MaterializerPass> X("SPIRMaterializer", "Bleh");
