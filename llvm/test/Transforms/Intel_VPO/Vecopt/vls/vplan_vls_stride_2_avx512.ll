;
; RUN: opt -disable-output -passes="vplan-vec,print" -mattr=+avx512f -enable-intel-advanced-opts < %s 2>&1 | FileCheck %s --check-prefix=LLVM-CHECK
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -mattr=+avx512f -enable-intel-advanced-opts < %s 2>&1 | FileCheck %s --check-prefix=HIR-CHECK
;
; LIT test to check that VLS optimization kicks in for stride 2 accesses with -xcore-avx512
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

;
; LLVM-CHECK-LABEL: vector.body:
; LLVM-CHECK:         %vls.load = load <8 x i64>, ptr %scalar.gep, align 8
; LLVM-CHECK-NEXT:    %1 = shufflevector <8 x i64> %vls.load, <8 x i64> %vls.load, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
; LLVM-CHECK-NEXT:    %2 = shufflevector <8 x i64> %vls.load, <8 x i64> %vls.load, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
;
; HIR-CHECK:       + DO i1 = 0, 1023, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR-CHECK-NEXT:  |   %.vls.load = (<8 x i64>*)(%lp)[2 * i1];
; HIR-CHECK-NEXT:  |   %vls.extract = shufflevector %.vls.load,  %.vls.load,  <i32 0, i32 2, i32 4, i32 6>;
; HIR-CHECK-NEXT:  |   %vls.extract2 = shufflevector %.vls.load,  %.vls.load,  <i32 1, i32 3, i32 5, i32 7>;
; HIR-CHECK-NEXT:  + END LOOP
;
define void @foo(ptr %lp) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.013 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %mul = shl nuw nsw i64 %l1.013, 1
  %arrayidx = getelementptr inbounds i64, ptr %lp, i64 %mul
  %0 = load i64, ptr %arrayidx, align 8
  %add = add nuw nsw i64 %mul, 1
  %arrayidx2 = getelementptr inbounds i64, ptr %lp, i64 %add
  %1 = load i64, ptr %arrayidx2, align 8
  %inc = add nuw nsw i64 %l1.013, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
