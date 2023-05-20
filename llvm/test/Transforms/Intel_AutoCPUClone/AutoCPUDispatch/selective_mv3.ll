; RUN: opt -passes=auto-cpu-clone < %s -S | FileCheck %s

; The test ensures that functions, that are called transitively from loop bodies,
; are considered for multi-versioning.

; CHECK-NOT: define dso_local void @TransitivelyCalledFn
; CHECK: @TransitivelyCalledFn.A
; CHECK: @TransitivelyCalledFn.b
; CHECK: @TransitivelyCalledFn.resolver

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @TransitivelyCalledFn() local_unnamed_addr #0 !llvm.auto.cpu.dispatch !3 {
entry:
  ret void
}

; Function Attrs: noinline nounwind uwtable
define dso_local void @DirectlyCalledFn() local_unnamed_addr #0 !llvm.auto.cpu.dispatch !3 {
entry:
  ret void
}

; Function Attrs: noinline nounwind uwtable
define dso_local void @Loop2Fn(i32 noundef %N) local_unnamed_addr #0 !llvm.auto.cpu.dispatch !3 {
entry:
  call void @TransitivelyCalledFn()
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 1, %entry ], [ %inc, %for.body ]
  %cmp = icmp slt i32 %i.0, %N
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret void

for.body:                                         ; preds = %for.cond
  call void @DirectlyCalledFn()
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !5
}

; Function Attrs: noinline nounwind uwtable
define dso_local void @Loop1Fn(i32 noundef %N) local_unnamed_addr #0 !llvm.auto.cpu.dispatch !3 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 1, %entry ], [ %inc, %for.body ]
  %cmp = icmp slt i32 %i.0, %N
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret void

for.body:                                         ; preds = %for.cond
  call void @Loop2Fn(i32 noundef %N)
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !7
}

attributes #0 = { noinline nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!3 = !{!"skylake"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
!7 = distinct !{!7, !6}
