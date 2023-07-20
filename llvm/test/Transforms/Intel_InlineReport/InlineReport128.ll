; RUN: opt -passes=inline -inline-report=0x2819 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes=inline -inline-report=0xf859 -disable-output < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-IND
; RUN: opt -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -inline-report=0x2898 -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -inline-report=0xf8d8 -S < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-IND

; CMPLRLLVM-38989: Check that the inlining report generated at opt report "med"
; level does not include indirect calls, but does at "max" level.

; CHECK: COMPILE FUNC: main
; CHECK: INLINE: foo
; CHECK: INLINE: foo
; CHECK-IND: INDIRECT:
; CHECK-IND: INDIRECT:
; CHECK: INLINE: foo

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@myglobal = dso_local local_unnamed_addr global i32 0, align 4
@myfp = dso_local local_unnamed_addr global ptr @bar, align 8

; Function Attrs: nounwind uwtable
define dso_local void @bar() #0 {
entry:
  %i = load i32, ptr @myglobal, align 4, !tbaa !3
  %dec = add nsw i32 %i, -1
  store i32 %dec, ptr @myglobal, align 4, !tbaa !3
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  call fastcc void @foo()
  call fastcc void @foo()
  %i = load ptr, ptr @myfp, align 8, !tbaa !7
  call void (...) %i() #1
  %i1 = load ptr, ptr @myfp, align 8, !tbaa !7
  call void (...) %i1() #1
  call fastcc void @foo()
  ret i32 0
}

; Function Attrs: nounwind uwtable
define internal fastcc void @foo() unnamed_addr #0 {
entry:
  %i = load i32, ptr @myglobal, align 4, !tbaa !3
  %inc = add nsw i32 %i, 1
  store i32 %inc, ptr @myglobal, align 4, !tbaa !3
  ret void
}

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"unspecified pointer", !5, i64 0}
