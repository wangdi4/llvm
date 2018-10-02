; RUN: opt -debug-only=hir-loop-interchange -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-interchange < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-interchange" -aa-pipeline="basic-aa" -debug-only=hir-loop-interchange < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; The left-most dimension except leading [0], which is [%q * %q * i2], is ignored because of blobs. Right-most two dimensions, [i1][i2], is aligned with enclosing loopnests, i1-i2. A perfect loopnest is not enabled. 

; <0>       BEGIN REGION { }
; <38>            + DO i1 = 0, 99, 1   <DO_LOOP>
; <39>            |   + DO i2 = 0, 99, 1   <DO_LOOP>
; <9>             |   |   %2 = (@B)[0][(%q * %q) * i2][i1][i2];
; <40>            |   |   
; <40>            |   |   + DO i3 = 0, 99, 1   <DO_LOOP>
; <14>            |   |   |   %3 = (@B)[0][i1][i2][i3];
; <17>            |   |   |   (@A)[0][i2][i3][i3] = %2 + %3;
; <40>            |   |   + END LOOP
; <39>            |   + END LOOP
; <38>            + END LOOP
; <0>       END REGION

; CHECK: MayInterchange: 0

;Module Before HIR; ModuleID = 'may-2.c'
source_filename = "may-2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [100 x [100 x [100 x i8]]] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [100 x [100 x [100 x i8]]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo(i32 %q) local_unnamed_addr #0 {
entry:
  %mul = mul nsw i32 %q, %q
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc30, %entry
  %indvars.iv57 = phi i64 [ 0, %entry ], [ %indvars.iv.next58, %for.inc30 ]
  br label %for.body3

for.body3:                                        ; preds = %for.inc27, %for.cond1.preheader
  %indvars.iv54 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next55, %for.inc27 ]
  %0 = trunc i64 %indvars.iv54 to i32
  %mul4 = mul nsw i32 %mul, %0
  %1 = zext i32 %mul4 to i64
  %arrayidx8 = getelementptr inbounds [100 x [100 x [100 x i8]]], [100 x [100 x [100 x i8]]]* @B, i64 0, i64 %1, i64 %indvars.iv57, i64 %indvars.iv54
  %2 = load i8, i8* %arrayidx8, align 1, !tbaa !2
  br label %for.body12

for.body12:                                       ; preds = %for.body12, %for.body3
  %indvars.iv = phi i64 [ 0, %for.body3 ], [ %indvars.iv.next, %for.body12 ]
  %arrayidx18 = getelementptr inbounds [100 x [100 x [100 x i8]]], [100 x [100 x [100 x i8]]]* @B, i64 0, i64 %indvars.iv57, i64 %indvars.iv54, i64 %indvars.iv
  %3 = load i8, i8* %arrayidx18, align 1, !tbaa !2
  %add = add i8 %3, %2
  %arrayidx26 = getelementptr inbounds [100 x [100 x [100 x i8]]], [100 x [100 x [100 x i8]]]* @A, i64 0, i64 %indvars.iv54, i64 %indvars.iv, i64 %indvars.iv
  store i8 %add, i8* %arrayidx26, align 1, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.inc27, label %for.body12

for.inc27:                                        ; preds = %for.body12
  %indvars.iv.next55 = add nuw nsw i64 %indvars.iv54, 1
  %exitcond56 = icmp eq i64 %indvars.iv.next55, 100
  br i1 %exitcond56, label %for.inc30, label %for.body3

for.inc30:                                        ; preds = %for.inc27
  %indvars.iv.next58 = add nuw nsw i64 %indvars.iv57, 1
  %exitcond59 = icmp eq i64 %indvars.iv.next58, 100
  br i1 %exitcond59, label %for.end32, label %for.cond1.preheader

for.end32:                                        ; preds = %for.inc30
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
