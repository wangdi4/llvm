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
; RUN: opt -enable-new-pm=0 -vplan-force-vf=1 -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -print-after=hir-vplan-vec -print-before=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-vplan-vec,print<hir>" -vplan-force-vf=1 -disable-output < %s 2>&1 | FileCheck %s

;
; CHECK: BEGIN REGION
; CHECK: %t4 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
; CHECK: DO i1 = 0, 1023, 1
; CHECK: @llvm.directive.region.exit(%t4); [ DIR.OMP.END.SIMD() ]
; CHECK: END REGION

; CHECK: BEGIN REGION
; CHECK: %t4 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
; CHECK: DO i1 = 0, 1023, 1
; CHECK: @llvm.directive.region.exit(%t4); [ DIR.OMP.END.SIMD() ]
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define i32* @foo(i64 %n, i32* returned %p) local_unnamed_addr #0 {
entry:
  %t4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %omp.inner.for.body ]
  %mul1 = mul nsw i64 %indvars.iv, %n
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %mul1
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %t4) [ "DIR.OMP.END.SIMD"() ]
  ret i32* %p
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)
