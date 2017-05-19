; RUN: opt -S -VPlanDriver -vplan-build-stress-test < %s | FileCheck %s

; CHECK: vector.body:
; CHECK:  %wide.masked.load = call {{.*}} @llvm.masked.load
; CHECK:  call {{.*}} @llvm.masked.store
; CHECK: middle.block:

; ModuleID = 'ts.c'
source_filename = "ts.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr1 = external local_unnamed_addr global [100 x i32], align 16
@arr2 = external local_unnamed_addr global [100 x i32], align 16
@arr3 = external local_unnamed_addr global [100 x i32], align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @arr1, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %tobool = icmp eq i32 %0, 0
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx2 = getelementptr inbounds [100 x i32], [100 x i32]* @arr2, i64 0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4, !tbaa !1
  %tobool3 = icmp eq i32 %1, 0
  br i1 %tobool3, label %for.inc, label %if.then4

if.then4:                                         ; preds = %if.then
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* @arr3, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx6, align 4, !tbaa !1
  %3 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %2, %3
  store i32 %add, i32* %arrayidx6, align 4, !tbaa !1
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body, %if.then4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 50
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 20949)"}
!1 = !{!2, !3, i64 0}
!2 = !{!"array@_ZTSA100_i", !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
