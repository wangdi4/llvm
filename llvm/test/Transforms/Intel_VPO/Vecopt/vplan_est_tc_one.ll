;
; Check that we don't bailout on a low estimated TC when vectorization is enforced.
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,print<hir>,hir-vplan-vec,print<hir>" -S < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr1 = common dso_local local_unnamed_addr global [1 x i32] zeroinitializer, align 16
;
; Incoming HIR
;CHECK: %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;CHECK: + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1> <vectorize>
;CHECK: |   (@arr1)[0][i1] = i1;
;CHECK: + END LOOP
;
; vectorized loop, omitting TC checks and remainder
;CHECK: + DO i1 = 0, %loop.ub, 2   <DO_LOOP> <auto-vectorized> <nounroll> <novectorize>
;CHECK: |  (<2 x i32>*)(@arr1)[0][i1] = i1 + <i32 0, i32 1>;
;CHECK: + END LOOP

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local void @foo(i32 %N) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i32 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1 x i32], ptr @arr1, i32 0, i32 %indvars.iv
  store i32 %indvars.iv, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond = icmp eq i32 %indvars.iv.next, %N
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !5

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!5 = distinct !{!5, !6, !7}
!6 = !{!"llvm.loop.vectorize.ignore_profitability"}
!7 = !{!"llvm.loop.vectorize.enable", i1 true}
