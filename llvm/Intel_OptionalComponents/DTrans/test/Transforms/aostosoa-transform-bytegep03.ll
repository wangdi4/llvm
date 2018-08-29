; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; This tests the AOS-to-SOA transform on cases of a byte-flattened GEP being
; passed to a select instruction.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i64, i32, i32 }

@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  %mem = call i8* @malloc(i64 160)
  store i8* %mem, i8** bitcast (%struct.test01** @g_test01ptr to i8**)
  call void @test01()
  call void @test02()
  call void @test03()
  ret i32 0
}

; In this case the select instruction operates on the address that was
; created via a byte-flattened GEP after it is cast back to its original
; pointer type. This case is equivalent to the simple cases where the
; pointer can be traced back through a bitcast to find the original pointer.
define void @test01() {
; CHECK-LABEL: define internal void @test01

  %var = load %struct.test01*, %struct.test01** @g_test01ptr

  ; Get a byte-pointer to the structure.
  %p = bitcast %struct.test01* %var to i8*

  ; Equivalent to: getelementptr %struct.test01, %struct.test01* %var, i64 0, i32 1
  %p8_B = getelementptr i8, i8* %p, i64 8
; CHECK:  [[AR1_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  [[AR1_BEGIN:%[0-9]+]] = load i32*, i32** [[AR1_PTR]]
; CHECK:  [[FIELD1_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 %var
; CHECK:  %p8_B = bitcast i32* [[FIELD1_PTR]] to i8*

  ; Equivalent to: getelementptr %struct.test01, %struct.test01* %var, i64 0, i32 2
  %p8_C = getelementptr i8, i8* %p, i64 12
; CHECK:  [[AR2_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK:  [[AR2_BEGIN:%[0-9]+]] = load i32*, i32** [[AR2_PTR]]
; CHECK:  [[FIELD2_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR2_BEGIN]], i64 %var
; CHECK:  %p8_C = bitcast i32* [[FIELD2_PTR]] to i8*

  %pB = bitcast i8* %p8_B to i32*
; CHECK:  %pB = bitcast i8* %p8_B to i32*

  %pC = bitcast i8* %p8_C to i32*
; CHECK:  %pC = bitcast i8* %p8_C to i32*

  %sel = select i1 undef, i32* %pB, i32* %pC
  store i32 1, i32* %sel

  ret void
}

; In this case, the select instruction operates on the byte-flattened GEP
; address, which is then cast back to the original pointer type. This case
; is equivalent to the simple cases where the pointer can be traced back
; through a bitcast to find the original pointer.
define void @test02() {
; CHECK-LABEL: define internal void @test02

  %var = load %struct.test01*, %struct.test01** @g_test01ptr

  ; Get a byte-pointer to the structure.
  %p = bitcast %struct.test01* %var to i8*

  ; Equivalent to: getelementptr %struct.test01, %struct.test01* %var, i64 0, i32 1
  %p8_B = getelementptr i8, i8* %p, i64 8
; CHECK:  [[AR1_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  [[AR1_BEGIN:%[0-9]+]] = load i32*, i32** [[AR1_PTR]]
; CHECK:  [[FIELD1_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 %var
; CHECK:  %p8_B = bitcast i32* [[FIELD1_PTR]] to i8*

  ; Equivalent to: getelementptr %struct.test01, %struct.test01* %var, i64 0, i32 2
  %p8_C = getelementptr i8, i8* %p, i64 12
; CHECK:  [[AR2_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK:  [[AR2_BEGIN:%[0-9]+]] = load i32*, i32** [[AR2_PTR]]
; CHECK:  [[FIELD2_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR2_BEGIN]], i64 %var
; CHECK:  %p8_C = bitcast i32* [[FIELD2_PTR]] to i8*

  %sel = select i1 undef, i8* %p8_B, i8* %p8_C
 ; CHECK:  %sel = select i1 undef, i8* %p8_B, i8* %p8_C

  %sel32 = bitcast i8* %sel to i32*
  store i32 1, i32* %sel32

  ret void
}

; In this case the select instruction operates on the base address of the
; structure before it is converted into a byte-flattened GEP. This case requires
; tracking to the source of the select instruction to find the array element
; being accessed.
define void @test03() {
; CHECK-LABEL: define internal void @test03

  %var1 = load %struct.test01*, %struct.test01** @g_test01ptr
  %var2 = load %struct.test01*, %struct.test01** @g_test01ptr

  ; Get a byte-pointer to the structure.
  %p1 = bitcast %struct.test01* %var1 to i8*
  %p2 = bitcast %struct.test01* %var2 to i8*

  %sel = select i1 undef, i8* %p1, i8* %p2
; CHECK:  [[SEL_I64:%[0-9]+]] = select i1 undef, i64 %var1, i64 %var2

  ; Equivalent to: getelementptr %struct.test01, %struct.test01* %var[1|2], i64 0, i32 1
  %p8_B = getelementptr i8, i8* %sel, i64 8
; CHECK:  [[AR1_PTR:%[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  [[AR1_BEGIN:%[0-9]+]] = load i32*, i32** [[AR1_PTR]]
; CHECK:  [[FIELD1_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 [[SEL_I64]]
; CHECK:  %p8_B = bitcast i32* [[FIELD1_PTR]] to i8*

  %pB = bitcast i8* %p8_B to i32*
; CHECK:  %pB = bitcast i8* %p8_B to i32*

  store i32 1, i32* %pB
  %vB = load i32, i32* %pB

  ret void
}

declare i8* @malloc(i64)
