; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-memory-reduction-sinking,print<hir>" 2>&1 < %s | FileCheck %s

; This test case checks that the reduction inside the If condition is sinked,
; even if the access is not linear. This is the same test as
; float-non-linear-reduction.ll, but with the reduction inside a condition.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %0 = (@B)[0][i1];
;       |   if (%0 > 10)
;       |   {
;       |      %add = (@A)[0][%0]  +  %t;
;       |      (@A)[0][%0] = %add;
;       |      %add7 = (@A)[0][5]  +  %t;
;       |      (@A)[0][5] = %add7;
;       |   }
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       %tmp = 0.000000e+00;
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   %0 = (@B)[0][i1];
; CHECK:       |   if (%0 > 10)
; CHECK:       |   {
; CHECK:       |      %add = (@A)[0][%0]  +  %t;
; CHECK:       |      (@A)[0][%0] = %add;
; CHECK:       |      %tmp = %tmp  +  %t;
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK:       if (%tmp !=u 0.000000e+00)
; CHECK:       {
; CHECK:          %add7 = (@A)[0][5]  +  %tmp;
; CHECK:          (@A)[0][5] = %add7;
; CHECK:       }
; CHECK: END REGION


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(float %t) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %cont ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %cmp1 = icmp sgt i32 %0, 10
  br i1 %cmp1, label %if.then, label %cont

if.then:
  %idxprom1 = sext i32 %0 to i64
  %arrayidx2 = getelementptr inbounds [100 x float], ptr @A, i64 0, i64 %idxprom1
  %1 = load float, ptr %arrayidx2, align 4
  %add = fadd fast float %1, %t
  store float %add, ptr %arrayidx2, align 4
  %2 = load float, ptr getelementptr inbounds ([100 x float], ptr @A, i64 0, i64 5), align 4
  %add7 = fadd fast float %2, %t
  store float %add7, ptr getelementptr inbounds ([100 x float], ptr @A, i64 0, i64 5), align 4
  br label %cont

cont:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

