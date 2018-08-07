; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; Test AOS-to-SOA transformation for updates to the instructions that use the
; result of a call to malloc.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i16, i64*, i32, i64 }

; This structure is where the pointer to the allocated memory is
; going to be stored.
%struct.test01dep = type { i16, %struct.test01* }

; Container that holds a pointer to the type being transformed.
@g_test01depptr = internal unnamed_addr global %struct.test01dep zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01(i64 15)
  ret i32 0
}

define void @test01(i64 %num) {
  ; Allocate %num elements. (structure size = 32)
  %size = mul nsw i64 %num, 32
  %mem = call i8* @malloc(i64 %size)

  ; Verify there is no impact to comparisons of the i8* pointer returned
  ; against null.
  %cmp1 = icmp eq i8* %mem, null
  %cmp2 = icmp eq i8* null, %mem
  %cmp3 = icmp ne i8* %mem, null
; CHECK:  %cmp1 = icmp eq i8* %mem, null
; CHECK:  %cmp2 = icmp eq i8* null, %mem
; CHECK:  %cmp3 = icmp ne i8* %mem, null

  ; Verify the pointer can be stored into a global variable
  store i8* %mem, i8** bitcast (%struct.test01** getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 1)  to i8**)
; CHECK:  [[INDEX1:%[0-9]+]] = inttoptr i64 1 to i8*
; CHECK:  store i8* [[INDEX1]], i8** bitcast (i64* getelementptr inbounds (%__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @g_test01depptr, i64 0, i32 1) to i8**)

  ; Verify the pointer can be cast the type being transformed, and stored.
  %st_mem = bitcast i8* %mem to %struct.test01*
  store %struct.test01* %st_mem, %struct.test01** getelementptr (%struct.test01dep, %struct.test01dep* @g_test01depptr, i64 0, i32 1)
; CHECK:    store i64 1, i64* getelementptr inbounds (%__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @g_test01depptr, i64 0, i32 1)

  ; Verify the pointer can be cast the type being transformed, and used.
  %st_mem2 = bitcast i8* %mem to %struct.test01*
  %array5 = getelementptr %struct.test01, %struct.test01* %st_mem2, i64 5
; CHECK:   %array5 = add i64 1, 5

  %array5field2 = getelementptr %struct.test01, %struct.test01* %st_mem2, i64 5, i32 2
; CHECK:  [[BASE:%[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK:  [[FIELD_ADDR:%[0-9]+]] = load i32*, i32** [[BASE]]
; CHECK:  [[AR_INDEX:%[0-9]+]] = add i64 1, 5
; CHECK:  %array5field2 = getelementptr i32, i32* [[FIELD_ADDR]], i64 [[AR_INDEX]]

  ret void
}

declare i8* @malloc(i64)
