;
; RUN: opt -disable-output -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-vec -print-after=hir-vplan-vec -print-after=vplan-vec -vplan-enable-peeling < %s 2>&1 | FileCheck %s
;
; LIT test to check that we suppress dynamic peeling in HIR path due to
; presence of simd aligned clause. This test will be updated once we start
; consuming aligned clause information from assumes.
;
; C Source:
;
; void foo(long *lp1, long *lp2)
; {
;  long l1;
;
; #pragma omp simd simdlen(4) aligned(lp1:16) aligned(lp2:32)
; for (l1 = 0; l1 < 10240; l1++)
;    lp1[l1] = lp2[l1] + 1;
; }
;
; CHECK:               BEGIN REGION { modified }
; CHECK-NEXT:                @llvm.assume(-1); [ align(&((%lp2)[0])32) ]
; CHECK-NEXT:                @llvm.assume(-1); [ align(&((%lp1)[0])16) ]
;
; CHECK:                    + DO i1 = 0, 1023, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:               |   %.vec = (<4 x i64>*)(%lp2)[i1];
; CHECK-NEXT:               |   (<4 x i64>*)(%lp1)[i1] = %.vec + 1;
; CHECK-NEXT:               + END LOOP
;
; CHECK:                    ret ;
; CHECK-NEXT:          END REGION
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i64* %lp1, i64* %lp2) {
entry:
  br label %for.ph

for.ph:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.ALIGNED:PTR_TO_PTR"(i64** null, i32 16), "QUAL.OMP.ALIGNED:PTR_TO_PTR"(i64** null, i32 32) ]
  br label %for.assume

for.assume:
  call void @llvm.assume(i1 true) [ "align"(i64* %lp2, i64 32) ]
  call void @llvm.assume(i1 true) [ "align"(i64* %lp1, i64 16) ]
  br label %for.body

for.body:
  %l1.06 = phi i64 [ 0, %for.assume ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i64, i64* %lp2, i64 %l1.06
  %0 = load i64, i64* %arrayidx, align 8
  %add = add nsw i64 %0, 1
  %arrayidx1 = getelementptr inbounds i64, i64* %lp1, i64 %l1.06
  store i64 %add, i64* %arrayidx1, align 8
  %inc = add nuw nsw i64 %l1.06, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.assume(i1 noundef)