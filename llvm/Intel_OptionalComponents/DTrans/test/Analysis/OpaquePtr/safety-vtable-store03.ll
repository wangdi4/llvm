; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Store a non-pointer value into a structure field identified as a
; VTable pointer type to verify the analysis does not crash. This
; is a contrived situation for a regression test of CMPLRLLVM-31983.

%"class.XMLPlatformUtilsException" = type { %"class.XMLException" }
%"class.XMLException" = type { ptr, i32 }

@XMLPlatformUtilsExceptionE.0 = global [2 x ptr] [ptr null, ptr @foo], !intel_dtrans_type !5

define internal void @XMLPlatformUtilsExceptionTest01(ptr "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !8 {
  %vtbl_addr = getelementptr %"class.XMLPlatformUtilsException", ptr %arg, i64 0, i32 0, i32 0

  ; This is the instruction under test, which should trigger a
  ; "Mismatched element access" on the pointer operand.
  store i32 0, ptr %vtbl_addr
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

!intel.dtrans.types = !{!9, !10}
