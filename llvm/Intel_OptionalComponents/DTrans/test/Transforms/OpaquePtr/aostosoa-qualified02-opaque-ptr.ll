; RUN: opt -S -opaque-pointers -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -dtrans-aostosoaop -dtrans-aostosoaop-index32=true %s 2>&1 | FileCheck %s
; RUN: opt -S -opaque-pointers -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=true %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA conversion when the calloc result for the type being
; transformed is used directly in GEP instructions without being bitcast to a
; pointer to the structure type. This covers GEPs that use 1 operand to get the
; address of different array elements, and 2 operands to get a field address.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { i64, ptr, ptr }
%struct.test01dep = type { i64, ptr }
@glob = internal global %struct.test01dep zeroinitializer
@glob2 = internal global ptr zeroinitializer, !intel_dtrans_type !2
@glob3 = internal global ptr zeroinitializer, !intel_dtrans_type !2
define i32 @main() {
  call void @test01(i64 5)
  ret i32 0
}

define void @test01(i64 %idx) {
; CHECK-LABEL: define void @test01
  %st = call ptr @calloc(i64 10, i64 240)

  %success1 = icmp eq ptr %st, null
; CHECK: %success1 = icmp eq ptr %st, null

 ; Verify the store for original allocation is changed to storing index
 ; element 1.
  %field = getelementptr %struct.test01dep, ptr @glob, i64 0, i32 1
  store ptr %st, ptr %field
; CHECK: store i32 1, ptr %field

  ; Verify that the address for %st[9] gets converted into storing index element
  ; 10 (because index values start from 1)
  %array9 = getelementptr %struct.test01, ptr %st, i64 9
  store ptr %array9, ptr @glob2
; CHECK: %[[SOA_INDEX1:[0-9]+]] = trunc i64 9 to i32
; CHECK: %array9 = add i32 1, %[[SOA_INDEX1]]
; CHECK: store i32 %array9, ptr @glob2, align 8

  ; Verify that the address for %st[%idx] gets converted to storing index
  ; element "%idx + 1".
  %arrayN = getelementptr %struct.test01, ptr %st, i64 %idx
  store ptr %arrayN, ptr @glob3
; CHECK: %[[SOA_INDEX2:[0-9]+]] = trunc i64 %idx to i32
; CHECK: %arrayN = add i32 1, %[[SOA_INDEX2]]
; CHECK: store i32 %arrayN, ptr @glob3, align 8

; Verify the field address for the store instructions get converted as being
; indexed starting from 1.
  %fa0 = getelementptr %struct.test01, ptr %st, i64 0, i32 0
  store i64 0, ptr %fa0
  %fa1 = getelementptr %struct.test01, ptr %st, i64 0, i32 1
  store ptr null, ptr %fa1
  %fa2 = getelementptr %struct.test01, ptr %st, i64 0, i32 2
  store ptr null, ptr %fa2

; CHECK: %[[SOA_ADDR0:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 0
; CHECK: %[[FIELD0_BASE:[0-9]+]] = load ptr, ptr %[[SOA_ADDR0]]
; CHECK: %[[FIELD0_INDEX0:[0-9]+]] = zext i32 1 to i64
; CHECK: %fa0 = getelementptr i64, ptr %[[FIELD0_BASE]], i64 %[[FIELD0_INDEX0]]
; CHECK: store i64 0, ptr %fa0

; CHECK: %[[SOA_ADDR1:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 1
; CHECK: %[[FIELD1_BASE:[0-9]+]] = load ptr, ptr %[[SOA_ADDR1]]
; CHECK: %[[FIELD1_INDEX0:[0-9]+]] = zext i32 1 to i64
; CHECK: %fa1 = getelementptr i32, ptr %[[FIELD1_BASE]], i64 %[[FIELD1_INDEX0]]
; CHECK: store i32 0, ptr %fa1

; CHECK: %[[SOA_ADDR2:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 2
; CHECK: %[[FIELD2_BASE:[0-9]+]] = load ptr, ptr %[[SOA_ADDR2]]
; CHECK: %[[FIELD2_INDEX0:[0-9]+]] = zext i32 1 to i64
; CHECK: %fa2 = getelementptr ptr, ptr %[[FIELD2_BASE]], i64 %[[FIELD2_INDEX0]]
; CHECK: store ptr null, ptr %fa2

  ; Verify the field addresses for the second array element access get converted
  ; correctly.
  %fb0 = getelementptr %struct.test01, ptr %st, i64 2, i32 0
  store i64 0, ptr %fb0
  %fb1 = getelementptr %struct.test01, ptr %st, i64 2, i32 1
  store ptr null, ptr %fb1
  %fb2 = getelementptr %struct.test01, ptr %st, i64 2, i32 2
  store ptr null, ptr %fb2

; CHECK: %[[SOA_ADDR0:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 0
; CHECK: %[[FIELD0_BASE:[0-9]+]] = load ptr, ptr %[[SOA_ADDR0]]
; CHECK: %[[SOA_INDEX2:[0-9]+]] = trunc i64 2 to i32
; CHECK: %[[OFFSET0:[0-9]+]] = add i32 1, %[[SOA_INDEX2]]
; CHECK: %[[ARRAY0_INDEX:[0-9]+]] = zext i32 %[[OFFSET0]] to i64
; CHECK: %fb0 = getelementptr i64, ptr %[[FIELD0_BASE]], i64 %[[ARRAY0_INDEX]]
; CHECK: store i64 0, ptr %fb0

; CHECK: %[[SOA_ADDR1:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 1
; CHECK: %[[FIELD1_BASE:[0-9]+]] = load ptr, ptr %[[SOA_ADDR1]]
; CHECK: %[[SOA_INDEX2:[0-9]+]] = trunc i64 2 to i32
; CHECK: %[[OFFSET1:[0-9]+]] = add i32 1, %[[SOA_INDEX2]]
; CHECK: %[[ARRAY1_INDEX:[0-9]+]] = zext i32 %[[OFFSET1]] to i64
; CHECK: %fb1 = getelementptr i32, ptr %[[FIELD1_BASE]], i64 %[[ARRAY1_INDEX]]
; CHECK: store i32 0, ptr %fb1

; CHECK: %[[SOA_ADDR2:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 2
; CHECK: %[[FIELD2_BASE:[0-9]+]] = load ptr, ptr %[[SOA_ADDR2]]
; CHECK: %[[SOA_INDEX2:[0-9]+]] = trunc i64 2 to i32
; CHECK: %[[OFFSET2:[0-9]+]] = add i32 1, %[[SOA_INDEX2]]
; CHECK: %[[ARRAY2_INDEX:[0-9]+]] = zext i32 %[[OFFSET2]] to i64
; CHECK: %fb2 = getelementptr ptr, ptr %[[FIELD2_BASE]], i64 %[[ARRAY2_INDEX]]
; CHECK: store ptr null, ptr %fb2

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
