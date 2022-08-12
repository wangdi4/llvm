; RUN: opt -S < %s -vplan-pragma-omp-simd-if | FileCheck %s
; RUN: opt -S < %s -passes="vplan-pragma-omp-simd-if" | FileCheck %s

; Tests whether vplan-pragma-omp-simd-if pass reduces pragma omp simd if clause into simdlen(1) clause
; for legacy and new pass managers.

; Test was produced from this source file using this command line:
; icx -mllvm -print-after-all -fiopenmp -c simd-if.cpp 2>ir2
;
; int foo(int len, const int *a) {
;   int ret = 0;
;   int tmp = 0;
; #pragma omp simd reduction(+ : ret) lastprivate(tmp) simdlen(8) if (len >= 8)
;   for (int i = 0; i < len; ++i) {
;     tmp = a[i];
;     ret += tmp;
;   }
;   return ret + tmp;
; }

; Checks that no QUAL.OMP.IF is left in the IR
; CHECK-NOT: "QUAL.OMP.IF"

; Checks that if clause is inseted, region is cloned and clone has simdlen(1) and not simdlen(8)
; CHECK: br i1 %cmp4, label %DIR.OMP.SIMD.138.split, label %DIR.OMP.SIMD.138.split.clone
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8)
; CHECK-NOT: "QUAL.OMP.IF"
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 1)
; CHECK-NOT: "QUAL.OMP.IF"

; ModuleID = 'simd-if.cpp'
source_filename = "simd-if.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nounwind uwtable
define dso_local i32 @_Z3fooiPKi(i32 %len, i32* nocapture readonly %a) local_unnamed_addr #0 {
entry:
  %cmp4 = icmp ugt i32 %len, 7
  br label %DIR.OMP.SIMD.138

DIR.OMP.SIMD.138:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.IF"(i1 %cmp4) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.138
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ 1, %omp.inner.for.body ]
  %exitcond.not = icmp eq i64 %indvars.iv, 1
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.3, label %omp.inner.for.body

DIR.OMP.END.SIMD.3:                               ; preds = %omp.inner.for.body
  br label %DIR.OMP.END.SIMD.337

DIR.OMP.END.SIMD.337:                             ; preds = %DIR.OMP.END.SIMD.3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.337, %entry
  ret i32 1
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2
