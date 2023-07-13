; REQUIRES: asserts
; RUN: opt -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA conversion of an allocation of the type being
; transformed. Uses calloc with constant arguments, and uses the result
; for supported operations, such as icmp, store, and bitcast.

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
  %mem = call ptr @calloc(i64 10, i64 24)

  %success1 = icmp eq ptr %mem, null
  %success1b = icmp eq ptr null, %mem
  %success2 = icmp eq ptr %mem, null
  %success2b = icmp eq ptr null, %mem

; Check that the comparison of the allocation result stays as a comparison
; against null, when done before or after the bitcast. The comparison after
; it's been cast will be converted to use the pre-cast variable, because
; the cast variable is going to be treated as index 1.
; CHECK: %success1 = icmp eq ptr %mem, null
; CHECK: %success1b = icmp eq ptr %mem, null
; CHECK: %success2 = icmp eq ptr %mem, null
; CHECK: %success2b = icmp eq ptr %mem, null

  store ptr %mem, ptr getelementptr (%struct.test01dep, ptr @glob, i64 0, i32 1)

 ; Verify the store for original allocation is changed to storing index
 ; element 1.
; CHECK: %alloc_idx = call ptr @llvm.ptr.annotation.p0.p0(ptr getelementptr inbounds (%__SOADT_struct.test01dep, ptr @glob, i64 0, i32 1), ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: store i64 1, ptr getelementptr inbounds (%__SOADT_struct.test01dep, ptr @glob, i64 0, i32 1)

  %fa0 = getelementptr %struct.test01, ptr %mem, i64 0, i32 0
  store i64 0, ptr %fa0

; Verify the field stores get converted as being array index 1 accesses.
; CHECK: %[[SOA_ADDR0:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 0
; CHECK: %[[FIELD0_BASE:[0-9]+]] = load ptr, ptr %[[SOA_ADDR0]]
; CHECK: %fa0 = getelementptr i64, ptr %[[FIELD0_BASE]], i64 1
; CHECK: store i64 0, ptr %fa0

  %fa1 = getelementptr %struct.test01, ptr %mem, i64 0, i32 1
  store ptr null, ptr %fa1
; CHECK: %[[SOA_ADDR1:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 1
; CHECK: %[[FIELD1_BASE:[0-9]+]] = load ptr, ptr %[[SOA_ADDR1]]
; CHECK: %fa1 = getelementptr i64, ptr %[[FIELD1_BASE]], i64 1
; CHECK: %alloc_idx1 = call ptr @llvm.ptr.annotation.p0.p0(ptr %fa1, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: store i64 0, ptr %fa1

  %fa2 = getelementptr %struct.test01, ptr %mem, i64 0, i32 2
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
  %st = load ptr, ptr getelementptr (%struct.test01dep, ptr @glob, i64 0, i32 1)
  %test1 = icmp eq ptr %st, null
  %test2 = icmp eq ptr %st, null
  call void @free(ptr %st)

; Testing the pointer as either the pre-cast type or the cast type will be
; converted to just use the type that is going to be passed to the 'free'
; call because the original structure type is being removed.

; CHECK: %[[SOA_FIELD0_ADDR:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 0
; CHECK: %[[SOA_PTR:[0-9]+]] = load ptr, ptr %[[SOA_FIELD0_ADDR]]
; CHECK: %test1 = icmp eq ptr %[[SOA_PTR]], null
; CHECK: %test2 = icmp eq ptr %[[SOA_PTR]], null
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
