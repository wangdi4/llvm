; RUN: opt -passes=inline -inline-report=0x2819 -disable-output < %s 2>&1 | FileCheck --strict-whitespace %s
; RUN: opt -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -inline-report=0x2898 -S < %s 2>&1 | FileCheck --strict-whitespace %s

; CMPLRLLVM-38939: Fix indentation of lines when "med" form of inlining report is
; used so that extra spaces are not emitted when EXTERN functions are excluded in
; the report.

; CHECK: COMPILE FUNC: main
; CHECK:   -> INLINE: foo
; CHECK    -> INLINE: foo
; CHECK:   -> INLINE: foo

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@myglobal = dso_local global i32 0, align 4

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  call void @foo()
  call void @foo()
  call void (...) @bar()
  call void (...) @bar()
  call void @foo()
  ret i32 0
}

declare dso_local void @bar(...) #1

; Function Attrs: nounwind uwtable
define internal void @foo() #0 {
entry:
  %0 = load i32, ptr @myglobal, align 4, !tbaa !3
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr @myglobal, align 4, !tbaa !3
  ret void
}

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}

