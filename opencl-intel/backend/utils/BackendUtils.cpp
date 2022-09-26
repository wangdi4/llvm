#include "BackendUtils.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

using namespace llvm;

namespace BackendUtils {
static void recordCtorDtors(iterator_range<orc::CtorDtorIterator> CtorDtors,
                            std::vector<std::string> &CtorDtorNames) {
  if (CtorDtors.empty())
    return;

  std::map<unsigned, std::vector<const Function *>> CtorDtorsByPriority;
  for (auto CtorDtor : CtorDtors) {
    assert(CtorDtor.Func && CtorDtor.Func->hasName() &&
           "Ctor/Dtor must be a named function");
    if (CtorDtor.Data && cast<GlobalValue>(CtorDtor.Data)->isDeclaration())
      continue;

    if (CtorDtor.Func->hasLocalLinkage()) {
      CtorDtor.Func->setLinkage(GlobalValue::ExternalLinkage);
      CtorDtor.Func->setVisibility(GlobalValue::HiddenVisibility);
    }

    CtorDtorsByPriority[CtorDtor.Priority].push_back(CtorDtor.Func);
  }

  for (auto &KV : CtorDtorsByPriority) {
    for (auto &Func : KV.second)
      CtorDtorNames.push_back(Func->getName().str());
  }
}

void recordGlobalCtorDtors(Module &M, std::vector<std::string> &CtorNames,
                           std::vector<std::string> &DtorNames) {
  recordCtorDtors(orc::getConstructors(M), CtorNames);
  recordCtorDtors(orc::getDestructors(M), DtorNames);
}

} // namespace BackendUtils

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
