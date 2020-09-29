; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test a call to a LibFunc that returns an i8*, but uses it as a pointer to an
; aggregate type. This should trigger the "Bad casting" safety flag.

%"class._ZTSSt9exception.std::exception" = type { i32 (...)** }
@_ZTVSt9exception = internal constant { [5 x i8*] } zeroinitializer, !dtrans_type !4

define void @test01() {
  %res = call i8* @__cxa_allocate_exception(i64 8)
  %res.cast = bitcast i8* %res to %"class._ZTSSt9exception.std::exception"*
  %addr = getelementptr %"class._ZTSSt9exception.std::exception", %"class._ZTSSt9exception.std::exception"* %res.cast, i64 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVSt9exception, i64 0, i32 0, i64 2) to i32 (...)**), i32 (...)*** %addr
  ret void
}
 ; TODO: "Mismatched element access" is triggered on this case because "addr" is
 ; detected as being type: "i32 (...)***", but it is stored to a location that
 ; is of type "i8**". The analysis for "store" instructions may need to be
 ; relaxed for "vtable" types.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: class._ZTSSt9exception.std::exception
; CHECK: Safety data: Bad casting | Mismatched element access | System object | Has vtable{{ *$}}


declare i8* @__cxa_allocate_exception(i64)

!1 = !{!"F", i1 true, i32 0, !2}  ; i32 (...)
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!1, i32 2}  ; i32 (...)**
!4 = !{!"L", i32 1, !5}  ; { [5 x i8*] }
!5 = !{!"A", i32 5, !6}  ; [5 x i8*]
!6 = !{i8 0, i32 1}  ; i8*
!7 = !{!"S", %"class._ZTSSt9exception.std::exception" zeroinitializer, i32 1, !3} ; { i32 (...)** }

!dtrans_types = !{!7}
