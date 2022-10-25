; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that storing to a VTable field is safe when the value
; stored does not alias a structure type.

%"class.XMLPlatformUtilsException" = type { %"class.XMLException" }
%"class.XMLException" = type { i32 (...)**, i32 }

@XMLPlatformUtilsExceptionE.0 = global [2 x i8*] [i8* null, i8* bitcast (void(%"class.XMLPlatformUtilsException")* @foo to i8*)], !intel_dtrans_type !5

define internal void @XMLPlatformUtilsExceptionTest01(%"class.XMLPlatformUtilsException"* "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !8 {
  %vtbl_addr = getelementptr %"class.XMLPlatformUtilsException", %"class.XMLPlatformUtilsException"* %arg, i64 0, i32 0, i32 0
  %field2 = getelementptr [2 x i8*], [2 x i8*]* @XMLPlatformUtilsExceptionE.0, i32 0, i64 2
  %field2.as.fptr = bitcast i8** %field2 to i32 (...)**

  ; This is the instruction under test. There should not be a safety flag set
  ; from writing an "i8**" that represents a function address to a field
  ; declared as "i32 (...)***"
  store i32 (...)** %field2.as.fptr, i32 (...)*** %vtbl_addr
  ret void
}

define void @foo(%"class.XMLPlatformUtilsException" %arg) {
  ret void
}

; CHECK:DTRANS_StructInfo:
; CHECK: LLVMType: %class.XMLException
; CHECK: Safety data: Nested structure | Has vtable{{ *}}
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
