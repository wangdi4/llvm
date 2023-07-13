; REQUIRES: asserts
; RUN: opt -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Basic test of the AOS-to-SOA conversion on allocation and free calls of the
; type being transformed when using 'malloc' to verify the allocation size
; argument gets updated. This case is similar to aostosoa-calloc01.ll.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { i64, ptr, ptr }
%struct.test01dep = type { i64, ptr }
@glob = internal global %struct.test01dep zeroinitializer

define i32 @main() {
  call void @test01()
  call void @test02()
  ret i32 0
}

; Test conversion of malloc call.
define void @test01() {
; CHECK-LABEL: define void @test01
  %mem = call ptr @malloc(i64 240)

; Verify the allocation size is increased by the size of 1 structure element.
; CHECK:  %mem = call ptr @malloc(i64 264)

  ; Allocated pointer is stored to memory to establish the object
  ; as the structure type.
  store ptr %mem, ptr getelementptr (%struct.test01dep, ptr @glob, i64 0, i32 1)

; Verify the store for original allocation is changed to storing index
; element 1.
; CHECK: %alloc_idx = call ptr @llvm.ptr.annotation.p0.p0(ptr getelementptr inbounds (%__SOADT_struct.test01dep, ptr @glob, i64 0, i32 1), ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: store i64 1, ptr getelementptr inbounds (%__SOADT_struct.test01dep, ptr @glob, i64 0, i32 1)

  ret void
}

; Test conversion of free call
define void @test02() {
; CHECK-LABEL: define void @test02
  %st = load ptr, ptr getelementptr (%struct.test01dep, ptr @glob, i64 0, i32 1)
  call void @free(ptr %st)

; CHECK: %[[SOA_FIELD0_ADDR:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 0
; CHECK: %[[SOA_PTR:[0-9]+]] = load ptr, ptr %[[SOA_FIELD0_ADDR]]
; CHECK: call void @free(ptr %[[SOA_PTR]])

  ret void
}

declare !intel.dtrans.func.type !5 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0
declare !intel.dtrans.func.type !6 void @free(ptr "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
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
