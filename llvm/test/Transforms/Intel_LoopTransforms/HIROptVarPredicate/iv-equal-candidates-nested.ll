; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate" --print-after=hir-opt-var-predicate -aa-pipeline="basic-aa" -S -disable-output  < %s 2>&1 | FileCheck %s

; Verify the compiler doesn't crash. The outer-if, whose condition
; is the same as the inner-ifs, is not selected as a candidate
; for HIROptVarPredicate.

;          BEGIN REGION { }
;                + DO i1 = 0, 99, 1   <DO_LOOP>
;                |   if (i1 == 20)
;                |   {
;                |      if (i1 == 20)
;                |      {
;                |         (%p)[20] = 21;
;                |         %3 = (%a)[20];
;                |         (%a)[20] = %3 + 1;
;                |         %4 = (%q)[20];
;                |         (%q)[20] = %4 + 1;
;                |      }
;                |      else
;                |      {
;                |         (%p)[21] = %n + -1;
;                |         %5 = (%a)[20];
;                |         (%a)[20] = %5 + 1;
;                |         %6 = (%q)[20];
;                |         (%q)[21] = %6 + -1;
;                |      }
;                |   }
;                |   else
;                |   {
;                |      if (i1 == 20)
;                |      {
;                |         (%p)[20] = 21;
;                |         %7 = (%a)[20];
;                |         (%a)[20] = %7 + 5;
;                |         %8 = (%q)[20];
;                |         (%q)[20] = %8 + 3;
;                |      }
;                |      else
;                |      {
;                |         (%p)[i1 + 1] = i1 + -3;
;                |         %12 = (%a)[i1];
;                |         (%a)[i1] = %12 + 5;
;                |         %13 = (%q)[i1];
;                |         (%q)[i1 + 1] = %13 + -5;
;                |      }
;                |   }
;                + END LOOP
;          END REGION

; Transformed into -->
;
;          BEGIN REGION { modified }
;                + DO i1 = 0, 19, 1   <DO_LOOP>
;                |   if (i1 == 20)
;                |   {
;                |      (%p)[21] = %n + -1;
;                |      %5 = (%a)[20];
;                |      (%a)[20] = %5 + 1;
;                |      %6 = (%q)[20];
;                |      (%q)[21] = %6 + -1;
;                |   }
;                |   else
;                |   {
;                |      (%p)[i1 + 1] = i1 + -3;
;                |      %12 = (%a)[i1];
;                |      (%a)[i1] = %12 + 5;
;                |      %13 = (%q)[i1];
;                |      (%q)[i1 + 1] = %13 + -5;
;                |   }
;                + END LOOP
;
;                (%p)[20] = 21;
;                %3 = (%a)[20];
;                (%a)[20] = %3 + 1;
;                %4 = (%q)[20];
;                (%q)[20] = %4 + 1;
;
;                + DO i1 = 0, 78, 1   <DO_LOOP>
;                |   if (i1 + 21 == 20)
;                |   {
;                |      (%p)[21] = %n + -1;
;                |      %5 = (%a)[20];
;                |      (%a)[20] = %5 + 1;
;                |      %6 = (%q)[20];
;                |      (%q)[21] = %6 + -1;
;                |   }
;                |   else
;                |   {
;                |      (%p)[i1 + 22] = i1 + 18;
;                |      %12 = (%a)[i1 + 21];
;                |      (%a)[i1 + 21] = %12 + 5;
;                |      %13 = (%q)[i1 + 21];
;                |      (%q)[i1 + 22] = %13 + -5;
;                |   }
;                + END LOOP
;          END REGION

;CHECK:          BEGIN REGION { modified }
;CHECK:                + DO i1 = 0, 19, 1   <DO_LOOP>
;CHECK:                |   if (i1 == 20)
;CHECK:                |   {
;CHECK:                |      (%p)[21] =
;CHECK:                |      (%q)[21] =
;CHECK:                |   }
;CHECK:                |   else
;CHECK:                |   {
;CHECK:                |      (%p)[i1 + 1] =
;CHECK:                |      (%q)[i1 + 1] =
;CHECK:                |   }
;CHECK:                + END LOOP
;
;CHECK:                (%p)[20] =
;CHECK:                (%q)[20] =
;
;CHECK:                + DO i1 = 0, 78, 1   <DO_LOOP>
;CHECK:                |   if (i1 + 21 == 20)
;CHECK:                |   {
;CHECK:                |      (%p)[21] =
;CHECK:                |      (%q)[21] =
;CHECK:                |   }
;CHECK:                |   else
;CHECK:                |   {
;CHECK:                |      (%p)[i1 + 22] =
;CHECK:                |      (%q)[i1 + 22] =
;CHECK:                |   }
;CHECK:                + END LOOP
;CHECK:          END REGION


;Module Before HIR
; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree norecurse nosync nounwind uwtable
define dso_local void @test(i32* nocapture noundef writeonly %p, i32* nocapture noundef %q, i32* nocapture noundef %a, i32 noundef %n) local_unnamed_addr #0 {
entry:
  %arrayidx28 = getelementptr inbounds i32, i32* %p, i64 20
  %0 = zext i32 20 to i64
  %arrayidx3697 = getelementptr inbounds i32, i32* %a, i64 20
  %arrayidx43 = getelementptr inbounds i32, i32* %q, i64 20
  %1 = add i32 %n, -1
  %2 = add nuw nsw i64 %0, 1
  %arrayidx4 = getelementptr inbounds i32, i32* %p, i64 %2
  %arrayidx6 = getelementptr inbounds i32, i32* %a, i64 %0
  %arrayidx19 = getelementptr inbounds i32, i32* %q, i64 %0
  %arrayidx23 = getelementptr inbounds i32, i32* %q, i64 %2
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %cmp1 = icmp eq i64 %indvars.iv, %0
  %cmp26 = icmp eq i64 %indvars.iv, 20
  br i1 %cmp1, label %if.then, label %if.else25

if.then:                                          ; preds = %for.body
  br i1 %cmp26, label %if.then11, label %if.else17

if.then11:                                        ; preds = %if.then
  store i32 21, i32* %arrayidx28, align 4, !tbaa !3
  %3 = load i32, i32* %arrayidx6, align 4, !tbaa !3
  %add794 = add nsw i32 %3, 1
  store i32 %add794, i32* %arrayidx6, align 4, !tbaa !3
  %4 = load i32, i32* %arrayidx19, align 4, !tbaa !3
  %add14 = add nsw i32 %4, 1
  store i32 %add14, i32* %arrayidx19, align 4, !tbaa !3
  br label %for.inc

if.else17:                                        ; preds = %if.then
  store i32 %1, i32* %arrayidx4, align 4, !tbaa !3
  %5 = load i32, i32* %arrayidx6, align 4, !tbaa !3
  %add7 = add nsw i32 %5, 1
  store i32 %add7, i32* %arrayidx6, align 4, !tbaa !3
  %6 = load i32, i32* %arrayidx19, align 4, !tbaa !3
  %sub20 = add nsw i32 %6, -1
  store i32 %sub20, i32* %arrayidx23, align 4, !tbaa !3
  br label %for.inc

if.else25:                                        ; preds = %for.body
  br i1 %cmp26, label %if.then41, label %if.else47

if.then41:                                        ; preds = %if.else25
  store i32 21, i32* %arrayidx28, align 4, !tbaa !3
  %7 = load i32, i32* %arrayidx3697, align 4, !tbaa !3
  %add3798 = add nsw i32 %7, 5
  store i32 %add3798, i32* %arrayidx3697, align 4, !tbaa !3
  %8 = load i32, i32* %arrayidx43, align 4, !tbaa !3
  %add44 = add nsw i32 %8, 3
  store i32 %add44, i32* %arrayidx43, align 4, !tbaa !3
  br label %for.inc

if.else47:                                        ; preds = %if.else25
  %9 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx33 = getelementptr inbounds i32, i32* %p, i64 %9
  %10 = trunc i64 %indvars.iv to i32
  %11 = add i32 %10, -3
  store i32 %11, i32* %arrayidx33, align 4, !tbaa !3
  %arrayidx36 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %12 = load i32, i32* %arrayidx36, align 4, !tbaa !3
  %add37 = add nsw i32 %12, 5
  store i32 %add37, i32* %arrayidx36, align 4, !tbaa !3
  %arrayidx49 = getelementptr inbounds i32, i32* %q, i64 %indvars.iv
  %13 = load i32, i32* %arrayidx49, align 4, !tbaa !3
  %sub50 = add nsw i32 %13, -5
  %arrayidx53 = getelementptr inbounds i32, i32* %q, i64 %9
  store i32 %sub50, i32* %arrayidx53, align 4, !tbaa !3
  br label %for.inc

for.inc:                                          ; preds = %if.else17, %if.then11, %if.else47, %if.then41
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !7
}

attributes #0 = { argmemonly nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
