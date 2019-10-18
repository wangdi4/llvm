; This test is to verify that DTrans Analysis can analyze a function call
; made via a GlobalAlias definition.

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Call a function using an alias to the function using a properly typed
; pointer to the structure.
; This is safe because we have the function's definition.
%struct.test01 = type { i32, i32 }
@f01_alias = internal alias void (%struct.test01*), void (%struct.test01*)* @f01

define internal void @f01(%struct.test01* %s) {
  %p = getelementptr %struct.test01, %struct.test01* %s, i64 0, i32 0
  %i = load i32, i32* %p
  ret void
}

define void @test01(%struct.test01* %s) {
  call void @f01_alias(%struct.test01* %s)
  ret void
}
; CHECK: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: No issues found


; Call a function using an alias to the function using a properly typed
; pointer to the structure.
; This case is not safe because the alias is defined using a weak linkage.
%struct.test02 = type { i32, i32 }
@f02_alias = weak alias void (%struct.test02*), void (%struct.test02*)* @f02

define internal void @f02(%struct.test02* %s) {
  %p = getelementptr %struct.test02, %struct.test02* %s, i64 0, i32 0
  %i = load i32, i32* %p
  ret void
}

define void @test02(%struct.test02* %s) {
  call void @f02_alias(%struct.test02* %s)
  ret void
}
; CHECK: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Safety data: Address taken


; Call a function using a bitcast alias to a function using different types.
; This case should set the "Mismatched argument use" safety bit, because it
; should behave equivalent to the call:
;   call void bitcast (void (%struct.test04b*)* @f04 to
;                      void (%struct.test04a*)*)(%struct.test04a* %s)
;
%struct.test03a = type { i32, i32 }
%struct.test03b = type { i32, i32 }
@f03_alias = internal alias void (%struct.test03a*), bitcast (void (%struct.test03b*)* @f03 to void (%struct.test03a*)*)

define internal void @f03(%struct.test03b* %s) {
  %p = getelementptr %struct.test03b, %struct.test03b* %s, i64 0, i32 0
  %i = load i32, i32* %p
  ret void
}

define void @test03(%struct.test03a* %s) {
  %p = getelementptr %struct.test03a, %struct.test03a* %s, i64 0, i32 0
  %i = load i32, i32* %p
  call void @f03_alias(%struct.test03a* %s)
  ret void
}
; CHECK: LLVMType: %struct.test03a = type { i32, i32 }
; CHECK: Safety data: Mismatched argument use
; CHECK: LLVMType: %struct.test03b = type { i32, i32 }
; CHECK: Safety data: Mismatched argument use
