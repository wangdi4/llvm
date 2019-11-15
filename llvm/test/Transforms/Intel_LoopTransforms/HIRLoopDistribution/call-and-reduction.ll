; RUN: opt -xmain-opt-level=3 -hir-loop-distribute-scex-cost=2 -print-after=hir-loop-distribute-memrec -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-distribute-memrec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -xmain-opt-level=3 -hir-loop-distribute-scex-cost=2 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that the loop with a user call may be distributed if another chunk contains no mem refs.

; BEGIN REGION { }
;       + DO i1 = 0, 63, 1   <DO_LOOP>
;       |   %call = @bar(i1);
;       |   %add = %call  +  (@A)[0][i1];
;       |   %add3 = %add  +  (@B)[0][i1];
;       |   %t2.014 = %t2.014  +  %add3;
;       + END LOOP
; END REGION
;
; CHECK: BEGIN REGION { modified }
; CHECK:      + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK:      |   %call = @bar(i1);
; CHECK:      |   %add = %call  +  (@A)[0][i1];
; CHECK:      |   %add3 = %add  +  (@B)[0][i1];
; CHECK:      |   (%.TempArray)[0][i1] = %add3;
; CHECK:      + END LOOP

; CHECK:      + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK:      |   %add3 = (%.TempArray)[0][i1];
; CHECK:      |   %t2.014 = %t2.014  +  %add3;
; CHECK:      + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@D = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %add4.lcssa = phi float [ %add4, %for.body ]
  store float %add4.lcssa, float* getelementptr inbounds ([100 x float], [100 x float]* @A, i64 0, i64 0), align 16
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %t2.014 = phi float [ 0.000000e+00, %entry ], [ %add4, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  %call = tail call float @bar(i32 %0) #2
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @A, i64 0, i64 %indvars.iv
  %1 = load float, float* %arrayidx, align 4
  %add = fadd float %call, %1
  %arrayidx2 = getelementptr inbounds [100 x float], [100 x float]* @B, i64 0, i64 %indvars.iv
  %2 = load float, float* %arrayidx2, align 4
  %add3 = fadd float %add, %2
  %add4 = fadd fast float %t2.014, %add3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

declare dso_local float @bar(i32) local_unnamed_addr #1

