;
; RUN: opt -disable-output -passes="vplan-vec,print" -mattr=+avx512f -enable-intel-advanced-opts < %s 2>&1 | FileCheck %s --check-prefix=LLVM-CHECK
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -mattr=+avx512f -enable-intel-advanced-opts < %s 2>&1 | FileCheck %s --check-prefix=HIR-CHECK
;
; LIT test to check that VLS optimization does not kick in for non-stride2 accesses with -xcore-avx512
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

;
; LLVM-CHECK:   %wide.masked.gather = call <4 x i64> @llvm.masked.gather.v4i64.v4p0(<4 x ptr> %mm_vectorGEP, i32 8, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i64> poison)
; LLVM-CHECK:   %wide.masked.gather4 = call <4 x i64> @llvm.masked.gather.v4i64.v4p0(<4 x ptr> %mm_vectorGEP3, i32 8, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i64> poison)
; LLVM-CHECK:   %wide.masked.gather6 = call <4 x i64> @llvm.masked.gather.v4i64.v4p0(<4 x ptr> %mm_vectorGEP5, i32 8, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i64> poison)
;
; HIR-CHECK:        + DO i1 = 0, 99, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR-CHECK-NEXT:   |   %.vec = (<4 x i64>*)(%lp)[3 * i1 + 3 * <i64 0, i64 1, i64 2, i64 3>];
; HIR-CHECK-NEXT:   |   %.vec2 = (<4 x i64>*)(%lp)[3 * i1 + 3 * <i64 0, i64 1, i64 2, i64 3> + 1];
; HIR-CHECK-NEXT:   |   %.vec3 = (<4 x i64>*)(%lp)[3 * i1 + 3 * <i64 0, i64 1, i64 2, i64 3> + 2];
; HIR-CHECK-NEXT:   + END LOOP
;
define void @foo(ptr %lp) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.020 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %mul = mul nuw nsw i64 %l1.020, 3
  %arrayidx = getelementptr inbounds i64, ptr %lp, i64 %mul
  %0 = load i64, ptr %arrayidx, align 8
  %add = add nuw nsw i64 %mul, 1
  %arrayidx2 = getelementptr inbounds i64, ptr %lp, i64 %add
  %1 = load i64, ptr %arrayidx2, align 8
  %add6 = add nuw nsw i64 %mul, 2
  %arrayidx7 = getelementptr inbounds i64, ptr %lp, i64 %add6
  %2 = load i64, ptr %arrayidx7, align 8
  %inc = add nuw nsw i64 %l1.020, 1
  %exitcond.not = icmp eq i64 %inc, 100
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
