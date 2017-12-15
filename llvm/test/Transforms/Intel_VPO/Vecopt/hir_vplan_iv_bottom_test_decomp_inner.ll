; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-print-after-simplify-cfg -disable-output < %s 2>&1 | FileCheck %s

; Verify that decomposer properly materializes the IV semi-phi, IV Next and
; bottom test condition.

; CHECK: [[IVPhi:%.*]] = semi-phi i64 0 [[IVNext:%.*]]
; CHECK: [[IVNext]] = add [[IVPhi]] i64 1
; CHECK: {{%.*}} = icmp [[IVNext]] {{%.*}}

target triple = "x86_64-unknown-linux-gnu"

@a = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@b = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@c = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16


define void @foo() local_unnamed_addr #0 {
entry:
  br label %omp.inner.for.body

omp.inner.for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %omp.inner.for.body ]
  %idx = getelementptr inbounds [1600 x i32], [1600 x i32]* @a, i64 0, i64 %iv
  %0 = load i32, i32* %idx
  %idx2 = getelementptr inbounds [1600 x i32], [1600 x i32]* @b, i64 0, i64 %iv
  %1 = load i32, i32* %idx2
  %add3 = add nsw i32 %1, %0
  %idx5 = getelementptr inbounds [1600 x i32], [1600 x i32]* @c, i64 0, i64 %iv
  store i32 %add3, i32* %idx5
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1600
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:
  ret void
}

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
