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

} // namespace loopopt
} // namespace llvm
