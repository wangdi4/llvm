; INTEL_CUSTOMIZATION
; REQUIRES: asserts
; RUN: opt -passes='default<O0>' -paropt=31 -debug-only=basicaa-opt-level -S %s 2>&1 | FileCheck %s -check-prefix=NPM

; Check that with legacy pass manager, basic-aa gets the opt-level 0
; propagated from Paropt.

; For legacy manager:
; Check for basic-aa's default initialization with 2:
; LPM: BasicAAResult: using OptLevel = 2
; Check for the opt-level 0 is propagated from Paropt to basic-aa:
; LPM-NEXT: BasicAAResult: using OptLevel = 0

; For new pass manager:
; Check that basic-aa's default initialization is done with 0
; (i.e., it gets the correct value from the XmainOptLevelAnalysis pass.)
; NPM-NOT: BasicAAResult: using OptLevel = 2
; NPM: BasicAAResult: using OptLevel = 0

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
; end INTEL_CUSTOMIZATION
