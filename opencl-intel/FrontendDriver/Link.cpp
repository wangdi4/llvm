// INTEL CONFIDENTIAL
//
// Copyright 2018 Intel Corporation.
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

#include "Link.h"
#include "FrontendResultImpl.h"
#include "cache_binary_handler.h"
#include "cl_env.h"
#include "elf_binary.h"

#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/StringSaver.h"
#include "llvm/Support/raw_ostream.h"

#include <memory>

using namespace llvm;
using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::ELFUtils;

#ifdef OCLFRONTEND_PLUGINS
#include "link_data.h"
#include "plugin_manager.h"
extern Intel::OpenCL::PluginManager g_pluginManager;
#endif // OCLFRONTEND_PLUGINS

enum LINK_OPT_ID {
  OPT_LINK_INVALID = 0, // This is not an option ID.
#define PREFIX(NAME, VALUE)
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, ALIASARGS, FLAGS,         \
               VISIBILITY, PARAM, HELPTEXT, METAVAR, VALUES)                   \
  OPT_LINK_##ID,
#include "OpenCLLinkOptions.inc"
  OPT_LINK_LAST_OPTION
#undef OPTION
#undef PREFIX
};

#define PREFIX(NAME, VALUE)                                                    \
  static constexpr StringLiteral NAME##_init[] = VALUE;                        \
  static constexpr ArrayRef<StringLiteral> NAME(NAME##_init,                   \
                                                std::size(NAME##_init) - 1);
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, ALIASARGS, FLAGS,         \
               VISIBILITY, PARAM, HELPTEXT, METAVAR, VALUES)
#include "OpenCLLinkOptions.inc"
#undef OPTION
#undef PREFIX

using namespace llvm::opt;
static constexpr opt::OptTable::Info ClangOptionsInfoTable[] = {
#define PREFIX(NAME, VALUE)
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, ALIASARGS, FLAGS,         \
               VISIBILITY, PARAM, HELPTEXT, METAVAR, VALUES)                   \
  {PREFIX,                                                                     \
   NAME,                                                                       \
   HELPTEXT,                                                                   \
   METAVAR,                                                                    \
   OPT_LINK_##ID,                                                              \
   opt::Option::KIND##Class,                                                   \
   PARAM,                                                                      \
   FLAGS,                                                                      \
   VISIBILITY,                                                                 \
   OPT_LINK_##GROUP,                                                           \
   OPT_LINK_##ALIAS,                                                           \
   ALIASARGS,                                                                  \
   VALUES},
#include "OpenCLLinkOptions.inc"
#undef OPTION
#undef PREFIX
};

namespace {
class ClangFEDiagnosticHandler : public DiagnosticHandler {
public:
  bool handleDiagnostics(const DiagnosticInfo &DI) override {
    unsigned Severity = DI.getSeverity();
    if (Severity == DS_Error) {
      std::string Err;
      raw_string_ostream OS(Err);
      DiagnosticPrinterRawOStream DP(OS);
      DI.print(DP);
      OS << "\n";
      throw Err;
    }
    return true;
  }
};
} // namespace

OpenCLLinkOptTable::OpenCLLinkOptTable()
    : opt::GenericOptTable(ClangOptionsInfoTable) {}

ClangLinkOptions::ClangLinkOptions(const char *pszOptions) {
  BumpPtrAllocator A;
  StringSaver Saver(A);
  cl::TokenizeGNUCommandLine(pszOptions, Saver, m_Args);
  m_pArgs.reset(new opt::InputArgList(
      m_optTbl.ParseArgs(m_Args, m_missingArgIndex, m_missingArgCount)));
}

bool ClangLinkOptions::checkOptions(char *pszUnknownOptions,
                                    size_t uiUnknownOptionsSize) {

  // Copy option spelling to pszUnknownOptions.
  auto copyOpt = [&](StringRef Spelling) {
    auto *end = std::copy_n(Spelling.data(),
                            std::min(Spelling.size(), uiUnknownOptionsSize - 1),
                            pszUnknownOptions);
    *end = '\0';
  };

  // check for options with missing argument error.
  if (m_missingArgCount) {
    copyOpt(m_pArgs->getArgString(m_missingArgIndex));
    return false;
  }

  // check for unknown options
  if (opt::Arg *A = m_pArgs->getLastArgNoClaim(OPT_LINK_UNKNOWN)) {
    copyOpt(A->getSpelling());
    return false;
  }

  // we do not support input options
  if (opt::Arg *A = m_pArgs->getLastArgNoClaim(OPT_LINK_INPUT)) {
    copyOpt(A->getSpelling());
    return false;
  }

  return true;
}

bool ClangLinkOptions::hasArg(int id) const { return m_pArgs->hasArg(id); }

// Check whether the module contains OpenCL or SYCL version metadata.
// Add "not-ocl-sycl" attribute to functions from neither ocl nor sycl
// binary.
void addAttrForNoneOCLSYCLCode(Module *M) {
  if (M->getNamedMetadata("opencl.ocl.version") == nullptr &&
      M->getNamedMetadata("spirv.Source") == nullptr) {
    for (auto &F : M->getFunctionList()) {
      F.addFnAttr("not-ocl-sycl", "true");
    }
  }
}

static bool isSameType(Type *A, Type *B, std::string &Message) {
  if (A == B)
    return true;
  // If both types are struct, we need to check if their layouts are identical.
  // ATM, llvm linker may not merge identical-layout structs, for example:
  //
  // clang-format off
  // Module A:
  // %struct.SomeType = { i64, i64 }
  // %struct.OtherType = { i64, i64 }
  // declare void @foo(%struct.SomeType *)
  // define void @bar() {
  //   call void @foo(%struct.SomeType *%x)
  // }
  //
  // Module B:
  // %struct.OtherType = { i64, i64 } ; This is not merged to A::%struct.SomeType,
  //                                  ; instead it's merged to A::%struct.OtherType.
  // define void @foo(%struct.OtherType *%x) {
  //   ...
  // }
  //
  // Linked module:
  // %struct.SomeType = { i64, i64 }
  // %struct.OtherType = { i64, i64 }
  // define void @foo(%struct.OtherType *%x) {
  //   ...
  // }
  // define void @bar() {
  //   call void bitcast (void (%struct.OtherType *)* @foo to void (%struct.SomeType *)*)(%struct.SomeType *%x)
  // }
  // clang-format on
  //
  // Therefore we end up with a function ptr bitcast here, but it doesn't imply
  // a semantical signature mismatch.
  auto *StructTyA = dyn_cast<StructType>(A);
  auto *StructTyB = dyn_cast<StructType>(B);
  if (StructTyA && StructTyB && StructTyA->isLayoutIdentical(StructTyB))
    return true;
  auto *PtrTyA = dyn_cast<PointerType>(A);
  auto *PtrTyB = dyn_cast<PointerType>(B);
  if (PtrTyA && PtrTyB) {
    if (PtrTyA->getAddressSpace() != PtrTyB->getAddressSpace()) {
      Message = "incompatible address space";
      return false;
    }
    return true;
  }
  Message = "incompatible type";
  return false;
}

static bool checkFuncCallArgs(const FunctionType *FuncTy,
                              ArrayRef<Value *> CIArgs,
                              std::string &BadSigDesc) {
  assert(FuncTy && "Func type not specified");

  if (CIArgs.size() != FuncTy->getNumParams()) {
    BadSigDesc = "wrong number of arguments to function call, expected " +
                 std::to_string(FuncTy->getNumParams()) + ", have " +
                 std::to_string(CIArgs.size());
    return false;
  }

  std::string TypeMismatchMsg;
  for (size_t I = 0; I < FuncTy->getNumParams(); ++I) {
    if (!isSameType(FuncTy->getParamType(I), CIArgs[I]->getType(),
                    TypeMismatchMsg)) {
      BadSigDesc = "passing parameter " + std::to_string(I + 1) + " with " +
                   TypeMismatchMsg;
      return false;
    }
  }

  return true;
}

// If function signature in definition and declaration in two modules differ
// then Linker selects signature in definition and may update function
// call with bitcast instruction that casts pointer on called function to
// a function pointer with signature at definition.
// Example:
//   call void bitcast (void (i32, i32)* @func to void (i32)*)(i32 sret %38)
// Check that there is no such function pointers bitcasts.
static bool checkAndThrowIfCallFuncCast(const Module &linkedModule,
                                        std::string &funcSigErr) {
  bool funcCallsValid = true;

  for (const Function &SF : linkedModule) {
    for (const BasicBlock &BB : SF) {
      for (const Instruction &I : BB) {
        // process ptr callinsts
        const auto *CI = dyn_cast<CallInst>(&I);
        if (!CI || CI->getCalledFunction())
          continue;

        const Value *VI = CI->getCalledOperand();
        if (!VI->getType()->isPointerTy())
          continue;

        // look through bitcast
        if (const auto *CE = dyn_cast<ConstantExpr>(VI)) {
          if (CE->getOpcode() == Instruction::BitCast)
            VI = CE->getOperand(0);
        } else if (const auto *BCI = dyn_cast<BitCastInst>(VI)) {
          VI = BCI->getOperand(0);
        }

        const auto *CF = dyn_cast<Function>(VI);
        if (!CF)
          continue;

        SmallVector<Value *, 4> params(CI->arg_begin(), CI->arg_end());
        std::string BadSigDesc;
        if (!checkFuncCallArgs(CF->getFunctionType(), ArrayRef<Value *>(params),
                               BadSigDesc)) {
          funcCallsValid = false;
          funcSigErr.append(CF->getName().str())
              .append(" [")
              .append(BadSigDesc)
              .append("]\n");
        }
      }
    }
  }

  return funcCallsValid;
}

static void saveKernelNames(Module *M, std::string *KernelsName) {
  if (KernelsName == nullptr)
    return;

  // Names between input programs are separated by ';'
  // Names in a program are separated by ','
  for (auto &F : M->getFunctionList())
    if (F.getCallingConv() == CallingConv::SPIR_KERNEL) {
      KernelsName->append(F.getName().data());
      KernelsName->append(",");
    }
  KernelsName->append(";");
}

static OCLFEBinaryResult *LinkInternal(const void **pInputBinaries,
                                       unsigned int uiNumBinaries,
                                       const size_t *puiBinariesSizes,
                                       const char *pszOptions,
                                       std::string *pKernelsName) {

  std::unique_ptr<OCLFEBinaryResult> pResult;

  try {
    pResult.reset(new OCLFEBinaryResult());

    if (0 == uiNumBinaries) {
      return pResult.release();
    }

    // Prepare the LLVM Context
    std::unique_ptr<LLVMContext> context(new LLVMContext());
    context->setDiagnosticHandler(std::make_unique<ClangFEDiagnosticHandler>());

    // Initialize the module with the first binary
    StringRef InputBinary(static_cast<const char *>(pInputBinaries[0]),
                          puiBinariesSizes[0]);
    auto pBinBuff(MemoryBuffer::getMemBuffer(InputBinary, "", false));
    auto ModuleOr =
        parseBitcodeFile(pBinBuff.get()->getMemBufferRef(), *context);
    if (!ModuleOr) {
      throw ModuleOr.takeError();
    }
    std::unique_ptr<Module> composite = std::move(ModuleOr.get());

    saveKernelNames(composite.get(), pKernelsName);

    // Add not-ocl-sycl attribute to functions from neither ocl nor sycl
    // binary.
    addAttrForNoneOCLSYCLCode(composite.get());
    // Parse options
    ClangLinkOptions optionsParser(pszOptions);

    for (unsigned int i = 1; i < uiNumBinaries; ++i) {
      StringRef InputBinary(static_cast<const char *>(pInputBinaries[i]),
                            puiBinariesSizes[i]);
      auto pBinBuff(MemoryBuffer::getMemBuffer(InputBinary, "", false));
      auto ModuleOr =
          parseBitcodeFile(pBinBuff.get()->getMemBufferRef(), *context);
      if (!ModuleOr) {
        throw ModuleOr.takeError();
      }
      std::unique_ptr<Module> module = std::move(ModuleOr.get());

      saveKernelNames(module.get(), pKernelsName);

      // Add not-ocl-sycl attribute to functions from neither ocl nor sycl
      // binary.
      addAttrForNoneOCLSYCLCode(module.get());

      if (Linker::linkModules(*composite, std::move(module))) {
        throw std::string("Linking has failed");
      }
    }

    std::string funcSigErr{};
    if (uiNumBinaries > 1 &&
        !checkAndThrowIfCallFuncCast(*composite, funcSigErr)) {
      throw std::string(
          "Error: call of function(s) with different signature:\n") +
          funcSigErr;
    }

    IR_TYPE binaryType = optionsParser.hasArg(OPT_LINK_create_library)
                             ? IR_TYPE_LIBRARY
                             : IR_TYPE_EXECUTABLE;
    pResult->setIRType(binaryType);

    raw_svector_ostream ir_ostream(pResult->getIRBufferRef());
    WriteBitcodeToFile(*composite.get(), ir_ostream);
    pResult->setResult(CL_SUCCESS);

    return pResult.release();
  } catch (std::bad_alloc &e) {
    if (pResult) {
      pResult->getLogRef() = "Internal error";
      pResult->setResult(CL_OUT_OF_HOST_MEMORY);
      return pResult.release();
    }
    throw e;
  } catch (std::string &err) {
    pResult->setLog(err);
    pResult->setResult(CL_LINK_PROGRAM_FAILURE);
    return pResult.release();
  } catch (Error &err) {
    std::string Message;
    handleAllErrors(std::move(err),
                    [&](ErrorInfoBase &EIB) { Message = EIB.message(); });

    pResult->setLog(Message);
    pResult->setResult(CL_LINK_PROGRAM_FAILURE);
    return pResult.release();
  }
}

//
// ClangFECompilerLinkTask calls implementation
//
int ClangFECompilerLinkTask::Link(IOCLFEBinaryResult **pBinaryResult) {
  std::vector<const void *> m_Binaries;
  std::vector<size_t> m_BinariesSizes;

  for (unsigned int i = 0; i < m_pProgDesc->uiNumBinaries; ++i) {
    if (CacheBinaryReader::IsValidCacheObject(
            m_pProgDesc->pBinaryContainers[i],
            m_pProgDesc->puiBinariesSizes[i])) {
      CacheBinaryReader reader(m_pProgDesc->pBinaryContainers[i],
                               m_pProgDesc->puiBinariesSizes[i]);
      m_Binaries.push_back(reader.GetSectionData(g_irSectionName));
      m_BinariesSizes.push_back(reader.GetSectionSize(g_irSectionName));
    } else if (OCLElfBinaryReader::IsValidOpenCLBinary(
                   (const char *)m_pProgDesc->pBinaryContainers[i],
                   m_pProgDesc->puiBinariesSizes[i])) {
      OCLElfBinaryReader reader((const char *)m_pProgDesc->pBinaryContainers[i],
                                m_pProgDesc->puiBinariesSizes[i]);
      const char *pBinaryData = nullptr;
      size_t uiBinaryDataSize = 0;
      reader.GetIR(pBinaryData, uiBinaryDataSize);
      m_Binaries.push_back(pBinaryData);
      m_BinariesSizes.push_back(uiBinaryDataSize);
    } else {
      m_Binaries.push_back(m_pProgDesc->pBinaryContainers[i]);
      m_BinariesSizes.push_back(m_pProgDesc->puiBinariesSizes[i]);
    }
  }

  std::unique_ptr<OCLFEBinaryResult> pResult;
  std::unique_ptr<OCLFELinkKernelNames> pKernelNames =
      std::make_unique<OCLFELinkKernelNames>();
  std::string kernelNames;
  int resultCode = CL_SUCCESS;
  try {
    pResult.reset(LinkInternal(m_Binaries.data(), m_pProgDesc->uiNumBinaries,
                               m_BinariesSizes.data(), m_pProgDesc->pszOptions,
                               &kernelNames));
    resultCode = pResult->getResult();
  } catch (std::bad_alloc &) {
    resultCode = CL_OUT_OF_HOST_MEMORY;
  }

#ifdef OCLFRONTEND_PLUGINS
  std::string Env;
  if (Intel::OpenCL::Utils::getEnvVar(Env, "OCLBACKEND_PLUGINS") &&
      Intel::OpenCL::Utils::getEnvVar(Env, "OCL_DISABLE_SOURCE_RECORDER")) {
    Intel::OpenCL::Frontend::LinkData linkData;

    for (unsigned int i = 0; i < m_Binaries.size(); ++i) {
      linkData.addInputBuffer(m_Binaries[i], m_BinariesSizes[i]);
    }
    linkData.setOptions(m_pProgDesc->pszOptions);
    linkData.setBinaryResult(pResult.get());
    g_pluginManager.OnLink(&linkData);
  }
#endif // OCLFRONTEND_PLUGINS

  if (pBinaryResult) {
    *pBinaryResult = pResult.release();
  }

  pKernelNames->SetAllKernelNames(kernelNames);
  if (m_pProgDesc->pKernelNames) {
    *m_pProgDesc->pKernelNames = pKernelNames.release();
  }

  return resultCode;
}
