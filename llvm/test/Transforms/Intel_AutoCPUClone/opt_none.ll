; RUN: opt -opaque-pointers -passes=auto-cpu-clone < %s -S | FileCheck %s

; The test checks that functions that have inline assembly are not
; multiversioned
; Source for this LLVM IR:
; __attribute__((optnone)) void foo() {
;   return;
; }


; CHECK: define dso_local void @_Z3foov()
; CHECK-NOT: @_Z3foov.A()
; CHECK-NOT: @_Z3foov.a()
; CHECK-NOT: @_Z3foov.resolver()


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3foov() #0 !llvm.auto.cpu.dispatch !2 {
entry:
  ret void
}

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!3}
!3 = !{!"auto-cpu-dispatch-target", !"skylake-avx512"}
