; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test passing a pointer to a structure to an external library function
; results in the type being marked as a "System object"

; Pass a value to a LibFunc
%"class._ZTSSt9exception.std::exception" = type { i32 (...)** }
define void @test01(%"class._ZTSSt9exception.std::exception"* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !5 {
  call void @_ZNSt9exceptionD1Ev(%"class._ZTSSt9exception.std::exception"* %in)
  ret void
}
declare !intel.dtrans.func.type !6 void @_ZNSt9exceptionD1Ev(%"class._ZTSSt9exception.std::exception"* "intel_dtrans_func_index"="1")

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %"class._ZTSSt9exception.std::exception"
; CHECK: Safety data: Address taken | System object | Has vtable{{ *$}}
; CHECK: End LLVMType: %"class._ZTSSt9exception.std::exception"


!1 = !{!"F", i1 true, i32 0, !2}  ; i32 (...)
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!1, i32 2}  ; i32 (...)**
!4 = !{%"class._ZTSSt9exception.std::exception" zeroinitializer, i32 1}  ; %"class._ZTSSt9exception.std::exception"*
!5 = distinct !{!4}
!6 = distinct !{!4}
!7 = !{!"S", %"class._ZTSSt9exception.std::exception" zeroinitializer, i32 1, !3} ; { i32 (...)** }

!intel.dtrans.types = !{!7}
