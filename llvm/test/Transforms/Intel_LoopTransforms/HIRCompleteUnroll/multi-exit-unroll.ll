; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-post-vec-complete-unroll,print<hir>" -hir-complete-unroll-multi-exit-loop-trip-threshold=8 2>&1 < %s | FileCheck %s --check-prefix=OVER-THRESHOLD

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-post-vec-complete-unroll,print<hir>" -hir-complete-unroll-multi-exit-loop-trip-threshold=12 2>&1 < %s | FileCheck %s --check-prefix=UNDER-THRESHOLD

; Input HIR-
; + DO i1 = 0, 9, 1   <DO_MULTI_EXIT_LOOP>
; |   %indvars.iv.out = i1;
; |   if ((%A)[i1 + 1] < 5)
; |   {
; |      if ((%A)[i1] > 10)
; |      {
; |         goto for.end.split.loop.exit;
; |      }
; |   }
; + END LOOP

; Verify that loop is unrolled when the trip count is smaller than multi-exit trip
; threshold but not when it is bigger than it.

; OVER-THRESHOLD: DO i1

; UNDER-THRESHOLD-NOT: DO i1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @foo(ptr nocapture readonly %A) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %ptridx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv.next
  %0 = load i32, ptr %ptridx, align 4, !tbaa !2
  %cmp1 = icmp slt i32 %0, 5
  br i1 %cmp1, label %land.lhs.true, label %for.inc

land.lhs.true:                                    ; preds = %for.body
  %ptridx3 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %1 = load i32, ptr %ptridx3, align 4, !tbaa !2
  %cmp4 = icmp sgt i32 %1, 10
  br i1 %cmp4, label %for.end.split.loop.exit, label %for.inc

for.inc:                                          ; preds = %for.body, %land.lhs.true
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.split.loop.exit:                          ; preds = %land.lhs.true
  %indvars.iv.lcssa = phi i64 [ %indvars.iv, %land.lhs.true ]
  %2 = trunc i64 %indvars.iv.lcssa to i32
  br label %for.end

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.end.split.loop.exit
  %i.0.lcssa = phi i32 [ %2, %for.end.split.loop.exit ], [ 10, %for.end.loopexit ]
  ret i32 %i.0.lcssa
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="broadwell" "target-features"="+adx,+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
