; When HIR Loop Interchange and HIR Loop Blocking do not happen, we will unsink the sinked instruction
;
; Source code:
;
;float a[100];
;float b[100];
;float c[100];
;
;void multiply() {
;  for (int i = 0; i < 100; i++) {
;    c[i] = 0;
;    for (int j = 0; j < 80; j++) {
;      c[i] += a[2 * i + j] * b[j];
;    }
;  }
;}
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-sinking-for-perfect-loopnest -hir-loop-distribute-loopnest -hir-undo-sinking-for-perfect-loopnest -print-after=hir-undo-sinking-for-perfect-loopnest < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-distribute-loopnest,hir-undo-sinking-for-perfect-loopnest,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s

;*** IR Dump After HIR Sinking For Perfect Loopnest ***
;Function: multiply
;
;<0>          BEGIN REGION { }
;<30>               + DO i1 = 0, 99, 1   <DO_LOOP>
;<3>                |   (@c)[0][i1] = 0.000000e+00;
;<31>               |
;<31>               |   + DO i2 = 0, 79, 1   <DO_LOOP>
;<32>               |   |   %1 = (@c)[0][i1];
;<14>               |   |   %mul9 = (@a)[0][2 * i1 + i2]  *  (@b)[0][i2];
;<15>               |   |   %1 = %1  +  %mul9;
;<23>               |   |   (@c)[0][i1] = %1;
;<31>               |   + END LOOP
;<30>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Loop Distribution LoopNest ***
;Function: multiply
;
;<0>          BEGIN REGION { modified }
;<33>               + DO i1 = 0, 99, 1   <DO_LOOP>
;<3>                |   (@c)[0][i1] = 0.000000e+00;
;<33>               + END LOOP
;<33>
;<34>
;<34>               + DO i1 = 0, 99, 1   <DO_LOOP>
;<31>               |   + DO i2 = 0, 79, 1   <DO_LOOP>
;<32>               |   |   %1 = (@c)[0][i1];
;<14>               |   |   %mul9 = (@a)[0][2 * i1 + i2]  *  (@b)[0][i2];
;<15>               |   |   %1 = %1  +  %mul9;
;<23>               |   |   (@c)[0][i1] = %1;
;<31>               |   + END LOOP
;<34>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR UndoSinking For Perfect Loopnest ***
;Function: multiply
;
; CHECK:    BEGIN REGION
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   (@c)[0][i1] = 0.000000e+00;
; CHECK:           + END LOOP
;
;
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |      %1 = 0.000000e+00;
; CHECK:           |   + DO i2 = 0, 79, 1   <DO_LOOP>
; CHECK:           |   |   %mul9 = (@a)[0][2 * i1 + i2]  *  (@b)[0][i2];
; CHECK:           |   |   %1 = %1  +  %mul9;
; CHECK:           |   + END LOOP
; CHECK:           |      (@c)[0][i1] = %1;
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 't1.c'
source_filename = "t1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@a = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @multiply() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.body:                                         ; preds = %for.cond.cleanup3, %entry
  %indvars.iv28 = phi i64 [ 0, %entry ], [ %indvars.iv.next29, %for.cond.cleanup3 ]
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @c, i64 0, i64 %indvars.iv28, !intel-tbaa !2
  store float 0.000000e+00, float* %arrayidx, align 4, !tbaa !2
  %0 = shl nuw nsw i64 %indvars.iv28, 1
  br label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4
  %add12.lcssa = phi float [ %add12, %for.body4 ]
  store float %add12.lcssa, float* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next29 = add nuw nsw i64 %indvars.iv28, 1
  %exitcond31 = icmp eq i64 %indvars.iv.next29, 100
  br i1 %exitcond31, label %for.cond.cleanup, label %for.body

for.body4:                                        ; preds = %for.body4, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body4 ]
  %1 = phi float [ 0.000000e+00, %for.body ], [ %add12, %for.body4 ]
  %2 = add nuw nsw i64 %indvars.iv, %0
  %arrayidx6 = getelementptr inbounds [100 x float], [100 x float]* @a, i64 0, i64 %2, !intel-tbaa !2
  %3 = load float, float* %arrayidx6, align 4, !tbaa !2
  %arrayidx8 = getelementptr inbounds [100 x float], [100 x float]* @b, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %4 = load float, float* %arrayidx8, align 4, !tbaa !2
  %mul9 = fmul float %3, %4
  %add12 = fadd float %1, %mul9
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 80
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
