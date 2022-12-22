; REQUIRES: asserts
; RUN: opt < %s -S -passes=vplan-vec  -mtriple=x86_64-unknown-unknown -mattr=+avx2 -debug \
; RUN:     -vec-threshold=50 -vplan-build-vect-candidates=1\
; RUN:     2>&1 | FileCheck %s

; REQUIRES: asserts
; RUN: opt < %s -S -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec' \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+avx2 -vec-threshold=50 -debug \
; RUN:     2>&1 | FileCheck %s

; REQUIRES: asserts
; RUN: opt < %s -S -passes=vplan-vec  -mtriple=x86_64-unknown-unknown -mattr=+avx2 -debug \
; RUN:     -vec-threshold=0 -vplan-build-vect-candidates=1 \
; RUN:     2>&1 | FileCheck %s --check-prefix=VEC-ALWAYS

; REQUIRES: asserts
; RUN: opt < %s -S -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec' \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+avx2 -vec-threshold=0 -debug \
; RUN:     2>&1 | FileCheck %s --check-prefix=VEC-ALWAYS


; Test that VPlan Cost Model drives the VF selection.

@arr.i32.1 = external local_unnamed_addr global [1024 x i32], align 16
@arr.i32.2 = external local_unnamed_addr global [1024 x i32], align 16
@arr.i32.3 = external local_unnamed_addr global [1024 x i32], align 16
@arr.i32.4 = external local_unnamed_addr global [1024 x i32], align 16

@arr.float.1 = external local_unnamed_addr global [1024 x float], align 16
@arr.float.2 = external local_unnamed_addr global [1024 x float], align 16
@arr.float.3 = external local_unnamed_addr global [1024 x float], align 16

define void @test_vectorize() local_unnamed_addr #0 {
; CHECK: Applying threshold
; VEC-ALWAYS: '#pragma vector always'/ '#pragma omp simd' is used for the given loop

entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]

  %float.ld.idx = getelementptr inbounds [1024 x float], [1024 x float]* @arr.float.1, i64 0, i64 %indvars.iv
  %float.ld = load float, float* %float.ld.idx
  %float2.ld.idx = getelementptr inbounds [1024 x float], [1024 x float]* @arr.float.2, i64 0, i64 %indvars.iv
  %float2.ld = load float, float* %float2.ld.idx

  %float.add = fadd fast float %float.ld, %float2.ld
  %float.add1 = fadd fast float %float.ld, %float.add
  %float.add2 = fadd fast float %float.ld, %float.add1
  %float.st.idx = getelementptr inbounds [1024 x float], [1024 x float]* @arr.float.3, i64 0, i64 %indvars.iv
  store float %float.add2, float* %float.st.idx

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
