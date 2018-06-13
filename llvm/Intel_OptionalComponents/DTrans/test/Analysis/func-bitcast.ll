; RUN: opt -dtransanalysis -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; XFAIL: *

; This test verifies that a bitcast of a function pointer in a call instruction
; is handled as a bitcast of the arguments.

; Test the case where the argument is known to match the function called.
%struct.test01 = type { i32, i32 }
define void @doNothing01(%struct.test01*) { ret void }
define void @test01(%struct.test01* %p) {
  %vp = bitcast %struct.test01* %p to i8*
  call void bitcast (void (%struct.test01*)* @doNothing01
                       to void (i8*)*)(i8* %vp)
  ret void
}

; CHECK: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Test the case where the argument is not known to match the function called.
%struct.test02 = type { i32, i32 }
define void @doNothing02(%struct.test02*) { ret void }
define void @test02(i8* %vp) {
  call void bitcast (void (%struct.test02*)* @doNothing02
                       to void (i8*)*)(i8* %vp)
  ret void
}

; CHECK: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Safety data: Bad bitcast

; Test the case where the argument is known to mismatch the function called.
%struct.test03.a = type { i32, i32 }
%struct.test03.b = type { i16, i16, i32 }
define void @doNothing03(%struct.test03.b*) { ret void }
define void @test03(%struct.test03.a* %p) {
  %vp = bitcast %struct.test03.a* %p to i8*
  call void bitcast (void (%struct.test03.b*)* @doNothing03
                       to void (i8*)*)(i8* %vp)
  ret void
}

; CHECK: LLVMType: %struct.test03.a = type { i32, i32 }
; CHECK: Safety data: Bad bitcast
; CHECK: LLVMType: %struct.test03.b = type { i16, i16, i32 }
; CHECK: Safety data: Bad bitcast

; Test the case where there is a second argument but it matches.
%struct.test04 = type { i32, i32 }
define void @doNothing04(%struct.test04*, i8*) { ret void }
define void @test04(%struct.test04* %p) {
  %vp = bitcast %struct.test04* %p to i8*
  call void bitcast (void (%struct.test04*, i8*)* @doNothing04
                       to void (i8*, i8*)*)(i8* %vp, i8* null)
  ret void
}

; CHECK: LLVMType: %struct.test04 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Test the case with two arguments and the arguments are reversed.
%struct.test05 = type { i32, i32 }
define void @doNothing05(i8* %p1, %struct.test05*) { ret void }
define void @test05(%struct.test05* %p) {
  %vp = bitcast %struct.test05* %p to i8*
  call void bitcast (void (i8*, %struct.test05*)* @doNothing05
                       to void (i8*, i8*)*)(i8* %vp, i8* null)
  ret void
}

; CHECK: LLVMType: %struct.test05 = type { i32, i32 }
; CHECK: Safety data: Bad bitcast

; Test the case where two structures are correctly passed.
%struct.test06.a = type { i32, i32 }
%struct.test06.b = type { i16, i16, i32 }
define void @doNothing06(%struct.test06.a*, %struct.test06.b*) { ret void }
define void @test06(%struct.test06.a* %pa, %struct.test06.b* %pb) {
  %vpa = bitcast %struct.test06.a* %pa to i8*
  %vpb = bitcast %struct.test06.b* %pb to i8*
  call void bitcast (void (%struct.test06.a*, %struct.test06.b*)* @doNothing06
                       to void (i8*, i8*)*)(i8* %vpa, i8* %vpb)
  ret void
}

; CHECK: LLVMType: %struct.test06.a = type { i32, i32 }
; CHECK: Safety data: Bad bitcast
; CHECK: LLVMType: %struct.test06.b = type { i16, i16, i32 }
; CHECK: Safety data: Bad bitcast

; Test the case where two structures are expected but i8* values are passed.
%struct.test07.a = type { i32, i32 }
%struct.test07.b = type { i16, i16, i32 }
define void @doNothing07(%struct.test07.a*, %struct.test07.b*) { ret void }
define void @test07(i8* %p1, i8* %p2) {
  call void bitcast (void (%struct.test07.a*, %struct.test07.b*)* @doNothing07
                       to void (i8*, i8*)*)(i8* %p1, i8* %p1)
  ret void
}

; CHECK: LLVMType: %struct.test07.a = type { i32, i32 }
; CHECK: Safety data: Bad bitcast
; CHECK: LLVMType: %struct.test07.b = type { i16, i16, i32 }
; CHECK: Safety data: Bad bitcast
