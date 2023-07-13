; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -dtrans-usecrulecompat -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test passing the address of a field to an indirect function call to trigger
; the "Field address taken call" safety bit.

%struct.test01a = type { i32, i32 }
@myarg = internal global %struct.test01a { i32 3, i32 5 }
@fp = internal global ptr null, !intel_dtrans_type !4

define i32 @target1(ptr %addr) !dtrans_type !5 {
  %val = load i32, ptr %addr
  store i32 0, ptr %addr
  ret i32 %val
}

define i32 @main() {
  %fptr = load ptr, ptr @fp
  %fieldAddr = getelementptr %struct.test01a, ptr @myarg, i64 0, i32 0
  %res = call i32 %fptr(ptr %fieldAddr), !intel_dtrans_type !2
  ret i32 %res
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Global instance | Has initializer list | Field address taken call{{ *$}}
; CHECK: End LLVMType: %struct.test01


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !1, !3}  ; i32 (i32*)
!3 = !{i32 0, i32 1}  ; i32*
!4 = !{!2, i32 1}  ; i32 (i32*)*
!5 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!5}
