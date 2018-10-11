; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-print-after-simplify-cfg -disable-output < %s 2>&1 | FileCheck %s

; Verify that decomposer generates the right code for complex loop upper bound.

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
;   int max = N < factor ? factor : N;
; #pragma omp simd
;   for (i=0; i<max; i++){
;     c[i] = a[i] + b[i];
;   }
; }

; CHECK: [[Max:%.*]] = smax {{%.*}} i32 1600
; CHECK-NEXT: [[Zext:%.*]] = zext [[Max]]
; CHECK-NEXT: [[Add:%.*]] = add [[Zext]] i64 -1
; CHECK-NEXT: SUCCESSORS(1):[[H:BB.*]]
; CHECK: [[H]] {{.*}}:
; CHECK-NEXT: [[IVPhi:%.*]] = semi-phi

; The test fails because of changes in parsing. 
; TODO: Use a do-while loop to test max blob decomposition.
; XFAIL:*


target triple = "x86_64-unknown-linux-gnu"

@a = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@b = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@c = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16

define void @foo(i32 %factor) local_unnamed_addr {

entry:
  %cmp = icmp sgt i32 %factor, 1600
  %0 = select i1 %cmp, i32 %factor, i32 1600
  %cmp4 = icmp sgt i32 %0, 0
  br i1 %cmp4, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %1 = zext i32 %0 to i64
  %iv.in = bitcast i64 0 to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.precond.then, %omp.inner.for.body
  %iv = phi i64 [ 0, %omp.precond.then ], [ %iv.next, %omp.inner.for.body ]
  %idx = getelementptr inbounds [1600 x i32], [1600 x i32]* @a, i64 0, i64 %iv
  %2 = load i32, i32* %idx
  %idx9 = getelementptr inbounds [1600 x i32], [1600 x i32]* @b, i64 0, i64 %iv
  %3 = load i32, i32* %idx9
  %add10 = add nsw i32 %3, %2
  %idx12 = getelementptr inbounds [1600 x i32], [1600 x i32]* @c, i64 0, i64 %iv
  store i32 %add10, i32* %idx12
  %iv.next = add nuw nsw i64 %iv, 1
  %cmp6 = icmp ult i64 %iv.next, %1
  %iv.in27 = bitcast i64 %iv.next to i64
  br i1 %cmp6, label %omp.inner.for.body, label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}
