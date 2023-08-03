; RUN: opt -passes=auto-cpu-clone < %s -S | FileCheck %s

; This test checks that functions that carry Attribute::Hot are multi-versioned.


; CHECK: @foo = dso_local ifunc void (), ptr @foo.resolver
; CHECK: @foo.A()
; CHECK: @foo.V()
; CHECK: @foo.resolver()


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: hot mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local void @foo() local_unnamed_addr #0 !llvm.auto.cpu.dispatch !3 {
entry:
  ret void
}

attributes #0 = { hot mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!3 = !{!"core-avx2"}
