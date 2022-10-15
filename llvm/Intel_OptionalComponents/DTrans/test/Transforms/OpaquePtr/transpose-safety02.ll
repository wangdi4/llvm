; This test verifies the safety analysis for the construction of dope vectors
; for cases supported by the transpose transformation.

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers < %s -disable-output -dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers < %s -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s

@test_var01 = internal global [9 x [9 x i32]] zeroinitializer
@test_var02 = internal global [9 x [9 x i32]] zeroinitializer
@test_var03 = internal global [9 x [9 x i32]] zeroinitializer
@test_var04 = internal global [9 x [9 x i32]] zeroinitializer
@test_var05 = internal global [9 x [9 x i32]] zeroinitializer
@test_var06 = internal global [9 x [9 x i32]] zeroinitializer
@test_var07 = internal global [9 x [9 x i32]] zeroinitializer
@test_var08 = internal global [9 x [9 x i32]] zeroinitializer


; In this case a dope vector is being created to represent the entire array.
; This should be a valid candidate for the transpose transformation.
define void @test01() {
bb:
  %"var$01" = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %"var$01_$field0$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$01", i64 0, i32 0
  %"var$01_$field1$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$01", i64 0, i32 1
  %"var$01_$field2$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$01", i64 0, i32 2
  %"var$01_$field3$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$01", i64 0, i32 3
  %"var$01_$field4$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$01", i64 0, i32 4
  %"var$01_$field6$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$01", i64 0, i32 6, i64 0
  %"var$01_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$01_$field6$", i64 0, i32 0
  %"var$01_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$01_$field6$", i64 0, i32 1
  %"var$01_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$01_$field6$", i64 0, i32 2
  store i64 4, ptr %"var$01_$field1$", align 8
  store i64 2, ptr %"var$01_$field4$", align 8
  store i64 0, ptr %"var$01_$field2$", align 8
  %t0 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$01_$field6$_$field1$", i32 0)
  store i64 4, ptr %t0, align 8
  %t1 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$01_$field6$_$field2$", i32 0)
  store i64 1, ptr %t1, align 8
  %t2 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$01_$field6$_$field0$", i32 0)
  store i64 9, ptr %t2, align 8
  %t3 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$01_$field6$_$field1$", i32 1)
  store i64 36, ptr %t3, align 8
  %t4 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$01_$field6$_$field2$", i32 1)
  store i64 1, ptr %t4, align 8
  %t5 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$01_$field6$_$field0$", i32 1)
  store i64 9, ptr %t5, align 8
  store ptr @test_var01, ptr %"var$01_$field0$", align 8
  store i64 1, ptr %"var$01_$field3$", align 8
  ret void
}
; CHECK: Transpose candidate: test_var01
; CHECK: IsValid{{ *}}: true

; Same as test 1, but uses the form of GEP indexing that lowered the addressing
; for the Lower Bound, Stride and Extent pointers into a single GEP.
; This should be a valid candidate for the transpose transformation.
define void @test02() {
bb:
  %"var$02" = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %"var$02_$field0$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 0
  %"var$02_$field1$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 1
  %"var$02_$field2$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 2
  %"var$02_$field3$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 3
  %"var$02_$field4$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 4
  %"var$02_$field6$_$field0$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 6, i64 0, i32 0
  %"var$02_$field6$_$field1$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 6, i64 0, i32 1
  %"var$02_$field6$_$field2$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 6, i64 0, i32 2
  store i64 4, ptr %"var$02_$field1$", align 8
  store i64 2, ptr %"var$02_$field4$", align 8
  store i64 0, ptr %"var$02_$field2$", align 8
  %t0 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$02_$field6$_$field1$", i32 0)
  store i64 4, ptr %t0, align 8
  %t1 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$02_$field6$_$field2$", i32 0)
  store i64 1, ptr %t1, align 8
  %t2 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$02_$field6$_$field0$", i32 0)
  store i64 9, ptr %t2, align 8
  %t3 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$02_$field6$_$field1$", i32 1)
  store i64 36, ptr %t3, align 8
  %t4 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$02_$field6$_$field2$", i32 1)
  store i64 1, ptr %t4, align 8
  %t5 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$02_$field6$_$field0$", i32 1)
  store i64 9, ptr %t5, align 8
  store ptr @test_var02, ptr %"var$02_$field0$", align 8
  store i64 1, ptr %"var$02_$field3$", align 8
  ret void
}
; CHECK: Transpose candidate: test_var02
; CHECK: IsValid{{ *}}: true

; In this case, a sub-object is being passed which skips every third element
; of the array for one of the dimensions.
; This case should be considered invalid.
define void @test03() {
bb:
  %"var$03" = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %"var$03_$field0$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$03", i64 0, i32 0
  %"var$03_$field1$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$03", i64 0, i32 1
  %"var$03_$field2$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$03", i64 0, i32 2
  %"var$03_$field3$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$03", i64 0, i32 3
  %"var$03_$field4$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$03", i64 0, i32 4
  %"var$03_$field6$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$03", i64 0, i32 6, i64 0
  %"var$03_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$03_$field6$", i64 0, i32 0
  %"var$03_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$03_$field6$", i64 0, i32 1
  %"var$03_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$03_$field6$", i64 0, i32 2
  store i64 4, ptr %"var$03_$field1$", align 8
  store i64 2, ptr %"var$03_$field4$", align 8
  store i64 0, ptr %"var$03_$field2$", align 8
  %t0 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$03_$field6$_$field1$", i32 0)
  store i64 4, ptr %t0, align 8
  %t1 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$03_$field6$_$field2$", i32 0)
  store i64 1, ptr %t1, align 8
  %t2 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$03_$field6$_$field0$", i32 0)
  store i64 9, ptr %t2, align 8
  %t3 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$03_$field6$_$field1$", i32 1)
  store i64 108, ptr %t3, align 8
  %t4 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$03_$field6$_$field2$", i32 1)
  store i64 1, ptr %t4, align 8
  %t5 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$03_$field6$_$field0$", i32 1)
  store i64 9, ptr %t5, align 8
  store ptr @test_var03, ptr %"var$03_$field0$", align 8
  store i64 1, ptr %"var$03_$field3$", align 8
  ret void
}
; CHECK: Transpose candidate: test_var03
; CHECK: IsValid{{ *}}: false

; In this case, there are unsupported uses of the dope vector fields.
; This case should be considered invalid.
define void @test04() {
bb:
  %"var$04" = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %"var$04_$field0$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$04", i64 0, i32 0
  %bad_bitcast = bitcast ptr %"var$04_$field0$" to ptr
  %"var$04_$field1$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$04", i64 0, i32 1
  %"var$04_$field2$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$04", i64 0, i32 2
  %"var$04_$field3$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$04", i64 0, i32 3
  %"var$04_$field4$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$04", i64 0, i32 4
  %"var$04_$field6$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$04", i64 0, i32 6, i64 0
  %"var$04_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$04_$field6$", i64 0, i32 0
  %"var$04_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$04_$field6$", i64 0, i32 1
  %"var$04_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$04_$field6$", i64 0, i32 2
  store i64 4, ptr %"var$04_$field1$", align 8
  store i64 2, ptr %"var$04_$field4$", align 8
  store i64 0, ptr %"var$04_$field2$", align 8
  %t0 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$04_$field6$_$field1$", i32 0)
  store i64 4, ptr %t0, align 8
  %t1 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$04_$field6$_$field2$", i32 0)
  store i64 1, ptr %t1, align 8
  %t2 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$04_$field6$_$field0$", i32 0)
  store i64 9, ptr %t2, align 8
  %t3 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$04_$field6$_$field1$", i32 1)
  store i64 36, ptr %t3, align 8
  %t4 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$04_$field6$_$field2$", i32 1)
  store i64 1, ptr %t4, align 8
  %t5 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$04_$field6$_$field0$", i32 1)
  store i64 9, ptr %t5, align 8
  store ptr @test_var04, ptr %"var$04_$field0$", align 8
  store i64 1, ptr %"var$04_$field3$", align 8
  ret void
}
; CHECK: Transpose candidate: test_var04
; CHECK: IsValid{{ *}}: false

; In this case, there are unsupported uses of the dope vector object.
; This case should be considered invalid.
define void @test05() {
bb:
  %"var$05" = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %var14ptr = bitcast ptr %"var$05" to ptr
  %"var$05_$field0$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$05", i64 0, i32 0
  %"var$05_$field1$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$05", i64 0, i32 1
  %"var$05_$field2$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$05", i64 0, i32 2
  %"var$05_$field3$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$05", i64 0, i32 3
  %"var$05_$field4$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$05", i64 0, i32 4
  %"var$05_$field6$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$05", i64 0, i32 6, i64 0
  %"var$05_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$05_$field6$", i64 0, i32 0
  %"var$05_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$05_$field6$", i64 0, i32 1
  %"var$05_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$05_$field6$", i64 0, i32 2
  store i64 4, ptr %"var$05_$field1$", align 8
  store i64 2, ptr %"var$05_$field4$", align 8
  store i64 0, ptr %"var$05_$field2$", align 8
  %t0 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$05_$field6$_$field1$", i32 0)
  store i64 4, ptr %t0, align 8
  %t1 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$05_$field6$_$field2$", i32 0)
  store i64 1, ptr %t1, align 8
  %t2 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$05_$field6$_$field0$", i32 0)
  store i64 9, ptr %t2, align 8
  %t3 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$05_$field6$_$field1$", i32 1)
  store i64 36, ptr %t3, align 8
  %t4 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$05_$field6$_$field2$", i32 1)
  store i64 1, ptr %t4, align 8
  %t5 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$05_$field6$_$field0$", i32 1)
  store i64 9, ptr %t5, align 8
  store ptr @test_var05, ptr %"var$05_$field0$", align 8
  store i64 1, ptr %"var$05_$field3$", align 8
  ret void
}
; CHECK: Transpose candidate: test_var05
; CHECK: IsValid{{ *}}: false

; Verify that if all the fields needed to configure the dope vector ranks for
; the creation routine are not found, we disqualify the dope vector. Here we
; skip the extent field. (This field is allowed to be missing when we
; analyze the use of the dope vector in a called function, but not in the
; function creating the dope vector).
define void @test06() {
bb:
  %"var$06" = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %"var$06_$field0$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$06", i64 0, i32 0
  %"var$06_$field1$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$06", i64 0, i32 1
  %"var$06_$field2$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$06", i64 0, i32 2
  %"var$06_$field3$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$06", i64 0, i32 3
  %"var$06_$field4$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$06", i64 0, i32 4
  %"var$06_$field6$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$06", i64 0, i32 6, i64 0
  %"var$06_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$06_$field6$", i64 0, i32 1
  %"var$06_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$06_$field6$", i64 0, i32 2
  store i64 4, ptr %"var$06_$field1$", align 8
  store i64 2, ptr %"var$06_$field4$", align 8
  store i64 0, ptr %"var$06_$field2$", align 8
  %t0 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$06_$field6$_$field1$", i32 0)
  store i64 4, ptr %t0, align 8
  %t1 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$06_$field6$_$field2$", i32 0)
  store i64 1, ptr %t1, align 8
  %t3 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$06_$field6$_$field1$", i32 1)
  store i64 36, ptr %t3, align 8
  %t4 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$06_$field6$_$field2$", i32 1)
  store i64 1, ptr %t4, align 8
  store ptr @test_var06, ptr %"var$06_$field0$", align 8
  store i64 1, ptr %"var$06_$field3$", align 8
  ret void
}
; CHECK: Transpose candidate: test_var06
; CHECK: IsValid{{ *}}: false

; In this case, a dope vector field is written more than once. This should not
; happen in normal operation. Verify that it gets rejected.
define void @test07() {
bb:
  %"var$07" = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %"var$07_$field0$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$07", i64 0, i32 0
  %"var$07_$field1$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$07", i64 0, i32 1
  %"var$07_$field2$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$07", i64 0, i32 2
  %"var$07_$field3$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$07", i64 0, i32 3
  %"var$07_$field4$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$07", i64 0, i32 4
  %"var$07_$field6$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$07", i64 0, i32 6, i64 0
  %"var$07_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$07_$field6$", i64 0, i32 0
  %"var$07_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$07_$field6$", i64 0, i32 1
  %"var$07_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$07_$field6$", i64 0, i32 2
  store i64 4, ptr %"var$07_$field1$", align 8
  store i64 2, ptr %"var$07_$field4$", align 8
  store i64 0, ptr %"var$07_$field2$", align 8
  %t0 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$07_$field6$_$field1$", i32 0)
  store i64 4, ptr %t0, align 8
  %t1 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$07_$field6$_$field2$", i32 0)
  store i64 2, ptr %t1, align 8
  store i64 1, ptr %t1, align 8
  %t2 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$07_$field6$_$field0$", i32 0)
  store i64 9, ptr %t2, align 8
  %t3 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$07_$field6$_$field1$", i32 1)
  store i64 36, ptr %t3, align 8
  %t4 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$07_$field6$_$field2$", i32 1)
  store i64 1, ptr %t4, align 8
  %t5 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$07_$field6$_$field0$", i32 1)
  store i64 9, ptr %t5, align 8
  store ptr @test_var07, ptr %"var$07_$field0$", align 8
  store i64 1, ptr %"var$07_$field3$", align 8
  ret void
}
; CHECK: Transpose candidate: test_var07
; CHECK: IsValid{{ *}}: false

; In this case, the address of one of the dope vector fields gets stored to
; another memory location. This should disqualify the candidate.
define void @test08() {
bb:
  %"var$08" = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %escape = alloca ptr, align 8
  %"var$08_$field0$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$08", i64 0, i32 0
  %"var$08_$field1$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$08", i64 0, i32 1
  %"var$08_$field2$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$08", i64 0, i32 2
  %"var$08_$field3$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$08", i64 0, i32 3
  %"var$08_$field4$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$08", i64 0, i32 4
  %"var$08_$field6$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$08", i64 0, i32 6, i64 0
  %"var$08_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$08_$field6$", i64 0, i32 0
  %"var$08_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$08_$field6$", i64 0, i32 1
  %"var$08_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$08_$field6$", i64 0, i32 2
  store i64 4, ptr %"var$08_$field1$", align 8
  store i64 2, ptr %"var$08_$field4$", align 8
  store i64 0, ptr %"var$08_$field2$", align 8
  %t0 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$08_$field6$_$field1$", i32 0)
  store i64 4, ptr %t0, align 8
  %t1 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$08_$field6$_$field2$", i32 0)
  store i64 1, ptr %t1, align 8
  %t2 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$08_$field6$_$field0$", i32 0)
  store i64 9, ptr %t2, align 8
  %t3 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$08_$field6$_$field1$", i32 1)
  store i64 36, ptr %t3, align 8
  %t4 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$08_$field6$_$field2$", i32 1)
  store i64 1, ptr %t4, align 8
  %t5 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$08_$field6$_$field0$", i32 1)
  store i64 9, ptr %t5, align 8
  store ptr @test_var08, ptr %"var$08_$field0$", align 8
  store i64 1, ptr %"var$08_$field3$", align 8
  store ptr %"var$08_$field0$", ptr %escape, align 8
  ret void
}
; CHECK: Transpose candidate: test_var08
; CHECK: IsValid{{ *}}: false

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #0

attributes #0 = { nounwind readnone speculatable }
