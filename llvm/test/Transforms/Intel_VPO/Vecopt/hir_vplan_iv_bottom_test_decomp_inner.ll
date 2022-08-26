; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-print-after-plain-cfg -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -vplan-print-after-plain-cfg -disable-output < %s 2>&1 | FileCheck %s


; Verify that decomposer properly materializes the IV phi, IV Next and
; bottom test condition.

; CHECK: [[IVPhi:%.*]] = phi  [ i64 0, {{BB.*}} ],  [ i64 [[IVNext:%.*]], {{BB.*}} ]
; CHECK: [[IVNext]] = add i64 [[IVPhi]] i64 1
; CHECK: {{%.*}} = icmp slt i64 [[IVNext]] i64 1600

target triple = "x86_64-unknown-linux-gnu"

@a = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@b = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@c = common local_unnamed_addr global [1600 x i32] zeroinitializer, align 16


define void @foo() local_unnamed_addr {
entry:
  br label %omp.inner.for.body

omp.inner.for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %omp.inner.for.body ]
  %idx = getelementptr inbounds [1600 x i32], [1600 x i32]* @a, i64 0, i64 %iv
  %0 = load i32, i32* %idx
  %idx2 = getelementptr inbounds [1600 x i32], [1600 x i32]* @b, i64 0, i64 %iv
  %1 = load i32, i32* %idx2
  %add3 = add nsw i32 %1, %0
  %idx5 = getelementptr inbounds [1600 x i32], [1600 x i32]* @c, i64 0, i64 %iv
  store i32 %add3, i32* %idx5
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1600
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:
  ret void
}
