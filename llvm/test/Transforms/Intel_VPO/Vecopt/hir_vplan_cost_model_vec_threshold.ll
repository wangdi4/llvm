; REQUIRES: asserts
; RUN: opt < %s -disable-output -hir-ssa-deconstruction -hir-vec-dir-insert \
; RUN:     -hir-vplan-vec -mtriple=x86_64-unknown-unknown -mattr=+avx2 \
; RUN:     -vec-threshold=20 -debug-only=LoopVectorizationPlanner \
; RUN:     -enable-intel-advanced-opts 2>&1 \
; RUN:     | FileCheck %s --check-prefix=HIR-VECTORIZE

; RUN: opt < %s -disable-output -hir-ssa-deconstruction -hir-vec-dir-insert \
; RUN:     -hir-vplan-vec -mtriple=x86_64-unknown-unknown -mattr=+avx2 \
; RUN:     -vec-threshold=80 -debug-only=LoopVectorizationPlanner \
; RUN:     -enable-intel-advanced-opts 2>&1 \
; RUN:     | FileCheck %s --check-prefix=HIR-NOVECTORIZE

; Test that VPlan Cost Model drives the VF selection.

@arr.i8.1 = external local_unnamed_addr global [1024 x i8], align 16
@arr.i8.2 = external local_unnamed_addr global [1024 x i8], align 16

define void @test_vectorize() local_unnamed_addr #0 {
; HIR-VECTORIZE: Selecting VPlan with VF={{2|4|8|16|32}}
; HIR-NOVECTORIZE: Selecting VPlan with VF=1

entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.skip ]

  %ld.idx = getelementptr inbounds [1024 x i8], [1024 x i8]* @arr.i8.1, i64 0, i64 %iv
  %ld.res = load i8, i8* %ld.idx

  %cmp.res = icmp eq i8 %ld.res, 0
  br i1 %cmp.res, label %for.skip, label %for.store

for.store:
  %st.idx = getelementptr inbounds [1024 x i8], [1024 x i8]* @arr.i8.2, i64 0, i64 %iv
  store i8 %ld.res, i8* %st.idx
  br label %for.skip

for.skip:
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
