; RUN: opt -passes=auto-cpu-clone -disable-selective-mv < %s -S | FileCheck %s

; This lit test checks that the resolvers of ifuncs in user code
; are never multi-versioned.
;
; Source for this LLVM IR:
; int f1 () { return 0; }
; typeof(f1) *resolver () { return f1; }
; int foo () __attribute__ ((ifunc ("resolver")));


; CHECK-NOT: @resolver = dso_local ifunc ptr (), ptr @resolver.resolver


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@foo = dso_local ifunc i32 (...), ptr @resolver

; Function Attrs: nounwind uwtable
define dso_local i32 @f1() #0 !llvm.auto.arch !3 {
entry:
  ret i32 0
}

; Function Attrs: nounwind uwtable
define dso_local ptr @resolver() #0 !llvm.auto.arch !3 {
entry:
  ret ptr @f1
}

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!"skylake-avx512"}
