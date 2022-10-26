; This test verifies the safety analysis for the usage of values
; loaded from a dope vector in called functions.

target triple = "x86_64-unknown-linux-gnu"

; Run the test with all uses of the dope vector being supported.
; RUN: opt < %s -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s


; This case is to verify that an unsupported use of the array pointer loaded
; from the dope vector field causes the candidate to be rejected.
@test_var01 = internal global [9 x [9 x i32]] zeroinitializer

; This routine is just for setting up the dope vector, and passing it to a function.
define void @test01() {
  %"var$14" = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %"var$14_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i64 0, i32 0
  %"var$14_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i64 0, i32 1
  %"var$14_$field2$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i64 0, i32 2
  %"var$14_$field3$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i64 0, i32 3
  %"var$14_$field4$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i64 0, i32 4

  ; This is form 1 of the pointer addressing to the Lower Bound, Stride, Extent
  ; addresses
  %"var$14_$field6$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i64 0, i32 6, i64 0
  %"var$14_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$14_$field6$", i64 0, i32 0
  %"var$14_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$14_$field6$", i64 0, i32 1
  %"var$14_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$14_$field6$", i64 0, i32 2
  store i64 4, i64* %"var$14_$field1$", align 8
  store i64 2, i64* %"var$14_$field4$", align 8
  store i64 0, i64* %"var$14_$field2$", align 8
  %t0 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$14_$field6$_$field1$", i32 0)
  store i64 4, i64* %t0, align 8
  %t1 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$14_$field6$_$field2$", i32 0)
  store i64 1, i64* %t1, align 8
  %t2 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$14_$field6$_$field0$", i32 0)
  store i64 9, i64* %t2, align 8
  %t3 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$14_$field6$_$field1$", i32 1)
  store i64 36, i64* %t3, align 8
  %t4 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$14_$field6$_$field2$", i32 1)
  store i64 1, i64* %t4, align 8
  %t5 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$14_$field6$_$field0$", i32 1)
  store i64 9, i64* %t5, align 8
  store i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @test_var01, i64 0, i64 0, i64 0), i32** %"var$14_$field0$", align 8
  store i64 1, i64* %"var$14_$field3$", align 8

  call void @test01dv({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14")
  ret void
}

define void @test01dv({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* noalias nocapture readonly %MYBLOCK) {
  %"MYBLOCK_$field0$" = getelementptr { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %MYBLOCK, i64 0, i32 0
  %"MYBLOCK_$field0$1" = load i32*, i32** %"MYBLOCK_$field0$", align 8

  ; Pass the address of the array directly to some function.
  call void @test01escape(i32* %"MYBLOCK_$field0$1")
  ret void
}

define void @test01escape(i32* %ptr) {
  ret void
}
; CHECK: Transpose candidate: test_var01
; CHECK: IsValid{{ *}}: false


; This case is to verify the checks that the stride value used was loaded
; from the dope vector. In this test, a constant stride is used, which
; should cause the candidate to be rejected.
@test_var02 = internal global [9 x [9 x i32]] zeroinitializer

; This routine is just for setting up the dope vector, and passing it to a function.
define void @test02() {
  %"var$14" = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %"var$14_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i64 0, i32 0
  %"var$14_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i64 0, i32 1
  %"var$14_$field2$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i64 0, i32 2
  %"var$14_$field3$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i64 0, i32 3
  %"var$14_$field4$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i64 0, i32 4

  ; This is form 1 of the pointer addressing to the Lower Bound, Stride, Extent
  ; addresses
  %"var$14_$field6$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14", i64 0, i32 6, i64 0
  %"var$14_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$14_$field6$", i64 0, i32 0
  %"var$14_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$14_$field6$", i64 0, i32 1
  %"var$14_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$14_$field6$", i64 0, i32 2
  store i64 4, i64* %"var$14_$field1$", align 8
  store i64 2, i64* %"var$14_$field4$", align 8
  store i64 0, i64* %"var$14_$field2$", align 8
  %t0 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$14_$field6$_$field1$", i32 0)
  store i64 4, i64* %t0, align 8
  %t1 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$14_$field6$_$field2$", i32 0)
  store i64 1, i64* %t1, align 8
  %t2 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$14_$field6$_$field0$", i32 0)
  store i64 9, i64* %t2, align 8
  %t3 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$14_$field6$_$field1$", i32 1)
  store i64 36, i64* %t3, align 8
  %t4 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$14_$field6$_$field2$", i32 1)
  store i64 1, i64* %t4, align 8
  %t5 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$14_$field6$_$field0$", i32 1)
  store i64 9, i64* %t5, align 8
  store i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @test_var02, i64 0, i64 0, i64 0), i32** %"var$14_$field0$", align 8
  store i64 1, i64* %"var$14_$field3$", align 8

  call void @test02dv({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$14")
  ret void
}

define void @test02dv({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* noalias nocapture readonly %MYBLOCK) {
  %"MYBLOCK_$field0$" = getelementptr { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %MYBLOCK, i64 0, i32 0
  %"MYBLOCK_$field0$1" = load i32*, i32** %"MYBLOCK_$field0$", align 8

  ; Use stride values as constants, rather than from dope vector.
  %t4 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* elementtype(i32) %"MYBLOCK_$field0$1", i64 5)
  %t5 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %t4, i64 5)
  store i32 5, i32* %t5, align 4
  ret void
}
; CHECK: Transpose candidate: test_var02
; CHECK: IsValid{{ *}}: false


 ; Args: Rank, Lower Bound, Unit Stride (in bytes), Ptr, Element Index
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32)
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64)
