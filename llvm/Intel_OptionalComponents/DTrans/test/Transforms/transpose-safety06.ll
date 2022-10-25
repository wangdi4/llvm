; This test verifies the safety analysis for the usage of dope vectors
; in uplevel variables for cases supported by the transpose transformation.

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -disable-output -dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck --check-prefix=CHECK-SAFE %s
; RUN: opt < %s -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck --check-prefix=CHECK-SAFE %s

; Variation to test that invalidates the candidate.
; RUN: sed -e s/.TEST_CAST:// %s | opt -disable-output -dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck --check-prefix=CHECK-UNSAFE %s
; RUN: sed -e s/.TEST_CAST:// %s | opt -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck --check-prefix=CHECK-UNSAFE %s


; Uplevel type consisting of a dope vector and an integer.
%uplevel_type = type { { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }*, i32 }

@test_var01 = internal global [9 x [9 x i32]] zeroinitializer

define void @test01() {
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

  call void @test01dv({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$01")
  ret void
}

; This case checks the analysis handling of the setup of an uplevel variable
; by declaring the variable, copying the address of a dope vector into it,
; and then passing the uplevel to a function.
define void @test01dv({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* noalias nocapture readonly %MYBLOCK) {
  %uplevel_rec = alloca %uplevel_type, align 8

  ; Load the dope vector into the uplevel var
  %ul_arg_0p.GEP = getelementptr inbounds %uplevel_type, %uplevel_type* %uplevel_rec, i64 0, i32 0
  store { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %MYBLOCK, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }** %ul_arg_0p.GEP, align 8

  ; Load the integer field of the uplevel var
  %ul_loc_1.0_var = getelementptr inbounds %uplevel_type, %uplevel_type* %uplevel_rec, i64 0, i32 1
  store i32 9, i32* %ul_loc_1.0_var, align 8

  call void @test01ul(%uplevel_type* %uplevel_rec)
  ret void
}

; This checks the usage of the dope vector that gets loaded from an uplevel
; variable.
define void @test01ul(%uplevel_type* %UL_IN_PTR) {
  %ul_arg_0p.GEP = getelementptr inbounds %uplevel_type, %uplevel_type* %UL_IN_PTR, i64 0, i32 0
  %ul_arg_0.GEP = load { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }*, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }** %ul_arg_0p.GEP, align 8

  ; Load the address of the array variable.
  %"ul_arg_0.GEP_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %ul_arg_0.GEP, i64 0, i32 0
  %"ul_arg_0.GEP_$field0$30" = load i32*, i32** %"ul_arg_0.GEP_$field0$", align 8

  ; Load the stride for each dimension of the array
  %"ul_arg_0.GEP_$field6$_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %ul_arg_0.GEP, i64 0, i32 6, i64 0, i32 1
  %stride0_addr = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"ul_arg_0.GEP_$field6$_$field1$", i32 0)
  %stride1_addr = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"ul_arg_0.GEP_$field6$_$field1$", i32 1)
  %stride0 = load i64, i64* %stride0_addr, align 8
  %stride1 = load i64, i64* %stride1_addr, align 8

  ; Get the address of the element from the original array
  %col_addr = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %stride1, i32* elementtype(i32) %"ul_arg_0.GEP_$field0$30", i64 0)
  %ptr_addr = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %stride0, i32* elementtype(i32) %col_addr, i64 0)
  store i32 0, i32* %ptr_addr, align 4

    ; Variation of test perform an unsupported operation on the uplevel to
    ; cause the candidate to be invalidated.
;TEST_CAST:  %cast = bitcast %uplevel_type* %UL_IN_PTR to i8*

  ret void
}
; matches for when candidate is safe
; CHECK-SAFE: Transpose candidate: test_var01
; CHECK-SAFE: IsValid{{ *}}: true

; matches for when candidate is not safe
; CHECK-UNSAFE: Transpose candidate: test_var01
; CHECK-UNSAFE: IsValid{{ *}}: false


; Uplevel type.
%uplevel_type.12 = type { { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* }
@test_var02 = internal global [9 x [9 x i32]] zeroinitializer

define void @test02() {
  %"var$02" = alloca { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %"var$02_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$02", i64 0, i32 0
  %"var$02_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$02", i64 0, i32 1
  %"var$02_$field2$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$02", i64 0, i32 2
  %"var$02_$field3$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$02", i64 0, i32 3
  %"var$02_$field4$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$02", i64 0, i32 4
  %"var$02_$field6$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$02", i64 0, i32 6, i64 0
  %"var$02_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$02_$field6$", i64 0, i32 0
  %"var$02_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$02_$field6$", i64 0, i32 1
  %"var$02_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$02_$field6$", i64 0, i32 2
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

  call void @test02dv({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$02")
  ret void
}

; This case checks the analysis handling of the setup of an uplevel variable
; by declaring the variable, copying the address of a dope vector into it,
; and then passing the uplevel to a function.
define void @test02dv({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* noalias nocapture readonly %MYBLOCK) {
  %uplevel_rec = alloca %uplevel_type.12, align 8

  ; Load the dope vector into the uplevel var
  %ul_arg_0p.GEP = getelementptr inbounds %uplevel_type.12, %uplevel_type.12* %uplevel_rec, i64 0, i32 0
  store { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %MYBLOCK, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }** %ul_arg_0p.GEP, align 8

  call void @test02ulfwd(%uplevel_type.12* %uplevel_rec)
  ret void
}

; Pass the uplevel variable to another function
define void @test02ulfwd(%uplevel_type.12* %UL_IN_PTR) {
  call void @test02ul(%uplevel_type.12* %UL_IN_PTR)
  ret void
}

; This checks the usage of the dope vector that gets loaded from an uplevel
; variable.
define void @test02ul(%uplevel_type.12* %UL_IN_PTR) {
  %ul_arg_0p.GEP = getelementptr inbounds %uplevel_type.12, %uplevel_type.12* %UL_IN_PTR, i64 0, i32 0
  %ul_arg_0.GEP = load { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }*, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }** %ul_arg_0p.GEP, align 8

  ; Load the address of the array variable.
  %"ul_arg_0.GEP_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %ul_arg_0.GEP, i64 0, i32 0
  %"ul_arg_0.GEP_$field0$30" = load i32*, i32** %"ul_arg_0.GEP_$field0$", align 8

  ; Load the stride for each dimension of the array
  %"ul_arg_0.GEP_$field6$_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %ul_arg_0.GEP, i64 0, i32 6, i64 0, i32 1
  %stride0_addr = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"ul_arg_0.GEP_$field6$_$field1$", i32 0)
  %stride1_addr = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"ul_arg_0.GEP_$field6$_$field1$", i32 1)
  %stride0 = load i64, i64* %stride0_addr, align 8
  %stride1 = load i64, i64* %stride1_addr, align 8

  ; Get the address of the element from the original array
  %col_addr = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %stride1, i32* elementtype(i32) %"ul_arg_0.GEP_$field0$30", i64 0)
  %ptr_addr = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %stride0, i32* elementtype(i32) %col_addr, i64 0)
  store i32 0, i32* %ptr_addr, align 4

    ; Variation of test perform an unsupported operation on the uplevel to
    ; cause the candidate to be invalidated.
;TEST_CAST:  %cast = bitcast %uplevel_type.12* %UL_IN_PTR to i8*

  ret void
}
; matches for when candidate is safe
; CHECK-SAFE: Transpose candidate: test_var02
; CHECK-SAFE: IsValid{{ *}}: true

; matches for when candidate is not safe
; CHECK-UNSAFE: Transpose candidate: test_var02
; CHECK-UNSAFE: IsValid{{ *}}: false


 ; Args: Rank, Lower Bound, Unit Stride (in bytes), Ptr, Element Index
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32)
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64)
