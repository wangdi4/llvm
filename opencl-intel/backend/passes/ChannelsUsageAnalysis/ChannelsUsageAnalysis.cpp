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

#include "ChannelsUsageAnalysis.h"

#include "CompilationUtils.h"
#include "OCLPassSupport.h"
#include "cl_env.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

#include <utility>

using namespace llvm;
using namespace DPCPPKernelMetadataAPI;
using Intel::OpenCL::DeviceBackend::CompilationUtils;
using Intel::OpenCL::DeviceBackend::PipeKind;
using Intel::OpenCL::DeviceBackend::PipeTypesHelper;

namespace intel {

char ChannelsUsageAnalysis::ID = 0;
OCL_INITIALIZE_PASS(ChannelsUsageAnalysis, "channels-usage-analysis",
    "Prints graph with nodes representing kernels and edges representing "
    "FPGA channels/pipes usage", true, true)

ChannelsUsageAnalysis::ChannelsUsageAnalysis() : ModulePass(ID) {}

bool ChannelsUsageAnalysis::runOnModule(Module &M) {
#ifndef INTEL_PRODUCT_RELEASE
  std::string GraphFilename;
  if (Intel::OpenCL::Utils::getEnvVar(GraphFilename,
                                      "VOLCANO_CHANNELS_USAGE_GRAPH")) {
    std::error_code ErrCode;
    raw_fd_ostream FStream(GraphFilename, ErrCode);
    if (ErrCode) {
      errs() << "Failed to open file " << GraphFilename
             << ". Error: " << ErrCode.value() << " " << ErrCode.message()
             << "\n";
      return false;
    }

    this->print(FStream, &M);
  }
#endif // INTEL_PRODUCT_RELEASE
  return false;
}

void ChannelsUsageAnalysis::print(raw_ostream &OS, const Module *M) const {
  PipeTypesHelper PipeTypes(*M);
  if (!PipeTypes.hasPipeTypes())
    return; // no pipes in module, nothing to do

  SmallPtrSet<Function *, 16> Functions;
  SmallPtrSet<const Value *, 16> GlobalPipes;
  SmallVector<Value *, 16> WorkList;
  DenseMap<const Value *, std::pair<Function *, PipeKind>> ReadMap,
      WriteMap;

  for (const GlobalVariable &GV : M->globals()) {
    // TODO: add support of pipe arrays
    if (PipeTypes.isPipe(&GV)/* || PipeTypes.isPipeArray(&GV)*/) {
      GlobalPipes.insert(&GV);
      WorkList.push_back(const_cast<GlobalVariable *>(&GV));
    }
  }

  Value *GlobalPipe = nullptr;
  while (!WorkList.empty()) {
    Value *Pipe = WorkList.back();
    WorkList.pop_back();

    if (GlobalPipes.count(Pipe))
      GlobalPipe = Pipe;
    assert(GlobalPipe && "GlobalChannel not found?");

    for (const auto &U : Pipe->uses()) {
      Value *PipeUser = U.getUser();
      WorkList.push_back(PipeUser);
      if (!isa<CallInst>(PipeUser))
        continue;

      CallInst *CI = cast<CallInst>(PipeUser);
      assert(CI->getCalledFunction() && "Indirect function call?");
      const PipeKind Kind =
          CompilationUtils::getPipeKind(std::string(CI->getCalledFunction()->getName()));
      if (!Kind)
        continue;

      Function *F = CI->getParent()->getParent();
      Functions.insert(F);

      auto &Map = (PipeKind::READ == Kind.Access) ? ReadMap : WriteMap;
      Map[GlobalPipe] = std::make_pair(F, Kind);
    }
  }

  OS << "digraph G {\n";
  for (auto *F : Functions) {
    auto KMD = KernelMetadataAPI(F);
    if (KMD.Autorun.hasValue() && KMD.Autorun.get()) {
      OS << "\t\"" << F->getName() << "\" [shape=circle];\n";
    } else {
      // TODO: different style for user-defined functions
      OS << "\t\"" << F->getName() << "\" [shape=box];\n";
    }
  }

  for (auto &WIt : WriteMap) {
    const Value *Channel = WIt.getFirst();
    std::pair<Function *, PipeKind> From = WIt.getSecond();
    if (ReadMap.count(Channel)) {
      std::pair<Function *, PipeKind> To = ReadMap[Channel];
      // TODO: group autorun kernels into subgraphs
      // TODO: support channel-by-value
      // TODO: support pipes
      // TODO: support of I/O channels and pipes
      StringRef ArrowTail = (From.second.Blocking) ? "box" : "obox";
      StringRef ArrowHead = (To.second.Blocking) ? "normal" : "empty";
      StringRef ChannelName = Channel->getName();
      ChannelName.consume_back(".pipe");

      OS << "\tedge [arrowtail=" << ArrowTail << ",arrowhead=" << ArrowHead;
      OS << ",dir=both,label=\"" << ChannelName << "\"];\n";
      OS << "\t\"" << From.first->getName() << "\" -> \"" << To.first->getName()
         << "\"\n";
    }
  }
  OS << "}\n";
}

void ChannelsUsageAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}

} // namespace intel

extern "C" {
  llvm::ModulePass *createChannelsUsageAnalysisPass() {
    return new intel::ChannelsUsageAnalysis();
  }
}

