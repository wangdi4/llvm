; This test verifies that Vectorizer properly caches '@llvm.assume' intrinsics.
; This means it caches:
;   1. Assumptions that affect values defined externally to the plan (LLVM Assumes).
;   2. Assumptions that appear internally within the plan (VPCall Assumes).

; RUN: opt -S < %s -passes='vplan-vec' -vplan-force-vf=2 -disable-output \
; RUN:   -debug-only=IntelVPAssumptionCache 2>&1 | FileCheck %s
; RUN: opt -S < %s -passes='hir-ssa-deconstruction,hir-vplan-vec' -vplan-force-vf=2 -disable-output \
; RUN:   -debug-only=IntelVPAssumptionCache 2>&1 | FileCheck %s

;CHECK: Registering assumption: call i1 true i8* [[NONNULL:%.*]] i8* [[ALIGN:%.*]] i64 32 void (i1)* @llvm.assume
;CHECK: Inserting assumption cache elem for 'i8* [[NONNULL]]'
;CHECK-NEXT: Assume: call i1 true i8* [[NONNULL]] i8* [[ALIGN]] i64 32 void (i1)* @llvm.assume
;CHECK-NEXT: Index:  0
;CHECK: Inserting assumption cache elem for 'i8* [[ALIGN]]'
;CHECK-NEXT: Assume: call i1 true i8* [[NONNULL]] i8* [[ALIGN]] i64 32 void (i1)* @llvm.assume
;CHECK-NEXT: Index:  1

define void @foo(i8* noalias %lp1, i8* noalias %lp2) {
entry:
  call void @llvm.assume(i1 true) [ "align"(i8* %lp1, i64 16) ]
  br label %for.ph

for.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %for.ph ], [ %add1, %for.body ]
  call void @llvm.assume(i1 true) [ "nonnull"(i8* %lp2), "align"(i8* %lp2, i64 32) ]
  %add1 = add nuw nsw i64 %iv, 1
  %elem = load i8, i8* %lp1, align 8
  %exitcond.not = icmp eq i64 %add1, 1024
  br i1 %exitcond.not, label %omp.simd.end, label %for.body

omp.simd.end:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %ret

ret:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.assume(i1 noundef)
