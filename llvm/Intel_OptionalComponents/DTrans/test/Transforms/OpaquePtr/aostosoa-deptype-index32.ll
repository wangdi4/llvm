; REQUIRES: asserts
; RUN: opt -S -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -S -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -S -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -S -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

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
%struct.test01dep = type { %struct.test01*, %struct.test01*, i32 }
@dep_var = internal global %struct.test01dep* zeroinitializer, !intel_dtrans_type !4

; The dependent data type should be converted to use a 32-bit index.
; CHECK: %__SOADT_struct.test01dep = type { i32, i32, i32 }

; Verify the offsets in the byte-gep instructions get updated.
define void @test01() {
; CHECK-LABEL: define void @test01

  %p = call i8* @malloc(i64 240)
; CHECK-NONOPAQUE: %p = call i8* @malloc(i64 120)
; CHECK-OPAQUE: %p = call ptr @malloc(i64 120)

  %p_test = bitcast i8* %p to %struct.test01dep*
  store %struct.test01dep* %p_test, %struct.test01dep** @dep_var

  ; Perform memfunc call on the depdendent structure type
  %pDst = bitcast %struct.test01dep* %p_test to i8*
  call void @llvm.memset.p0i8.i64(i8* %pDst, i8 0, i64 240, i1 false)
; CHECK-NONOPAQUE: call void @llvm.memset.p0i8.i64(i8* %pDst, i8 0, i64 120, i1 false)
; CHECK-OPAQUE: call void @llvm.memset.p0.i64(ptr %pDst, i8 0, i64 120, i1 false)

  ; Perform byte-flattened GEP accesses on the dependent structure type.
  %p8_A = getelementptr i8, i8* %p, i64 0
  %p_test_A = bitcast i8* %p8_A to %struct.test01**
  %value_A = load %struct.test01*, %struct.test01** %p_test_A
  store %struct.test01* null, %struct.test01** %p_test_A
; CHECK-NONOPAQUE: %p8_A = getelementptr i8, i8* %p, i64 0
; CHECK-NONOPAQUE: %p_test_A = bitcast i8* %p8_A to i32*
; CHECK-NONOPAQUE: %alloc_idx = call i32* @llvm.ptr.annotation.p0i32(i32* %p_test_A, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: %value_A = load i32, i32* %p_test_A, align 8
; CHECK-NONOPAQUE: %alloc_idx1 = call i32* @llvm.ptr.annotation.p0i32(i32* %p_test_A, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: store i32 0, i32* %p_test_A
; CHECK-OPAQUE: %p8_A = getelementptr i8, ptr %p, i64 0
; CHECK-OPAQUE: %p_test_A = bitcast ptr %p8_A to ptr
; CHECK-OPAQUE: %alloc_idx = call ptr @llvm.ptr.annotation.p0(ptr %p_test_A, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK-OPAQUE: %value_A = load i32, ptr %p_test_A
; CHECK-OPAQUE: %alloc_idx1 = call ptr @llvm.ptr.annotation.p0(ptr %p_test_A, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK-OPAQUE: store i32 0, ptr %p_test_A

  %p8_B = getelementptr i8, i8* %p, i64 8
  %p_test_B = bitcast i8* %p8_B to %struct.test01**
  store %struct.test01* %value_A, %struct.test01** %p_test_B
; CHECK-NONOPAQUE: %p8_B = getelementptr i8, i8* %p, i64 4
; CHECK-NONOPAQUE: %p_test_B = bitcast i8* %p8_B to i32*
; CHECK-NONOPAQUE: %alloc_idx2 = call i32* @llvm.ptr.annotation.p0i32(i32* %p_test_B, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0), i8* getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0), i32 0, i8* null)
; CHECK-NONOPAQUE: store i32 %value_A, i32* %p_test_B
; CHECK-OPAQUE: %p8_B = getelementptr i8, ptr %p, i64 4
; CHECK-OPAQUE: %p_test_B = bitcast ptr %p8_B to ptr
; CHECK-OPAQUE: %alloc_idx2 = call ptr @llvm.ptr.annotation.p0(ptr %p_test_B, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK-OPAQUE: store i32 %value_A, ptr %p_test_B

  %p8_C = getelementptr i8, i8* %p, i64 16
  %p_test_C = bitcast i8* %p8_C to i32*
  store i32 2, i32* %p_test_C
; CHECK-NONOPAQUE: %p8_C = getelementptr i8, i8* %p, i64 8
; CHECK-NONOPAQUE: %p_test_C = bitcast i8* %p8_C to i32*
; CHECK-NONOPAQUE: store i32 2, i32* %p_test_C
; CHECK-OPAQUE: %p8_C = getelementptr i8, ptr %p, i64 8
; CHECK-OPAQUE: %p_test_C = bitcast ptr %p8_C to ptr
; CHECK-OPAQUE: store i32 2, ptr %p_test_C, align 4

  ; Perform element count idiom on the dependent type.
  %obj0 = getelementptr %struct.test01dep, %struct.test01dep* %p_test, i64 0
  %obj4 = getelementptr %struct.test01dep, %struct.test01dep* %p_test, i64 4
  %obj0.int = ptrtoint %struct.test01dep* %obj0 to i64
  %obj4.int = ptrtoint %struct.test01dep* %obj4 to i64
  %byte_count = sub i64 %obj0.int, %obj4.int
  %elems = sdiv i64 %byte_count, 24
; CHECK-NONOPAQUE:  %elems = sdiv i64 %byte_count, 12
; CHECK-OPAQUE:  %elems = sdiv i64 %byte_count, 12

  ret void
}

declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !7 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1" nocapture writeonly, i8, i64, i1 immarg)

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
