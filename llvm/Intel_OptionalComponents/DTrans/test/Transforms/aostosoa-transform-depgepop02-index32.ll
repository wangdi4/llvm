; RUN: opt < %s -S -dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt < %s -S -passes=dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s

; This test verifies that when loads/stores of pointers to a type
; being transformed, which are fields within a global variable of a
; dependent structure type, the attributes is preserved when updating
; the load/store instruction, and the alignment is updated for a
; a 32-bit peeling index type.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
%struct.test01 = type { i64, i64 }
%struct.test01dep = type { i32, %struct.test01*, %struct.test01** }
@g_test01depptr = internal unnamed_addr global %struct.test01dep zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  ; Allocate the type to be transformed
  %alloc01 = call i8* @calloc(i64 10, i64 16)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  call void @test01()

  ret i32 0
}

define internal void @test01() {
; CHECK-LABEL: define internal void @test01

  %test_struct = load atomic i8*, i8**
    bitcast (%struct.test01**
      getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 1)
    to i8**) unordered, align 8
; CHECK:  [[LOADED:%[0-9]+]] = load atomic i32, i32* getelementptr inbounds (%__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @g_test01depptr, i64 0, i32 1) unordered, align 4

  store atomic i8* %test_struct, i8**
    bitcast (%struct.test01**
      getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 1)
    to i8**) monotonic, align 8
; CHECK:  store atomic i32 [[LOADED]], i32* getelementptr inbounds (%__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @g_test01depptr, i64 0, i32 1) monotonic, align 4

  %cmp = icmp eq i8* %test_struct, null
  ret void
}

declare i8* @calloc(i64, i64)
