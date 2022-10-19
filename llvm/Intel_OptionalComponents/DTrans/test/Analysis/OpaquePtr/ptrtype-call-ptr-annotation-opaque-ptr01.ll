; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; TODO: Remove the -opaque-pointers option. It is currently needed
; because global variables are not recognized as being opaque pointers yet.

; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test that calls to @llvm.ptr.annotation.p0 are analyzed as producing a
; result type that matches the type of the first argument when using opaque
; pointers.

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

; CHECK: %p0 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 0
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK:        i64*
; CHECK:      Element pointees:
; CHECK:        %__SOADT_struct.test01dep @ 0

  %alloc_idx = call i64* @llvm.ptr.annotation.p0(ptr %p0,
                                                 ptr getelementptr inbounds ([33 x i8], [33 x i8]* @__intel_dtrans_aostosoa_index, i32 0, i32 0),
                                                 ptr getelementptr inbounds ([1 x i8], [1 x i8]* @__intel_dtrans_aostosoa_filename, i32 0, i32 0),
                                                 i32 0,
                                                 ptr null)
; CHECK: %alloc_idx = call ptr @llvm.ptr.annotation.p0(ptr %p0, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK:    LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK:      Aliased types:
; CHECK:        i64*
; CHECK:      Element pointees:
; CHECK:        %__SOADT_struct.test01dep @ 0

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
