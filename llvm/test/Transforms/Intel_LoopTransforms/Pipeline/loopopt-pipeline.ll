; RUN: opt -enable-new-pm=0 -O2 -loopopt -debug-pass=Structure < %s -o /dev/null 2>&1 | FileCheck %s
; RUN: opt -passes='default<O2>' -loopopt -debug-pass-manager=quiet < %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=NEWPM

; Verify that loopopt marker pass runs at the very beginning of the pipeline just after module verification
; for old pm.
; Notice there could be a few more passes between the two.

; REQUIRES: asserts

; CHECK:   FunctionPass Manager
; CHECK-NEXT:     Module Verifier
; CHECK-NEXT:     LoopOpt Marker

; NEWPM: VerifierPass on [module]
; NEWPM-NEXT: XmainOptLevelAnalysisInit on [module]
; NEWPM-NEXT: Annotation2MetadataPass on [module]
; NEWPM-NEXT: ForceFunctionAttrsPass on [module]
; NEWPM-NEXT: InferFunctionAttrsPass on [module]
; NEWPM-NEXT: InlineReportSetupPass on [module]
; NEWPM-NEXT: InlineListsPass on [module]
; NEWPM-NEXT: CoroEarlyPass on [module]
; NEWPM-NEXT: LoopOptMarkerPass on f

define void @f() "loopopt-pipeline"="full" {
  ret void
}
