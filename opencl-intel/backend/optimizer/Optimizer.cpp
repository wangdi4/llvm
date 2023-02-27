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
#include "llvm/Transforms/SYCLTransforms/SYCLKernelAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/VFAnalysis.h"

#if INTEL_CUSTOMIZATION
// Enable vectorization at O0 optimization level.
extern cl::opt<bool> EnableO0Vectorization;
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

using namespace intel;
namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

// Load Table-Gen'erated VectInfo.gen
static constexpr llvm::VectItem Vect[] = {
#include "VectInfo.gen"
};
static constexpr llvm::ArrayRef<llvm::VectItem> VectInfos(Vect);

llvm::ArrayRef<llvm::VectItem> Optimizer::getVectInfos() { return VectInfos; }
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
/// - Handles VFAnalysisDiagInfo emitted by VFAnalysis.
class OCLDiagnosticHandler : public llvm::DiagnosticHandler {
public:
  OCLDiagnosticHandler(llvm::raw_ostream &OS) : OS(OS) {}
  bool handleDiagnostics(const llvm::DiagnosticInfo &DI) override {
    // Handle VFAnalysisDiagInfo emitted by VFAnalysis.
    if (auto *VFADI = dyn_cast<llvm::VFAnalysisDiagInfo>(&DI)) {
      OS << llvm::LLVMContext::getDiagnosticMessagePrefix(VFADI->getSeverity())
         << ": ";
      VFADI->print(OS);
      OS << ".\n";
      if (VFADI->getSeverity() == DS_Error)
        throw Exceptions::CompilerException(
            "Checking vectorization factor failed", CL_DEV_INVALID_BINARY);
      return true;
    }
    if (auto *DKADI = dyn_cast<llvm::SYCLKernelAnalysisDiagInfo>(&DI)) {
      OS << llvm::LLVMContext::getDiagnosticMessagePrefix(DKADI->getSeverity())
         << ": ";
      DKADI->print(OS);
      OS << ".\n";
      if (DKADI->getSeverity() == DS_Error)
        throw Exceptions::CompilerException(
            "Analyzing SYCL kernel properties failed", CL_DEV_INVALID_BINARY);
      return true;
    }
    return false;
  }

private:
  llvm::DiagnosticPrinterRawOStream OS;
};

Optimizer::Optimizer(llvm::Module &M,
                     llvm::SmallVectorImpl<llvm::Module *> &RtlModules,
                     const intel::OptimizerConfig &OptConfig)
    : m_M(M), m_RtlModules(RtlModules.begin(), RtlModules.end()),
      Config(OptConfig),
      m_IsSPIRV(llvm::CompilationUtils::generatedFromSPIRV(M)),
      m_IsSYCL(llvm::CompilationUtils::isGeneratedFromOCLCPP(M)),
      m_IsOMP(llvm::CompilationUtils::isGeneratedFromOMP(M)),
      m_IsFpgaEmulator(Config.isFpgaEmulator()), UnrollLoops(true) {
  assert(Config.GetCpuId() && "Invalid optimizer config");
  ISA = VectorizerUtils::getCPUIdISA(Config.GetCpuId());
  CPUPrefix = Config.GetCpuId()->GetCPUPrefix();
  SYCLForceOptnone = Config.GetDisableOpt();
  m_IsOcl20 = llvm::CompilationUtils::fetchCLVersionFromMetadata(M) >=
              llvm::CompilationUtils::OclVersion::CL_VER_2_0;
  m_debugType = getDebuggingServiceType(Config.GetDebugInfoFlag(), &M,
                                        Config.GetUseNativeDebuggerFlag());
  m_UseTLSGlobals = (m_debugType == intel::Native);

  static llvm::once_flag OptionOnceFlag;
  llvm::call_once(OptionOnceFlag, [&] {
    *DebugPM;
    *DisableVPlanCM; // INTEL
    *VerifyEachPass;
  });
}

void Optimizer::setDiagnosticHandler(llvm::raw_ostream &LogStream) {
  m_M.getContext().setDiagnosticHandler(
      std::make_unique<OCLDiagnosticHandler>(LogStream));
}

bool Optimizer::hasUnsupportedRecursion() {
  return m_IsSYCL
             ? !GetInvalidFunctions(InvalidFunctionType::RECURSION_WITH_BARRIER)
                    .empty()
             : !GetInvalidFunctions(InvalidFunctionType::RECURSION).empty();
}

bool Optimizer::hasFpgaPipeDynamicAccess() const {
  return !GetInvalidFunctions(InvalidFunctionType::FPGA_PIPE_DYNAMIC_ACCESS)
              .empty();
}

bool Optimizer::hasVectorVariantFailure() const {
  return !GetInvalidFunctions(InvalidFunctionType::VECTOR_VARIANT_FAILURE)
              .empty();
}

bool Optimizer::hasFPGAChannelsWithDepthIgnored() const {
  return !GetInvalidGlobals(InvalidGVType::FPGA_DEPTH_IS_IGNORED).empty();
}

std::vector<std::string> Optimizer::GetInvalidGlobals(InvalidGVType Ty) const {
  std::vector<std::string> Res;

  for (auto &GV : m_M.globals()) {
    auto GVM = SYCLKernelMetadataAPI::GlobalVariableMetadataAPI(&GV);

    switch (Ty) {
    case FPGA_DEPTH_IS_IGNORED:
      if (GVM.DepthIsIgnored.hasValue() && GVM.DepthIsIgnored.get()) {
        assert(GV.getName().endswith(".pipe") &&
               "Only global pipes are expected");
        Res.push_back(std::string(GV.getName().drop_back(5)));
      }
    }
  }

  return Res;
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
      Invalid = F.hasFnAttribute(llvm::KernelAttribute::RecursionWithBarrier);
      break;
    case FPGA_PIPE_DYNAMIC_ACCESS:
      Invalid = KMD.FpgaPipeDynamicAccess.hasValue() &&
                KMD.FpgaPipeDynamicAccess.get();
      break;
    case VECTOR_VARIANT_FAILURE:
      Invalid = F.hasFnAttribute(llvm::KernelAttribute::VectorVariantFailure);
      break;
    }

    if (Invalid) {
      std::string Message;
      llvm::raw_string_ostream MStr(Message);
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
