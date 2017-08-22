; RUN: opt -hir-cg -force-hir-cg -S < %s | FileCheck %s
; Verifies UB is calcualted once before loop
; Verifies UB is correctly sext
; Verifies UB is correctly used in bt

; basic cg
; CHECK: region.0: 
;  sext.i32.i64(%n) + -1 is our ub for loop
; should be calculated but once, before we enter loop
; CHECK: [[SEXT_OP1:%[0-9]+]] = sext i32 %n to i64
; CHECK-NEXT:  [[UB1:%[0-9]+]] = add i64 [[SEXT_OP1]], -1
; CHECK-NEXT: br label %[[L1Label:loop.[0-9]+]]

; CHECK [[L1Label]]:

; Check wrap flags on IV
; CHECK: [[IV_UPDATE:%nextiv.*]] = add nuw nsw i64 {{%.*}}, 1

; make sure ub is used in bt of loop
; CHECK: %cond[[L1Label]] = icmp sle i64 [[IV_UPDATE]], [[UB1]]
; CHECK-NEXT: br i1 %cond[[L1Label]], label %[[L1Label]], label %after[[L1Label]]

; CHECK: after[[L1Label]]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [10 x i32] zeroinitializer, align 16
@A = common global [10 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo(i32 %n) #0 {
entry:
  %cmp6 = icmp sgt i32 %n, 0
  br i1 %cmp6, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %i.07 = phi i64 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %i.07
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %arrayidx2 = getelementptr inbounds [10 x i32], [10 x i32]* @A, i64 0, i64 %i.07
  store i32 %1, i32* %arrayidx2, align 4, !tbaa !1
  %inc = add nuw nsw i64 %i.07, 1
  %exitcond = icmp eq i64 %inc, %0
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 637) (llvm/branches/loopopt 690)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

