; This test is to verify the IR modifications done by the Transpose transformation.

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers < %s -S -passes=dtrans-transpose -dtrans-transpose-override=test_var01,0,1 2>&1 | FileCheck %s

%uplevel_type = type { ptr }

@test_var01 = internal global [9 x [9 x i32]] zeroinitializer

; Test to verify the stride field gets updated in the subscript call when
; swapping the dimensions on this 2 dimension array.
define void @test01() {
bb:
  %ptr_part1 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) @test_var01, i64 0)
  %ptr = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %ptr_part1, i64 0)
  store i32 0, ptr %ptr, align 4
  ret void
}
; CHECK: call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) @test_var01, i64 0)
; CHECK: call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 36, ptr elementtype(i32) %ptr_part1, i64 0)

; Test to verify the stride value gets updated in the creation of the dope when
; swapping the dimensions on the 2 dimension array.
define void @test02() {
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
  call void @test02dv(ptr %"var$01")
  ret void
}
; CHECK: %t0 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$01_$field6$_$field1$", i32 0)
; CHECK: store i64 36, ptr %t0, align 8
; CHECK: %t3 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$01_$field6$_$field1$", i32 1)
; CHECK: store i64 4, ptr %t3, align 8

define void @test02dv(ptr %MYBLOCK) {
bb:
  %uplevel_rec = alloca %uplevel_type, align 8
  %"MYBLOCK_$field0$" = getelementptr { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %MYBLOCK, i64 0, i32 0
  %"MYBLOCK_$field6$_$field1$" = getelementptr { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %MYBLOCK, i64 0, i32 6, i64 0, i32 1
  %"MYBLOCK_$field0$1" = load ptr, ptr %"MYBLOCK_$field0$", align 8
  %t0 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"MYBLOCK_$field6$_$field1$", i32 0)
  %t1 = load i64, ptr %t0, align 8
  %t2 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"MYBLOCK_$field6$_$field1$", i32 1)
  %t3 = load i64, ptr %t2, align 8
  %test02dv_col_addr = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %t3, ptr elementtype(i32) %"MYBLOCK_$field0$1", i64 5)
  %test02dv_ptr_addr = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %t1, ptr elementtype(i32) %test02dv_col_addr, i64 5)
  store i32 5, ptr %test02dv_ptr_addr, align 4
  %ul_arg_0p.GEP = getelementptr inbounds %uplevel_type, ptr %uplevel_rec, i64 0, i32 0
  store ptr %MYBLOCK, ptr %ul_arg_0p.GEP, align 8
  call void @test02ul(ptr %uplevel_rec)
  ret void
}
; Check that rank parameter was updated.
;CHECK: %test02dv_col_addr = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %t3, ptr elementtype(i32) %"MYBLOCK_$field0$1", i64 5)
;CHECK: %test02dv_ptr_addr = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %t1, ptr elementtype(i32) %test02dv_col_addr, i64 5)

; This checks the usage of the dope vector that gets loaded from an uplevel
; variable.
define void @test02ul(ptr %UL_IN_PTR) {
bb:
  %ul_arg_0p.GEP = getelementptr inbounds %uplevel_type, ptr %UL_IN_PTR, i64 0, i32 0
  %ul_arg_0.GEP = load ptr, ptr %ul_arg_0p.GEP, align 8
  %"ul_arg_0.GEP_$field0$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %ul_arg_0.GEP, i64 0, i32 0
  %"ul_arg_0.GEP_$field0$30" = load ptr, ptr %"ul_arg_0.GEP_$field0$", align 8
  %"ul_arg_0.GEP_$field6$_$field1$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %ul_arg_0.GEP, i64 0, i32 6, i64 0, i32 1
  %stride0_addr = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"ul_arg_0.GEP_$field6$_$field1$", i32 0)
  %stride1_addr = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"ul_arg_0.GEP_$field6$_$field1$", i32 1)
  %stride0 = load i64, ptr %stride0_addr, align 8
  %stride1 = load i64, ptr %stride1_addr, align 8
  %test02ul_col_addr = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %stride1, ptr elementtype(i32) %"ul_arg_0.GEP_$field0$30", i64 0)
  %test02ul_ptr_addr = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %stride0, ptr elementtype(i32) %test02ul_col_addr, i64 0)
  store i32 0, ptr %test02ul_ptr_addr, align 4
  ret void
}
; Check that rank parameter was updated.
;CHECK: %test02ul_col_addr = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %stride1, ptr elementtype(i32) %"ul_arg_0.GEP_$field0$30", i64 0)
;CHECK: %test02ul_ptr_addr = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %stride0, ptr elementtype(i32) %test02ul_col_addr, i64 0)

declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #0
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { nounwind readnone speculatable }
