; RUN: opt -S < %s -vplan-pragma-omp-simd-if | FileCheck %s
; RUN: opt -S < %s -passes="vplan-pragma-omp-simd-if" | FileCheck %s

; Tests whether vplan-pragma-omp-simd-if pass converts all if clauses for nested loops
; with reversed loop basic block order

; Test was produced from this source file using this command line:
; icx -mllvm -print-after-all -fiopenmp -c simd-if2.cpp 2>ir2

; int foo(int len, const int *a) {
;   int ret = 0;
;   int tmp = 0;
; #pragma omp simd reduction(+ : ret) lastprivate(tmp) simdlen(8) if (len >= 8)
;   for (int i = 0; i < len; ++i) {
;     int s = 0;
;     tmp = a[i];
; #pragma omp simd reduction(+ : s) simdlen(4) if (tmp >= 4)
;     for (int j = 0; j < tmp; j++)
;       s += a[j];
;     ret += tmp + s;
;   }
;   return ret + tmp;
; }

; Checks that no QUAL.OMP.IF is left in the IR
; CHECK-NOT: "QUAL.OMP.IF"

; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4)
; CHECK-NOT: "QUAL.OMP.IF"
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8)
; CHECK-NOT: "QUAL.OMP.IF"
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 1)
; CHECK-NOT: "QUAL.OMP.IF"
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 1)
; CHECK-NOT: "QUAL.OMP.IF"
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4)
; CHECK-NOT: "QUAL.OMP.IF"
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 1)
; CHECK-NOT: "QUAL.OMP.IF"

; ModuleID = 'simd-if2.cpp'
source_filename = "simd-if2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nounwind uwtable
define dso_local noundef i32 @_Z3fooiPKi(i32 noundef %len, i32* nocapture noundef readonly %a) local_unnamed_addr #0 {
entry:
  %cmp4 = icmp ugt i32 %len, 7
  br label %DIR.OMP.SIMD.1104

DIR.OMP.SIMD.2105:                                ; preds = %omp.inner.for.body
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.IF"(i1 %cmp17) ]
  br label %omp.inner.for.body20

DIR.OMP.SIMD.1104:                                ; preds = %DIR.OMP.SIMD.1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.IF"(i1 %cmp4) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.1104, %omp.precond.end
  %indvars.iv93 = phi i64 [ 0, %DIR.OMP.SIMD.1104 ], [ 1, %omp.precond.end ]
  %cmp17 = icmp ugt i32 %len, 3
  br label %DIR.OMP.SIMD.2105

omp.inner.for.body20:                             ; preds = %DIR.OMP.SIMD.2105, %omp.inner.for.body20
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2105 ], [ 1, %omp.inner.for.body20 ]
  %exitcond.not = icmp eq i64 %indvars.iv, 1
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.5103, label %omp.inner.for.body20

DIR.OMP.END.SIMD.5103:                            ; preds = %omp.inner.for.body20
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.5103, %omp.inner.for.body
  %exitcond97.not = icmp eq i64 %indvars.iv, 1
  br i1 %exitcond97.not, label %DIR.OMP.END.SIMD.6, label %omp.inner.for.body

DIR.OMP.END.SIMD.6:                               ; preds = %omp.precond.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end34

omp.precond.end34:                                ; preds = %DIR.OMP.END.SIMD.6
  ret i32 1
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2
