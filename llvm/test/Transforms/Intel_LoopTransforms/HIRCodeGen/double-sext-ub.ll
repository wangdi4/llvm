; RUN: opt -hir-cg -force-hir-cg -S < %s | FileCheck %s
; This text verifies that UB is correctly sext and is calculated
; only once, before loop
; Also verifies loop nest is correctly nested, i1 then i2
; basic cg
; CHECK: region.0: 
;  sext.i32.i64(%n) + -1 is our ub for loop 1
; CHECK: [[SEXT_OP1:%[0-9]+]] = sext i32 %n to i64
; CHECK-NEXT:  [[UB1:%[0-9]+]] = add i64 [[SEXT_OP1]], -1
; CHECK-NEXT: br label %[[L1Label:loop.[0-9]+]]

; CHECK: [[L1Label]]:
; check ub of l2 is also sext and calculated before loop2
; CHECK: [[SEXT_OP2:%[0-9]+]] = sext i32 %n to i64
; CHECK-NEXT:  [[UB2:%[0-9]+]] = add i64 [[SEXT_OP2]], -1
; CHECK-NEXT: br label %[[L2Label:loop.[0-9]+]]

; make sure loop 2 ends then loop 2 ends
; CHECK: after[[L2Label]]:
; CHECK: after[[L1Label]]:
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [10 x i32] zeroinitializer, align 16
@A = common global [10 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo(i32 %n, i32 %k) #0 {
entry:
  %cmp20 = icmp sgt i32 %n, 0
  br i1 %cmp20, label %for.cond2.preheader.lr.ph, label %for.end12

for.cond2.preheader.lr.ph:                        ; preds = %entry
  %conv7 = sext i32 %k to i64
  %0 = sext i32 %n to i64
  br label %for.body6.lr.ph

for.body6.lr.ph:                                  ; preds = %for.inc10, %for.cond2.preheader.lr.ph
  %i.021 = phi i64 [ 0, %for.cond2.preheader.lr.ph ], [ %inc11, %for.inc10 ]
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %i.021
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %mul = shl i64 %i.021, 1
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.body6.lr.ph
  %j.019 = phi i64 [ 0, %for.body6.lr.ph ], [ %inc, %for.body6 ]
  %mul8 = mul nsw i64 %j.019, %conv7
  %add = add nsw i64 %mul8, %mul
  %arrayidx9 = getelementptr inbounds [10 x i32], [10 x i32]* @A, i64 0, i64 %add
  store i32 %1, i32* %arrayidx9, align 4, !tbaa !1
  %inc = add nuw nsw i64 %j.019, 1
  %exitcond = icmp eq i64 %inc, %0
  br i1 %exitcond, label %for.inc10, label %for.body6

for.inc10:                                        ; preds = %for.body6
  %inc11 = add nuw nsw i64 %i.021, 1
  %exitcond22 = icmp eq i64 %inc11, %0
  br i1 %exitcond22, label %for.end12.loopexit, label %for.body6.lr.ph

for.end12.loopexit:                               ; preds = %for.inc10
  br label %for.end12

for.end12:                                        ; preds = %for.end12.loopexit, %entry
  ret void
}
attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 637) (llvm/branches/loopopt 690)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

