; RUN: opt < %s -S -dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt < %s -S -passes=dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s

; This test verifies that when loads/stores of pointers to a type
; being transformed, which are fields within a global variable of a
; dependent structure type, the size read or written is changed to
; only access 32 bits when the memory address is computed via a GEPOperator.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
%struct.test01 = type { i64, i64 }
%struct.test01dep = type { i32, %struct.test01*, %struct.test01** }
@g_test01depptr = internal unnamed_addr global %struct.test01dep zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  ; Allocate the type to be transformed
  %alloc01 = call i8* @calloc(i64 10, i64 16)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  call void @test01()
  call void @test02()
  call void @test03(%struct.test01* %struct01_mem)

  ; Allocate a ptr-to-ptr of the type to be transformed
  %alloc02 = call i8* @calloc(i64 10, i64 8)
  %struct01_ptrtoptr = bitcast i8* %alloc02 to %struct.test01**
  call void @test04(%struct.test01** %struct01_ptrtoptr)
  call void @test05(%struct.test01** %struct01_ptrtoptr)

  ret i32 0
}

; Verify a load using bitcast GEPOperator is changed.
define internal void @test01() {
; CHECK-LABEL: define internal void @test01

  %test_struct = load i8*, i8**
    bitcast (%struct.test01**
      getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 1)
    to i8**)
; CHECK: [[LOADED:%[0-9]+]] = load i32, i32* getelementptr inbounds (%__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @g_test01depptr, i64 0, i32 1)
; CHECK-NEXT: %test_struct = inttoptr i32 [[LOADED]] to i8*

  %cmp = icmp eq i8* %test_struct, null
  ret void
}

; Verify that bitcasting also supports a cast to a pointer-sized int.
define internal void @test02() {
; CHECK-LABEL: define internal void @test02

  %test_struct = load i64, i64*
    bitcast (%struct.test01**
      getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 1)
    to i64*)
; CHECK: [[LOADED:%[0-9]+]] = load i32, i32* getelementptr inbounds (%__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @g_test01depptr, i64 0, i32 1)
; CHECK-NEXT: %test_struct = zext i32 [[LOADED]] to i64

  %cmp2 = icmp eq i64 %test_struct, 0
  ret void
}

; Verify a store using a bitcast GEPOperator is changed.
define internal void @test03(%struct.test01* %in) {
; CHECK-LABEL: define internal void @test03

  ; Load the value without any bitcasting. This load is handled by the
  ; AOS-to-SOA base class type remaapper.
  %val = load %struct.test01*, %struct.test01** getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 1)
; CHECK:  %val = load i32, i32* getelementptr inbounds (%__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @g_test01depptr, i64 0, i32 1)

  ; Store the value as an i8*. Verify that the store instruction gets replaced
  ; to only write the 32-bit index.
  %val_i8 = bitcast %struct.test01* %val to i8*
  store i8* %val_i8, i8**
    bitcast (%struct.test01**
      getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 1)
    to i8**)
; CHECK: store i32 %val, i32* getelementptr inbounds (%__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @g_test01depptr, i64 0, i32 1)

  ; Do another store using the same bitcast gep. The bitcast and gep
  ; operators are constants within LLVM internally, so the pointer operand for
  ; both stores point to the same object, so be sure the conversion properly
  ; handles all uses.
  %in_i8 = bitcast %struct.test01* %in to i8*
  store i8* %in_i8, i8**
    bitcast (%struct.test01**
      getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 1)
    to i8**)
; CHECK: store i32 %in, i32* getelementptr inbounds (%__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @g_test01depptr, i64 0, i32 1)

  ret void
}

; Verify a load with a bitcast GEPOperator for a ptr-to-ptr of the type
; being transformed results in a load of just a pointer to the peeling index
; type.
define internal void @test04(%struct.test01** %in) {
; CHECK-LABEL: define internal void @test04

  %ptrtoptr_struct = load i8*, i8**
    bitcast (%struct.test01***
       getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 2)
    to i8**)

; CHECK: [[LOADED:%[0-9]+]] = load i32*, i32** getelementptr inbounds (%__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @g_test01depptr, i64 0, i32 2)
; CHECK: %ptrtoptr_struct = bitcast i32* [[LOADED]] to i8*

  %in8 = bitcast %struct.test01** %in to i8*
  %same = icmp eq i8* %ptrtoptr_struct, %in8

  ret void
}

; Verify a store with a bitcast GEPOperator for a ptr-to-ptr of the type
; being transformed results in a load of just a pointer to the peeling index
; type.
define internal void @test05(%struct.test01** %in) {
; CHECK-LABEL: define internal void @test05

  %in8 = bitcast %struct.test01** %in to i8*
  store i8* %in8, i8**
    bitcast (%struct.test01***
      getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 2)
    to i8**)
; CHECK: store i32* %in, i32** getelementptr inbounds (%__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @g_test01depptr, i64 0, i32 2)

  ret void
}

declare i8* @calloc(i64, i64)
