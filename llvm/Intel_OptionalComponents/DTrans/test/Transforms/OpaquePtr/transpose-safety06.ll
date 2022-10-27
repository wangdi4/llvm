; This test verifies the safety analysis for the usage of dope vectors
; in uplevel variables for cases supported by the transpose transformation.

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers < %s -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck --check-prefix=CHECK-SAFE %s

; Variation to test that invalidates the candidate.
; RUN: sed -e s/.TEST_CAST:// %s | opt -opaque-pointers -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck --check-prefix=CHECK-UNSAFE %s


; Uplevel type consisting of a dope vector and an integer.
%uplevel_type = type { ptr, i32 }

@test_var01 = internal global [9 x [9 x i32]] zeroinitializer
@test_var02 = internal global [9 x [9 x i32]] zeroinitializer

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
  call void @test01dv(ptr %"var$01")
  ret void
}

; This case checks the analysis handling of the setup of an uplevel variable
; by declaring the variable, copying the address of a dope vector into it,
; and then passing the uplevel to a function.
define void @test01dv(ptr noalias nocapture readonly %MYBLOCK) {
bb:
  %uplevel_rec = alloca %uplevel_type, align 8
  %ul_arg_0p.GEP = getelementptr inbounds %uplevel_type, ptr %uplevel_rec, i64 0, i32 0
  store ptr %MYBLOCK, ptr %ul_arg_0p.GEP, align 8
  %ul_loc_1.0_var = getelementptr inbounds %uplevel_type, ptr %uplevel_rec, i64 0, i32 1
  store i32 9, ptr %ul_loc_1.0_var, align 8
  call void @test01ul(ptr %uplevel_rec)
  ret void
}

; Uplevel type.
%uplevel_type.12 = type { ptr }

; This checks the usage of the dope vector that gets loaded from an uplevel
; variable.
define void @test01ul(ptr %UL_IN_PTR) {
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
  %col_addr = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %stride1, ptr elementtype(i32) %"ul_arg_0.GEP_$field0$30", i64 0)
  %ptr_addr = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %stride0, ptr elementtype(i32) %col_addr, i64 0)
  store i32 0, ptr %ptr_addr, align 4

    ; Variation of test perform an unsupported operation on the uplevel to
    ; cause the candidate to be invalidated.
;TEST_CAST:  %cast = bitcast ptr %UL_IN_PTR to ptr

  ret void
}
; matches for when candidate is safe
; CHECK-SAFE: Transpose candidate: test_var01
; CHECK-SAFE: IsValid{{ *}}: true

; matches for when candidate is not safe
; CHECK-UNSAFE: Transpose candidate: test_var01
; CHECK-UNSAFE: IsValid{{ *}}: false

define void @test02() {
bb:
  %"var$02" = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %"var$02_$field0$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 0
  %"var$02_$field1$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 1
  %"var$02_$field2$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 2
  %"var$02_$field3$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 3
  %"var$02_$field4$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 4
  %"var$02_$field6$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 6, i64 0
  %"var$02_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$02_$field6$", i64 0, i32 0
  %"var$02_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$02_$field6$", i64 0, i32 1
  %"var$02_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$02_$field6$", i64 0, i32 2
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
  call void @test02dv(ptr %"var$02")
  ret void
}

; This case checks the analysis handling of the setup of an uplevel variable
; by declaring the variable, copying the address of a dope vector into it,
; and then passing the uplevel to a function.
define void @test02dv(ptr noalias nocapture readonly %MYBLOCK) {
bb:
  %uplevel_rec = alloca %uplevel_type.12, align 8
  %ul_arg_0p.GEP = getelementptr inbounds %uplevel_type.12, ptr %uplevel_rec, i64 0, i32 0
  store ptr %MYBLOCK, ptr %ul_arg_0p.GEP, align 8
  call void @test02ulfwd(ptr %uplevel_rec)
  ret void
}

; Pass the uplevel variable to another function
define void @test02ulfwd(ptr %UL_IN_PTR) {
bb:
  call void @test02ul(ptr %UL_IN_PTR)
  ret void
}

; This checks the usage of the dope vector that gets loaded from an uplevel
; variable.
define void @test02ul(ptr %UL_IN_PTR) {
bb:
  %ul_arg_0p.GEP = getelementptr inbounds %uplevel_type.12, ptr %UL_IN_PTR, i64 0, i32 0
  %ul_arg_0.GEP = load ptr, ptr %ul_arg_0p.GEP, align 8
  %"ul_arg_0.GEP_$field0$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %ul_arg_0.GEP, i64 0, i32 0
  %"ul_arg_0.GEP_$field0$30" = load ptr, ptr %"ul_arg_0.GEP_$field0$", align 8
  %"ul_arg_0.GEP_$field6$_$field1$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %ul_arg_0.GEP, i64 0, i32 6, i64 0, i32 1
  %stride0_addr = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"ul_arg_0.GEP_$field6$_$field1$", i32 0)
  %stride1_addr = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"ul_arg_0.GEP_$field6$_$field1$", i32 1)
  %stride0 = load i64, ptr %stride0_addr, align 8
  %stride1 = load i64, ptr %stride1_addr, align 8
  %col_addr = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %stride1, ptr elementtype(i32) %"ul_arg_0.GEP_$field0$30", i64 0)
  %ptr_addr = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %stride0, ptr elementtype(i32) %col_addr, i64 0)
  store i32 0, ptr %ptr_addr, align 4

    ; Variation of test perform an unsupported operation on the uplevel to
    ; cause the candidate to be invalidated.
;TEST_CAST:  %cast = bitcast ptr %UL_IN_PTR to ptr

  ret void
}

; matches for when candidate is safe
; CHECK-SAFE: Transpose candidate: test_var02
; CHECK-SAFE: IsValid{{ *}}: true

; matches for when candidate is not safe
; CHECK-UNSAFE: Transpose candidate: test_var02
; CHECK-UNSAFE: IsValid{{ *}}: false

; Args: Rank, Lower Bound, Unit Stride (in bytes), Ptr, Element Index
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #0
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { nounwind readnone speculatable }
