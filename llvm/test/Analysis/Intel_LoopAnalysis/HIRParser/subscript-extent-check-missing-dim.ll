; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -disable-output 2>&1 | FileCheck %s

; Check that we are able to parse this case successfully.

; Previously, we compfailed as we tried to perform extent mismatch check for
; dimension 0. Since the subscript with dimension 1 is missing, we tried to
; dereference a null ptr. Now, we are able to successfully give up on the
; mismatch. The extent size is calculated by dividing stride of dimension 2
; by stride of dimension 0 = 32 /8 = 4.


; CHECK: + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK: |   %p1 = &((%A)[10]);
; CHECK: |   (%p1)[0][0] = 1.000000e+00;
; CHECK: + END LOOP

define void @foo(ptr %A) {
entry:
  br label %loop

loop:
  %i = phi i32 [0, %entry], [%ip, %loop]
  %p1 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr elementtype(double) nonnull %A, i64 10), !ifx.array_extent !0
  %p2 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 0, i64 32, ptr elementtype(double) nonnull %p1, i64 0)
  %p3 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr elementtype(double) nonnull %p2, i64 0)
  store double 1.0, ptr %p3
  %ip = add nsw nuw i32 %i, 1
  %cmp = icmp ult i32 %ip, 2
  br i1 %cmp, label %loop, label %exit

exit:
  ret void
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)

!0 = !{i64 200}
