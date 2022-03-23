; Verify that OMP SIMD directives are removed by the clean-up pass or by vectorizer after executing VPO passes at all optimization levels.
; Input LLVM IR generated for below C program:

; float arr[1024];
;
; float  foo(int n1)
; {
;     int index;
;
; #pragma omp simd
;     for (index = 0; index < 1024; index++) {
;         if (arr[index] > 0) {
;             arr[index + n1] = index + n1 * n1 + 3;
;         }
;     }
;     return arr[0];
; }

; RUN: opt -O0 -paropt=0x7 -S %s | FileCheck %s
; RUN: opt -O1 -paropt=0x7 -S %s | FileCheck %s
; RUN: opt -O2 -paropt=0x7 -S %s | FileCheck %s
; RUN: opt -O3 -paropt=0x7 -S %s | FileCheck %s

; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()
; CHECK-NOT: call void @llvm.directive.region.exit(token {{.*}}) [ "DIR.OMP.END.SIMD"() ]


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = common dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16

; Function Attrs: noinline nounwind optnone uwtable
define dso_local float @foo(i32 %n1) local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  %mul2 = mul nsw i32 %n1, %n1
  %add3 = add nuw i32 %mul2, 3
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.inc ], [ 0, %omp.inner.for.body.lr.ph ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @arr, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %1 = load float, float* %arrayidx, align 4, !tbaa !2
  %cmp1 = fcmp ogt float %1, 0.000000e+00
  br i1 %cmp1, label %if.then, label %omp.inner.for.inc

if.then:                                          ; preds = %omp.inner.for.body
  %2 = trunc i64 %indvars.iv to i32
  %add4 = add i32 %add3, %2
  %conv = sitofp i32 %add4 to float
  %add5 = add nsw i32 %2, %n1
  %idxprom6 = sext i32 %add5 to i64
  %arrayidx7 = getelementptr inbounds [1024 x float], [1024 x float]* @arr, i64 0, i64 %idxprom6, !intel-tbaa !2
  store float %conv, float* %arrayidx7, align 4, !tbaa !2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %if.then, %omp.inner.for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.inc
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  %3 = load float, float* getelementptr inbounds ([1024 x float], [1024 x float]* @arr, i64 0, i64 0), align 16, !tbaa !2
  ret float %3
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
