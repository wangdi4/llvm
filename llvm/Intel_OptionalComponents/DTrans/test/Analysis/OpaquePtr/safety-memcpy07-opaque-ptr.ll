; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that memcpy analysis does not crash when the destination pointer
; type collected by the PtrTypeAnalyzer is an opaque structure type.
; (CMPLRLLVM-33257)
;
; This test is the same as safety-memcpy07.ll, but fully uses
; opaque pointer types without bitcasts.

%struct.test01 = type opaque
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01 = type opaque

@ppCCGElem = internal global ptr zeroinitializer, !intel_dtrans_type !1
@p81 = internal global i8 zeroinitializer
@p82 = internal global i8 zeroinitializer

define void @test1(i64 %offset) {
  %i105 = call ptr @getter1()
  %i111 = call ptr @getter2()
  %i123 = call ptr @getter3()
  store ptr %i123, ptr @ppCCGElem

  %i125 = getelementptr inbounds ptr, ptr %i111, i64 %offset
  store ptr %i123, ptr %i125

  %i126 = getelementptr inbounds ptr, ptr %i105, i64 %offset
  %i128 = load ptr, ptr %i126

  call void @llvm.memcpy.p0i8.p0i8.i64(ptr %i123, ptr %i128, i64 8, i1 false)

  ret void
}

define "intel_dtrans_func_index"="1" ptr @getter1() !intel.dtrans.func.type !3 {
  ret ptr @ppCCGElem
}

define "intel_dtrans_func_index"="1" ptr @getter2() !intel.dtrans.func.type !5 {
  ret ptr @p81
}

define "intel_dtrans_func_index"="1" ptr @getter3() !intel.dtrans.func.type !6 {
  ret ptr @p82
}

define void @user(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !7 {
  ret void
}

declare !intel.dtrans.func.type !8 void @llvm.memcpy.p0i8.p0i8.i64(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", i64, i1)

!1 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!2 = !{%struct.test01 zeroinitializer, i32 2}  ; %struct.test01**
!3 = distinct !{!2}
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4}
!6 = distinct !{!4}
!7 = distinct !{!2}
!8 = distinct !{!4, !4}
!9 = !{!"S", %struct.test01 zeroinitializer, i32 0} ; opaque

!intel.dtrans.types = !{!9}

