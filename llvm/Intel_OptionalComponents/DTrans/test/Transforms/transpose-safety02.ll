; This test verifies the safety analysis for the construction of dope vectors
; for cases supported by the transpose transformation.

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -disable-output -dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s

; In this case a dope vector is being created to represent the entire array.
; This should be a valid candidate for the transpose transformation.
@test_var01 = internal global [9 x [9 x i32]] zeroinitializer
define void @test01() {
  %"var$01" = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %"var$01_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 0
  %"var$01_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 1
  %"var$01_$field2$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 2
  %"var$01_$field3$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 3
  %"var$01_$field4$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 4

  ; This is form 1 of the pointer addressing to the Lower Bound, Stride, Extent
  ; addresses
  %"var$01_$field6$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 6, i64 0
  %"var$01_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$01_$field6$", i64 0, i32 0
  %"var$01_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$01_$field6$", i64 0, i32 1
  %"var$01_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$01_$field6$", i64 0, i32 2
  store i64 4, i64* %"var$01_$field1$", align 8
  store i64 2, i64* %"var$01_$field4$", align 8
  store i64 0, i64* %"var$01_$field2$", align 8
  %t0 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$01_$field6$_$field1$", i32 0)
  store i64 4, i64* %t0, align 8
  %t1 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$01_$field6$_$field2$", i32 0)
  store i64 1, i64* %t1, align 8
  %t2 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$01_$field6$_$field0$", i32 0)
  store i64 9, i64* %t2, align 8
  %t3 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$01_$field6$_$field1$", i32 1)
  store i64 36, i64* %t3, align 8
  %t4 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$01_$field6$_$field2$", i32 1)
  store i64 1, i64* %t4, align 8
  %t5 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$01_$field6$_$field0$", i32 1)
  store i64 9, i64* %t5, align 8
  store i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @test_var01, i64 0, i64 0, i64 0), i32** %"var$01_$field0$", align 8
  store i64 1, i64* %"var$01_$field3$", align 8
  ret void
}
; CHECK: Transpose candidate: test_var01
; CHECK: IsValid{{ *}}: true


; Same as test 1, but uses the form of GEP indexing that lowered the addressing
; for the Lower Bound, Stride and Extent pointers into a single GEP.
; This should be a valid candidate for the transpose transformation.
@test_var02 = internal global [9 x [9 x i32]] zeroinitializer
define void @test02() {
  %"var$02" = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %"var$02_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$02", i64 0, i32 0
  %"var$02_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$02", i64 0, i32 1
  %"var$02_$field2$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$02", i64 0, i32 2
  %"var$02_$field3$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$02", i64 0, i32 3
  %"var$02_$field4$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$02", i64 0, i32 4

  ; This is form 2 of the pointer addressing to the Lower Bound, Stride, Extent
  ; addresses
  %"var$02_$field6$_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$02", i64 0, i32 6, i64 0, i32 0
  %"var$02_$field6$_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$02", i64 0, i32 6, i64 0, i32 1
  %"var$02_$field6$_$field2$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$02", i64 0, i32 6, i64 0, i32 2
  store i64 4, i64* %"var$02_$field1$", align 8
  store i64 2, i64* %"var$02_$field4$", align 8
  store i64 0, i64* %"var$02_$field2$", align 8
  %t0 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$02_$field6$_$field1$", i32 0)
  store i64 4, i64* %t0, align 8
  %t1 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$02_$field6$_$field2$", i32 0)
  store i64 1, i64* %t1, align 8
  %t2 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$02_$field6$_$field0$", i32 0)
  store i64 9, i64* %t2, align 8
  %t3 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$02_$field6$_$field1$", i32 1)
  store i64 36, i64* %t3, align 8
  %t4 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$02_$field6$_$field2$", i32 1)
  store i64 1, i64* %t4, align 8
  %t5 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$02_$field6$_$field0$", i32 1)
  store i64 9, i64* %t5, align 8
  store i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @test_var02, i64 0, i64 0, i64 0), i32** %"var$02_$field0$", align 8
  store i64 1, i64* %"var$02_$field3$", align 8
  ret void
}
; CHECK: Transpose candidate: test_var02
; CHECK: IsValid{{ *}}: true


; In this case, a sub-object is being passed which skips every third element
; of the array for one of the dimensions.
; This case should be considered invalid.
@test_var03 = internal global [9 x [9 x i32]] zeroinitializer
define void @test03() {
  %"var$03" = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %"var$03_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$03", i64 0, i32 0
  %"var$03_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$03", i64 0, i32 1
  %"var$03_$field2$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$03", i64 0, i32 2
  %"var$03_$field3$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$03", i64 0, i32 3
  %"var$03_$field4$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$03", i64 0, i32 4
  %"var$03_$field6$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$03", i64 0, i32 6, i64 0
  %"var$03_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$03_$field6$", i64 0, i32 0
  %"var$03_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$03_$field6$", i64 0, i32 1
  %"var$03_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$03_$field6$", i64 0, i32 2
  store i64 4, i64* %"var$03_$field1$", align 8
  store i64 2, i64* %"var$03_$field4$", align 8
  store i64 0, i64* %"var$03_$field2$", align 8
  %t0 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$03_$field6$_$field1$", i32 0)
  store i64 4, i64* %t0, align 8
  %t1 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$03_$field6$_$field2$", i32 0)
  store i64 1, i64* %t1, align 8
  %t2 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$03_$field6$_$field0$", i32 0)
  store i64 9, i64* %t2, align 8
  %t3 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$03_$field6$_$field1$", i32 1)

  ; stride is every third element.
  store i64 108, i64* %t3, align 8
  %t4 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$03_$field6$_$field2$", i32 1)
  store i64 1, i64* %t4, align 8
  %t5 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$03_$field6$_$field0$", i32 1)
  store i64 9, i64* %t5, align 8
  store i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @test_var03, i64 0, i64 0, i64 0), i32** %"var$03_$field0$", align 8
  store i64 1, i64* %"var$03_$field3$", align 8
  ret void
}
; CHECK: Transpose candidate: test_var03
; CHECK: IsValid{{ *}}: false


; In this case, there are unsupported uses of the dope vector fields.
; This case should be considered invalid.
@test_var04 = internal global [9 x [9 x i32]] zeroinitializer
define void @test04() {
  %"var$04" = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %"var$04_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$04", i64 0, i32 0

  ; introduce an unsupported use of the dope vector field
  %bad_bitcast = bitcast i32** %"var$04_$field0$" to i8*

  %"var$04_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$04", i64 0, i32 1
  %"var$04_$field2$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$04", i64 0, i32 2
  %"var$04_$field3$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$04", i64 0, i32 3
  %"var$04_$field4$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$04", i64 0, i32 4

  ; This is form 1 of the pointer addressing to the Lower Bound, Stride, Extent
  ; addresses
  %"var$04_$field6$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$04", i64 0, i32 6, i64 0
  %"var$04_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$04_$field6$", i64 0, i32 0
  %"var$04_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$04_$field6$", i64 0, i32 1
  %"var$04_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$04_$field6$", i64 0, i32 2
  store i64 4, i64* %"var$04_$field1$", align 8
  store i64 2, i64* %"var$04_$field4$", align 8
  store i64 0, i64* %"var$04_$field2$", align 8
  %t0 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$04_$field6$_$field1$", i32 0)
  store i64 4, i64* %t0, align 8
  %t1 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$04_$field6$_$field2$", i32 0)
  store i64 1, i64* %t1, align 8
  %t2 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$04_$field6$_$field0$", i32 0)
  store i64 9, i64* %t2, align 8
  %t3 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$04_$field6$_$field1$", i32 1)
  store i64 36, i64* %t3, align 8
  %t4 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$04_$field6$_$field2$", i32 1)
  store i64 1, i64* %t4, align 8
  %t5 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$04_$field6$_$field0$", i32 1)
  store i64 9, i64* %t5, align 8
  store i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @test_var04, i64 0, i64 0, i64 0), i32** %"var$04_$field0$", align 8
  store i64 1, i64* %"var$04_$field3$", align 8
  ret void
}
; CHECK: Transpose candidate: test_var04
; CHECK: IsValid{{ *}}: false


; In this case, there are unsupported uses of the dope vector object.
; This case should be considered invalid.
@test_var05 = internal global [9 x [9 x i32]] zeroinitializer
define void @test05() {
  %"var$05" = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8

  ; Introduce an unsupported use of the dope vector object
  %var14ptr = bitcast { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$05" to i8*

  %"var$05_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$05", i64 0, i32 0
  %"var$05_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$05", i64 0, i32 1
  %"var$05_$field2$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$05", i64 0, i32 2
  %"var$05_$field3$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$05", i64 0, i32 3
  %"var$05_$field4$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$05", i64 0, i32 4

  ; This is form 1 of the pointer addressing to the Lower Bound, Stride, Extent
  ; addresses
  %"var$05_$field6$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$05", i64 0, i32 6, i64 0
  %"var$05_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$05_$field6$", i64 0, i32 0
  %"var$05_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$05_$field6$", i64 0, i32 1
  %"var$05_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$05_$field6$", i64 0, i32 2
  store i64 4, i64* %"var$05_$field1$", align 8
  store i64 2, i64* %"var$05_$field4$", align 8
  store i64 0, i64* %"var$05_$field2$", align 8
  %t0 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$05_$field6$_$field1$", i32 0)
  store i64 4, i64* %t0, align 8
  %t1 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$05_$field6$_$field2$", i32 0)
  store i64 1, i64* %t1, align 8
  %t2 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$05_$field6$_$field0$", i32 0)
  store i64 9, i64* %t2, align 8
  %t3 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$05_$field6$_$field1$", i32 1)
  store i64 36, i64* %t3, align 8
  %t4 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$05_$field6$_$field2$", i32 1)
  store i64 1, i64* %t4, align 8
  %t5 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$05_$field6$_$field0$", i32 1)
  store i64 9, i64* %t5, align 8
  store i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @test_var05, i64 0, i64 0, i64 0), i32** %"var$05_$field0$", align 8
  store i64 1, i64* %"var$05_$field3$", align 8
  ret void
}
; CHECK: Transpose candidate: test_var05
; CHECK: IsValid{{ *}}: false


; Verify that if all the fields needed to configure the dope vector ranks for
; the creation routine are not found, we disqualify the dope vector. Here we
; skip the extent field. (This field is allowed to be missing when we
; analyze the use of the dope vector in a called function, but not in the
; function creating the dope vector).
@test_var06 = internal global [9 x [9 x i32]] zeroinitializer
define void @test06() {
  %"var$06" = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %"var$06_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$06", i64 0, i32 0
  %"var$06_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$06", i64 0, i32 1
  %"var$06_$field2$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$06", i64 0, i32 2
  %"var$06_$field3$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$06", i64 0, i32 3
  %"var$06_$field4$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$06", i64 0, i32 4

  %"var$06_$field6$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$06", i64 0, i32 6, i64 0
  ; Skip extent field
  ;%"var$06_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$06_$field6$", i64 0, i32 0
  %"var$06_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$06_$field6$", i64 0, i32 1
  %"var$06_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$06_$field6$", i64 0, i32 2
  store i64 4, i64* %"var$06_$field1$", align 8
  store i64 2, i64* %"var$06_$field4$", align 8
  store i64 0, i64* %"var$06_$field2$", align 8
  %t0 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$06_$field6$_$field1$", i32 0)
  store i64 4, i64* %t0, align 8
  %t1 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$06_$field6$_$field2$", i32 0)
  store i64 1, i64* %t1, align 8
  ; Skip extent field
  ;%t2 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$06_$field6$_$field0$", i32 0)
  ;store i64 9, i64* %t2, align 8
  %t3 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$06_$field6$_$field1$", i32 1)
  store i64 36, i64* %t3, align 8
  %t4 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$06_$field6$_$field2$", i32 1)
  store i64 1, i64* %t4, align 8
  ; Skip extent field
  ;%t5 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$06_$field6$_$field0$", i32 1)
  ;store i64 9, i64* %t5, align 8
  store i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @test_var06, i64 0, i64 0, i64 0), i32** %"var$06_$field0$", align 8
  store i64 1, i64* %"var$06_$field3$", align 8
  ret void
}
; CHECK: Transpose candidate: test_var06
; CHECK: IsValid{{ *}}: false


; In this case, a dope vector field is written more than once. This should not
; happen in normal operation. Verify that it gets rejected.
@test_var07 = internal global [9 x [9 x i32]] zeroinitializer
define void @test07() {
  %"var$07" = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %"var$07_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$07", i64 0, i32 0
  %"var$07_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$07", i64 0, i32 1
  %"var$07_$field2$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$07", i64 0, i32 2
  %"var$07_$field3$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$07", i64 0, i32 3
  %"var$07_$field4$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$07", i64 0, i32 4
  %"var$07_$field6$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$07", i64 0, i32 6, i64 0
  %"var$07_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$07_$field6$", i64 0, i32 0
  %"var$07_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$07_$field6$", i64 0, i32 1
  %"var$07_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$07_$field6$", i64 0, i32 2
  store i64 4, i64* %"var$07_$field1$", align 8
  store i64 2, i64* %"var$07_$field4$", align 8
  store i64 0, i64* %"var$07_$field2$", align 8
  %t0 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$07_$field6$_$field1$", i32 0)
  store i64 4, i64* %t0, align 8
  %t1 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$07_$field6$_$field2$", i32 0)

  ; Introduce two writes to the field to trigger it being rejected.
  store i64 2, i64* %t1, align 8
  store i64 1, i64* %t1, align 8

  %t2 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$07_$field6$_$field0$", i32 0)
  store i64 9, i64* %t2, align 8
  %t3 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$07_$field6$_$field1$", i32 1)
  store i64 36, i64* %t3, align 8
  %t4 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$07_$field6$_$field2$", i32 1)
  store i64 1, i64* %t4, align 8
  %t5 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$07_$field6$_$field0$", i32 1)
  store i64 9, i64* %t5, align 8
  store i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @test_var07, i64 0, i64 0, i64 0), i32** %"var$07_$field0$", align 8
  store i64 1, i64* %"var$07_$field3$", align 8
  ret void
}
; CHECK: Transpose candidate: test_var07
; CHECK: IsValid{{ *}}: false


; In this case, the address of one of the dope vector fields gets stored to
; another memory location. This should disqualify the candidate.
@test_var08 = internal global [9 x [9 x i32]] zeroinitializer
define void @test08() {
  %"var$08" = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %escape = alloca i32**
  %"var$08_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$08", i64 0, i32 0
  %"var$08_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$08", i64 0, i32 1
  %"var$08_$field2$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$08", i64 0, i32 2
  %"var$08_$field3$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$08", i64 0, i32 3
  %"var$08_$field4$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$08", i64 0, i32 4
  %"var$08_$field6$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$08", i64 0, i32 6, i64 0
  %"var$08_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$08_$field6$", i64 0, i32 0
  %"var$08_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$08_$field6$", i64 0, i32 1
  %"var$08_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$08_$field6$", i64 0, i32 2
  store i64 4, i64* %"var$08_$field1$", align 8
  store i64 2, i64* %"var$08_$field4$", align 8
  store i64 0, i64* %"var$08_$field2$", align 8
  %t0 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$08_$field6$_$field1$", i32 0)
  store i64 4, i64* %t0, align 8
  %t1 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$08_$field6$_$field2$", i32 0)
  store i64 1, i64* %t1, align 8
  %t2 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$08_$field6$_$field0$", i32 0)
  store i64 9, i64* %t2, align 8
  %t3 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$08_$field6$_$field1$", i32 1)
  store i64 36, i64* %t3, align 8
  %t4 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$08_$field6$_$field2$", i32 1)
  store i64 1, i64* %t4, align 8
  %t5 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$08_$field6$_$field0$", i32 1)
  store i64 9, i64* %t5, align 8
  store i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @test_var08, i64 0, i64 0, i64 0), i32** %"var$08_$field0$", align 8
  store i64 1, i64* %"var$08_$field3$", align 8

  ; Store the field to another memory location.
  store i32** %"var$08_$field0$", i32*** %escape
  ret void
}
; CHECK: Transpose candidate: test_var08
; CHECK: IsValid{{ *}}: false


 ; Args: Rank, Lower Bound, Unit Stride (in bytes), Ptr, Element Index
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32)
