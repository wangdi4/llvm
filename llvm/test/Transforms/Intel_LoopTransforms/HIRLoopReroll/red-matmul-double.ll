
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" -hir-verify-cf-def-level < %s 2>&1 | FileCheck %s

; CHECK: Function: foo
;
; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:               |   + DO i2 = 0, 999, 1   <DO_LOOP>
; CHECK:               |   |   %0 = (@C)[0][i1][i2];
; CHECK:               |   |
; CHECK:               |   |   + DO i3 = 0, 499, 1   <DO_LOOP>
; CHECK:               |   |   |   %mul = (@A)[0][i1][2 * i3]  *  (@B)[0][2 * i3][i2];
; CHECK:               |   |   |   %add = %0  +  %mul;
; CHECK:               |   |   |   %mul29 = (@A)[0][i1][2 * i3 + 1]  *  (@B)[0][2 * i3 + 1][i2];
; CHECK:               |   |   |   %0 = %add  +  %mul29;
; CHECK:               |   |   + END LOOP
; CHECK:               |   |
; CHECK:               |   |   (@C)[0][i1][i2] = %0;
; CHECK:               |   + END LOOP
; CHECK:               + END LOOP
; CHECK:         END REGION
;
; CHECK: Function: foo
;
; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:               |   + DO i2 = 0, 999, 1   <DO_LOOP>
; CHECK:               |   |   %0 = (@C)[0][i1][i2];
; CHECK:               |   |
; CHECK:               |   |   + DO i3 = 0, 999, 1   <DO_LOOP>
; CHECK:               |   |   |   %mul = (@A)[0][i1][i3]  *  (@B)[0][i3][i2];
; CHECK:               |   |   |   %0 = %0  +  %mul;
; CHECK:               |   |   + END LOOP
; CHECK:               |   |
; CHECK:               |   |   (@C)[0][i1][i2] = %0;
; CHECK:               |   + END LOOP
; CHECK:               + END LOOP
; CHECK:         END REGION

;Module Before HIR; ModuleID = 'matmul-basic.c'
source_filename = "matmul-basic.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [1000 x [1000 x double]] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [1000 x [1000 x double]] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [1000 x [1000 x double]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %indvars.iv65 = phi i64 [ 0, %entry ], [ %indvars.iv.next66, %for.cond.cleanup3 ]
  br label %for.cond5.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond5.preheader:                              ; preds = %for.cond.cleanup7, %for.cond1.preheader
  %indvars.iv63 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next64, %for.cond.cleanup7 ]
  %arrayidx18 = getelementptr inbounds [1000 x [1000 x double]], ptr @C, i64 0, i64 %indvars.iv65, i64 %indvars.iv63, !intel-tbaa !2
  %arrayidx18.promoted = load double, ptr %arrayidx18, align 8, !tbaa !2
  br label %for.body8

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %indvars.iv.next66 = add nuw nsw i64 %indvars.iv65, 1
  %exitcond67 = icmp eq i64 %indvars.iv.next66, 1000
  br i1 %exitcond67, label %for.cond.cleanup, label %for.cond1.preheader

for.cond.cleanup7:                                ; preds = %for.body8
  %add34.lcssa = phi double [ %add34, %for.body8 ]
  store double %add34.lcssa, ptr %arrayidx18, align 8, !tbaa !2
  %indvars.iv.next64 = add nuw nsw i64 %indvars.iv63, 1
  %exitcond = icmp eq i64 %indvars.iv.next64, 1000
  br i1 %exitcond, label %for.cond.cleanup3, label %for.cond5.preheader

for.body8:                                        ; preds = %for.cond5.preheader, %for.body8
  %indvars.iv = phi i64 [ 0, %for.cond5.preheader ], [ %indvars.iv.next, %for.body8 ]
  %0 = phi double [ %arrayidx18.promoted, %for.cond5.preheader ], [ %add34, %for.body8 ]
  %arrayidx10 = getelementptr inbounds [1000 x [1000 x double]], ptr @A, i64 0, i64 %indvars.iv65, i64 %indvars.iv, !intel-tbaa !2
  %1 = load double, ptr %arrayidx10, align 16, !tbaa !2
  %arrayidx14 = getelementptr inbounds [1000 x [1000 x double]], ptr @B, i64 0, i64 %indvars.iv, i64 %indvars.iv63, !intel-tbaa !2
  %2 = load double, ptr %arrayidx14, align 8, !tbaa !2
  %mul = fmul double %1, %2
  %add = fadd double %0, %mul
  %3 = or i64 %indvars.iv, 1
  %arrayidx23 = getelementptr inbounds [1000 x [1000 x double]], ptr @A, i64 0, i64 %indvars.iv65, i64 %3, !intel-tbaa !2
  %4 = load double, ptr %arrayidx23, align 8, !tbaa !2
  %arrayidx28 = getelementptr inbounds [1000 x [1000 x double]], ptr @B, i64 0, i64 %3, i64 %indvars.iv63, !intel-tbaa !2
  %5 = load double, ptr %arrayidx28, align 8, !tbaa !2
  %mul29 = fmul double %4, %5
  %add34 = fadd double %add, %mul29
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp6 = icmp ult i64 %indvars.iv.next, 1000
  br i1 %cmp6, label %for.body8, label %for.cond.cleanup7
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 98f3d88535d4d7e43c9ac44efe92dac55a045e10) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm ef251167d49f359125c109fa9d1fb6706ca2b0d0)"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA1000_A1000_d", !4, i64 0}
!4 = !{!"array@_ZTSA1000_d", !5, i64 0}
!5 = !{!"double", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
