; This test verifies that Vectorizer properly caches '@llvm.assume' intrinsics.
; This means it caches:
;   1. Assumptions that affect values defined externally to the plan (LLVM Assumes).
;   2. Assumptions that appear internally within the plan (VPCall Assumes).
;
; Additionally, when importing external assumptions into the cache, check that
; we only import assumptions which are valid (i.e. dominate the VPlan loop.)

; RUN: opt -S < %s -passes='vplan-vec' -vplan-force-vf=2 -disable-output \
; RUN:   -debug-only=IntelVPAssumptionCache 2>&1 | FileCheck %s

; RUN: opt < %s -passes='hir-ssa-deconstruction,hir-vplan-vec' \
; RUN: -vplan-force-vf=2 -disable-output -hir-create-function-level-region \
; RUN: -debug-only=IntelVPAssumptionCache 2>&1 | FileCheck %s

;CHECK: Registering assumption: call i1 true i8* [[NONNULL:%.*]] i8* [[ALIGN:%.*]] i64 32 void (i1)* @llvm.assume
;CHECK: Inserting assumption cache elem for 'i8* [[NONNULL]]'
;CHECK-NEXT: Assume: call i1 true i8* [[NONNULL]] i8* [[ALIGN]] i64 32 void (i1)* @llvm.assume
;CHECK-NEXT: Index:  0
;CHECK: Inserting assumption cache elem for 'i8* [[ALIGN]]'
;CHECK-NEXT: Assume: call i1 true i8* [[NONNULL]] i8* [[ALIGN]] i64 32 void (i1)* @llvm.assume
;CHECK-NEXT: Index:  1

;CHECK: Inserting assumption cache elem for 'i8* %lp1'
;CHECK-NEXT: Assume: call void @llvm.assume(i1 true) [ "align"(i8* %lp1, i64 16) ]
;CHECK-NEXT: Index:  0

;CHECK-NOT: Inserting assumption cache elem for 'i8* %lp2'

;CHECK: Inserting assumption cache elem for 'i8* %lp3':
;CHECK-NEXT: Assume: call void @llvm.assume(i1 true) [ "align"(i8* %lp3, i64 128) ]
;CHECK-NEXT: Index: 0

; (mostly) equivalent C source to LLVM-IR below:
;
;    bool cond(void);
;    void foo(char *lp1, char *lp2, char *lp3) {
;      // valid external assumption
;      __builtin_assume_aligned(lp1, 16);
;
;      // invalid external assumption (non-dominating)
;      if (cond()) __builtin_assume_aligned(lp2, 64)
;
;      for (long I = 0; I < 1024; ++I) {
;        // valid external assumption
;        __builtin_assume_aligned(lp3, 128)
;
;        // invalid external assumption (non-dominating, inside outer loop)
;        if (cond()) __builtin_assume_aligned(lp2, 128)
;
;        #pragma omp simd
;        for (long J = 0; J < 1024; ++J) {
;           // valid internal assumptions (inside VPlan loop)
;           __builtin_assume_aligned(lp1, 32);
;           __builtin_assume_aligned(lp2, 32);
;          (void)*lp1, (void)*lp2;
;        }
;      }
;    }

declare i1 @cond()

define void @foo(i8* noalias %lp1, i8* noalias %lp2, i8* noalias %lp3) {
entry:
  call void @llvm.assume(i1 true) [ "align"(i8* %lp1, i64 16) ]
  %cond = call i1 @cond()
  br i1 %cond, label %invalid.external.assume, label %outer.for.ph

invalid.external.assume:
  call void @llvm.assume(i1 true) [ "align"(i8* %lp2, i64 64) ]
  br label %outer.for.ph

outer.for.ph:
  br label %outer.for.header

outer.for.header:
  %outer.iv = phi i64 [ 0, %outer.for.ph ], [ %outer.add, %outer.for.latch ]
  call void @llvm.assume(i1 true) [ "align"(i8* %lp3, i64 128) ]
  %cond2 = call i1 @cond()
  br i1 %cond2, label %invalid.loop.assume, label %invalid.loop.after

invalid.loop.assume:
  call void @llvm.assume(i1 true) [ "align"(i8* %lp2, i64 128) ]
  br label %invalid.loop.after

invalid.loop.after:
  br label %inner.for.ph

inner.for.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %inner.for.body

inner.for.body:
  %inner.iv = phi i64 [ 0, %inner.for.ph ], [ %inner.add, %inner.for.body ]
  call void @llvm.assume(i1 true) [ "nonnull"(i8* %lp2), "align"(i8* %lp2, i64 32) ]
  %inner.add = add nuw nsw i64 %inner.iv, 1
  %elem = load i8, i8* %lp1, align 8
  %elem2 = load i8, i8* %lp2, align 8
  %elem3 = load i8, i8* %lp3, align 8
  %inner.exitcond.not = icmp eq i64 %inner.add, 1024
  br i1 %inner.exitcond.not, label %omp.simd.end, label %inner.for.body

omp.simd.end:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %outer.for.latch

outer.for.latch:
  %outer.add = add nuw nsw i64 %outer.iv, 1
  %outer.exitcond.not = icmp eq i64 %outer.add, 1024
  br i1 %outer.exitcond.not, label %ret, label %outer.for.header

ret:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.assume(i1 noundef)
