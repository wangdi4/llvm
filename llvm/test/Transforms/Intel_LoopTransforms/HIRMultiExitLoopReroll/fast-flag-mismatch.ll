; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-multi-exit-loop-reroll,print<hir>" -xmain-opt-level=3 -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-multi-exit-loop-reroll" -xmain-opt-level=3 -print-changed -disable-output 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED

; Verify that multi-exit loop reroll pass does not trigger on this loop.
; The fast flags for fadd are not all the same

; CHECK-NOT: modified
;       BEGIN REGION { }
;             + DO i1 = 0, %n + -1, 1   <DO_MULTI_EXIT_LOOP>
;             |   %arrayidx = &((%A)[4 * i1]);
;             |   %add = (%C)[4 * i1]  +  %0;
; CHECK:      |   if ((%A)[4 * i1] == %add)
;             |   {
;             |      goto cleanup49.loopexit.split.loop.exit102;
;             |   }
;             |   %arrayidx9 = &((%A)[4 * i1 + 1]);
;             |   %add14 = (%C)[4 * i1 + 1]  +  %0;
; CHECK:      |   if ((%A)[4 * i1 + 1] == %add14)
;             |   {
;             |      goto cleanup49.loopexit.split.loop.exit100;
;             |   }
;             |   %arrayidx23 = &((%A)[4 * i1 + 2]);
;             |   %add28 = (%C)[4 * i1 + 2]  +  %0;
;             |   if ((%A)[4 * i1 + 2] == %add28)
;             |   {
;             |      goto cleanup49.loopexit.split.loop.exit98;
;             |   }
;             |   %arrayidx37 = &((%A)[4 * i1 + 3]);
;             |   %add42 = (%C)[4 * i1 + 3]  +  %0;
;             |   if ((%A)[4 * i1 + 3] == %add42)
;             |   {
;             |      goto cleanup49.loopexit.split.loop.exit;
;             |   }
;             + END LOOP
;       END REGION

; Verify that pass is not dumped with print-changed if it bails out.

; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED-NOT: Dump After HIRMultiExitLoopReroll


;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree norecurse nosync nounwind readonly uwtable
define dso_local ptr @foo(ptr noalias noundef readonly %A, ptr noalias nocapture noundef readonly %B, ptr noalias nocapture noundef readonly %C, i64 noundef %n) local_unnamed_addr #0 {
entry:
  %cmp86 = icmp sgt i64 %n, 0
  br i1 %cmp86, label %for.body.lr.ph, label %cleanup49

for.body.lr.ph:                                   ; preds = %entry
  %0 = load float, ptr %B, align 4, !tbaa !3
  br label %for.body

for.cond:                                         ; preds = %if.end34
  %inc = add nuw nsw i64 %i.087, 1
  %exitcond.not = icmp eq i64 %inc, %n
  br i1 %exitcond.not, label %cleanup49.loopexit, label %for.body, !llvm.loop !7

for.body:                                         ; preds = %for.body.lr.ph, %for.cond
  %i.087 = phi i64 [ 0, %for.body.lr.ph ], [ %inc, %for.cond ]
  %mul = shl nsw i64 %i.087, 2
  %arrayidx = getelementptr inbounds float, ptr %A, i64 %mul
  %1 = load float, ptr %arrayidx, align 4, !tbaa !3
  %arrayidx3 = getelementptr inbounds float, ptr %C, i64 %mul
  %2 = load float, ptr %arrayidx3, align 4, !tbaa !3
  %add = fadd reassoc float %2, %0
  %cmp4 = fcmp fast oeq float %1, %add
  br i1 %cmp4, label %cleanup49.loopexit.split.loop.exit102, label %if.end

if.end:                                           ; preds = %for.body
  %add8 = or i64 %mul, 1
  %arrayidx9 = getelementptr inbounds float, ptr %A, i64 %add8
  %3 = load float, ptr %arrayidx9, align 4, !tbaa !3
  %arrayidx13 = getelementptr inbounds float, ptr %C, i64 %add8
  %4 = load float, ptr %arrayidx13, align 4, !tbaa !3
  %add14 = fadd ninf float %4, %0
  %cmp15 = fcmp fast oeq float %3, %add14
  br i1 %cmp15, label %cleanup49.loopexit.split.loop.exit100, label %if.end20

if.end20:                                         ; preds = %if.end
  %add22 = or i64 %mul, 2
  %arrayidx23 = getelementptr inbounds float, ptr %A, i64 %add22
  %5 = load float, ptr %arrayidx23, align 4, !tbaa !3
  %arrayidx27 = getelementptr inbounds float, ptr %C, i64 %add22
  %6 = load float, ptr %arrayidx27, align 4, !tbaa !3
  %add28 = fadd contract float %6, %0
  %cmp29 = fcmp fast oeq float %5, %add28
  br i1 %cmp29, label %cleanup49.loopexit.split.loop.exit98, label %if.end34

if.end34:                                         ; preds = %if.end20
  %add36 = or i64 %mul, 3
  %arrayidx37 = getelementptr inbounds float, ptr %A, i64 %add36
  %7 = load float, ptr %arrayidx37, align 4, !tbaa !3
  %arrayidx41 = getelementptr inbounds float, ptr %C, i64 %add36
  %8 = load float, ptr %arrayidx41, align 4, !tbaa !3
  %add42 = fadd reassoc float %8, %0
  %cmp43 = fcmp fast oeq float %7, %add42
  br i1 %cmp43, label %cleanup49.loopexit.split.loop.exit, label %for.cond

cleanup49.loopexit.split.loop.exit:               ; preds = %if.end34
  %arrayidx37.lcssa = phi ptr [ %arrayidx37, %if.end34 ]
  br label %cleanup49

cleanup49.loopexit.split.loop.exit98:             ; preds = %if.end20
  %arrayidx23.lcssa = phi ptr [ %arrayidx23, %if.end20 ]
  br label %cleanup49

cleanup49.loopexit.split.loop.exit100:            ; preds = %if.end
  %arrayidx9.lcssa = phi ptr [ %arrayidx9, %if.end ]
  br label %cleanup49

cleanup49.loopexit.split.loop.exit102:            ; preds = %for.body
  %arrayidx.lcssa = phi ptr [ %arrayidx, %for.body ]
  br label %cleanup49

cleanup49.loopexit:                               ; preds = %for.cond
  br label %cleanup49

cleanup49:                                        ; preds = %cleanup49.loopexit, %cleanup49.loopexit.split.loop.exit, %cleanup49.loopexit.split.loop.exit98, %cleanup49.loopexit.split.loop.exit100, %cleanup49.loopexit.split.loop.exit102, %entry
  %t.0 = phi ptr [ undef, %entry ], [ %arrayidx37.lcssa, %cleanup49.loopexit.split.loop.exit ], [ %arrayidx23.lcssa, %cleanup49.loopexit.split.loop.exit98 ], [ %arrayidx9.lcssa, %cleanup49.loopexit.split.loop.exit100 ], [ %arrayidx.lcssa, %cleanup49.loopexit.split.loop.exit102 ], [ undef, %cleanup49.loopexit ]
  ret ptr %t.0
}

attributes #0 = { argmemonly nofree norecurse nosync nounwind readonly uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
