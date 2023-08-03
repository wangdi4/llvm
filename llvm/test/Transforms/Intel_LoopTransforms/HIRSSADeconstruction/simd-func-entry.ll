; RUN: opt -passes="hir-ssa-deconstruction" -print-after=hir-ssa-deconstruction -disable-output < %s 2>&1 | FileCheck %s

; No VPlanDriverHIR in new PM
; opt -passes="hir-ssa-deconstruction,print" -disable-output < %s 2>&1 | FileCheck %s

; Entry BB should be split because it contains llvm.directive.region.entry

; CHECK: entry
; CHECK-NOT: llvm.directive.region.entry
; CHECK: entry.split
; CHECK: llvm.directive.region.entry

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(ptr nocapture %a) local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 4
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.inc ], [ 0, %entry ]
  %1 = sub nuw nsw i64 32, %indvars.iv
  %2 = load i32, ptr %arrayidx, align 4
  %cmp1 = icmp eq i32 %2, 20
  br i1 %cmp1, label %omp.inner.for.inc, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx2 = getelementptr inbounds i32, ptr %a, i64 %1
  %3 = trunc i64 %1 to i32
  store i32 %3, ptr %arrayidx2, align 4
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.inner.for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 31
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.inc
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }


