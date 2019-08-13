; Source code:
;
;float a[100][100];
;float b[100][100];
;float c[100][100];
;
;void multiply() {
;  for (int i = 0; i < 100; i++) {
;    for (int j = 0; j < 80; j++) {
;      c[i][j] = 0;
;      for (int k = 0; k < 60; k++) {
;        c[i][j] += a[i][k] * b[k][j];
;      }
;    }
;  }
;}
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-sinking-for-perfect-loopnest -print-after=hir-sinking-for-perfect-loopnest < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Sinking For Perfect Loopnest ***
;
;<0>          BEGIN REGION { }
;<38>               + DO i1 = 0, 99, 1   <DO_LOOP>
;<39>               |   + DO i2 = 0, 79, 1   <DO_LOOP>
;<6>                |   |   (@c)[0][i1][i2] = 0.000000e+00;
;<8>                |   |   %add44 = 0.000000e+00;
;<40>               |   |
;<40>               |   |   + DO i3 = 0, 59, 1   <DO_LOOP>
;<15>               |   |   |   %mul = (@a)[0][i1][i3]  *  (@b)[0][i3][i2];
;<16>               |   |   |   %add44 = %add44  +  %mul;
;<40>               |   |   + END LOOP
;<40>               |   |
;<24>               |   |   (@c)[0][i1][i2] = %add44;
;<39>               |   + END LOOP
;<38>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Sinking For Perfect Loopnest ***
;
; CHECK:    BEGIN REGION { }
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 79, 1   <DO_LOOP>
; CHECK:           |   |   (@c)[0][i1][i2] = 0.000000e+00;
; CHECK:           |   |
; CHECK:           |   |   + DO i3 = 0, 59, 1   <DO_LOOP>
; CHECK:           |   |   |   %add44 = (@c)[0][i1][i2];
; CHECK:           |   |   |   %mul = (@a)[0][i1][i3]  *  (@b)[0][i3][i2];
; CHECK:           |   |   |   %add44 = %add44  +  %mul;
; CHECK:           |   |   |   (@c)[0][i1][i2] = %add44;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION

;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common dso_local local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16
@a = common dso_local local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @multiply() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %indvar = phi i64 [ 0, %entry ], [ %indvar.next, %for.cond.cleanup3 ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond.cleanup3:                                ; preds = %for.cond.cleanup9
  %indvar.next = add nuw nsw i64 %indvar, 1
  %exitcond51 = icmp eq i64 %indvar.next, 100
  br i1 %exitcond51, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.cond.cleanup9, %for.cond1.preheader
  %indvars.iv47 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next48, %for.cond.cleanup9 ]
  %arrayidx6 = getelementptr inbounds [100 x [100 x float]], [100 x [100 x float]]* @c, i64 0, i64 %indvar, i64 %indvars.iv47, !intel-tbaa !2
  store float 0.000000e+00, float* %arrayidx6, align 4, !tbaa !2
  br label %for.body10

for.cond.cleanup9:                                ; preds = %for.body10
  %add.lcssa = phi float [ %add, %for.body10 ]
  store float %add.lcssa, float* %arrayidx6, align 4, !tbaa !2
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %exitcond49 = icmp eq i64 %indvars.iv.next48, 80
  br i1 %exitcond49, label %for.cond.cleanup3, label %for.body4

for.body10:                                       ; preds = %for.body10, %for.body4
  %indvars.iv = phi i64 [ 0, %for.body4 ], [ %indvars.iv.next, %for.body10 ]
  %add44 = phi float [ 0.000000e+00, %for.body4 ], [ %add, %for.body10 ]
  %arrayidx14 = getelementptr inbounds [100 x [100 x float]], [100 x [100 x float]]* @a, i64 0, i64 %indvar, i64 %indvars.iv, !intel-tbaa !2
  %0 = load float, float* %arrayidx14, align 4, !tbaa !2
  %arrayidx18 = getelementptr inbounds [100 x [100 x float]], [100 x [100 x float]]* @b, i64 0, i64 %indvars.iv, i64 %indvars.iv47, !intel-tbaa !2
  %1 = load float, float* %arrayidx18, align 4, !tbaa !2
  %mul = fmul float %0, %1
  %add = fadd float %add44, %mul
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 60
  br i1 %exitcond, label %for.cond.cleanup9, label %for.body10
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA100_A100_f", !4, i64 0}
!4 = !{!"array@_ZTSA100_f", !5, i64 0}
!5 = !{!"float", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
