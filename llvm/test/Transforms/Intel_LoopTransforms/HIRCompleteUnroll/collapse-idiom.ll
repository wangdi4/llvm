; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that we identify the collapse idiom for this loopnest and skip
; unrolling the inner two loops. Note that only the inner two loops were
; considered for unrolling because the total trip count of all three loops
; exceeds the threshold.

; BaseCost of memref = 2
; Extra cost of memref which can be collapsed = 4
; Total cost of memref which can be collapsed = 2 + 4 = 6
; Number of memrefs in loop = 3
; Total trip count of loop = 20 * 3 = 60
; Total GEPCost = 6 * 3 * 60 = 1080
 
; CHECK: GEPCost: 1080

; CHECK: + DO i1 = 0, 49, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 19, 1   <DO_LOOP>
; CHECK: |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
; CHECK: |   |   |   %mul = (@B)[0][i1][i2][i3]  *  (@A)[0][i1][i2][i3];
; CHECK: |   |   |   (%Tmp)[0][i1][i2][i3] = %mul;
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [50 x [20 x [3 x float]]] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [50 x [20 x [3 x float]]] zeroinitializer, align 16

; Function Attrs: nofree nosync nounwind readonly uwtable
define dso_local float @foo() local_unnamed_addr #0 {
entry:
  %Tmp = alloca [50 x [20 x [3 x float]]], align 16
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc26
  %indvars.iv53 = phi i64 [ 0, %entry ], [ %indvars.iv.next54, %for.inc26 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond1.preheader, %for.inc23
  %indvars.iv50 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next51, %for.inc23 ]
  br label %for.body6

for.body6:                                        ; preds = %for.cond4.preheader, %for.body6
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx10 = getelementptr inbounds [50 x [20 x [3 x float]]], ptr @A, i64 0, i64 %indvars.iv53, i64 %indvars.iv50, i64 %indvars.iv
  %t1 = load float, ptr %arrayidx10, align 4
  %arrayidx16 = getelementptr inbounds [50 x [20 x [3 x float]]], ptr @B, i64 0, i64 %indvars.iv53, i64 %indvars.iv50, i64 %indvars.iv
  %t2 = load float, ptr %arrayidx16, align 4
  %mul = fmul fast float %t2, %t1
  %arrayidx22 = getelementptr inbounds [50 x [20 x [3 x float]]], ptr %Tmp, i64 0, i64 %indvars.iv53, i64 %indvars.iv50, i64 %indvars.iv
  store float %mul, ptr %arrayidx22, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 3
  br i1 %exitcond.not, label %for.inc23, label %for.body6

for.inc23:                                        ; preds = %for.body6
  %indvars.iv.next51 = add nuw nsw i64 %indvars.iv50, 1
  %exitcond52.not = icmp eq i64 %indvars.iv.next51, 20
  br i1 %exitcond52.not, label %for.inc26, label %for.cond4.preheader

for.inc26:                                        ; preds = %for.inc23
  %indvars.iv.next54 = add nuw nsw i64 %indvars.iv53, 1
  %exitcond55.not = icmp eq i64 %indvars.iv.next54, 50
  br i1 %exitcond55.not, label %for.end28, label %for.cond1.preheader

for.end28:                                        ; preds = %for.inc26
  %arrayidx31 = getelementptr inbounds [50 x [20 x [3 x float]]], ptr %Tmp, i64 0, i64 2, i64 4, i64 1
  %t3 = load float, ptr %arrayidx31, align 4
  ret float %t3
}

