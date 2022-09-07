; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; This test verifies that a bitcast of a function pointer in a call instruction
; is handled as a bitcast of the arguments.

; Test the case where the bitcast call was created by the devirtualizer,
; and marked with metadata to indicate that the 'this' pointer is safe.
; and therefore should not be marked as "mismatched argument use".
%class.test01.base = type { i32 (...)** }
%class.test01.base.123 = type { i32 (...)** }
%class.test01.derived = type { %class.test01.base.123, i32 }
define void @classDoSomething01(%class.test01.derived* %derived) !_Intel.Devirt.Target !1 {
  %addr = getelementptr %class.test01.derived, %class.test01.derived* %derived, i64 0, i32 1
  store i32 0, i32* %addr
  ret void
}
define void @classTest01(%class.test01.base* %base) {
  call void bitcast (void (%class.test01.derived*)* @classDoSomething01
                  to void (%class.test01.base*)*)
                          (%class.test01.base* %base), !_Intel.Devirt.Call !0
  ret void
}

; CHECK: %class.test01.base = type { i32 (...)** }
; CHECK: Safety data: Has vtable
; CHECK: %class.test01.base.123 = type { i32 (...)** }
; CHECK: Safety data: Nested structure | Has vtable
; CHECK: %class.test01.derived = type { %class.test01.base.123, i32 }
; CHECK: Safety data: Contains nested structure{{ *$}}


; Test that mismatched arg use is not set on 'this' pointer argument, but
; does get set on other arguments that don't match.
%class.test02.base = type { i32 (...)** }
%class.test02.base.123 = type { i32 (...)** }
%class.test02.derived = type { %class.test02.base.123, i32 }
%class.test02.other = type { i64 }
%class.test02.other.2 = type { i64, i64 }
define void @classDoSomething02(%class.test02.derived* %derived, %class.test02.other* %other) !_Intel.Devirt.Target !1 {
  %addr = getelementptr %class.test02.derived, %class.test02.derived* %derived, i64 0, i32 1
  store i32 0, i32* %addr
  %addr2 = getelementptr %class.test02.other, %class.test02.other* %other, i64 0, i32 0
  store i64 0, i64* %addr2
  ret void
}
define void @classTest02(%class.test02.base* %base, %class.test02.other.2* %other) {
  %addr = getelementptr %class.test02.other.2, %class.test02.other.2* %other, i64 0, i32 0
  store i64 1, i64* %addr
  call void bitcast (void (%class.test02.derived*, %class.test02.other*)* @classDoSomething02
                  to void (%class.test02.base*, %class.test02.other.2*)*)
                          (%class.test02.base* %base, %class.test02.other.2* %other), !_Intel.Devirt.Call !0
  ret void
}
; CHECK: %class.test02.base = type { i32 (...)** }
; CHECK: Safety data: Has vtable
; CHECK: %class.test02.base.123 = type { i32 (...)** }
; CHECK: Safety data: Nested structure | Has vtable
; CHECK: %class.test02.derived = type { %class.test02.base.123, i32 }
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: %class.test02.other = type { i64 }
; CHECK: Safety data: Mismatched argument use
; CHECK: %class.test02.other.2 = type { i64, i64 }
; CHECK: Safety data: Mismatched argument use

; Test the case where the argument is known to match the function called.
@myglobal = common dso_local local_unnamed_addr global i32 0, align 4
%struct.test01 = type { i32, i32 }
define void @doSomething01(%struct.test01* %str) {
  %temp = load i32, i32* @myglobal, align 4
  %f2 = getelementptr inbounds %struct.test01, %struct.test01* %str, i64 0, i32 1
  store i32 %temp, i32* %f2, align 4
  ret void
}
define void @test01(%struct.test01* %p) {
  %vp = bitcast %struct.test01* %p to i8*
  call void bitcast (void (%struct.test01*)* @doSomething01
                       to void (i8*)*)(i8* %vp)
  ret void
}

; CHECK: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Test the case where the argument is not known to match the function called.
%struct.test02 = type { i32, i32 }
define void @doSomething02(%struct.test02* %str) {
  %temp = load i32, i32* @myglobal, align 4
  %f2 = getelementptr inbounds %struct.test02, %struct.test02* %str, i64 0, i32 1
  store i32 %temp, i32* %f2, align 4
  ret void
}
define void @test02(i8** %pp) {
  %vp = load i8*, i8** %pp
  call void bitcast (void (%struct.test02*)* @doSomething02
                       to void (i8*)*)(i8* %vp)
  ret void
}

; CHECK: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Safety data: Mismatched argument use

; Test the case where the argument is known to mismatch the function called.
%struct.test03.a = type { i32, i32 }
%struct.test03.b = type { i16, i16, i32 }
define void @doSomething03(%struct.test03.b* %str) {
  %temp = load i32, i32* @myglobal, align 4
  %f2 = getelementptr inbounds %struct.test03.b, %struct.test03.b* %str, i64 0, i32 2
  store i32 %temp, i32* %f2, align 4
  ret void
}
define void @test03(%struct.test03.a* %p) {
  %vp = bitcast %struct.test03.a* %p to i8*
  call void bitcast (void (%struct.test03.b*)* @doSomething03
                       to void (i8*)*)(i8* %vp)
  ret void
}

; CHECK: LLVMType: %struct.test03.a = type { i32, i32 }
; CHECK: Safety data: Mismatched argument use
; CHECK: LLVMType: %struct.test03.b = type { i16, i16, i32 }
; CHECK: Safety data: Mismatched argument use

; Test the case where there is a second argument but it matches.
%struct.test04 = type { i32, i32 }
define void @doSomething04(%struct.test04* %str, i8* %mybyte) {
  %temp = load i32, i32* @myglobal, align 4
  %f2 = getelementptr inbounds %struct.test04, %struct.test04* %str, i64 0, i32 1
  store i32 %temp, i32* %f2, align 4
  store i8 0, i8* %mybyte, align 1
  ret void
}
define void @test04(%struct.test04* %p) {
  %vp = bitcast %struct.test04* %p to i8*
  call void bitcast (void (%struct.test04*, i8*)* @doSomething04
                       to void (i8*, i8*)*)(i8* %vp, i8* null)
  ret void
}

; CHECK: LLVMType: %struct.test04 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Test the case with two arguments and the arguments are reversed.
%struct.test05 = type { i32, i32 }
define void @doSomething05(i8* %mybyte, %struct.test05* %str) {
  %temp = load i32, i32* @myglobal, align 4
  %f2 = getelementptr inbounds %struct.test05, %struct.test05* %str, i64 0, i32 1
  store i32 %temp, i32* %f2, align 4
  store i8 0, i8* %mybyte, align 1
  ret void
}
define void @test05(%struct.test05* %p) {
  %vp = bitcast %struct.test05* %p to i8*
  call void bitcast (void (i8*, %struct.test05*)* @doSomething05
                       to void (i8*, i8*)*)(i8* %vp, i8* null)
  ret void
}

; CHECK: LLVMType: %struct.test05 = type { i32, i32 }
; CHECK: Safety data: Mismatched argument use

; Test the case where two structures are correctly passed.
%struct.test06.a = type { i32, i32 }
%struct.test06.b = type { i16, i16, i32 }
define void @doSomething06(%struct.test06.a* %str1, %struct.test06.b* %str2) {
  %t0 = load i32, i32* @myglobal, align 4
  %f2 = getelementptr inbounds %struct.test06.a, %struct.test06.a* %str1, i64 0, i32 1
  store i32 %t0, i32* %f2, align 4
  %f3 = getelementptr inbounds %struct.test06.b, %struct.test06.b* %str2, i64 0, i32 2
  store i32 %t0, i32* %f3, align 4
  ret void
}
define void @test06(%struct.test06.a* %pa, %struct.test06.b* %pb) {
  %vpa = bitcast %struct.test06.a* %pa to i8*
  %vpb = bitcast %struct.test06.b* %pb to i8*
  call void bitcast (void (%struct.test06.a*, %struct.test06.b*)* @doSomething06
                       to void (i8*, i8*)*)(i8* %vpa, i8* %vpb)
  ret void
}

; CHECK: LLVMType: %struct.test06.a = type { i32, i32 }
; CHECK: Safety data: No issues found
; CHECK: LLVMType: %struct.test06.b = type { i16, i16, i32 }
; CHECK: Safety data: No issues found

; Test the case where two structures are expected but i8* values are passed.
%struct.test07.a = type { i32, i32 }
%struct.test07.b = type { i16, i16, i32 }
define void @doSomething07(%struct.test07.a* %str1, %struct.test07.b* %str2) {
  %t0 = load i32, i32* @myglobal, align 4
  %f2 = getelementptr inbounds %struct.test07.a, %struct.test07.a* %str1, i64 0, i32 1
  store i32 %t0, i32* %f2, align 4
  %f3 = getelementptr inbounds %struct.test07.b, %struct.test07.b* %str2, i64 0, i32 2
  store i32 %t0, i32* %f3, align 4
  ret void
}
define void @test07(i8* %p1, i8* %p2) {
  call void bitcast (void (%struct.test07.a*, %struct.test07.b*)* @doSomething07
                       to void (i8*, i8*)*)(i8* %p1, i8* %p1)
  ret void
}

; CHECK: LLVMType: %struct.test07.a = type { i32, i32 }
; CHECK: Safety data: Mismatched argument use
; CHECK: LLVMType: %struct.test07.b = type { i16, i16, i32 }
; CHECK: Safety data: Mismatched argument use

; Test that the memory space allocated refers to the
; proper structure
declare noalias i8* @malloc(i64)
%struct.test08 = type { i32, i32 }
define void @doSomething08(%struct.test08* %str) {
  %temp = load i32, i32* @myglobal, align 4
  %f2 = getelementptr inbounds %struct.test08, %struct.test08* %str, i64 0, i32 1
  store i32 %temp, i32* %f2, align 4
  ret void
}
define void @test08() {
  %p = call i8* @malloc(i64 16)
  %ps = bitcast i8* %p to %struct.test08*
  call void bitcast (void (%struct.test08*)* @doSomething08
                       to void (i8*)*)(i8* %p)
  ret void
}

; CHECK: LLVMType: %struct.test08 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Test for checking type casting inside the function
%struct.test10 = type { i32, i32 }
define void @doSomething10(i8* %p) {
  %ps = bitcast i8* %p to %struct.test10*
  ret void
}
define void @test10(%struct.test10* %p) {
  call void bitcast (void (i8*)* @doSomething10
                       to void (%struct.test10*)*)(%struct.test10* %p)
  ret void
}

; CHECK: LLVMType: %struct.test10 = type { i32, i32 }
; CHECK: Safety data: No issues found


; Test a case where the argument is passed through an intermediate function.

%struct.test11 = type { i32, i32 }
define void @doSomething11(%struct.test11* %str) {
  %temp = load i32, i32* @myglobal, align 4
  %f2 = getelementptr inbounds %struct.test11, %struct.test11* %str, i64 0, i32 1
  store i32 %temp, i32* %f2, align 4
  ret void
}
define void @passthru11(i8* %p) {
  call void (i8*) bitcast (void (%struct.test11 *)*
                                    @doSomething11
                                  to void (i8*)*)
                         (i8* %p)
  ret void
}
define void @test11() {
  %p = call i8* @malloc(i64 8)
  bitcast i8* %p to %struct.test11*
  call void @passthru11(i8* %p)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test11 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Test the case layout incompatibility
%struct.test12.a = type { i32, i32 }
%struct.test12.b = type { i32 }
define void @doSomething12(%struct.test12.a* %str1, %struct.test12.b* %str2) {
  %t0 = load i32, i32* @myglobal, align 4
  %f2 = getelementptr inbounds %struct.test12.a, %struct.test12.a* %str1, i64 0, i32 1
  store i32 %t0, i32* %f2, align 4
  %f3 = getelementptr inbounds %struct.test12.b, %struct.test12.b* %str2, i64 0, i32 0
  store i32 %t0, i32* %f3, align 4
  ret void
}
define void @test12(%struct.test12.b** %pp) {
  %vp = load %struct.test12.b*, %struct.test12.b** %pp
  call void bitcast (void (%struct.test12.a*, %struct.test12.b*)* @doSomething12
                       to void (i32, %struct.test12.b*)*)(i32 0, %struct.test12.b* %vp)
  ret void
}

; CHECK: LLVMType: %struct.test12.a = type { i32, i32 }
; CHECK: Safety data: Mismatched argument use
; CHECK: LLVMType: %struct.test12.b = type { i32 }
; CHECK: Safety data: Mismatched argument use

; Test the case for nested structures
%struct.test9a = type { i32, i32 }
%struct.test9b = type { %struct.test9a, i32 }
define void @doSomething9(%struct.test9b* %str) {
  %temp = load i32, i32* @myglobal, align 4
  %f2 = getelementptr inbounds %struct.test9b, %struct.test9b* %str, i64 0, i32 1
  store i32 %temp, i32* %f2, align 4
  ret void
}
define void @test9(i8** %pp) {
  %vp = load i8*, i8** %pp
  call void bitcast (void (%struct.test9b*)* @doSomething9
                       to void (i8*)*)(i8* %vp)
  ret void
}

; CHECK: LLVMType: %struct.test9a = type { i32, i32 }
; CHECK: Safety data: Nested structure | Mismatched argument use
; CHECK: LLVMType: %struct.test9b = type { %struct.test9a, i32 }
; CHECK: Safety data: Contains nested structure | Mismatched argument use

!0 = !{!"_Intel.Devirt.Call"}
!1 = !{!"_Intel.Devirt.Target"}
