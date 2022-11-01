; RUN: opt -opaque-pointers -passes=auto-cpu-clone < %s -S | FileCheck %s

; The test checks that functions that have loops or intrinsics are multi-versioned
; and others are not.

; CHECK: define dso_local noundef i32 @_Z6noneFnv(
; CHECK-NOT: @_Z6noneFnv.A
; CHECK-NOT: @_Z6noneFnv.a
; CHECK-NOT: @_Z6noneFnv.resolver

; CHECK-NOT: define dso_local noundef i32 @_Z11intrinsicFnii(
; CHECK-DAG: @_Z11intrinsicFnii.A
; CHECK-DAG: @_Z11intrinsicFnii.a
; CHECK-DAG: @_Z11intrinsicFnii.resolver

; CHECK-NOT: define dso_local noundef i32 @_Z6loopFnv(
; CHECK-DAG: @_Z6loopFnv.A
; CHECK-DAG: @_Z6loopFnv.a
; CHECK-DAG: @_Z6loopFnv.resolver


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [50 x i32] zeroinitializer, align 16

; Function Attrs: mustprogress nofree noinline nosync nounwind readnone willreturn uwtable
define dso_local noundef i32 @_Z11intrinsicFnii(i32 noundef %sum, i32 noundef %i) local_unnamed_addr #0 !llvm.auto.cpu.dispatch !0 {
entry:
  %add = add nsw i32 %i, %sum
  %0 = tail call i32 @llvm.smin.i32(i32 %add, i32 32767)
  ret i32 %0
}

; Function Attrs: mustprogress nofree noinline nosync nounwind readonly willreturn uwtable
define dso_local noundef i32 @_Z6loopFnv() local_unnamed_addr #1 !llvm.auto.cpu.dispatch !0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret i32 %add

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %sum.07 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds [50 x i32], [50 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %call = tail call noundef i32 @_Z11intrinsicFnii(i32 noundef %sum.07, i32 noundef %0)
  %add = add nsw i32 %call, %sum.07
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 50
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !2
}

; Function Attrs: mustprogress nofree nosync nounwind readonly willreturn uwtable
define dso_local noundef i32 @_Z6noneFnv() local_unnamed_addr #2 !llvm.auto.cpu.dispatch !0 {
entry:
  %call = tail call noundef i32 @_Z6loopFnv()
  ret i32 %call
}

declare i32 @llvm.smin.i32(i32, i32) #3

attributes #0 = { mustprogress nofree noinline nosync nounwind readnone willreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nofree noinline nosync nounwind readonly willreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress nofree nosync nounwind readonly willreturn uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nocallback nofree nosync nounwind readnone speculatable willreturn }

!0 = !{!1}
!1 = !{!"auto-cpu-dispatch-target", !"skylake-avx512"}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.mustprogress"}

