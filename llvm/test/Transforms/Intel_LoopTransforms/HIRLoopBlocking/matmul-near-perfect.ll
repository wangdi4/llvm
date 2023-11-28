
; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-sinking-for-perfect-loopnest,hir-loop-blocking,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s -disable-output | FileCheck %s --check-prefix=DEFAULT
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-sinking-for-perfect-loopnest,hir-loop-blocking,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s -disable-output | FileCheck %s --check-prefix=NOLIBIRC


; DEFAULT: Function: matmul
;
; DEFAULT:        BEGIN REGION { }
; DEFAULT:              + DO i1 = 0, 4095, 1   <DO_LOOP>
; DEFAULT:              |   + DO i2 = 0, 4095, 1   <DO_LOOP>
; DEFAULT:              |   |   %0 = (@b)[0][i1][i2];
; DEFAULT:              |   |
; DEFAULT:              |   |   + DO i3 = 0, 4095, 1   <DO_LOOP>
; DEFAULT:              |   |   |   %mul = (@a)[0][i2][i3]  *  %0;
; DEFAULT:              |   |   |   %add = (@c)[0][i1][i3]  +  %mul;
; DEFAULT:              |   |   |   (@c)[0][i1][i3] = %add;
; DEFAULT:              |   |   + END LOOP
; DEFAULT:              |   + END LOOP
; DEFAULT:              + END LOOP
; DEFAULT:        END REGION
;
; DEFAULT: Function: matmul
;
; DEFAULT:       BEGIN REGION { modified }
; DEFAULT:               + DO i1 = 0, 63, 1   <DO_LOOP>
; DEFAULT:               |   + DO i2 = 0, 63, 1   <DO_LOOP>
; DEFAULT:               |   |   + DO i3 = 0, 63, 1   <DO_LOOP>
; DEFAULT:               |   |   |   + DO i4 = 0, 63, 1   <DO_LOOP>
; DEFAULT:               |   |   |   |   + DO i5 = 0, 63, 1   <DO_LOOP>
; DEFAULT:               |   |   |   |   |   + DO i6 = 0, 63, 1   <DO_LOOP>
; DEFAULT:               |   |   |   |   |   |   %0 = (@b)[0][64 * i1 + i4][64 * i2 + i5];
; DEFAULT:               |   |   |   |   |   |   %mul = (@a)[0][64 * i2 + i5][64 * i3 + i6]  *  %0;
; DEFAULT:               |   |   |   |   |   |   %add = (@c)[0][64 * i1 + i4][64 * i3 + i6]  +  %mul;
; DEFAULT:               |   |   |   |   |   |   (@c)[0][64 * i1 + i4][64 * i3 + i6] = %add;
; DEFAULT:               |   |   |   |   |   + END LOOP
; DEFAULT:               |   |   |   |   + END LOOP
; DEFAULT:               |   |   |   + END LOOP
; DEFAULT:               |   |   + END LOOP
; DEFAULT:               |   + END LOOP
; DEFAULT:               + END LOOP
; DEFAULT:        END REGION


; Verify that transformation is not triggered without libIRC.
; NOLIBIRC-NOT: modified


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
  %arrayidx14 = getelementptr inbounds [4096 x [4096 x double]], ptr @b, i64 0, i64 %indvars.iv43, i64 %indvars.iv40, !intel-tbaa !2
  %0 = load double, ptr %arrayidx14, align 8, !tbaa !2
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
  %arrayidx10 = getelementptr inbounds [4096 x [4096 x double]], ptr @a, i64 0, i64 %indvars.iv40, i64 %indvars.iv, !intel-tbaa !2
  %1 = load double, ptr %arrayidx10, align 8, !tbaa !2
  %mul = fmul double %1, %0
  %arrayidx18 = getelementptr inbounds [4096 x [4096 x double]], ptr @c, i64 0, i64 %indvars.iv43, i64 %indvars.iv, !intel-tbaa !2
  %2 = load double, ptr %arrayidx18, align 8, !tbaa !2
  %add = fadd double %2, %mul
  store double %add, ptr %arrayidx18, align 8, !tbaa !2
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
