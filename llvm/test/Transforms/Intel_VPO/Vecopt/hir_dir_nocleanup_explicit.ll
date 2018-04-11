; LLVM IR generated from the following test using:
; icx -O1 -S -emit-llvm -Qoption,c,-fintel-openmp -fopenmp simdtest.c -mllvm -disable-vpo-directive-cleanup
; int *foo(long n, int *p)
; {
;   int i1;
; 
; #pragma omp simd
;   for (i1 = 0; i1 < 1024; i1++)  {
;     p[n * i1] = i1;
;   }
; 
;   return p;
; }
; 
; Test to check that we leave the simd directives around when we do not vectorize the explicit simd loop.
; RUN: opt -vplan-force-vf=1 -hir-ssa-deconstruction -hir-framework -VPlanDriverHIR -print-after=VPlanDriverHIR -print-before=VPlanDriverHIR -S < %s 2>&1 | FileCheck %s
; 
; CHECK: IR Dump Before VPlan Vectorization Driver HIR 
; CHECK: BEGIN REGION   
; CHECK: llvm.intel.directive
; CHECK: DO i1 = 0, 1023, 1
; CHECK: llvm.intel.directive
; CHECK: END REGION
; CHECK: IR Dump After VPlan Vectorization Driver HIR 
; CHECK: BEGIN REGION   
; CHECK: llvm.intel.directive
; CHECK: DO i1 = 0, 1023, 1
; CHECK: llvm.intel.directive
; CHECK: END REGION
; ModuleID = 'simdtest.c'
source_filename = "simdtest.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define i32* @foo(i64 %n, i32* returned %p) local_unnamed_addr #0 {
entry:
  call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %omp.inner.for.body ]
  %mul1 = mul nsw i64 %indvars.iv, %n
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %mul1
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  ret i32* %p
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 0b4a517eb9d8148972ed8a9ef18c3616c05891fc) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm f6900f6421cdbe4464ab7bf111d426e4c0f9d393)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
