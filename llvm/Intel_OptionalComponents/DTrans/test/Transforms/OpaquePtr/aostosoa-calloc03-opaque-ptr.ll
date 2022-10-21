; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; TODO: The -opaque-pointers option should not be necessary for this test
; because it is written with fully opaque pointers, but currently the way the
; DTransOptOPBase class resolves whether opaque pointers are in use is not from
; checking if any opaque pointers exist in the IR, but based on this flag.

; RUN: opt -S -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s

; Test AOS-to-SOA conversion of an allocation of the type being
; transformed. Uses calloc with constant arguments, and uses the result
; for supported operations, such as icmp, store, and getelementptr.
;
; This test uses opaque pointers without any intervening bitcast.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { i64, ptr, ptr }
%struct.test01dep = type { i64, ptr }
@glob = internal global %struct.test01dep zeroinitializer
define i32 @main() {
  call void @test01()
  ret i32 0
}

; Test conversion of calloc call.
define void @test01() {
; CHECK-LABEL: define void @test01
  %st = call ptr @calloc(i64 10, i64 24)

  %success1 = icmp eq ptr %st, null
  %success1b = icmp eq ptr null, %st

; Check that the comparison of the allocation result stays as a comparison
; against null,, and is not replaced by index value 1.
; CHECK: %success1 = icmp eq ptr %st, null
; CHECK: %success1b = icmp eq ptr %st, null

  %field = getelementptr %struct.test01dep, ptr @glob, i64 0, i32 1
  store ptr %st, ptr %field

 ; Verify the store for original allocation is changed to storing index
 ; element 1.
; CHECK: %alloc_idx = call ptr @llvm.ptr.annotation.p0(ptr %field, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: store i64 1, ptr %field

  %fa0 = getelementptr %struct.test01, ptr %st, i64 0, i32 0
  store i64 0, ptr %fa0

; Verify the field stores get converted as being array index 1 accesses.
; CHECK: %[[SOA_ADDR0:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 0
; CHECK: %[[FIELD0_BASE:[0-9]+]] = load ptr, ptr %[[SOA_ADDR0]]
; CHECK: %fa0 = getelementptr i64, ptr %[[FIELD0_BASE]], i64 1
; CHECK: store i64 0, ptr %fa0

  %fa1 = getelementptr %struct.test01, ptr %st, i64 0, i32 1
  store ptr null, ptr %fa1
; CHECK: %[[SOA_ADDR1:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 1
; CHECK: %[[FIELD1_BASE:[0-9]+]] = load ptr, ptr %[[SOA_ADDR1]]
; CHECK: %fa1 = getelementptr i64, ptr %[[FIELD1_BASE]], i64 1
; CHECK: %alloc_idx1 = call ptr @llvm.ptr.annotation.p0(ptr %fa1, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: store i64 0, ptr %fa1

  %fa2 = getelementptr %struct.test01, ptr %st, i64 0, i32 2
  store ptr null, ptr %fa2
; CHECK: %[[SOA_ADDR2:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 2
; CHECK: %[[FIELD2_BASE:[0-9]+]] = load ptr, ptr %[[SOA_ADDR2]]
; CHECK: %fa2 = getelementptr ptr, ptr %[[FIELD2_BASE]], i64 1
; CHECK: store ptr null, ptr %fa2

  ret void
}

; Test conversion of free call with 'icmp' null pointer tests
define void @test02() {
; CHECK-LABEL: define void @test02
  %field = getelementptr %struct.test01dep, ptr @glob, i64 0, i32 1
  %st = load ptr, ptr %field
  %test1 = icmp eq ptr %st, null
  call void @free(ptr %st)

; Verify the test condition for the argument to 'free' remains as a
; 'null' pointer test, and is not replaced by the interger index 1.
; CHECK: %[[SOA_FIELD0_ADDR:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 0
; CHECK: %[[SOA_PTR:[0-9]+]] = load ptr, ptr %[[SOA_FIELD0_ADDR]]
; CHECK: %test1 = icmp eq ptr %[[SOA_PTR]], null
; CHECK: call void @free(ptr %[[SOA_PTR]])

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
