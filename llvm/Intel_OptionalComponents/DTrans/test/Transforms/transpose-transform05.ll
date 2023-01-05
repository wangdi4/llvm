; This test is to verify the IR modifications done by the Transpose transformation.

target triple = "x86_64-unknown-linux-gnu"

; This test is similar to transpose-transform01.ll, but uses the new form of
; uplevel type names.

; RUN: opt < %s -S -passes=dtrans-transpose -dtrans-transpose-override=test_var01,0,1 2>&1 | FileCheck %s

%TEST02.uplevel_type = type { { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* }

; Test to verify the stride field gets updated in the subscript call when
; swapping the dimensions on this 2 dimension array.
@test_var01 = internal global [9 x [9 x i32]] zeroinitializer
define void @test01() {
  %ptr_part1 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* elementtype(i32) getelementptr ([9 x [9 x i32]], [9 x [9 x i32]]* @test_var01, i64 0, i64 0, i64 0), i64 0)
  %ptr = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %ptr_part1, i64 0)
  store i32 0, i32* %ptr
  ret void
}
; CHECK: call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @test_var01, i64 0, i64 0, i64 0), i64 0)
; CHECK: call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 36, i32* elementtype(i32) %ptr_part1, i64 0)

; Test to verify the stride value gets updated in the creation of the dope when
; swapping the dimensions on the 2 dimension array.
define void @test02() {
  %"var$01" = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %"var$01_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 0
  %"var$01_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 1
  %"var$01_$field2$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 2
  %"var$01_$field3$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 3
  %"var$01_$field4$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 4
  %"var$01_$field6$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 6, i64 0
  %"var$01_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$01_$field6$", i64 0, i32 0
  %"var$01_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$01_$field6$", i64 0, i32 1
  %"var$01_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$01_$field6$", i64 0, i32 2
  store i64 4, i64* %"var$01_$field1$", align 8
  store i64 2, i64* %"var$01_$field4$", align 8
  store i64 0, i64* %"var$01_$field2$", align 8

  ; Populate rank 0 fields
  %t0 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field1$", i32 0)
  ; Store stride
  store i64 4, i64* %t0, align 8
  %t1 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field2$", i32 0)
  store i64 1, i64* %t1, align 8
  %t2 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field0$", i32 0)
  store i64 9, i64* %t2, align 8

  ; Populate rank 1 fields
  %t3 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field1$", i32 1)
  ; Store stride
  store i64 36, i64* %t3, align 8
  %t4 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field2$", i32 1)
  store i64 1, i64* %t4, align 8
  %t5 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field0$", i32 1)
  store i64 9, i64* %t5, align 8
  store i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @test_var01, i64 0, i64 0, i64 0), i32** %"var$01_$field0$", align 8

  store i64 1, i64* %"var$01_$field3$", align 8

  call void @test02dv({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$01")
  ret void
}
; CHECK: %t0 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field1$", i32 0)
; CHECK: store i64 36, i64* %t0, align 8
; CHECK: %t3 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field1$", i32 1)
; CHECK: store i64 4, i64* %t3, align 8

define void @test02dv({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %MYBLOCK) {
  %uplevel_rec = alloca %TEST02.uplevel_type, align 8

  ; Use the dope vector to access the array
  %"MYBLOCK_$field0$" = getelementptr { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %MYBLOCK, i64 0, i32 0
  %"MYBLOCK_$field6$_$field1$" = getelementptr { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %MYBLOCK, i64 0, i32 6, i64 0, i32 1
  %"MYBLOCK_$field0$1" = load i32*, i32** %"MYBLOCK_$field0$", align 8
  %t0 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"MYBLOCK_$field6$_$field1$", i32 0)
  %t1 = load i64, i64* %t0, align 8

  %t2 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"MYBLOCK_$field6$_$field1$", i32 1)
  %t3 = load i64, i64* %t2, align 8
  %test02dv_col_addr = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %t3, i32* elementtype(i32) %"MYBLOCK_$field0$1", i64 5)
  %test02dv_ptr_addr = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %t1, i32* elementtype(i32) %test02dv_col_addr, i64 5)
  store i32 5, i32* %test02dv_ptr_addr, align 4

  ; Store the dope vector into the uplevel var
  %ul_arg_0p.GEP = getelementptr inbounds %TEST02.uplevel_type, %TEST02.uplevel_type* %uplevel_rec, i64 0, i32 0
  store { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %MYBLOCK, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }** %ul_arg_0p.GEP, align 8
  call void @test02ul(%TEST02.uplevel_type* %uplevel_rec)

  ret void
}
; Check that rank parameter was updated.
;CHECK: %test02dv_col_addr = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %t3, i32* elementtype(i32) %"MYBLOCK_$field0$1", i64 5)
;CHECK: %test02dv_ptr_addr = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %t1, i32* elementtype(i32) %test02dv_col_addr, i64 5)

; This checks the usage of the dope vector that gets loaded from an uplevel
; variable.
define void @test02ul(%TEST02.uplevel_type* %UL_IN_PTR) {
  %ul_arg_0p.GEP = getelementptr inbounds %TEST02.uplevel_type, %TEST02.uplevel_type* %UL_IN_PTR, i64 0, i32 0
  %ul_arg_0.GEP = load { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }*, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }** %ul_arg_0p.GEP, align 8

  ; Load the address of the array variable.
  %"ul_arg_0.GEP_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %ul_arg_0.GEP, i64 0, i32 0
  %"ul_arg_0.GEP_$field0$30" = load i32*, i32** %"ul_arg_0.GEP_$field0$", align 8

  ; Load the stride for each dimension of the array
  %"ul_arg_0.GEP_$field6$_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %ul_arg_0.GEP, i64 0, i32 6, i64 0, i32 1
  %stride0_addr = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"ul_arg_0.GEP_$field6$_$field1$", i32 0)
  %stride1_addr = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"ul_arg_0.GEP_$field6$_$field1$", i32 1)
  %stride0 = load i64, i64* %stride0_addr, align 8
  %stride1 = load i64, i64* %stride1_addr, align 8

  ; Get the address of the element from the original array
  %test02ul_col_addr = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %stride1, i32* elementtype(i32) %"ul_arg_0.GEP_$field0$30", i64 0)
  %test02ul_ptr_addr = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %stride0, i32* elementtype(i32) %test02ul_col_addr, i64 0)
  store i32 0, i32* %test02ul_ptr_addr, align 4

  ret void
}
; Check that rank parameter was updated.
;CHECK: %test02ul_col_addr = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %stride1, i32* elementtype(i32) %"ul_arg_0.GEP_$field0$30", i64 0)
;CHECK: %test02ul_ptr_addr = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %stride0, i32* elementtype(i32) %test02ul_col_addr, i64 0)

declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32)
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64)
