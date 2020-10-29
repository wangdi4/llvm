; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test passing a pointer to a structure to an external library function
; results in the type being marked as a "System object"

; Pass a value to a LibFunc
%"class._ZTSSt9exception.std::exception" = type { i32 (...)** }
define void @test01(%"class._ZTSSt9exception.std::exception"* %in) !dtrans_type !4 {
  call void @_ZNSt9exceptionD1Ev(%"class._ZTSSt9exception.std::exception"* %in)
  ret void
}
declare void @_ZNSt9exceptionD1Ev(%"class._ZTSSt9exception.std::exception"*)

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: class._ZTSSt9exception.std::exception
; CHECK: Safety data: Address taken | System object | Has vtable{{ *$}}


!1 = !{!"F", i1 true, i32 0, !2}  ; i32 (...)
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!1, i32 2}  ; i32 (...)**
!4 = !{!"F", i1 false, i32 1, !5, !6}  ; void (%"class._ZTSSt9exception.std::exception"*)
!5 = !{!"void", i32 0}  ; void
!6 = !{!7, i32 1}  ; %"class._ZTSSt9exception.std::exception"*
!7 = !{!"R", %"class._ZTSSt9exception.std::exception" zeroinitializer, i32 0}  ; %"class._ZTSSt9exception.std::exception"
!8 = !{!"S", %"class._ZTSSt9exception.std::exception" zeroinitializer, i32 1, !3} ; { i32 (...)** }
!9 = !{!"_ZNSt9exceptionD1Ev", !4}

!dtrans_types = !{!8}
!dtrans_decl_types = !{!9}
