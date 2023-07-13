; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the DTrans delete pass is able to handle global
; variables accessed by GEP operators when the type of a field whose index
; changes is also updated.

; delete fields from both of these structures
%struct.test = type { i16, ptr, i32, i64 }
%struct.testdep = type { i16, ptr }

; Container that holds a pointer to the type being transformed.
@g_testdepptr = internal unnamed_addr global %struct.testdep zeroinitializer
@g_sum = internal unnamed_addr global i64 zeroinitializer

define "intel_dtrans_func_index"="1" ptr @test(i64 %num) !intel.dtrans.func.type !6 {
  ; Allocate %num elements. (structure size = 32)
  %size = mul nsw i64 %num, 32
  %mem1 = call ptr @malloc(i64 %size)

  ; Save the allocated memory into global variable's field
  store ptr %mem1, ptr getelementptr inbounds (%struct.testdep, ptr @g_testdepptr, i64 0, i32 1)
  %f0 = getelementptr %struct.test, ptr %mem1, i64 0, i32 0
  %f2 = getelementptr %struct.test, ptr %mem1, i64 0, i32 2
  %f3 = getelementptr %struct.test, ptr %mem1, i64 0, i32 3
  store i16 0, ptr %f0
  store i32 0, ptr %f2
  store i64 0, ptr %f3
  %v0 = load i16, ptr %f0
  %v2 = load i32, ptr %f2
  %v3 = load i64, ptr %f3

  %z0 = zext i16 %v0 to i64
  %z2 = zext i32 %v2 to i64
  %add0 = add i64 %z0, %z2
  %add1 = add i64 %add0, %v3
  store i64 %add1, ptr @g_sum

  %res = load ptr, ptr getelementptr inbounds (%struct.testdep, ptr @g_testdepptr, i64 0, i32 1)
  ret ptr %res
}

; CHECK: @test
; CHECK: %size = mul nsw i64 %num, 16
; CHECK: %mem1 = call ptr @malloc(i64 %size)

; CHECK: store ptr %mem1, ptr @g_testdepptr
; CHECK: %f0 = getelementptr %__DFT_struct.test, ptr %mem1, i64 0, i32 0
; CHECK: %f2 = getelementptr %__DFT_struct.test, ptr %mem1, i64 0, i32 1
; CHECK: %f3 = getelementptr %__DFT_struct.test, ptr %mem1, i64 0, i32 2
; CHECK: %res = load ptr, ptr @g_testdepptr

declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i16 0, i32 0}  ; i16
!2 = !{i64 0, i32 1}  ; i64*
!3 = !{i32 0, i32 0}  ; i32
!4 = !{i64 0, i32 0}  ; i64
!5 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!6 = distinct !{!5}
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7}
!9 = !{!"S", %struct.test zeroinitializer, i32 4, !1, !2, !3, !4} ; { i16, i64*, i32, i64 }
!10 = !{!"S", %struct.testdep zeroinitializer, i32 2, !1, !5} ; { i16, %struct.test* }

!intel.dtrans.types = !{!9, !10}
