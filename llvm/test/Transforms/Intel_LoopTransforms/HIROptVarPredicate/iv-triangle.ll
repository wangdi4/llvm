; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-opt-var-predicate -disable-output -print-before=hir-opt-var-predicate -print-after=hir-opt-var-predicate %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-var-predicate" -disable-output -print-before=hir-opt-var-predicate -print-after=hir-opt-var-predicate %s 2>&1 | FileCheck %s
;
; Make sure triangular conditions are handled.
;
; Source:
; void foo(int *p, int *q, long n, long m, long d) {
;   long i,j;
;   for (i=0;i<m;++i) {
;   for (j=0;j<n;++j) {
;     if (j == i + d) { // condition has two different levels of IVs
;       p[j] = j;
;     } else {
;       q[j] = j;
;     }
;   }
;   }
; }
;
;
; CHECK: Function: foo
;
; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, %m + -1, 1   <DO_LOOP>
; CHECK:               |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:               |   |   if (i2 == i1 + %d)
; CHECK:               |   |   {
; CHECK:               |   |      (%p)[i1 + %d] = i1 + %d;
; CHECK:               |   |   }
; CHECK:               |   |   else
; CHECK:               |   |   {
; CHECK:               |   |      (%q)[i2] = i2;
; CHECK:               |   |   }
; CHECK:               |   + END LOOP
; CHECK:               + END LOOP
; CHECK:         END REGION
;
; CHECK: Function: foo
;
; CHECK:         BEGIN REGION { modified }
; CHECK:               + DO i1 = 0, %m + -1, 1   <DO_LOOP>
; CHECK:               |   %ivcopy = i1 + %d;
; CHECK:               |   if (%n > 0)
; CHECK:               |   {
; CHECK:               |      + DO i2 = 0, smin((-1 + %n), (-1 + %ivcopy)), 1   <DO_LOOP>
; CHECK:               |      |   (%q)[i2] = i2;
; CHECK:               |      + END LOOP
; CHECK:               |
; CHECK:               |      if (smax(0, %ivcopy) < smin((-1 + %n), %ivcopy) + 1)
; CHECK:               |      {
; CHECK:               |         (%p)[i1 + %d] = i1 + %d;
; CHECK:               |      }
; CHECK:               |
; CHECK:               |      + DO i2 = 0, %n + -1 * smax(0, (1 + %ivcopy)) + -1, 1   <DO_LOOP>
; CHECK:               |      |   (%q)[i2 + smax(0, (1 + %ivcopy))] = i2 + smax(0, (1 + %ivcopy));
; CHECK:               |      + END LOOP
; CHECK:               |   }
; CHECK:               + END LOOP
; CHECK:         END REGION


;Module Before HIR
; ModuleID = 'iv-nested.c'
source_filename = "iv-nested.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable writeonly
define dso_local void @foo(i32* nocapture writeonly %p, i32* nocapture writeonly %q, i64 %n, i64 %m, i64 %d) local_unnamed_addr #0 {
entry:
  %cmp220 = icmp sgt i64 %n, 0
  %cmp23 = icmp sgt i64 %m, 0
  br i1 %cmp23, label %for.cond1.preheader.preheader, label %for.end9

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.inc7
  %i.024 = phi i64 [ %inc8, %for.inc7 ], [ 0, %for.cond1.preheader.preheader ]
  %add = add nsw i64 %i.024, %d
  br i1 %cmp220, label %for.body3.preheader, label %for.inc7

for.body3.preheader:                              ; preds = %for.cond1.preheader
  %conv = trunc i64 %add to i32
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %add
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.inc
  %j.021 = phi i64 [ %inc, %for.inc ], [ 0, %for.body3.preheader ]
  %cmp4 = icmp eq i64 %j.021, %add
  br i1 %cmp4, label %if.then, label %if.else

if.then:                                          ; preds = %for.body3
  store i32 %conv, i32* %arrayidx, align 4, !tbaa !3
  br label %for.inc

if.else:                                          ; preds = %for.body3
  %conv5 = trunc i64 %j.021 to i32
  %arrayidx6 = getelementptr inbounds i32, i32* %q, i64 %j.021
  store i32 %conv5, i32* %arrayidx6, align 4, !tbaa !3
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %inc = add nuw nsw i64 %j.021, 1
  %exitcond.not = icmp eq i64 %inc, %n
  br i1 %exitcond.not, label %for.inc7.loopexit, label %for.body3, !llvm.loop !7

for.inc7.loopexit:                                ; preds = %for.inc
  br label %for.inc7

for.inc7:                                         ; preds = %for.inc7.loopexit, %for.cond1.preheader
  %inc8 = add nuw nsw i64 %i.024, 1
  %exitcond25.not = icmp eq i64 %inc8, %m
  br i1 %exitcond25.not, label %for.end9.loopexit, label %for.cond1.preheader, !llvm.loop !9

for.end9.loopexit:                                ; preds = %for.inc7
  br label %for.end9

for.end9:                                         ; preds = %for.end9.loopexit, %entry
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
!9 = distinct !{!9, !8}
