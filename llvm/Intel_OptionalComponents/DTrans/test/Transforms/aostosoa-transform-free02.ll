; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; Test AOS-to-SOA transformation for calls to free when the pointer
; to the structure is directly loaded as an i8* from memory.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, i32, i32 }

; This structure is where the pointer to the allocated memory is
; going to be stored.
%struct.test01dep = type { i16, %struct.test01* }

; Container that holds a pointer to the type being transformed.
@g_test01depptr = internal unnamed_addr global %struct.test01dep zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  %mem = call i8* @malloc(i64 96)
  store i8* %mem, i8** bitcast (%struct.test01** getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 1) to i8**)

  call void @test01()
  ret i32 0
}

define void @test01() {
; CHECK define internal void@test01
  %mem2 =  load i8*, i8** bitcast (%struct.test01** getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 1) to i8**)

  ; Verify the parameter passed to 'free' is changed to be the address
  ; stored in the first field of the peeled structure variable.
  call void @free(i8* %mem2)
; CHECK:  [[ADDR1:%[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK:  [[PTR1:%[0-9]+]] = load i32*, i32** [[ADDR1]]
; CHECK:  [[PTR1_I8:%[0-9]+]] = bitcast i32* [[PTR1]] to i8*
; CHECK:  call void @free(i8* [[PTR1_I8]])

  ret void
}

declare i8* @malloc(i64)
declare void @free(i8*)
