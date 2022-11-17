; INTEL_CUSTOMIZATION
; REQUIRES: asserts
; RUN: opt -enable-new-pm=1 -passes='default<O0>' -paropt=31 -debug-only=basicaa-opt-level -S %s 2>&1 | FileCheck %s -check-prefix=NPM

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

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
; end INTEL_CUSTOMIZATION
