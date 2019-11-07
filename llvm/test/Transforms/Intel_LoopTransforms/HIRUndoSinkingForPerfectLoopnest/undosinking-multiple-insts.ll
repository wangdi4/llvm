; Check the insts are undo sinked in correct order
; Source code:
;
;float a[100];
;float b[100];
;float c[100];
;void multiply() {
;  for (int i = 0; i < 100; i++) {
;    for (int j = 0; j < 80; j++) {
;      a[i] += b[i + j];
;      c[i] += b[i + 2 * j];
;    }
;  }
;}
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-sinking-for-perfect-loopnest -hir-undo-sinking-for-perfect-loopnest -print-after=hir-sinking-for-perfect-loopnest -print-after=hir-undo-sinking-for-perfect-loopnest < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,print<hir>,hir-undo-sinking-for-perfect-loopnest,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Sinking For Perfect Loopnest ***
;
;<0>          BEGIN REGION { }
;<35>               + DO i1 = 0, 99, 1   <DO_LOOP>
;<5>                |   %add1329 = (@c)[0][i1];
;<4>                |   %add728 = (@a)[0][i1];
;<36>               |
;<36>               |   + DO i2 = 0, 79, 1   <DO_LOOP>
;<14>               |   |   %add728 = (@b)[0][i1 + i2]  +  %add728;
;<19>               |   |   %add1329 = (@b)[0][i1 + 2 * i2]  +  %add1329;
;<36>               |   + END LOOP
;<36>               |
;<27>               |   (@a)[0][i1] = %add728;
;<28>               |   (@c)[0][i1] = %add1329;
;<35>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Sinking For Perfect Loopnest ***
;
; CHECK:      BEGIN REGION { }
; CHECK:            + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:            |   + DO i2 = 0, 79, 1   <DO_LOOP>
; CHECK:            |   |   %add1329 = (@c)[0][i1];
; CHECK:            |   |   %add728 = (@a)[0][i1];
; CHECK:            |   |   %add728 = (@b)[0][i1 + i2]  +  %add728;
; CHECK:            |   |   %add1329 = (@b)[0][i1 + 2 * i2]  +  %add1329;
; CHECK:            |   |   (@a)[0][i1] = %add728;
; CHECK:            |   |   (@c)[0][i1] = %add1329;
; CHECK:            |   + END LOOP
; CHECK:            + END LOOP
; CHECK:      END REGION
;
;*** IR Dump After HIR Undo Sinking For Perfect Loopnest ***
;
; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |      %add1329 = (@c)[0][i1];
; CHECK:           |      %add728 = (@a)[0][i1];
; CHECK:           |   + DO i2 = 0, 79, 1   <DO_LOOP>
; CHECK:           |   |   %add728 = (@b)[0][i1 + i2]  +  %add728;
; CHECK:           |   |   %add1329 = (@b)[0][i1 + 2 * i2]  +  %add1329;
; CHECK:           |   + END LOOP
; CHECK:           |      (@a)[0][i1] = %add728;
; CHECK:           |      (@c)[0][i1] = %add1329;
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@a = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@c = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @multiply() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %indvars.iv34 = phi i64 [ 0, %entry ], [ %indvars.iv.next35, %for.cond.cleanup3 ]
  %arrayidx6 = getelementptr inbounds [100 x float], [100 x float]* @a, i64 0, i64 %indvars.iv34, !intel-tbaa !2
  %arrayidx12 = getelementptr inbounds [100 x float], [100 x float]* @c, i64 0, i64 %indvars.iv34, !intel-tbaa !2
  %arrayidx6.promoted = load float, float* %arrayidx6, align 4, !tbaa !2
  %arrayidx12.promoted = load float, float* %arrayidx12, align 4, !tbaa !2
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond.cleanup3:                                ; preds = %for.body4
  %add7.lcssa = phi float [ %add7, %for.body4 ]
  %add13.lcssa = phi float [ %add13, %for.body4 ]
  store float %add7.lcssa, float* %arrayidx6, align 4, !tbaa !2
  store float %add13.lcssa, float* %arrayidx12, align 4, !tbaa !2
  %indvars.iv.next35 = add nuw nsw i64 %indvars.iv34, 1
  %exitcond36 = icmp eq i64 %indvars.iv.next35, 100
  br i1 %exitcond36, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %add1329 = phi float [ %arrayidx12.promoted, %for.cond1.preheader ], [ %add13, %for.body4 ]
  %add728 = phi float [ %arrayidx6.promoted, %for.cond1.preheader ], [ %add7, %for.body4 ]
  %0 = add nuw nsw i64 %indvars.iv, %indvars.iv34
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @b, i64 0, i64 %0, !intel-tbaa !2
  %1 = load float, float* %arrayidx, align 4, !tbaa !2
  %add7 = fadd float %1, %add728
  %2 = shl nuw nsw i64 %indvars.iv, 1
  %3 = add nuw nsw i64 %2, %indvars.iv34
  %arrayidx10 = getelementptr inbounds [100 x float], [100 x float]* @b, i64 0, i64 %3, !intel-tbaa !2
  %4 = load float, float* %arrayidx10, align 4, !tbaa !2
  %add13 = fadd float %4, %add1329
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 80
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
