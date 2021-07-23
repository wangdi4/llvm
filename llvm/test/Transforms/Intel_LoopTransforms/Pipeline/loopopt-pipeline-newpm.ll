; RUN: opt -passes='default<O2>' -loopopt -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s
; RUN: opt -passes='default<O2>' -loopopt -print-before-all -print-after-all < %s -o /dev/null 2>&1 | FileCheck %s --check-prefix="CHECKPRINT"

; Verify that XmainOptLevelAnalysis is run early in module pass pipeline before Annotation2MetadataPass.

; CHECK: Running pass: XmainOptLevelAnalysisInit
; CHECK: Running pass: Annotation2MetadataPass


; Verify that LoopOptMarker is run at the very beginning of function pass pipeline.

; CHECK: Running pass: LoopOptMarkerPass


; Verify that HIRFramework is invalidated immediately after HIRCodeGen even
; if HIRCodeGen didn't modify IR. (names shouldn't have the "loopopt::" prefix)

; CHECK: Running pass: HIRCodeGenPass
; CHECK-NEXT: Invalidating analysis: HIRFrameworkAnalysis

; Verify that pass names in "IR Dump" lines don't have the "loopopt::" prefix
; for HIR passes.

; CHECKPRINT: IR Dump Before HIRTempCleanupPass
; CHECKPRINT: IR Dump After HIRTempCleanupPass

define void @f() {
  ret void
}
