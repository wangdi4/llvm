; REQUIRES: asserts
; RUN: opt <%s -hir-ssa-deconstruction -hir-temp-cleanup -hir-opt-var-predicate  -print-before=hir-opt-var-predicate -print-after=hir-opt-var-predicate -hir-details -debug-only=hir-opt-var-predicate -disable-output 2>&1 | FileCheck %s
; RUN: opt <%s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-var-predicate"  -print-before=hir-opt-var-predicate -print-after=hir-opt-var-predicate -hir-details -print-after=hir-opt-var-predicate -debug-only=hir-opt-var-predicate -disable-output 2>&1 | FileCheck %s

; Verify that LEGAL_MAX_TC can be used for mayIVOverflowCE

; CHECK: Function: foo
; CHECK:        BEGIN REGION { }
; CHECK:               + DO i32 i1 = 0, sext.i16.i32(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 32767>  <LEGAL_MAX_TC = 32767>
; CHECK:               |   if (i1 == %d)
; CHECK:               |   <RVAL-REG> LINEAR trunc.i32.i16(i1) {sb:2}
; CHECK:               |   <RVAL-REG> LINEAR i16 %d {sb:7}
; CHECK:               |   {
; CHECK:               |      (%p)[i1] = i1;
; CHECK:               |
; CHECK:               |   }
; CHECK:               |   else
; CHECK:               |   {
; CHECK:               |      (%q)[i1] = i1;
; CHECK:               |   }
; CHECK:               + END LOOP
; CHECK:         END REGION

; CHECK: Using LEGAL_MAX_TC
;
; CHECK: Function: foo
; CHECK:        BEGIN REGION { modified }
; CHECK:              + DO i32 i1 = 0, smin(sext.i16.i32((-1 + %d)), (-1 + sext.i16.i32(%n))), 1   <DO_LOOP>  <MAX_TC_EST = 32767>  <LEGAL_MAX_TC = 32767>
; CHECK:              |   (%q)[i1] = i1;
; CHECK:              + END LOOP
;
; CHECK:              if (smax(0, sext.i16.i32(%d)) < smin(sext.i16.i32(%d), (-1 + sext.i16.i32(%n))) + 1)
; CHECK:              {
; CHECK:                 (%p)[smax(0, sext.i16.i32(%d))] = smax(0, sext.i16.i32(%d));
; CHECK:              }
;
; CHECK:              + DO i32 i1 = 0, sext.i16.i32(%n) + -1 * smax(0, sext.i16.i32((1 + %d))) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 32767>  <LEGAL_MAX_TC = 32767>
; CHECK:              |   (%q)[i1 + smax(0, sext.i16.i32((1 + %d)))] = i1 + smax(0, sext.i16.i32((1 + %d)));
; CHECK:              + END LOOP
; CHECK:        END REGION

;Module Before HIR
; ModuleID = 'new-2.c'
source_filename = "new-2.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable writeonly
define dso_local void @foo(i32* nocapture writeonly %p, i32* nocapture writeonly %q, i16 signext %n, i16 signext %d) local_unnamed_addr #0 {
entry:
  %conv = sext i16 %n to i32
  %cmp16.not = icmp sle i16 %n, 0
  br i1 %cmp16.not, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %j.017 = phi i32 [ %inc, %for.inc ], [ 0, %for.body.preheader ]
  %conv3 = trunc i32 %j.017 to i16
  %cmp5 = icmp eq i16 %conv3, %d
  br i1 %cmp5, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %idxprom = zext i32 %j.017 to i64
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %idxprom
  store i32 %j.017, i32* %arrayidx, align 4, !tbaa !3
  br label %for.inc

if.else:                                          ; preds = %for.body
  %idxprom7 = zext i32 %j.017 to i64
  %arrayidx8 = getelementptr inbounds i32, i32* %q, i64 %idxprom7
  store i32 %j.017, i32* %arrayidx8, align 4, !tbaa !3
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %inc = add nuw nsw i32 %j.017, 1
  %exitcond.not = icmp eq i32 %inc, %conv
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !7

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind uwtable writeonly "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

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
