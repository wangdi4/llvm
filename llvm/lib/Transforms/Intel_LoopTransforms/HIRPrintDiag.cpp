#include "HIRPrintDiag.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {

namespace loopopt {

void printDiag(StringRef FuncNameKnobFromCommandLine,
               unsigned DiagLevelKnobFromCommandLine, StringRef Msg,
               StringRef FuncName, const HLLoop *Loop, StringRef Header,
               unsigned DiagLevel) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (DiagLevel > DiagLevelKnobFromCommandLine)
    return;
  if (!FuncNameKnobFromCommandLine.empty() &&
      !FuncNameKnobFromCommandLine.equals(FuncName)) {
    return;
  }
  dbgs() << "Func: " << FuncName << ", ";
  dbgs() << Header << " " << Msg << "\n";
  if (Loop) {
    dbgs() << "Loop:" << Loop->getNumber() << "\n";
  }
#endif
}

void printMarker(bool PrintInfo, StringRef Marker,
                 ArrayRef<const HLNode *> Nodes, bool DumpNode, bool Detail) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (!PrintInfo)
    return;

  dbgs() << Marker;
  for (auto Node : Nodes) {
    if (!DumpNode)
      dbgs() << " " << Node->getNumber();
    else {
      dbgs() << "\n";
      Node->dump(Detail);
    }
  }
  dbgs() << "\n";
#endif
}

void printMarker(bool PrintInfo, StringRef Marker,
                 ArrayRef<const RegDDRef *> Refs, bool Detail, bool PrintDim) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (!PrintInfo)
    return;

  dbgs() << Marker;
  if (!PrintDim) {
    for (auto Ref : Refs) {
      dbgs() << " ";
      Ref->dump(Detail);
    }
    dbgs() << "\n";
    return;
  }

  for (auto Ref : Refs) {
    dbgs() << " ";
    Ref->dumpDims(Detail);
  }
  dbgs() << "\n";

#endif
}

} // namespace loopopt
} // namespace llvm
