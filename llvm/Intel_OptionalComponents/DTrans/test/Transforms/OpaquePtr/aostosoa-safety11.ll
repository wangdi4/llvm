; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -disable-output -opaque-pointers -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -debug-only=dtrans-aostosoaop %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the AOS-to-SOA transformation rejects structures that
; directly the allocated pointer without any intervening bitscast instruction
; when using the byte-flattened GEP form on the allocated result. This is currently
; not supported because the conversion of the allocation site inserts some
; instructions to convert users of the allocation to use index element 1. The
; byte-GEP conversion does not currently handle this because the original IR
; will not have this form, so it would require special handling.

; CHECK: AOS-to-SOA rejecting -- Unsupported allocation user: %struct.test01

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { i64, ptr, ptr }
%struct.test01dep = type { i64, ptr }

@glob = internal global %struct.test01dep zeroinitializer
define i32 @main() {
  call void @test01()
  ret i32 0
}

define void @test01() {
  %st = call ptr @calloc(i64 10, i64 24)

  %field = getelementptr %struct.test01dep, ptr @glob, i64 0, i32 1
  store ptr %st, ptr %field

  ; Use byte-flattened GEPs on the allocated pointer.
  %fa0 = getelementptr i8, ptr %st, i64 0
  store i64 0, ptr %fa0

  %fa1 = getelementptr i8, ptr %st, i64 8
  store ptr null, ptr %fa1

  %fa2 = getelementptr i8, ptr %st, i64 16
  store ptr null, ptr %fa2

  ret void
}

define void @test02() {
  %field = getelementptr %struct.test01dep, ptr @glob, i64 0, i32 1
  %st = load ptr, ptr %field
  %test1 = icmp eq ptr %st, null
  call void @free(ptr %st)
  ret void
}

declare !intel.dtrans.func.type !5 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0
declare !intel.dtrans.func.type !6 void @free(ptr "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{i64 0, i32 1}  ; i64*
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4}
!6 = distinct !{!4}
!7 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i64, %struct.test01*, i64* }
!8 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !1, !2} ; { i64, %struct.test01* }

!intel.dtrans.types = !{!7, !8}
