; ModuleID = 'il1.c'
; source_filename = "il1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
; void foo(long *ip)
; {
;   long index;
;
; #pragma omp simd simdlen(4)
;   for (index = 0; index < 1024; index++)
;     ip[index] = index;
; }
; RUN: opt -VPlanDriver -S -enable-vp-value-codegen=false %s | FileCheck %s
; RUN: opt -VPlanDriver -S -enable-vp-value-codegen %s | FileCheck %s

; This test checks that scalar IV steps are not generated in vector loop
; CHECK: vector.ph:
; CHECK: vector.body:
; CHECK:   [[IV_PHI:%.*]] = phi i64 [ 0, %vector.ph ], [ [[IV_NEXT:%.*]], %vector.body ]
; CHECK-NOT: {{.*}} = add i64 [[IV_PHI]], {{[0123]}}
; CHECK: {{.*}} = getelementptr inbounds i64, i64* %ip, i64 [[IV:%.*]]
; Function Attrs: nounwind uwtable
define void @foo(i64* nocapture %ip)  {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %entry
  %.omp.iv.07 = phi i64 [ 0, %entry ], [ %add1, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i64, i64* %ip, i64 %.omp.iv.07
  store i64 %.omp.iv.07, i64* %arrayidx, align 8
  %add1 = add nuw nsw i64 %.omp.iv.07, 1
  %exitcond = icmp eq i64 %add1, 1024
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: argmemonly nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: argmemonly nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

