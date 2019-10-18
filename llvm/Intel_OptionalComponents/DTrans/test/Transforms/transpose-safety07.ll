; This test verifies that the safety analysis for the usage of dope vectors
; in uplevel variables detects a case where the DV is loaded back from
; the uplevel in the function that created the uplevel, and is used
; in an unsupported way.

; RUN: opt < %s -disable-output -dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s


; Uplevel type consisting of a dope vector and an integer.
%uplevel_type = type { { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }*, i32 }

@test_var01 = internal global [9 x [9 x i32]] zeroinitializer

define void @test01() {
  ; Set up the dope vector for a 9 x 9 array.
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
  %t0 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"var$01_$field6$_$field1$", i32 0)
  store i64 4, i64* %t0, align 8
  %t1 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"var$01_$field6$_$field2$", i32 0)
  store i64 1, i64* %t1, align 8
  %t2 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"var$01_$field6$_$field0$", i32 0)
  store i64 9, i64* %t2, align 8
  %t3 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"var$01_$field6$_$field1$", i32 1)
  store i64 36, i64* %t3, align 8
  %t4 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"var$01_$field6$_$field2$", i32 1)
  store i64 1, i64* %t4, align 8
  %t5 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"var$01_$field6$_$field0$", i32 1)
  store i64 9, i64* %t5, align 8
  store i32* getelementptr inbounds ([9 x [9 x i32]], [9 x [9 x i32]]* @test_var01, i64 0, i64 0, i64 0), i32** %"var$01_$field0$", align 8
  store i64 1, i64* %"var$01_$field3$", align 8

  call void @test01dv({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"var$01")
  ret void
}

; This case checks the analysis handling of the setup of an uplevel variable
; by declaring the variable, copying the address of a dope vector into it,
; and then performing an unsupported operation to invalidate the dope vector
; analysis.
define void @test01dv({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* noalias nocapture readonly %MYBLOCK) {
  %uplevel_rec = alloca %uplevel_type, align 8

  ; Load the dope vector into the uplevel var
  %ul_arg_0p.GEP = getelementptr inbounds %uplevel_type, %uplevel_type* %uplevel_rec, i64 0, i32 0
  store { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %MYBLOCK, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }** %ul_arg_0p.GEP, align 8

  ; Load the dope vector back, and perform an unsupported operation to
  ; invalidate the DV associated with the uplevel.
  %ul_dv_alias = load { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }*, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }** %ul_arg_0p.GEP, align 8
  %bad_cast = bitcast { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %ul_dv_alias to i8*

  ret void
}

; CHECK: Transpose candidate: test_var01
; CHECK: IsValid{{ *}}: false


; Args: Rank, Lower Bound, Unit Stride (in bytes), Ptr, Element Index
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32)
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64)
