;; This test verifies correct cg for an HLIf, representing ztt of an i2 loop 
;; Also verifies correct cg for a double subscript address calculation
;;
; RUN: opt -hir-cg -force-hir-cg -S < %s | FileCheck %s
; basic cg
; CHECK: region.0:

; loop 1
; CHECK: br label %[[L1Label:loop.[0-9]+]]
; CHECK: [[L1Label]]:

; if (m > 0) {do i2 ...}
; CHECK: %hir.cmp.[[CMP_NUM:[0-9]+]] = icmp sgt i32 %m, 0
; CHECK-NEXT: br i1 %hir.cmp.[[CMP_NUM]], label %then.[[CMP_NUM]], label %ifmerge.[[CMP_NUM]]

;From here on, the bb's can be in any order
; assume hircgs ordering of appending doesnt change?
; CHECK: then.[[CMP_NUM]]:
; lod of b[] should be before i2 loop
; CHECK: getelementptr{{.*}} @B
; 
; i2 loop
; CHECK: br label %[[L2Label:loop.[0-9]+]]
; CHECK: [[L2Label]]:
; these arent fully reorderable, but verifier should eliminate illegal ones
; CHECK-DAG: [[I1:%[0-9]+]] = load i64, i64* %i1.i64
; CHECK-DAG: [[I1_MUL_2:%[0-9]+]] = mul i64 2, [[I1]]
; CHECK-DAG: [[I2:%[0-9]+]] = load i64, i64* %i2.i64
; CHECK-DAG: [[SEXT_K:%[0-9]+]] = sext i32 %k to i64
; CHECK-DAG: [[I2_MUL_K:%[0-9]+]] = mul i64 [[SEXT_K]], [[I2]]

; (@A)[2 * i1][sext.i32.i64(%k) * i2]
; CHECK:  getelementptr inbounds [10 x [10 x i32]], [10 x [10 x i32]]* @A, i64 0, i64 [[I1_MUL_2]], i64 [[I2_MUL_K]]

; after i2, we jump to if's merge point
; CHECK: after[[L2Label]]:
; CHECK: br label %ifmerge.[[CMP_NUM]]

;if merge should contain bt for i1 loop, just loop for label in br 
; CHECK: ifmerge.[[CMP_NUM]]:
; CHECK: label %[[L1Label]]
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [10 x i32] zeroinitializer, align 16
@A = common global [10 x [10 x i32]] zeroinitializer, align 16
; Function Attrs: nounwind uwtable
define void @foo(i32 %n, i32 %m, i32 %k) #0 {
entry:
  %cmp20 = icmp sgt i32 %n, 0
  br i1 %cmp20, label %for.cond2.preheader.lr.ph, label %for.end13

for.cond2.preheader.lr.ph:                        ; preds = %entry
  %cmp418 = icmp sgt i32 %m, 0
  %conv7 = sext i32 %k to i64
  %0 = sext i32 %m to i64
  %1 = sext i32 %n to i64
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %for.inc11, %for.cond2.preheader.lr.ph
  %i.021 = phi i64 [ 0, %for.cond2.preheader.lr.ph ], [ %inc12, %for.inc11 ]
  br i1 %cmp418, label %for.body6.lr.ph, label %for.inc11

for.body6.lr.ph:                                  ; preds = %for.cond2.preheader
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %i.021
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %mul8 = shl nsw i64 %i.021, 1
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.body6.lr.ph
  %j.019 = phi i64 [ 0, %for.body6.lr.ph ], [ %inc, %for.body6 ]
  %mul = mul nsw i64 %j.019, %conv7
  %arrayidx10 = getelementptr inbounds [10 x [10 x i32]], [10 x [10 x i32]]* @A, i64 0, i64 %mul8, i64 %mul
  store i32 %2, i32* %arrayidx10, align 4, !tbaa !1
  %inc = add nuw nsw i64 %j.019, 1
  %exitcond = icmp eq i64 %inc, %0
  br i1 %exitcond, label %for.inc11.loopexit, label %for.body6

for.inc11.loopexit:                               ; preds = %for.body6
  br label %for.inc11

for.inc11:                                        ; preds = %for.inc11.loopexit, %for.cond2.preheader
  %inc12 = add nuw nsw i64 %i.021, 1
  %exitcond22 = icmp eq i64 %inc12, %1
  br i1 %exitcond22, label %for.end13.loopexit, label %for.cond2.preheader

for.end13.loopexit:                               ; preds = %for.inc11
  br label %for.end13

for.end13:                                        ; preds = %for.end13.loopexit, %entry
  ret void
}
attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 637) (llvm/branches/loopopt 690)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

