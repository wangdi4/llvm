; REQUIRES: asserts
; RUN: opt -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that size offsets get updated for operations on dependent
; types when pointer shrinking is enabled.
; Specific items verified:
; - Allocation size is updated
; - Memintrinsic call size argument is updated
; - Byte-flattened GEP indices are updated
; - The element count idiom size is updated

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
%struct.test01 = type { i32, i64 }
%struct.test01dep = type { ptr, ptr, i32 }
@dep_var = internal global ptr zeroinitializer, !intel_dtrans_type !4

; The dependent data type should be converted to use a 32-bit index.
; CHECK: %__SOADT_struct.test01dep = type { i32, i32, i32 }

; Verify the offsets in the byte-gep instructions get updated.
define void @test01() {
; CHECK-LABEL: define void @test01

  %p = call ptr @malloc(i64 240)
; CHECK: %p = call ptr @malloc(i64 120)

  store ptr %p, ptr @dep_var

  ; Perform memfunc call on the depdendent structure type
  call void @llvm.memset.p0.i64(ptr %p, i8 0, i64 240, i1 false)
; CHECK: call void @llvm.memset.p0.i64(ptr %p, i8 0, i64 120, i1 false)

  ; Perform byte-flattened GEP accesses on the dependent structure type.
  %p8_A = getelementptr i8, ptr %p, i64 0
  %value_A = load ptr, ptr %p8_A
  store ptr null, ptr %p8_A
; CHECK: %p8_A = getelementptr i8, ptr %p, i64 0
; CHECK: %alloc_idx = call ptr @llvm.ptr.annotation.p0.p0(ptr %p8_A, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: %value_A = load i32, ptr %p8_A
; CHECK: %alloc_idx1 = call ptr @llvm.ptr.annotation.p0.p0(ptr %p8_A, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: store i32 0, ptr %p8_A

  %p8_B = getelementptr i8, ptr %p, i64 8
  store ptr %value_A, ptr %p8_B
; CHECK: %p8_B = getelementptr i8, ptr %p, i64 4
; CHECK: %alloc_idx2 = call ptr @llvm.ptr.annotation.p0.p0(ptr %p8_B, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: store i32 %value_A, ptr %p8_B

  %p8_C = getelementptr i8, ptr %p, i64 16
  store i32 2, ptr %p8_C
; CHECK: %p8_C = getelementptr i8, ptr %p, i64 8
; CHECK: store i32 2, ptr %p8_C, align 4

  ; Perform element count idiom on the dependent type.
  %obj0 = getelementptr %struct.test01dep, ptr %p, i64 0
  %obj4 = getelementptr %struct.test01dep, ptr %p, i64 4
  %obj0.int = ptrtoint ptr %obj0 to i64
  %obj4.int = ptrtoint ptr %obj4 to i64
  %byte_count = sub i64 %obj0.int, %obj4.int
  %elems = sdiv i64 %byte_count, 24
; CHECK:  %elems = sdiv i64 %byte_count, 12

  ret void
}

declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0
declare !intel.dtrans.func.type !7 void @llvm.memset.p0.i64(ptr "intel_dtrans_func_index"="1" nocapture writeonly, i8, i64, i1 immarg)

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = !{%struct.test01dep zeroinitializer, i32 1}  ; %struct.test01dep*
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = distinct !{!5}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i32, i64 }
!9 = !{!"S", %struct.test01dep zeroinitializer, i32 3, !3, !3, !1} ; { %struct.test01*, %struct.test01*, i32 }

!intel.dtrans.types = !{!8, !9}
