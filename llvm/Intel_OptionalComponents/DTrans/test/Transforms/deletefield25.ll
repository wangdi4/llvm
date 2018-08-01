; RUN: opt -whole-program-assume -dtrans-identify-unused-values=false  -dtrans-deletefield -S -o - %s | FileCheck %s
; RUN: opt -whole-program-assume -dtrans-identify-unused-values=false -passes=dtrans-deletefield -S -o - %s | FileCheck %s

; This test verifies that the dtrans delete pass is able to handle global
; variables accessed by GEP operators when the type of a field whose index
; changes is also updated.

; delete fields from both of these structures
%struct.test01 = type { i16, i64*, i32, i64 }
%struct.test01dep = type { i16, %struct.test01* }

; Container that holds a pointer to the type being transformed.
@g_test01depptr = internal unnamed_addr global %struct.test01dep zeroinitializer

define %struct.test01* @test01(i64 %num) {
  ; Allocate %num elements. (structure size = 32)
  %size = mul nsw i64 %num, 32
  %mem1 = call i8* @malloc(i64 %size)

  ; Save the allocated memory into global variable's field
  %st_mem1 = bitcast i8* %mem1 to %struct.test01*
  store %struct.test01* %st_mem1, %struct.test01**
                                      getelementptr (%struct.test01dep,
                                          %struct.test01dep* @g_test01depptr,
                                          i64 0, i32 1)
  %f0 = getelementptr %struct.test01, %struct.test01* %st_mem1, i64 0, i32 0
  %f2 = getelementptr %struct.test01, %struct.test01* %st_mem1, i64 0, i32 2
  %f3 = getelementptr %struct.test01, %struct.test01* %st_mem1, i64 0, i32 3
  store i16 0, i16* %f0
  store i32 0, i32* %f2
  store i64 0, i64* %f3
  %v0 = load i16, i16* %f0
  %v2 = load i32, i32* %f2
  %v3 = load i64, i64* %f3

  %res = load %struct.test01*, %struct.test01**
                                   getelementptr (%struct.test01dep,
                                            %struct.test01dep* @g_test01depptr,
                                            i64 0, i32 1)
  ret %struct.test01* %res
}

; CHECK: define internal %__DFT_struct.test01* @test01.1(i64 %num)
; CHECK: %size = mul nsw i64 %num, 16
; CHECK: %mem1 = call i8* @malloc(i64 %size)
; CHECK: %st_mem1 = bitcast i8* %mem1 to %__DFT_struct.test01*
; CHECK: store %__DFT_struct.test01* %st_mem1, %__DFT_struct.test01**
; CHECK-SAME:                 getelementptr inbounds (%__DFT_struct.test01dep,
; CHECK-SAME:                          %__DFT_struct.test01dep* @g_test01depptr,
; CHECK-SAME:                          i64 0, i32 0)
; CHECK: %f0 = getelementptr %__DFT_struct.test01,
; CHECK-SAME:                %__DFT_struct.test01* %st_mem1, i64 0, i32 0
; CHECK: %f2 = getelementptr %__DFT_struct.test01,
; CHECK-SAME:                %__DFT_struct.test01* %st_mem1, i64 0, i32 1
; CHECK: %f3 = getelementptr %__DFT_struct.test01,
; CHECK-SAME:                %__DFT_struct.test01* %st_mem1, i64 0, i32 2
; CHECK: %res = load %__DFT_struct.test01*, %__DFT_struct.test01**
; CHECK-SAME:                 getelementptr inbounds (%__DFT_struct.test01dep,
; CHECK-SAME:                         %__DFT_struct.test01dep* @g_test01depptr,
; CHECK-SAME:                         i64 0, i32 0)

declare i8* @malloc(i64)
