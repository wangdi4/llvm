; RUN: opt  < %s -S -whole-program-assume -dtrans-optbasetest -dtrans-optbasetest-typelist=struct.test01,struct.test02,struct.test03 | FileCheck %s

; This test verifies that the base class functionality for type remapping
; can handle literal structures.

; Test global/constant initializers which use literal structure type.
%struct.test01 = type { i32, i32, i32 }

; Simple case, where initialization is zero
@test01_zi = internal global { void (%struct.test01*)* } zeroinitializer

; Complex cases, where an initializers needs to be remapped using the same
; literal type object.
@test01_global = internal global { void (%struct.test01*)* } { void (%struct.test01*)* @func01 }
@test01_const = internal constant <{ void (%struct.test01*)* }> <{ void (%struct.test01*)* @func01 }>

; Test literal structure array initializer.
@test01_ar_global = internal global [1 x {void (%struct.test01*)* }]
  [{ void (%struct.test01*)*} {void (%struct.test01*) *@func01}]

  ; Test with multiple uses of the same literal type.
@test01_instance1 = internal global %struct.test01 zeroinitializer
@test01_instance2 = internal global %struct.test01 zeroinitializer
@test01_dbl_lit = internal global { {i32, %struct.test01*}, {i32, %struct.test01*} }
  { {i32, %struct.test01*} {i32 0, %struct.test01* @test01_instance1 },
    {i32, %struct.test01*} {i32 1, %struct.test01* @test01_instance2 } }

; CHECK-DAG: %__DTT_struct.test01 = type { i32, i32, i32 }
; CHECK-DAG: %__DTT_struct.test02 = type { i64, i64 }
; CHECK-DAG: %__DTT_struct.test03 = type { i16, i16 }

; CHECK-DAG: @test01_zi = internal global { void (%__DTT_struct.test01*)* } zeroinitializer
; CHECK-DAG: @test01_global = internal global { void (%__DTT_struct.test01*)* } { void (%__DTT_struct.test01*)* @func01.2 }
; CHECK-DAG: @test01_const = internal constant <{ void (%__DTT_struct.test01*)* }> <{ void (%__DTT_struct.test01*)* @func01.2 }>
; CHECK-DAG: @test01_ar_global = internal global [1 x { void (%__DTT_struct.test01*)* }] [{ void (%__DTT_struct.test01*)* } { void (%__DTT_struct.test01*)* @func01.2 }]
; CHECK-DAG: @test01_dbl_lit = internal global { { i32, %__DTT_struct.test01* }, { i32, %__DTT_struct.test01* } } { { i32, %__DTT_struct.test01* } { i32 0, %__DTT_struct.test01* @test01_instance1 }, { i32, %__DTT_struct.test01* } { i32 1, %__DTT_struct.test01* @test01_instance2 } }

; Test with literal struct that is a dependent type.
%struct.test02 = type { i64, i64 }
define void @test02() {
  %tmp = alloca { i32, %struct.test02*, i64, %struct.test02* }
  %tmp2 = bitcast { i32, %struct.test02*, i64, %struct.test02* }* %tmp to i8*
  ret void
}
; CHECK-LABEL: void @test02
; CHECK: %tmp = alloca { i32, %__DTT_struct.test02*, i64, %__DTT_struct.test02* }
; CHECK: %tmp2 = bitcast { i32, %__DTT_struct.test02*, i64, %__DTT_struct.test02* }* %tmp to i8*

; Test with literal struct that triggers the need to clone the routine.
%struct.test03 = type { i16, i16}
define void @test03({%struct.test03}* %lit_in) {
  %tmp = bitcast {%struct.test03}* %lit_in to i8*
  ret void
}
; CHECK-LABEL: void @test03.1({ %__DTT_struct.test03 }* %lit_in)
; CHECK: %tmp = bitcast { %__DTT_struct.test03 }* %lit_in to i8*

define void @func01(%struct.test01* %in) {
  ret void
}
; CHECK-LABEL: void @func01.2(%__DTT_struct.test01* %in)
