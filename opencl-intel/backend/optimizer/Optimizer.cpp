//
// Copyright 2012-2023 Intel Corporation.
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

#include "Optimizer.h"
#include "VectorizerUtils.h"
#include "exceptions.h"

#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/SYCLTransforms/Intel_VectorVariant/IndirectCallLowering.h"
#include "llvm/Transforms/SYCLTransforms/SYCLKernelAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/Utils/DiagnosticInfo.h"
#include "llvm/Transforms/SYCLTransforms/VFAnalysis.h"

using namespace llvm;

#if INTEL_CUSTOMIZATION
// Enable vectorization at O0 optimization level.
extern cl::opt<bool> SYCLEnableO0Vectorization;
#endif // INTEL_CUSTOMIZATION

// If set, then optimization passes will process functions as if they have the
// optnone attribute.
extern bool SYCLForceOptnone;

namespace {

struct CreateDebugPM {
  static void *call() {
    return new cl::opt<DebugLogging>(
        "debug-pass-manager", cl::Hidden, cl::ValueOptional,
        cl::desc("Print pass management debugging information"),
        cl::init(DebugLogging::None),
        cl::values(
            clEnumValN(DebugLogging::Normal, "", ""),
            clEnumValN(DebugLogging::Quiet, "quiet",
                       "Skip printing info about analyses"),
            clEnumValN(
                DebugLogging::Verbose, "verbose",
                "Print extra information about adaptors and pass managers")));
  }
};
ManagedStatic<cl::opt<DebugLogging>, CreateDebugPM> DebugPM;

struct CreateVerifyEachPass {
  static void *call() {
    return new cl::opt<bool>("verify-each-pass", cl::Hidden,
                             cl::desc("Verify IR after each pass"),
                             cl::init(false));
  }
};
ManagedStatic<cl::opt<bool>, CreateVerifyEachPass> VerifyEachPass;

#if INTEL_CUSTOMIZATION
struct CreateDisableVPlanCM {
  static void *call() {
    return new cl::opt<bool>(
        "disable-ocl-vplan-cost-model", cl::init(false), cl::Hidden,
        cl::desc("Disable cost model for VPlan vectorizer"));
  }
};
ManagedStatic<cl::opt<bool>, CreateDisableVPlanCM> DisableVPlanCM;
#endif // INTEL_CUSTOMIZATION

} // namespace

using CPUDetect = Intel::OpenCL::Utils::CPUDetect;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

// Load Table-Gen'erated VectInfo.gen
static constexpr VectItem Vect[] = {
#include "VectInfo.gen"
};
static constexpr ArrayRef<VectItem> VectInfos(Vect);

ArrayRef<VectItem> Optimizer::getVectInfos() { return VectInfos; }
#if INTEL_CUSTOMIZATION
const StringSet<> &Optimizer::getVPlanMaskedFuncs() {
  static const StringSet<> VPlanMaskedFuncs =
#define IMPORT_VPLAN_MASKED_VARIANTS
#include "VectInfo.gen"
#undef IMPORT_VPLAN_MASKED_VARIANTS
      ;
  return VPlanMaskedFuncs;
}
#endif // INTEL_CUSTOMIZATION

/// Customized diagnostic handler to be registered to LLVMContext before running
/// passes. Prints error messages and throw exception if received an error
/// diagnostic.
class OCLDiagnosticHandler : public DiagnosticHandler {
public:
  OCLDiagnosticHandler(raw_ostream &OS) : OS(OS) {}
  bool handleDiagnostics(const DiagnosticInfo &DI) override {
    std::string ExceptionMsg;
    if (isa<VFAnalysisDiagInfo>(&DI))
      ExceptionMsg = "Checking vectorization factor failed";
    else if (isa<SYCLKernelAnalysisDiagInfo>(&DI))
      ExceptionMsg = "Analyzing SYCL kernel properties failed";
    else if (isa<VectorVariantDiagInfo>(&DI))
      ExceptionMsg = "Vector-variant failure";
    else if (!isa<OptimizationWarningDiagInfo>(&DI))
      return false;

    OS << LLVMContext::getDiagnosticMessagePrefix(DI.getSeverity()) << ": ";
    DI.print(OS);
    OS << "\n";

    if (DI.getSeverity() == DS_Error)
      throw Exceptions::CompilerException(ExceptionMsg, CL_DEV_INVALID_BINARY);

    return true;
  }

private:
  DiagnosticPrinterRawOStream OS;
};

Optimizer::Optimizer(Module &M, SmallVectorImpl<Module *> &RtlModules,
                     const intel::OptimizerConfig &OptConfig)
    : m_M(M), m_RtlModules(RtlModules.begin(), RtlModules.end()),
      Config(OptConfig), m_IsSYCL(CompilationUtils::isGeneratedFromOCLCPP(M)),
      m_IsOMP(CompilationUtils::isGeneratedFromOMP(M)),
      m_IsFpgaEmulator(Config.isFpgaEmulator()) {
  assert(Config.GetCpuId() && "Invalid optimizer config");
  ISA = VectorizerUtils::getCPUIdISA(Config.GetCpuId());
  CPUPrefix = Config.GetCpuId()->GetCPUPrefix();
  SYCLForceOptnone = Config.GetDisableOpt();
  m_HasOcl20 = CompilationUtils::hasOcl20Support(M);
  m_UseTLSGlobals = !M.debug_compile_units().empty();

  static llvm::once_flag OptionOnceFlag;
  llvm::call_once(OptionOnceFlag, [&] {
    *DebugPM;
    *DisableVPlanCM; // INTEL
    *VerifyEachPass;
  });
}

void Optimizer::setDiagnosticHandler(raw_ostream &LogStream) {
  m_M.getContext().setDiagnosticHandler(
      std::make_unique<OCLDiagnosticHandler>(LogStream));
}

bool Optimizer::hasUnsupportedRecursion() {
  return m_IsSYCL
             ? !GetInvalidFunctions(InvalidFunctionType::RECURSION_WITH_BARRIER)
                    .empty()
             : !GetInvalidFunctions(InvalidFunctionType::RECURSION).empty();
}

std::vector<std::string>
Optimizer::GetInvalidFunctions(InvalidFunctionType Ty) const {
  std::vector<std::string> Res;

  for (auto &F : m_M) {
    auto KMD = SYCLKernelMetadataAPI::FunctionMetadataAPI(&F);

    bool Invalid = false;

    switch (Ty) {
    case RECURSION:
      Invalid = KMD.RecursiveCall.hasValue() && KMD.RecursiveCall.get();
      break;
    case RECURSION_WITH_BARRIER:
      Invalid = F.hasFnAttribute(KernelAttribute::RecursionWithBarrier);
      break;
    }

    if (Invalid) {
      std::string Message;
      raw_string_ostream MStr(Message);
      MStr << std::string(F.getName());
      if (auto SP = F.getSubprogram()) {
        MStr << " at ";
        MStr << "file: " << SP->getFilename() << ", line:" << SP->getLine();
      }
      Res.push_back(std::move(Message));
    }
  }

  return Res;
}

void Optimizer::initOptimizerOptions() {
  *DebugPM;
  *DisableVPlanCM; // INTEL
  *VerifyEachPass;
}

DebugLogging Optimizer::getDebugPM() { return *DebugPM; }

bool Optimizer::getDisableVPlanCM() { return *DisableVPlanCM; } // INTEL

bool Optimizer::getVerifyEachPass() { return *VerifyEachPass; }

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
