; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -disable-output 2>&1 | FileCheck %s

; Check that %p1 will not be merged with the rest of the chain as it has
; different extent information on dimension 0 (=200) than what we get by 
; dividing stride of subscript %p3 (dimension 1) by %p4 (dimension 0) which is
; 32 / 8 = 4. This is an attempt to deduce changes in array shape at the src
; level.

; Previously, we parsed the store as (%A)[0][0][10] and retained the extent
; information as 4. Such merging can cause out of range indices in the merged
; subscript index as 10 is outside the extent of 4.


; CHECK: + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK: |   %p1 = &((%A)[10]);
; CHECK: |   (%p1)[0][0][0] = 1.000000e+00;
; CHECK: + END LOOP

define void @foo(ptr %A) {
entry:
  br label %loop

loop:
  %i = phi i32 [0, %entry], [%ip, %loop]
  %p1 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr elementtype(double) nonnull %A, i64 10), !ifx.array_extent !0
  %p2 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 0, i64 128, ptr elementtype(double) nonnull %p1, i64 0)
  %p3 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 32, ptr elementtype(double) nonnull %p2, i64 0)
  %p4 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr elementtype(double) nonnull %p3, i64 0)
  store double 1.0, ptr %p4
  %ip = add nsw nuw i32 %i, 1
  %cmp = icmp ult i32 %ip, 2
  br i1 %cmp, label %loop, label %exit

exit:
  ret void
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)

!0 = !{i64 200}
