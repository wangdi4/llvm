; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test a call to a LibFunc that returns an i8*, but uses it as a pointer to an
; aggregate type. This should trigger the "Bad casting" safety flag.

%"class._ZTSSt9exception.std::exception" = type { i32 (...)** }
@_ZTVSt9exception = internal constant { [5 x i8*] } zeroinitializer, !intel_dtrans_type !4

define void @test01() {
  %res = call i8* @__cxa_allocate_exception(i64 8)
  %res.cast = bitcast i8* %res to %"class._ZTSSt9exception.std::exception"*
  %addr = getelementptr %"class._ZTSSt9exception.std::exception", %"class._ZTSSt9exception.std::exception"* %res.cast, i64 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [5 x i8*] }, { [5 x i8*] }* @_ZTVSt9exception, i64 0, i32 0, i64 2) to i32 (...)**), i32 (...)*** %addr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %"class._ZTSSt9exception.std::exception"
; CHECK: Safety data: Bad casting | System object | Has vtable{{ *$}}
; CHECK: End LLVMType: %"class._ZTSSt9exception.std::exception"


declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" i8* @__cxa_allocate_exception(i64)

!1 = !{!"F", i1 true, i32 0, !2}  ; i32 (...)
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!1, i32 2}  ; i32 (...)**
!4 = !{!"L", i32 1, !5}  ; { [5 x i8*] }
!5 = !{!"A", i32 5, !6}  ; [5 x i8*]
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %"class._ZTSSt9exception.std::exception" zeroinitializer, i32 1, !3} ; { i32 (...)** }

!intel.dtrans.types = !{!8}
