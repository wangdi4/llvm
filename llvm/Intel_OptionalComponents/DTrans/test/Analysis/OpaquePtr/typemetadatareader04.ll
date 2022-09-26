; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt -dtrans-typemetadatareader -dtrans-typemetadatareader-errors -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes=dtrans-typemetadatareader -dtrans-typemetadatareader-errors -disable-output < %s 2>&1 | FileCheck %s

; This test is to check the metadata reader for cases where the metadata does
; not match the expected types for the values marked. This test will only work
; when pointers are not opaque types, because it relies on the IR containing
; the actual type to be checked.

%struct.test01 = type { i64, i32, i32 }
%struct.test02 = type { i32, i32, i32, i32 }

; Check a global definition
@test01_var = internal global %struct.test01* null, !intel_dtrans_type !3
; CHECK: ERROR: Metadata type does not match expected type: test01_var
; CHECK: IR: %struct.test01*
; CHECK: MD: %struct.test02*

; Check a function definition
define internal "intel_dtrans_func_index"="1" %struct.test01* @test01() !intel.dtrans.func.type !6 {
  %mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %mem to %struct.test01*
  ret %struct.test01* %st
}
; CHECK: ERROR: Metadata type does not match expected type: test01
; CHECK: IR: %struct.test01* ()
; CHECK: MD: %struct.test02* ()

define internal void @test02(%struct.test01* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !8 {
  ret void
}
; CHECK: ERROR: Metadata type does not match expected type: test02
; CHECK: IR: void (%struct.test01*)
; CHECK: MD: void (%struct.test02*)

; Check an alloca instruction
define internal void @test03() {
  %local = alloca %struct.test01*, !intel_dtrans_type !3
  %mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %mem to %struct.test01*
  store %struct.test01* %st, %struct.test01** %local
  ret void
}
; CHECK: ERROR: Metadata type does not match expected type: test03 :  %local = alloca %struct.test01*
; CHECK: IR: %struct.test01*
; CHECK: MD: %struct.test02*


; Check an indirect function call
define internal void @test04(i64 %in) {
  %local4 = alloca %struct.test01
  %ptr = inttoptr i64 %in to %struct.test01*(%struct.test01*)*
  %res = call %struct.test01* %ptr(%struct.test01* %local4), !intel_dtrans_type !14
  ret void
}
; CHECK: ERROR: Metadata type does not match expected type: test04 :  %res = call
; CHECK: IR: %struct.test01* (%struct.test01*)
; CHECK: MD: %struct.test02* (%struct.test02*)

declare !intel.dtrans.func.type !11 noalias "intel_dtrans_func_index"="1" i8* @malloc(i64)


!1 = !{i64 0, i32 0}  ; i64
!2 = !{i32 0, i32 0}  ; i32

!3 = !{!4, i32 1}  ; %struct.test02*
; The next type should be a %struct.test01*, but has been changed to cause the
; recovered type to not match the IR type for testing.
!4 = !{%struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!6 = distinct !{!3}
!8 = distinct !{!3}
!9 = !{i8 0, i32 1}  ; i8*
!11 = distinct !{!9}
!12 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !2} ; { i64, i32, i32 }
!13 = !{!"S", %struct.test02 zeroinitializer, i32 4, !2, !2, !2, !2} ; { i32, i32, i32, i32 }
!14 = !{!"F", i1 false, i32 1, !3, !3 }

!intel.dtrans.types = !{!12, !13}
