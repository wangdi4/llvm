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

;CHECK: Registering assumption: call i1 true ptr [[NONNULL:%.*]] ptr [[ALIGN:%.*]] i64 32 ptr @llvm.assume
;CHECK: Inserting assumption cache elem for 'ptr [[NONNULL]]'
;CHECK-NEXT: Assume: call i1 true ptr [[NONNULL]] ptr [[ALIGN]] i64 32 ptr @llvm.assume
;CHECK-NEXT: Index:  0
;CHECK: Inserting assumption cache elem for 'ptr [[ALIGN]]'
;CHECK-NEXT: Assume: call i1 true ptr [[NONNULL]] ptr [[ALIGN]] i64 32 ptr @llvm.assume
;CHECK-NEXT: Index:  1

;CHECK: Registering assumption: call i1 [[COND:%.*]] ptr @llvm.assume
;CHECK: Inserting assumption cache elem for 'i1 [[COND]]'
;CHECK-NEXT: Assume: call i1 [[COND]] ptr @llvm.assume
;CHECK-NEXT: Index:  <expr>

;CHECK: Inserting assumption cache elem for 'ptr %lp1'
;CHECK-NEXT: Assume: call void @llvm.assume(i1 true) [ "align"(ptr %lp1, i64 16) ]
;CHECK-NEXT: Index:  0

;CHECK-NOT: Inserting assumption cache elem for 'ptr %lp2'

;CHECK: Inserting assumption cache elem for 'ptr %lp3':
;CHECK-NEXT: Assume: call void @llvm.assume(i1 true) [ "align"(ptr %lp3, i64 64) ]
;CHECK-NEXT: Index: 0

;CHECK: Inserting assumption cache elem for 'ptr %lp3':
;CHECK-NEXT: Assume: call void @llvm.assume(i1 true) [ "align"(ptr %lp3, i64 128) ]
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
;      for (long I = 0; I < 4; ++I) {
;         // valid external assumption (dominating, loop is aways executed)
;         __builtin_assume_aligned(lp3, 64);
;         (void)*lp3;
;      }
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

define void @foo(ptr noalias %lp1, ptr noalias %lp2, ptr noalias %lp3) {
entry:
  call void @llvm.assume(i1 true) [ "align"(ptr %lp1, i64 16) ]
  %cond = call i1 @cond()
  br i1 %cond, label %invalid.external.assume, label %invalid.external.assume.after

invalid.external.assume:
  call void @llvm.assume(i1 true) [ "align"(ptr %lp2, i64 64) ]
  br label %invalid.external.assume.after

invalid.external.assume.after:
  br label %sibling.for.ph

sibling.for.ph:
  br label %sibling.for.body

sibling.for.body:
  %sibling.iv = phi i64 [ 0, %sibling.for.ph ], [ %sibling.iv.next, %sibling.for.body ]
  call void @llvm.assume(i1 true) [ "align"(ptr %lp3, i64 64) ]
  %0 = load i8, ptr %lp3, align 1
  %sibling.iv.next = add nuw nsw i64 %sibling.iv, 1
  %sibling.exitcond = icmp eq i64 %sibling.iv.next, 4
  br i1 %sibling.exitcond, label %sibling.for.exit, label %sibling.for.body

sibling.for.exit:
  br label %outer.for.ph

outer.for.ph:
  br label %outer.for.header

outer.for.header:
  %outer.iv = phi i64 [ 0, %outer.for.ph ], [ %outer.add, %outer.for.latch ]
  call void @llvm.assume(i1 true) [ "align"(ptr %lp3, i64 128) ]
  %cond2 = call i1 @cond()
  br i1 %cond2, label %invalid.loop.assume, label %invalid.loop.after

invalid.loop.assume:
  call void @llvm.assume(i1 true) [ "align"(ptr %lp2, i64 128) ]
  br label %invalid.loop.after

invalid.loop.after:
  br label %inner.for.ph

inner.for.ph:
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %inner.for.body

inner.for.body:
  %inner.iv = phi i64 [ 0, %inner.for.ph ], [ %inner.add, %inner.for.body ]
  call void @llvm.assume(i1 true) [ "nonnull"(ptr %lp2), "align"(ptr %lp2, i64 32) ]
  %assume.cond = icmp eq i64 0, 0
  call void @llvm.assume(i1 %assume.cond)
  %inner.add = add nuw nsw i64 %inner.iv, 1
  %elem = load i8, ptr %lp1, align 8
  %elem2 = load i8, ptr %lp2, align 8
  %elem3 = load i8, ptr %lp3, align 8
  %inner.exitcond.not = icmp eq i64 %inner.add, 1024
  br i1 %inner.exitcond.not, label %omp.simd.end, label %inner.for.body

omp.simd.end:
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
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
