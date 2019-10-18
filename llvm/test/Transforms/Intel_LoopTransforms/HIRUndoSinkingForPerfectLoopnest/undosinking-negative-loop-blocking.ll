; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-sinking-for-perfect-loopnest -hir-loop-blocking -hir-undo-sinking-for-perfect-loopnest -print-after=hir-undo-sinking-for-perfect-loopnest < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-blocking,hir-undo-sinking-for-perfect-loopnest,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
;
;*** IR Dump Before HIR Sinking For Perfect Loopnest ***
;Function: matmul
;
;<0>          BEGIN REGION { }
;<37>               + DO i1 = 0, 4095, 1   <DO_LOOP>
;<38>               |   + DO i2 = 0, 4095, 1   <DO_LOOP>
;<6>                |   |   %0 = (@b)[0][i1][i2];
;<39>               |   |
;<39>               |   |   + DO i3 = 0, 4095, 1   <DO_LOOP>
;<12>               |   |   |   %mul = (@a)[0][i2][i3]  *  %0;
;<15>               |   |   |   %add = (@c)[0][i1][i3]  +  %mul;
;<16>               |   |   |   (@c)[0][i1][i3] = %add;
;<39>               |   |   + END LOOP
;<38>               |   + END LOOP
;<37>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Sinking For Perfect Loopnest ***
;Function: matmul
;
;<0>          BEGIN REGION { }
;<37>               + DO i1 = 0, 4095, 1   <DO_LOOP>
;<38>               |   + DO i2 = 0, 4095, 1   <DO_LOOP>
;<39>               |   |   + DO i3 = 0, 4095, 1   <DO_LOOP>
;<6>                |   |   |   %0 = (@b)[0][i1][i2];
;<12>               |   |   |   %mul = (@a)[0][i2][i3]  *  %0;
;<15>               |   |   |   %add = (@c)[0][i1][i3]  +  %mul;
;<16>               |   |   |   (@c)[0][i1][i3] = %add;
;<39>               |   |   + END LOOP
;<38>               |   + END LOOP
;<37>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Loop Blocking ***
;Function: matmul
;
;<0>          BEGIN REGION { modified }
;<40>               + DO i1 = 0, 255, 1   <DO_LOOP>
;<37>               |   + DO i2 = 0, 255, 1   <DO_LOOP>
;<41>               |   |   + DO i3 = 0, 255, 1   <DO_LOOP>
;<38>               |   |   |   + DO i4 = 0, 15, 1   <DO_LOOP>
;<42>               |   |   |   |   + DO i5 = 0, 15, 1   <DO_LOOP>
;<39>               |   |   |   |   |   + DO i6 = 0, 15, 1   <DO_LOOP>
;<6>                |   |   |   |   |   |   %0 = (@b)[0][16 * i1 + i4][16 * i2 + i5];
;<12>               |   |   |   |   |   |   %mul = (@a)[0][16 * i2 + i5][16 * i3 + i6]  *  %0;
;<15>               |   |   |   |   |   |   %add = (@c)[0][16 * i1 + i4][16 * i3 + i6]  +  %mul;
;<16>               |   |   |   |   |   |   (@c)[0][16 * i1 + i4][16 * i3 + i6] = %add;
;<39>               |   |   |   |   |   + END LOOP
;<42>               |   |   |   |   + END LOOP
;<38>               |   |   |   + END LOOP
;<41>               |   |   + END LOOP
;<37>               |   + END LOOP
;<40>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Undo Sinking For Perfect Loopnest ***
;Function: matmul
;
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 255, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 255, 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, 255, 1   <DO_LOOP>
; CHECK:           |   |   |   + DO i4 = 0, 15, 1   <DO_LOOP>
; CHECK:           |   |   |   |   + DO i5 = 0, 15, 1   <DO_LOOP>
; CHECK:           |   |   |   |   |   + DO i6 = 0, 15, 1   <DO_LOOP>
; CHECK:           |   |   |   |   |   |   %0 = (@b)[0][16 * i1 + i4][16 * i2 + i5];
; CHECK:           |   |   |   |   |   |   %mul = (@a)[0][16 * i2 + i5][16 * i3 + i6]  *  %0;
; CHECK:           |   |   |   |   |   |   %add = (@c)[0][16 * i1 + i4][16 * i3 + i6]  +  %mul;
; CHECK:           |   |   |   |   |   |   (@c)[0][16 * i1 + i4][16 * i3 + i6] = %add;
; CHECK:           |   |   |   |   |   + END LOOP
; CHECK:           |   |   |   |   + END LOOP
; CHECK:           |   |   |   + END LOOP
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 'matmul-near-perfect.c'
source_filename = "matmul-near-perfect.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [4096 x [4096 x double]] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [4096 x [4096 x double]] zeroinitializer, align 16
@c = common dso_local local_unnamed_addr global [4096 x [4096 x double]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @matmul() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %indvars.iv43 = phi i64 [ 0, %entry ], [ %indvars.iv.next44, %for.cond.cleanup3 ]
  br label %for.cond5.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond5.preheader:                              ; preds = %for.cond.cleanup7, %for.cond1.preheader
  %indvars.iv40 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next41, %for.cond.cleanup7 ]
  %arrayidx14 = getelementptr inbounds [4096 x [4096 x double]], [4096 x [4096 x double]]* @b, i64 0, i64 %indvars.iv43, i64 %indvars.iv40, !intel-tbaa !2
  %0 = load double, double* %arrayidx14, align 8, !tbaa !2
  br label %for.body8

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %indvars.iv.next44 = add nuw nsw i64 %indvars.iv43, 1
  %exitcond45 = icmp eq i64 %indvars.iv.next44, 4096
  br i1 %exitcond45, label %for.cond.cleanup, label %for.cond1.preheader

for.cond.cleanup7:                                ; preds = %for.body8
  %indvars.iv.next41 = add nuw nsw i64 %indvars.iv40, 1
  %exitcond42 = icmp eq i64 %indvars.iv.next41, 4096
  br i1 %exitcond42, label %for.cond.cleanup3, label %for.cond5.preheader

for.body8:                                        ; preds = %for.body8, %for.cond5.preheader
  %indvars.iv = phi i64 [ 0, %for.cond5.preheader ], [ %indvars.iv.next, %for.body8 ]
  %arrayidx10 = getelementptr inbounds [4096 x [4096 x double]], [4096 x [4096 x double]]* @a, i64 0, i64 %indvars.iv40, i64 %indvars.iv, !intel-tbaa !2
  %1 = load double, double* %arrayidx10, align 8, !tbaa !2
  %mul = fmul double %1, %0
  %arrayidx18 = getelementptr inbounds [4096 x [4096 x double]], [4096 x [4096 x double]]* @c, i64 0, i64 %indvars.iv43, i64 %indvars.iv, !intel-tbaa !2
  %2 = load double, double* %arrayidx18, align 8, !tbaa !2
  %add = fadd double %2, %mul
  store double %add, double* %arrayidx18, align 8, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4096
  br i1 %exitcond, label %for.cond.cleanup7, label %for.body8
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang f8a7e30bf3a7ffc2178242a3749327dc2213cb68) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 9fdcc685c1558e3c5d143dd281ae5dc5a1f9b948)"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA4096_A4096_d", !4, i64 0}
!4 = !{!"array@_ZTSA4096_d", !5, i64 0}
!5 = !{!"double", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
