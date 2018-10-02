
; RUN: opt -debug-only=hir-loop-interchange -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-interchange < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-interchange" -aa-pipeline="basic-aa" -debug-only=hir-loop-interchange < %s 2>&1 | FileCheck %s
; REQUIRES: asserts
;
; A perfect loop nest is forced even when all references in innermost loop are all unit strided due to the instruction in line <6>.
; First two left-most dimensions are in shape of [i2][i1]. 
; So interchange might be helpful. After that, it is up to interchange pass's locality calculation logic whther actual interchange will be done or not.

; <0>       BEGIN REGION { }
; <35>            + DO i1 = 0, 99, 1   <DO_LOOP>
; <36>            |   + DO i2 = 0, 99, 1   <DO_LOOP>
; <6>             |   |   %0 = (@B)[0][i2][i1][i2];
; <37>            |   |   
; <37>            |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
; <11>            |   |   |   %1 = (@B)[0][i1][i2][i3];
; <14>            |   |   |   (@A)[0][i2][i3][i3] = %0 + %1;
; <37>            |   |   + END LOOP
; <36>            |   + END LOOP
; <35>            + END LOOP
; <0>       END REGION

; CHECK: MayInterchange: 1

;Module Before HIR; ModuleID = 'may-interchange-not.c'
source_filename = "may-interchange-not.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [100 x [100 x [100 x i8]]] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [100 x [100 x [100 x i8]]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo(i32 %q) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc29, %entry
  %indvars.iv53 = phi i64 [ 0, %entry ], [ %indvars.iv.next54, %for.inc29 ]
  br label %for.body3

for.body3:                                        ; preds = %for.inc26, %for.cond1.preheader
  %indvars.iv50 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next51, %for.inc26 ]
  %arrayidx7 = getelementptr inbounds [100 x [100 x [100 x i8]]], [100 x [100 x [100 x i8]]]* @B, i64 0, i64 %indvars.iv50, i64 %indvars.iv53, i64 %indvars.iv50
  %0 = load i8, i8* %arrayidx7, align 1, !tbaa !2
  br label %for.body11

for.body11:                                       ; preds = %for.body11, %for.body3
  %indvars.iv = phi i64 [ 0, %for.body3 ], [ %indvars.iv.next, %for.body11 ]
  %arrayidx17 = getelementptr inbounds [100 x [100 x [100 x i8]]], [100 x [100 x [100 x i8]]]* @B, i64 0, i64 %indvars.iv53, i64 %indvars.iv50, i64 %indvars.iv
  %1 = load i8, i8* %arrayidx17, align 1, !tbaa !2
  %add = add i8 %1, %0
  %arrayidx25 = getelementptr inbounds [100 x [100 x [100 x i8]]], [100 x [100 x [100 x i8]]]* @A, i64 0, i64 %indvars.iv50, i64 %indvars.iv, i64 %indvars.iv
  store i8 %add, i8* %arrayidx25, align 1, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.inc26, label %for.body11

for.inc26:                                        ; preds = %for.body11
  %indvars.iv.next51 = add nuw nsw i64 %indvars.iv50, 1
  %exitcond52 = icmp eq i64 %indvars.iv.next51, 100
  br i1 %exitcond52, label %for.inc29, label %for.body3

for.inc29:                                        ; preds = %for.inc26
  %indvars.iv.next54 = add nuw nsw i64 %indvars.iv53, 1
  %exitcond55 = icmp eq i64 %indvars.iv.next54, 100
  br i1 %exitcond55, label %for.end31, label %for.cond1.preheader

for.end31:                                        ; preds = %for.inc29
  ret i32 undef
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang fc25755a7dd8cc64339b342d0cba7a81391fbd6e)"}
!2 = !{!3, !6, i64 0}
!3 = !{!"array@_ZTSA100_A100_A100_c", !4, i64 0}
!4 = !{!"array@_ZTSA100_A100_c", !5, i64 0}
!5 = !{!"array@_ZTSA100_c", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
