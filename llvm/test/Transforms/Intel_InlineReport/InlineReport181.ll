; RUN: opt -passes='always-inline' -S -inline-report=0xf847 < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='inlinereportsetup,always-inline,inlinereportemitter' -S -inline-report=0xf886 < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Check that the always inliner contributes to the inlining report.

; CHECK-CL-NOT: call i32 @foo()
; CHECK-CL: COMPILE FUNC: foo
; CHECK: COMPILE FUNC: main
; INLINE: foo {{.*}}Callee is always inline
; CHECK-MD: COMPILE FUNC: foo
; CHECK-MD-NOT: call i32 @foo()

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %call = call i32 @foo() #1
  ret i32 %call
}

; Function Attrs: noinline nounwind optnone uwtable
define internal i32 @foo() #0 {
entry:
  ret i32 5
}

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { alwaysinline }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.1.0 (2024.x.0.YYYYMMDD)"}
