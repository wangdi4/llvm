; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-print-after-plain-cfg -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -vplan-print-after-plain-cfg -disable-output < %s 2>&1 | FileCheck %s


; Verify that decomposer generates the right code for CanonExprs from
; terminal RegDDRefs.

; Source code
; #define N 1600
;
; int a[N];
; int b[N];
; int c[N];
;
; void foo(int factor) {
;
;   int i;
; #pragma omp simd
;   for (i=0; i<N; i++){
;     c[i] = (a[i]*factor+6)/7 + (factor*i*15)/2;
;   }
; }

; CHECK: [[IVPhi:%.*]] = phi
; CHECK: [[ALoad:%.*]] = load i32
; CHECK-NEXT: [[MulAF:%.*]] = mul i32 [[ALoad]] i32 {{%.*}}
; CHECK-NEXT: [[AddM6:%.*]] = add i32 [[MulAF]] i32 6
; CHECK-NEXT: [[Div7:%.*]] = sdiv i32 [[AddM6]] i32 7
; CHECK: [[Mul15F:%.*]] = mul i32 15 i32 {{%.*}}
; CHECK-NEXT: [[Trunc:%.*]] = trunc i64 [[IVPhi]]
; CHECK-NEXT: [[MulMIV:%.*]] = mul i32 [[Mul15F]] i32 [[Trunc]]
; CHECK-NEXT: [[Div2:%.*]] = sdiv i32 [[MulMIV]] i32 2
; CHECK-NEXT: [[FinalAdd:%.*]] = add i32 [[Div7]] i32 [[Div2]]
; CHECK: store i32 [[FinalAdd]]


target triple = "x86_64-unknown-linux-gnu"

@a = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@c = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@b = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16

define void @foo(i32 %factor) local_unnamed_addr {
entry:
  %mul3 = mul i32 %factor, 15
  br label %omp.inner.for.body

omp.inner.for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds [1600 x i32], [1600 x i32]* @a, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx
  %mul1 = mul nsw i32 %0, %factor
  %add2 = add nsw i32 %mul1, 6
  %div = sdiv i32 %add2, 7
  %1 = trunc i64 %indvars.iv to i32
  %mul4 = mul i32 %mul3, %1
  %div5 = sdiv i32 %mul4, 2
  %add6 = add nsw i32 %div, %div5
  %arrayidx8 = getelementptr inbounds [1600 x i32], [1600 x i32]* @c, i64 0, i64 %indvars.iv
  store i32 %add6, i32* %arrayidx8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1600
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:
  ret void
}
