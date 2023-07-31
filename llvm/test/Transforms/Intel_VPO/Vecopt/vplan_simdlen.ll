; RUN: opt %s -S -passes="vpo-cfg-restructuring,vplan-vec" 2>&1 | FileCheck %s
; Verify that VPlan understands simdlen directive information.

; CHECK: load <2 x float>, ptr 
; CHECK: load <2 x float>, ptr 
; CHECK: store <2 x float> 

;icx -fopenmp -Qoption,c,-fintel-openmp -c -O1 -no-vec -mllvm -vpoopt=0 -S -emit-llvm

;void foo(float *a, float *b, float *c, int N) {
;
;  int i;
;
;  #pragma omp simd simdlen(2)
;    for (i = 0; i < N; i++) {
;      a[i] = b[i] + c[i];
;    }
;}


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(ptr nocapture %a, ptr nocapture readonly %b, ptr nocapture readonly %c, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp = icmp sgt i32 %N, 0
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2) ]
  %wide.trip.count = zext i32 %N to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.precond.then
  %indvars.iv = phi i64 [ 0, %omp.precond.then ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds float, ptr %b, i64 %indvars.iv
  %0 = load float, ptr %arrayidx, align 4, !tbaa !1
  %arrayidx8 = getelementptr inbounds float, ptr %c, i64 %indvars.iv
  %1 = load float, ptr %arrayidx8, align 4, !tbaa !1
  %add9 = fadd float %0, %1
  %arrayidx11 = getelementptr inbounds float, ptr %a, i64 %indvars.iv
  store float %add9, ptr %arrayidx11, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 21280)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
