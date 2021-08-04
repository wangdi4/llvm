; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-usecrulecompat -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -dtrans-usecrulecompat -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test passing the address of a field to an indirect function call to trigger
; the "Field address taken call" safety bit.

%struct.test01a = type { i32, i32 }
@myarg = internal  global %struct.test01a { i32 3, i32 5 }
@fp = internal  global i32 (i32*)* null, !intel_dtrans_type !4

define i32 @target1(i32 *%addr) !dtrans_type !5 {
  %val = load i32, i32* %addr
  store i32 0, i32* %addr
  ret i32 %val
}

define i32 @main() {
  %fptr = load i32 (i32*)*, i32 (i32*)** @fp
  %fieldAddr = getelementptr %struct.test01a, %struct.test01a* @myarg, i64 0, i32 0
  %res = call i32 %fptr(i32* %fieldAddr), !intel_dtrans_type !2
  ret i32 %res
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Global instance | Has initializer list | Field address taken call{{ *$}}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !1, !3}  ; i32 (i32*)
!3 = !{i32 0, i32 1}  ; i32*
!4 = !{!2, i32 1}  ; i32 (i32*)*
!5 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!5}
