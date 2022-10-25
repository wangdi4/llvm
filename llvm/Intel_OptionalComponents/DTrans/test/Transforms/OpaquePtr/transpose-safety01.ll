; This test verifies the analysis of direct uses of a candidate variable for
; the transpose transformation.

target triple = "x86_64-unknown-linux-gnu"

; This test also tests the internal option -dtrans-transpose-min-dim

; RUN: opt -opaque-pointers < %s -disable-output -dtrans-transpose -dtrans-transpose-min-dim=3 -dtrans-transpose-print-candidates 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers < %s -disable-output -passes=dtrans-transpose -dtrans-transpose-min-dim=3 -dtrans-transpose-print-candidates 2>&1 | FileCheck %s

@test_var1 = internal global [3 x [3 x [3 x i32]]] zeroinitializer
@test_var2 = internal global [3 x [3 x [3 x i32]]] zeroinitializer
@test_var3 = internal global [9 x [9 x i32]] zeroinitializer
@test_var4 = internal global [9 x [9 x i32]] zeroinitializer
@test_var5 = internal global [9 x [9 x [9 x i32]]] zeroinitializer

; This test verifies that an array that is directly accessed (such as a
; C array), rather than the expected format of the subscript operations
; used for Fortran, is rejected.
define void @test01() {
bb:
  %arrayIdx = getelementptr [3 x [3 x [3 x i32]]], ptr @test_var1, i64 0, i64 0, i64 0, i64 0
  %gepload = load i32, ptr %arrayIdx, align 4
  ret void
}
; CHECK: Transpose candidate: test_var1
; CHECK: IsValid{{ *}}: false

; This test verifies that an array element address that is not via the
; subscript intrinsic is rejected.
define void @test02(i64 %row, i64 %col) {
bb:
  %arrayIdx = getelementptr [3 x [3 x [3 x i32]]], ptr @test_var2, i64 0, i64 0, i64 %row, i64 %col
  ret void
}
; CHECK: Transpose candidate: test_var2
; CHECK: IsValid{{ *}}: false

; This test verifies that if the stride is not the expected value for
; the subscript call, the candidate is rejected.
define void @test03() {
bb:
  %ptr_part1 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 72, ptr elementtype(i32) @test_var3, i64 0)
  %ptr = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(i32) %ptr_part1, i64 0)
  store i32 0, ptr %ptr, align 4
  ret void
}
; CHECK: Transpose candidate: test_var3
; CHECK: IsValid{{ *}}: false

; This test verifies that if the lower bound does not start at 1, we reject
; the candidate. This may be more conservative than necessary, but for now
; we enforce that the lower bound is 1.
define void @test04() {
bb:
  %ptr_part1 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) @test_var4, i64 0)
  %ptr = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 4, i64 4, ptr elementtype(i32) %ptr_part1, i64 0)
  store i32 0, ptr %ptr, align 4
  ret void
}
; CHECK: Transpose candidate: test_var4
; CHECK: IsValid{{ *}}: false

; This test verifies that use of the expected lower bound and stride values pass
; the validation test.
define void @test05() {
bb:
  %ptr_part1 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 324, ptr elementtype(i32) @test_var5, i64 0)
  %ptr_part2 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %ptr_part1, i64 0)
  %ptr = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %ptr_part2, i64 0)
  store i32 0, ptr %ptr, align 4
  ret void
}
; CHECK: Transpose candidate: test_var5
; CHECK: IsValid{{ *}}: true

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { nounwind readnone speculatable }
