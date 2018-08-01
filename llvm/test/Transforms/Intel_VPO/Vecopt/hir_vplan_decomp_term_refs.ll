; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-print-after-simplify-cfg -disable-output < %s 2>&1 | FileCheck %s

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

; CHECK: [[IVPhi:%.*]] = semi-phi
; CHECK: [[ALoad:%.*]] = load
; CHECK-NEXT: [[MulAF:%.*]] = mul [[ALoad]] {{%.*}}
; CHECK-NEXT: [[AddM6:%.*]] = add [[MulAF]] i32 6
; CHECK-NEXT: [[Div7:%.*]] = sdiv [[AddM6]] i32 7
; CHECK: [[Mul15F:%.*]] = mul i32 15 {{%.*}}
; CHECK-NEXT: [[MulMIV:%.*]] = mul [[Mul15F]] [[IVPhi]]
; CHECK-NEXT: [[Div2:%.*]] = sdiv [[MulMIV]] i32 2
; CHECK-NEXT: [[FinalAdd:%.*]] = add [[Div7]] [[Div2]]
; CHECK: store [[FinalAdd]]


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
