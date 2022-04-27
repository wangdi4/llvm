; RUN: opt -hir-ssa-deconstruction -hir-opt-var-predicate -S -print-after=hir-opt-var-predicate -disable-output  < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate" -print-after=hir-opt-var-predicate -aa-pipeline="basic-aa" -S -disable-output  < %s 2>&1 | FileCheck %s

; Verify that if (trunc.i64.i32(15) <u 15) is not removed as a redundant node because of cast in the left hand side.

; Before -
;        BEGIN REGION { }
;              + DO i1 = 0, 24, 1   <DO_LOOP>
;              |   if (i1 <u 15)
;              |   {
;              |      %0 = (%q)[i1];
;              |      (%p)[i1] = 2 * %0;
;              |   }
;              |   else
;              |   {
;              |      if (i1 == 15)
;              |      {
;              |         %1 = (%p)[15];
;              |         (%p)[15] = %1 + 15;
;              |      }
;              |      else
;              |      {
;              |         %2 = (%q)[i1];
;              |         (%q)[i1] = 2 * %2;
;              |      }
;              |   }
;              + END LOOP
;        END REGION

; After -
; CHECK: Function: foo
;
; CHECK: BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 14, 1   <DO_LOOP>
;               |   if (i1 <u 15)
;               |   {
;               |      %0 = (%q)[i1];
;               |      (%p)[i1] = 2 * %0;
;               |   }
;               |   else
;               |   {
;               |      %2 = (%q)[i1];
;               |      (%q)[i1] = 2 * %2;
;               |   }
;               + END LOOP
;
; CHECK:        if (15 <u 15)
;               {
;                  %0 = (%q)[15];
;                  (%p)[15] = 2 * %0;
;               }
; CHECK:        else
;               {
;                  %1 = (%p)[15];
;                  (%p)[15] = %1 + 15;
;               }
;
; CHECK:        + DO i1 = 0, 8, 1   <DO_LOOP>
;               |   if (i1 + 16 <u 15)
;               |   {
;               |      %0 = (%q)[i1 + 16];
;               |      (%p)[i1 + 16] = 2 * %0;
;               |   }
;               |   else
;               |   {
;               |      %2 = (%q)[i1 + 16];
;               |      (%q)[i1 + 16] = 2 * %2;
;               |   }
;               + END LOOP
;         END REGION

;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @foo(i32* nocapture noundef %p, i32* nocapture noundef %q) local_unnamed_addr #0 {
entry:
  %arrayidx8 = getelementptr inbounds i32, i32* %p, i64 15
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %mytmp = trunc i64 %indvars.iv to i32
  %cmp1 = icmp ult i32 %mytmp, 15
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, i32* %q, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %mul = shl nsw i32 %0, 1
  %arrayidx3 = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  store i32 %mul, i32* %arrayidx3, align 4, !tbaa !3
  br label %for.inc

if.else:                                          ; preds = %for.body
  %cmp5 = icmp eq i64 %indvars.iv, 15
  br i1 %cmp5, label %if.then6, label %if.else11

if.then6:                                         ; preds = %if.else
  %1 = load i32, i32* %arrayidx8, align 4, !tbaa !3
  %add = add nsw i32 %1, 15
  store i32 %add, i32* %arrayidx8, align 4, !tbaa !3
  br label %for.inc

if.else11:                                        ; preds = %if.else
  %arrayidx13 = getelementptr inbounds i32, i32* %q, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx13, align 4, !tbaa !3
  %mul14 = shl nsw i32 %2, 1
  store i32 %mul14, i32* %arrayidx13, align 4, !tbaa !3
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else11, %if.then6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 25
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !7
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
