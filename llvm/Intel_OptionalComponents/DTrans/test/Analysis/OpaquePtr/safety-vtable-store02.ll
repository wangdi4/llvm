; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that storing to a VTable field with a pointer that aliases a structure
; type sets safety bits as being unsafe.

%"class.XMLPlatformUtilsException" = type { %"class.XMLException" }
%"class.XMLException" = type { ptr, i32 }
%struct.test01 = type { i32, i32 }

@XMLPlatformUtilsExceptionE.0 = global [2 x ptr] [ptr null, ptr @foo], !intel_dtrans_type !5

define internal void @XMLPlatformUtilsExceptionTest01(ptr "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !8 {
  %local = alloca %struct.test01
  %vtbl_addr = getelementptr %"class.XMLPlatformUtilsException", ptr %arg, i64 0, i32 0, i32 0

  ; This is the instruction under test, which should trigger an "Unsafe pointer
  ; store" on the value operand and "Mismatched element access" on the pointer
  ; operand.
  store ptr %local, ptr %vtbl_addr
  ret void
}

define void @foo(%"class.XMLPlatformUtilsException" %arg) {
  ret void
}

; CHECK:DTRANS_StructInfo:
; CHECK: LLVMType: %class.XMLException
; CHECK: Safety data: Bad casting | Mismatched element access | Nested structure | Has vtable{{ *}}
; CHECK: End LLVMType: %class.XMLException

; CHECK:DTRANS_StructInfo:
; CHECK: LLVMType: %class.XMLPlatformUtilsException
; CHECK: Safety data: Contains nested structure{{ *}}
; CHECK: End LLVMType: %class.XMLPlatformUtilsException

; CHECK:DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad casting | Unsafe pointer store | Local instance{{ *}}
; CHECK: End LLVMType: %struct.test01


!1 = !{%"class.XMLException" zeroinitializer, i32 0}  ; %"class.XMLException"
!2 = !{!"F", i1 true, i32 0, !3}  ; i32 (...)
!3 = !{i32 0, i32 0}  ; i32
!4 = !{!2, i32 2}  ; i32 (...)**
!5 = !{!"A", i32 2, !6}  ; [2 x i8*]
!6 = !{i8 0, i32 1}  ; i8*
!7 = !{%"class.XMLPlatformUtilsException" zeroinitializer, i32 1}  ; %"class.XMLPlatformUtilsException"*
!8 = distinct !{!7}
!9 = !{!"S", %"class.XMLPlatformUtilsException" zeroinitializer, i32 1, !1} ; { %"class.XMLException" }
!10 = !{!"S", %"class.XMLException" zeroinitializer, i32 2, !4, !3} ; { i32 (...)**, i32 }
!11 = !{!"S", %struct.test01 zeroinitializer, i32 2, !3, !3} ; { i32, i32 }

!intel.dtrans.types = !{!9, !10, !11}
