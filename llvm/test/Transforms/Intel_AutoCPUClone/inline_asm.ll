; RUN: opt -opaque-pointers -passes=auto-cpu-clone < %s -S | FileCheck %s

; The test checks that functions that have inline assembly are not
; multiversioned
; Source for this LLVM IR:
; void foo () {
;   __asm__("bar:" :);
; }


; CHECK: define dso_local void @_Z3foov()
; CHECK-NOT: @_Z3foov.A()
; CHECK-NOT: @_Z3foov.V()
; CHECK-NOT: @_Z3foov.resolver()


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z3foov() #0 !llvm.auto.cpu.dispatch !3 {
entry:
  call void asm sideeffect "bar:", "~{dirflag},~{fpsr},~{flags}"() #1, !srcloc !5
  ret void
}

attributes #0 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4}
!4 = !{!"auto-cpu-dispatch-target", !"core-avx2"}
!5 = !{i64 27}
