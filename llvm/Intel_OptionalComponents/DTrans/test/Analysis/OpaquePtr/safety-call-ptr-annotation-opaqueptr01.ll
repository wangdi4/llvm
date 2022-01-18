; REQUIRES: asserts

; TODO: Remove the -opaque-pointers option. It is currently needed
; because global variables are not recognized as being opaque pointers yet.

; RUN: opt -opaque-pointers -disable-output -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types < %s 2>&1 | FileCheck %s

; Test that calls to @llvm.ptr.annotation.p0 do not trigger safety flags on
; the structure types when using opaque pointers.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%__SOA_struct.test01 = type { ptr, ptr, ptr }
%__SOADT_struct.test01dep = type { i64, i32 }

@__soa_struct.test01 = internal global %__SOA_struct.test01 zeroinitializer
@var01 = internal global %__SOADT_struct.test01dep zeroinitializer
@__intel_dtrans_aostosoa_alloc = private constant [38 x i8] c"{dtrans} AOS-to-SOA allocation {id:0}\00"
@__intel_dtrans_aostosoa_index = private constant [33 x i8] c"{dtrans} AOS-to-SOA index {id:0}\00"
@__intel_dtrans_aostosoa_filename = private constant [1 x i8] zeroinitializer

define i32 @test01() {
  %p0 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 0
  %alloc_idx = call i64* @llvm.ptr.annotation.p0(ptr %p0,
                                                 ptr getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0),
                                                 ptr getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0),
                                                 i32 0,
                                                 ptr null)
  %p1 = load i64, i64* %p0, align 8
  ret i32 0
}

declare i64* @llvm.ptr.annotation.p0(i64*, i8*, i8*, i32, i8*)

!1 = !{i32 0, i32 1}  ; i32*
!2 = !{i64 0, i32 1}  ; i64*
!3 = !{i64 0, i32 0}  ; i64
!4 = !{i32 0, i32 0}  ; i32
!5 = !{!"S", %__SOA_struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32*, i64*, i32* }
!6 = !{!"S", %__SOADT_struct.test01dep zeroinitializer, i32 2, !3, !4} ; { i64, i32 }

!intel.dtrans.types = !{!5, !6}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %__SOADT_struct.test01dep
; CHECK: Safety data: Global instance{{ *}}
; CHECK: End LLVMType: %__SOADT_struct.test01dep

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %__SOA_struct.test01
; CHECK: Safety data: Global instance{{ *}}
; CHECK: End LLVMType: %__SOA_struct.test01
