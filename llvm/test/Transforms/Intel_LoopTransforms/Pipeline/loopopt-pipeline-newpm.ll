; RUN: opt -passes='default<O2>' -loopopt -debug-pass-manager < %s -o /dev/null 2>&1 | FileCheck %s

; Verify that XmainOptLevelAnalysis is run early in module pass pipeline before Annotation2MetadataPass.

; CHECK: Running pass: XmainOptLevelAnalysisInit
; CHECK: Running pass: Annotation2MetadataPass


; Verify that LoopOptMarker is run at the very beginning of function pass pipeline.

; CHECK: Running pass: LoopOptMarkerPass


; Verify that HIRFramework is invalidated immediately after HIRCodeGen even
; if HIRCodeGen didn't modify IR.

; CHECK: Running pass: loopopt::HIRCodeGenPass
; CHECK-NEXT: Invalidating analysis: loopopt::HIRFrameworkAnalysis

define void @f() {
  ret void
}
