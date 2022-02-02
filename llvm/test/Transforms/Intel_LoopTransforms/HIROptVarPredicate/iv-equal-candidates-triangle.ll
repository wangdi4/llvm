; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-opt-var-predicate -disable-output -print-before=hir-opt-var-predicate -print-after=hir-opt-var-predicate %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-var-predicate" -disable-output -print-before=hir-opt-var-predicate -print-after=hir-opt-var-predicate %s 2>&1 | FileCheck %s

; Check that both equal candidates (i2 == i1 + %d) will be handled at once.

; CHECK: Function: foo
;
; CHECK:          BEGIN REGION { }
; CHECK:                + DO i1 = 0, %m + -1, 1   <DO_LOOP>
; CHECK:                |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:                |   |   if (i2 == i1 + %d)
; CHECK:                |   |   {
; CHECK:                |   |      (%p)[20] = 21;
; CHECK:                |   |   }
; CHECK:                |   |   else
; CHECK:                |   |   {
; CHECK:                |   |      (%p)[i2 + 1] = i2 + -1;
; CHECK:                |   |   }
; CHECK:                |   |   %1 = (%a)[i1];
; CHECK:                |   |   (%a)[i1] = %1 + 1;
; CHECK:                |   |   if (i2 == i1 + %d)
; CHECK:                |   |   {
; CHECK:                |   |      %2 = (%q)[i1 + %d];
; CHECK:                |   |      (%q)[i1 + %d] = %2 + 1;
; CHECK:                |   |   }
; CHECK:                |   |   else
; CHECK:                |   |   {
; CHECK:                |   |      %3 = (%q)[i2];
; CHECK:                |   |      (%q)[i2 + 1] = %3 + -1;
; CHECK:                |   |   }
; CHECK:                |   + END LOOP
; CHECK:                + END LOOP
; CHECK:          END REGION
;
; CHECK: Function: foo
;
; CHECK:          BEGIN REGION { modified }
; CHECK:                + DO i1 = 0, %m + -1, 1   <DO_LOOP>
; CHECK:                |   %ivcopy = i1 + %d;
; CHECK:                |   if (%n > 0)
; CHECK:                |   {
; CHECK:                |      + DO i2 = 0, smin((-1 + %n), (-1 + %ivcopy)), 1   <DO_LOOP>
; CHECK:                |      |   (%p)[i2 + 1] = i2 + -1;
; CHECK:                |      |   %1 = (%a)[i1];
; CHECK:                |      |   (%a)[i1] = %1 + 1;
; CHECK:                |      |   %3 = (%q)[i2];
; CHECK:                |      |   (%q)[i2 + 1] = %3 + -1;
; CHECK:                |      + END LOOP
; CHECK:                |
; CHECK:                |      if (smax(0, %ivcopy) < smin((-1 + %n), %ivcopy) + 1)
; CHECK:                |      {
; CHECK:                |         (%p)[20] = 21;
; CHECK:                |         %1 = (%a)[i1];
; CHECK:                |         (%a)[i1] = %1 + 1;
; CHECK:                |         %2 = (%q)[i1 + %d];
; CHECK:                |         (%q)[i1 + %d] = %2 + 1;
; CHECK:                |      }
; CHECK:                |
; CHECK:                |      + DO i2 = 0, %n + -1 * smax(0, (1 + %ivcopy)) + -1, 1   <DO_LOOP>
; CHECK:                |      |   (%p)[i2 + smax(0, (1 + %ivcopy)) + 1] = i2 + trunc.i64.i32(smax(0, (1 + %ivcopy))) + -1;
; CHECK:                |      |   %1 = (%a)[i1];
; CHECK:                |      |   (%a)[i1] = %1 + 1;
; CHECK:                |      |   %3 = (%q)[i2 + smax(0, (1 + %ivcopy))];
; CHECK:                |      |   (%q)[i2 + smax(0, (1 + %ivcopy)) + 1] = %3 + -1;
; CHECK:                |      + END LOOP
; CHECK:                |   }
; CHECK:                + END LOOP
; CHECK:          END REGION

;Module Before HIR
; ModuleID = 'iv.c'
source_filename = "iv.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @foo(i32* nocapture writeonly %p, i32* nocapture %q, i32* nocapture %a, i64 %n, i64 %m, i64 %d) local_unnamed_addr #0 {
entry:
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 20
  %cmp248 = icmp sgt i64 %n, 0
  %cmp52 = icmp sgt i64 %m, 0
  br i1 %cmp52, label %for.cond1.preheader.preheader, label %for.end25

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.inc23
  %i.053 = phi i64 [ %inc24, %for.inc23 ], [ 0, %for.cond1.preheader.preheader ]
  %add = add nsw i64 %i.053, %d
  %arrayidx7 = getelementptr inbounds i32, i32* %a, i64 %i.053
  br i1 %cmp248, label %for.body3.preheader, label %for.inc23

for.body3.preheader:                              ; preds = %for.cond1.preheader
  %arrayidx14 = getelementptr inbounds i32, i32* %q, i64 %add
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.inc
  %j.049 = phi i64 [ %inc.pre-phi, %for.inc ], [ 0, %for.body3.preheader ]
  %cmp4 = icmp eq i64 %j.049, %add
  br i1 %cmp4, label %if.then, label %if.else

if.then:                                          ; preds = %for.body3
  store i32 21, i32* %arrayidx, align 4, !tbaa !3
  br label %if.end

if.else:                                          ; preds = %for.body3
  %0 = trunc i64 %j.049 to i32
  %conv = add i32 %0, -1
  %add5 = add nuw nsw i64 %j.049, 1
  %arrayidx6 = getelementptr inbounds i32, i32* %p, i64 %add5
  store i32 %conv, i32* %arrayidx6, align 4, !tbaa !3
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %1 = load i32, i32* %arrayidx7, align 4, !tbaa !3
  %add8 = add nsw i32 %1, 1
  store i32 %add8, i32* %arrayidx7, align 4, !tbaa !3
  br i1 %cmp4, label %if.then13, label %if.else17

if.then13:                                        ; preds = %if.end
  %2 = load i32, i32* %arrayidx14, align 4, !tbaa !3
  %add15 = add nsw i32 %2, 1
  store i32 %add15, i32* %arrayidx14, align 4, !tbaa !3
  %.pre = add nuw nsw i64 %j.049, 1
  br label %for.inc

if.else17:                                        ; preds = %if.end
  %arrayidx18 = getelementptr inbounds i32, i32* %q, i64 %j.049
  %3 = load i32, i32* %arrayidx18, align 4, !tbaa !3
  %sub19 = add nsw i32 %3, -1
  %add20 = add nuw nsw i64 %j.049, 1
  %arrayidx21 = getelementptr inbounds i32, i32* %q, i64 %add20
  store i32 %sub19, i32* %arrayidx21, align 4, !tbaa !3
  br label %for.inc

for.inc:                                          ; preds = %if.then13, %if.else17
  %inc.pre-phi = phi i64 [ %.pre, %if.then13 ], [ %add20, %if.else17 ]
  %exitcond.not = icmp eq i64 %inc.pre-phi, %n
  br i1 %exitcond.not, label %for.inc23.loopexit, label %for.body3, !llvm.loop !7

for.inc23.loopexit:                               ; preds = %for.inc
  br label %for.inc23

for.inc23:                                        ; preds = %for.inc23.loopexit, %for.cond1.preheader
  %inc24 = add nuw nsw i64 %i.053, 1
  %exitcond54.not = icmp eq i64 %inc24, %m
  br i1 %exitcond54.not, label %for.end25.loopexit, label %for.cond1.preheader, !llvm.loop !9

for.end25.loopexit:                               ; preds = %for.inc23
  br label %for.end25

for.end25:                                        ; preds = %for.end25.loopexit, %entry
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
!9 = distinct !{!9, !8}
