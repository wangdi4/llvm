;RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-memrec,hir-vec-dir-insert,print<hir>" -aa-pipeline="basic-aa"  < %s 2>&1 | FileCheck %s

; Lexically forward loop-carried dependence 10-13 doesn't prevent vectorization.
; Since the loop is vectorizable, we don't distribute the loop.

;we want to dist on 10-13 to remove loop carried dep
;          BEGIN REGION { }
;<29>         + DO i1 = 0, 99998, 1   <DO_LOOP>
;<28>         |   + DO i2 = 0, 99998, 1   <DO_LOOP>
;<5>          |   |   %0 = (@B)[0][i1 + 1][i2 + 1];
;<7>          |   |   %1 = (@C)[0][i1 + 1][i2 + 1];
;<8>          |   |   %add = %0  +  %1;
;<10>         |   |   (@A)[0][i1 + 1][i2 + 1] = %add;
;<13>         |   |   %3 = (@A)[0][i1 + 1][i2];
;<14>         |   |   %conv18 = %3  *  2.000000e+00;
;<16>         |   |   (@D)[0][i1 + 1][i2 + 1] = %conv18;
;<28>         |   + END LOOP
;<29>         + END LOOP

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, 99998, 1   <DO_LOOP>
; CHECK: |   %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; CHECK: |
; CHECK: |   + DO i2 = 0, 99998, 1   <DO_LOOP>
; CHECK: |   |   %0 = (@B)[0][i1 + 1][i2 + 1];
; CHECK: |   |   %1 = (@C)[0][i1 + 1][i2 + 1];
; CHECK: |   |   %add = %0  +  %1;
; CHECK: |   |   (@A)[0][i1 + 1][i2 + 1] = %add;
; CHECK: |   |   %3 = (@A)[0][i1 + 1][i2];
; CHECK: |   |   %conv18 = %3  *  2.000000e+00;
; CHECK: |   |   (@D)[0][i1 + 1][i2 + 1] = %conv18;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = global [100000 x [100000 x float]] zeroinitializer, align 16
@B = global [100000 x [100000 x float]] zeroinitializer, align 16
@C = global [100000 x [100000 x float]] zeroinitializer, align 16
@D = global [100000 x [100000 x float]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @_Z3foov() {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.23, %entry
  %indvars.iv43 = phi i64 [ 1, %entry ], [ %indvars.iv.next44, %for.inc.23 ]
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3, %for.cond.1.preheader
  %indvars.iv = phi i64 [ 1, %for.cond.1.preheader ], [ %indvars.iv.next, %for.body.3 ]
  %arrayidx5 = getelementptr inbounds [100000 x [100000 x float]], ptr @B, i64 0, i64 %indvars.iv43, i64 %indvars.iv
  %0 = load float, ptr %arrayidx5, align 4
  %arrayidx9 = getelementptr inbounds [100000 x [100000 x float]], ptr @C, i64 0, i64 %indvars.iv43, i64 %indvars.iv
  %1 = load float, ptr %arrayidx9, align 4
  %add = fadd float %0, %1
  %arrayidx13 = getelementptr inbounds [100000 x [100000 x float]], ptr @A, i64 0, i64 %indvars.iv43, i64 %indvars.iv
  store float %add, ptr %arrayidx13, align 4
  %2 = add nsw i64 %indvars.iv, -1
  %arrayidx17 = getelementptr inbounds [100000 x [100000 x float]], ptr @A, i64 0, i64 %indvars.iv43, i64 %2
  %3 = load float, ptr %arrayidx17, align 4
  %conv18 = fmul float %3, 2.000000e+00
  %arrayidx22 = getelementptr inbounds [100000 x [100000 x float]], ptr @D, i64 0, i64 %indvars.iv43, i64 %indvars.iv
  store float %conv18, ptr %arrayidx22, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100000
  br i1 %exitcond, label %for.inc.23, label %for.body.3

for.inc.23:                                       ; preds = %for.body.3
  %indvars.iv.next44 = add nuw nsw i64 %indvars.iv43, 1
  %exitcond45 = icmp eq i64 %indvars.iv.next44, 100000
  br i1 %exitcond45, label %for.end.25, label %for.cond.1.preheader

for.end.25:                                       ; preds = %for.inc.23
  ret void
}



; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, ptr nocapture)

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, ptr nocapture)

