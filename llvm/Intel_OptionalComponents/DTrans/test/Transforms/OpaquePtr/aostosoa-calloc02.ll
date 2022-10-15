; REQUIRES: asserts
; RUN: opt -S -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -S -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -S -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -S -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA conversion of an allocation of the type being
; transformed. Uses calloc with constant arguments, and uses the result
; for supported operations, such as icmp, store, and bitcast.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { i64, %struct.test01*, i64* }
%struct.test01dep = type { i64, %struct.test01* }
@glob = internal global %struct.test01dep zeroinitializer

define i32 @main() {
  call void @test01()
  ret i32 0
}

; Test conversion of calloc call.
define void @test01() {
; CHECK-LABEL: define void @test01
  %mem = call i8* @calloc(i64 10, i64 24)

  %success1 = icmp eq i8* %mem, null
  %success1b = icmp eq i8* null, %mem
  %st = bitcast i8* %mem to %struct.test01*
  %success2 = icmp eq %struct.test01* %st, null
  %success2b = icmp eq %struct.test01* null, %st

; Check that the comparison of the allocation result stays as a comparison
; against null, when done before or after the bitcast. The comparison after
; it's been cast will be converted to use the pre-cast variable, because
; the cast variable is going to be treated as index 1.
; CHECK-NONOPAQUE: %success1 = icmp eq i8* %mem, null
; CHECK-NONOPAQUE: %success1b = icmp eq i8* %mem, null
; CHECK-NONOPAQUE: %success2 = icmp eq i8* %mem, null
; CHECK-NONOPAQUE: %success2b = icmp eq i8* %mem, null
; CHECK-OPAQUE: %success1 = icmp eq ptr %mem, null
; CHECK-OPAQUE: %success1b = icmp eq ptr %mem, null
; CHECK-OPAQUE: %success2 = icmp eq ptr %mem, null
; CHECK-OPAQUE: %success2b = icmp eq ptr %mem, null

  store %struct.test01* %st, %struct.test01** getelementptr (%struct.test01dep, %struct.test01dep* @glob, i64 0, i32 1)

 ; Verify the store for original allocation is changed to storing index
 ; element 1.
; CHECK-NONOPAQUE: %alloc_idx = call i64* @llvm.ptr.annotation.p0i64(i64* getelementptr inbounds (%__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @glob, i64 0, i32 1), i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: store i64 1, i64* getelementptr inbounds (%__SOADT_struct.test01dep, %__SOADT_struct.test01dep* @glob, i64 0, i32 1)
; CHECK-OPAQUE: %alloc_idx = call ptr @llvm.ptr.annotation.p0(ptr getelementptr inbounds (%__SOADT_struct.test01dep, ptr @glob, i64 0, i32 1), ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK-OPAQUE: store i64 1, ptr getelementptr inbounds (%__SOADT_struct.test01dep, ptr @glob, i64 0, i32 1)

  %fa0 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  store i64 0, i64* %fa0

; Verify the field stores get converted as being array index 1 accesses.
; CHECK-NONOPAQUE: %[[SOA_ADDR0:[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK-NONOPAQUE: %[[FIELD0_BASE:[0-9]+]] = load i64*, i64** %[[SOA_ADDR0]]
; CHECK-NONOPAQUE: %fa0 = getelementptr i64, i64* %[[FIELD0_BASE]], i64 1
; CHECK-NONOPAQUE: store i64 0, i64* %fa0
; CHECK-OPAQUE: %[[SOA_ADDR0:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 0
; CHECK-OPAQUE: %[[FIELD0_BASE:[0-9]+]] = load ptr, ptr %[[SOA_ADDR0]]
; CHECK-OPAQUE: %fa0 = getelementptr i64, ptr %[[FIELD0_BASE]], i64 1
; CHECK-OPAQUE: store i64 0, ptr %fa0

  %fa1 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 1
  store %struct.test01* null, %struct.test01** %fa1
; CHECK-NONOPAQUE: %[[SOA_ADDR1:[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK-NONOPAQUE: %[[FIELD1_BASE:[0-9]+]] = load i64*, i64** %[[SOA_ADDR1]]
; CHECK-NONOPAQUE: %fa1 = getelementptr i64, i64* %[[FIELD1_BASE]], i64 1
; CHECK-NONOPAQUE: %alloc_idx1 = call i64* @llvm.ptr.annotation.p0i64(i64* %fa1, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: store i64 0, i64* %fa1
; CHECK-OPAQUE: %[[SOA_ADDR1:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 1
; CHECK-OPAQUE: %[[FIELD1_BASE:[0-9]+]] = load ptr, ptr %[[SOA_ADDR1]]
; CHECK-OPAQUE: %fa1 = getelementptr i64, ptr %[[FIELD1_BASE]], i64 1
; CHECK-OPAQUE: %alloc_idx1 = call ptr @llvm.ptr.annotation.p0(ptr %fa1, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK-OPAQUE: store i64 0, ptr %fa1

  %fa2 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 2
  store i64* null, i64** %fa2
; CHECK-NONOPAQUE: %[[SOA_ADDR2:[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK-NONOPAQUE: %[[FIELD2_BASE:[0-9]+]] = load i64**, i64*** %[[SOA_ADDR2]]
; CHECK-NONOPAQUE: %fa2 = getelementptr i64*, i64** %[[FIELD2_BASE]], i64 1
; CHECK-NONOPAQUE: store i64* null, i64** %fa2
; CHECK-OPAQUE: %[[SOA_ADDR2:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 2
; CHECK-OPAQUE: %[[FIELD2_BASE:[0-9]+]] = load ptr, ptr %[[SOA_ADDR2]]
; CHECK-OPAQUE: %fa2 = getelementptr ptr, ptr %[[FIELD2_BASE]], i64 1
; CHECK-OPAQUE: store ptr null, ptr %fa2

  ret void
}

; Test conversion of free call with 'icmp' null pointer tests
define void @test02() {
; CHECK-LABEL: define void @test02
  %st = load %struct.test01*, %struct.test01** getelementptr (%struct.test01dep, %struct.test01dep* @glob, i64 0, i32 1)
  %test1 = icmp eq %struct.test01* %st, null
  %mem = bitcast %struct.test01* %st to i8*
  %test2 = icmp eq i8* %mem, null
  call void @free(i8* %mem)
; CHECK-NONOPAQUE: %[[SOA_FIELD0_ADDR:[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK-NONOPAQUE: %[[SOA_PTR:[0-9]+]] = load i64*, i64** %[[SOA_FIELD0_ADDR]]
; CHECK-NONOPAQUE: %[[SOA_PTR_CAST:[0-9]+]] = bitcast i64* %[[SOA_PTR]] to i8*

; Testing the pointer as either the pre-cast type or the cast type will be
; converted to just use the type that is going to be passed to the 'free'
; call because the original structure type is being removed.
; CHECK-NONOPAQUE: %test1 = icmp eq i8* %[[SOA_PTR_CAST]], null
; CHECK-NONOPAQUE: %test2 = icmp eq i8* %[[SOA_PTR_CAST]], null
; CHECK-NONOPAQUE: call void @free(i8* %[[SOA_PTR_CAST]])

; CHECK-OPAQUE: %[[SOA_FIELD0_ADDR:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 0
; CHECK-OPAQUE: %[[SOA_PTR:[0-9]+]] = load ptr, ptr %[[SOA_FIELD0_ADDR]]
; CHECK-OPAQUE: %test1 = icmp eq ptr %[[SOA_PTR]], null
; CHECK-OPAQUE: %test2 = icmp eq ptr %[[SOA_PTR]], null
; CHECK-OPAQUE: call void @free(ptr %[[SOA_PTR]])

  ret void
}

declare !intel.dtrans.func.type !5 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64) #0
declare !intel.dtrans.func.type !6 void @free(i8* "intel_dtrans_func_index"="1") #1

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
