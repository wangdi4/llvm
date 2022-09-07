; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; This test verifies the DTrans analysis behavavior for cascading the
; mismatched argument safety data for cases where it should be pointer
; carried.

; RUN: opt < %s -disable-output -whole-program-assume -internalize -dtransanalysis -dtrans-print-types 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -whole-program-assume -passes="internalize,require<dtransanalysis>" -dtrans-print-types 2>&1 | FileCheck %s

; Test that mismatched argumnet use is carried through pointers when call bitcasts
; the structure pointer to a type known not to match

; Test the case where the argument is not known to match the function called.
; The mistmatched arg use should carry through the pointers that are field
; members.
%struct.test01 = type { %struct.test01.a*, %struct.test01.b* }
%struct.test01.a = type { i32 }
%struct.test01.b = type { i32 }

define void @doSomething01(%struct.test01* %str) {
  %f1 = getelementptr %struct.test01, %struct.test01* %str, i64 0, i32 0
  store %struct.test01.a* null, %struct.test01.a** %f1
  %f2 = getelementptr %struct.test01, %struct.test01* %str, i64 0, i32 1
  store %struct.test01.b* null, %struct.test01.b** %f2
  ret void
}
define void @test01(i8** %pp) {
  %vp = load i8*, i8** %pp
  call void bitcast (void (%struct.test01*)* @doSomething01
                       to void (i8*)*)(i8* %vp)
  ret void
}

; CHECK: %struct.test01 = type { %struct.test01.a*, %struct.test01.b* }
; CHECK: Safety data: Mismatched argument use
; CHECK: %struct.test01.a = type { i32 }
; CHECK: Safety data: Mismatched argument use
; CHECK: %struct.test01.b = type { i32 }
; CHECK: Safety data: Mismatched argument use


; Test that the safety bit is carried through nested structures
%struct.test02 = type { %struct.test02.a, %struct.test02.b }
%struct.test02.a = type { i32 }
%struct.test02.b = type { i32 }

define void @doSomething02(%struct.test02* %str) {
  %f1 = getelementptr %struct.test02, %struct.test02* %str, i64 0, i32 0
  %f11 = getelementptr %struct.test02.a, %struct.test02.a* %f1, i64 0, i32 0
  store i32 0, i32* %f11
  %f2 = getelementptr %struct.test02, %struct.test02* %str, i64 0, i32 1
  %f21 = getelementptr %struct.test02.b, %struct.test02.b* %f2, i64 0, i32 0
  store i32 0, i32* %f21
  ret void
}
define void @test02(i8** %pp) {
  %vp = load i8*, i8** %pp
  call void bitcast (void (%struct.test02*)* @doSomething02
                       to void (i8*)*)(i8* %vp)
  ret void
}

; CHECK: %struct.test02 = type { %struct.test02.a, %struct.test02.b }
; CHECK: Safety data: Contains nested structure | Mismatched argument use
; CHECK: %struct.test02.a = type { i32 }
; CHECK: Safety data: Nested structure | Mismatched argument use
; CHECK: %struct.test02.b = type { i32 }
; CHECK: Safety data: Nested structure | Mismatched argument use


; In this case, the pointer carried safety data for mismatched argument use should
; only be carried to fields that do not match in the target structure when the
; structure is bitcast for the function call.
%struct.test03 = type { %struct.test03.a*, %struct.test03.b*, %struct.test03.c* }
%struct.test03.123 = type { %struct.test03.a.45*, %struct.test03.b*, %struct.test03.c.67* }
%struct.test03.a = type { i32 }
%struct.test03.a.45 = type { i16, i16 }
%struct.test03.b = type { i32 }
%struct.test03.c = type { i32 }
%struct.test03.c.67 = type { i16, i16 }

@globTest03a = global %struct.test03.a.45 zeroinitializer
@globTest03b = global %struct.test03.b zeroinitializer
@globTest03c = global %struct.test03.c.67 zeroinitializer

define void @doSomething03(%struct.test03* %str) {
  %f1 = getelementptr %struct.test03, %struct.test03* %str, i64 0, i32 0
  %f2 = getelementptr %struct.test03, %struct.test03* %str, i64 0, i32 1
  %f3 = getelementptr %struct.test03, %struct.test03* %str, i64 0, i32 2
  %s1 = load %struct.test03.a*, %struct.test03.a** %f1
  %s2 = load %struct.test03.b*, %struct.test03.b** %f2
  %s3 = load %struct.test03.c*, %struct.test03.c** %f3
  ret void
}
define internal void @test03(%struct.test03.123** %pp) {
  %vp = load %struct.test03.123*, %struct.test03.123** %pp
  %f1 = getelementptr %struct.test03.123, %struct.test03.123* %vp, i64 0, i32 0
  %f2 = getelementptr %struct.test03.123, %struct.test03.123* %vp, i64 0, i32 1
  %f3 = getelementptr %struct.test03.123, %struct.test03.123* %vp, i64 0, i32 2
  store %struct.test03.a.45* @globTest03a, %struct.test03.a.45** %f1
  store %struct.test03.b* @globTest03b, %struct.test03.b** %f2
  store %struct.test03.c.67* @globTest03c, %struct.test03.c.67** %f3
  call void bitcast (void (%struct.test03*)* @doSomething03
                       to void (%struct.test03.123*)*)(%struct.test03.123* %vp)
  ret void
}

; CHECK: %struct.test03 = type { %struct.test03.a*, %struct.test03.b*, %struct.test03.c* }
; CHECK: Safety data: Mismatched argument use
; CHECK: %struct.test03.123 = type { %struct.test03.a.45*, %struct.test03.b*, %struct.test03.c.67* }
; CHECK: Safety data: Mismatched argument use
; CHECK: %struct.test03.a = type { i32 }
; CHECK: Safety data: Mismatched argument use
; CHECK: %struct.test03.a.45 = type { i16, i16 }
; CHECK: Global instance | Mismatched argument use
; CHECK: %struct.test03.b = type { i32 }
; CHECK: Safety data: Global instance
; CHECK: %struct.test03.c = type { i32 }
; CHECK: Safety data: Mismatched argument use
; CHECK: %struct.test03.c.67 = type { i16, i16 }
; CHECK: Global instance | Mismatched argument use


; In this case, the pointer carried safety data for mismatched argument use should
; not be carried for fields that never get dereferenced by one of the bitcast types
%struct.test04 = type { %struct.test04.a*, %struct.test04.b*, %struct.test04.c* }
%struct.test04.123 = type { %struct.test04.a.45*, %struct.test04.b.99*, %struct.test04.c.67* }
%struct.test04.a = type { i32 }
%struct.test04.a.45 = type { i16, i16 }
%struct.test04.b = type { i32 }
%struct.test04.b.99 = type { i32 }
%struct.test04.c = type { i32 }
%struct.test04.c.67 = type { i16, i16 }

@globTest04a = global %struct.test04.a.45 zeroinitializer

define void @doSomething04(%struct.test04* %str) {
  %f1 = getelementptr %struct.test04, %struct.test04* %str, i64 0, i32 0
  %f2 = getelementptr %struct.test04, %struct.test04* %str, i64 0, i32 1
  %f3 = getelementptr %struct.test04, %struct.test04* %str, i64 0, i32 2
  %s1 = load %struct.test04.a*, %struct.test04.a** %f1
  %s2 = load %struct.test04.b*, %struct.test04.b** %f2
  %s3 = load %struct.test04.c*, %struct.test04.c** %f3
  ret void
}
define void @test04(%struct.test04.123** %pp) {
  %vp = load %struct.test04.123*, %struct.test04.123** %pp
  %f1 = getelementptr %struct.test04.123, %struct.test04.123* %vp, i64 0, i32 0
  store %struct.test04.a.45* @globTest04a, %struct.test04.a.45** %f1
  call void bitcast (void (%struct.test04*)* @doSomething04
                       to void (%struct.test04.123*)*)(%struct.test04.123* %vp)
  ret void
}

; CHECK: %struct.test04 = type { %struct.test04.a*, %struct.test04.b*, %struct.test04.c* }
; CHECK: Safety data: Mismatched argument use
; CHECK: %struct.test04.123 = type { %struct.test04.a.45*, %struct.test04.b.99*, %struct.test04.c.67* }
; CHECK: Safety data: Mismatched argument use
; CHECK: %struct.test04.a = type { i32 }
; CHECK: Safety data: Mismatched argument use
; CHECK: %struct.test04.a.45 = type { i16, i16 }
; CHECK: Global instance | Mismatched argument use
; CHECK: %struct.test04.b = type { i32 }
; CHECK: Safety data: No issues found
; CHECK: %struct.test04.b.99 = type { i32 }
; CHECK: Safety data: No issues found
; CHECK: %struct.test04.c = type { i32 }
; CHECK: Safety data: No issues found
; CHECK: %struct.test04.c.67 = type { i16, i16 }
; CHECK: Safety data: No issues found


; In this case, the pointer carried safety data for mismatched argument use should
; be carried to all fields because the structure sizes do not match. This may be
; more conservative than necessary, since some fields may match.
%struct.test05 = type { %struct.test05.a* }
%struct.test05.123 = type { %struct.test05.a*, %struct.test05.b* }
%struct.test05.a = type { i32 }
%struct.test05.b = type { i32 }

@globTest05a = global %struct.test05.a zeroinitializer
@globTest05b = global %struct.test05.b zeroinitializer

define void @doSomething05(%struct.test05* %str) {
  %f1 = getelementptr %struct.test05, %struct.test05* %str, i64 0, i32 0
  %s1 = load %struct.test05.a*, %struct.test05.a** %f1
  ret void
}
define void @test05(%struct.test05.123** %pp) {
  %vp = load %struct.test05.123*, %struct.test05.123** %pp
  %f1 = getelementptr %struct.test05.123, %struct.test05.123* %vp, i64 0, i32 0
  %f2 = getelementptr %struct.test05.123, %struct.test05.123* %vp, i64 0, i32 1
  store %struct.test05.a* @globTest05a, %struct.test05.a** %f1
  store %struct.test05.b* @globTest05b, %struct.test05.b** %f2
  call void bitcast (void (%struct.test05*)* @doSomething05
                       to void (%struct.test05.123*)*)(%struct.test05.123* %vp)
  ret void
}

; CHECK: %struct.test05 = type { %struct.test05.a* }
; CHECK: Safety data: Mismatched argument use
; CHECK: %struct.test05.123 = type { %struct.test05.a*, %struct.test05.b* }
; CHECK: Safety data: Mismatched argument use
; CHECK: %struct.test05.a = type { i32 }
; CHECK: Safety data: Global instance | Mismatched argument use
; CHECK: %struct.test05.b = type { i32 }
; CHECK: Safety data: Global instance | Mismatched argument use


; In the case the argument offsets do not line up, so the remaining
; fields are all marked as mismatched.
%struct.test06 = type { %struct.test06.a, %struct.test06.b* }
%struct.test06.123 = type { i64*, %struct.test06.b* }
%struct.test06.a = type { i32, %struct.test06.a*, %struct.test06.c* }
%struct.test06.b = type { i32 }
%struct.test06.c = type { i16, i16 }

define void @doSomething06(%struct.test06* %str) {
  %f1 = getelementptr %struct.test06, %struct.test06* %str, i64 0, i32 0
  %f11 = getelementptr %struct.test06.a, %struct.test06.a* %f1, i64 0, i32 0
  store i32 0, i32* %f11
  ret void
}
define void @test06(%struct.test06.123** %pp) {
  %vp = load %struct.test06.123*, %struct.test06.123** %pp
  call void bitcast (void (%struct.test06*)* @doSomething06
                       to void (%struct.test06.123*)*)(%struct.test06.123* %vp)
  ret void
}

; CHECK: %struct.test06 = type { %struct.test06.a, %struct.test06.b* }
; CHECK: Safety data: Contains nested structure | Mismatched argument use
; CHECK: %struct.test06.123 = type { i64*, %struct.test06.b* }
; CHECK: Safety data: Mismatched argument use
; CHECK: %struct.test06.a = type { i32, %struct.test06.a*, %struct.test06.c* }
; CHECK: Safety data: Nested structure | Mismatched argument use
; CHECK: %struct.test06.b = type { i32 }
; CHECK: Safety data: Mismatched argument use
; CHECK: %struct.test06.c = type { i16, i16 }
; CHECK: Safety data: Mismatched argument use


; In this case, verify the pointer carried safety check is carried through
; multiple levels of structures at the way to %struct.test07.d
%struct.test07 = type { %struct.test07.a*, %struct.test07.b*, %struct.test07.c* }
%struct.test07.123 = type { %struct.test07.a.45*, %struct.test07.b*, %struct.test07.c.67* }
%struct.test07.a = type { %struct.test07.d* }
%struct.test07.a.45 = type { %struct.test07.d.8* }
%struct.test07.b = type { i32 }
%struct.test07.c = type { i32 }
%struct.test07.c.67 = type { i16, i16 }
%struct.test07.d = type { i32 }
%struct.test07.d.8 = type { i32 }

@globTest07a = global %struct.test07.a.45 zeroinitializer
@globTest07b = global %struct.test07.b zeroinitializer
@globTest07c = global %struct.test07.c.67 zeroinitializer
@globTest07d = global %struct.test07.d.8 zeroinitializer

define void @doSomething07(%struct.test07* %str) {
  %f1 = getelementptr %struct.test07, %struct.test07* %str, i64 0, i32 0
  %f2 = getelementptr %struct.test07, %struct.test07* %str, i64 0, i32 1
  %f3 = getelementptr %struct.test07, %struct.test07* %str, i64 0, i32 2
  %s1 = load %struct.test07.a*, %struct.test07.a** %f1
  %a1 = getelementptr %struct.test07.a, %struct.test07.a* %s1, i64 0, i32 0
  %d = load %struct.test07.d*, %struct.test07.d** %a1
  %s2 = load %struct.test07.b*, %struct.test07.b** %f2
  %s3 = load %struct.test07.c*, %struct.test07.c** %f3
  ret void
}
define void @test07(%struct.test07.123** %pp) {
  %vp = load %struct.test07.123*, %struct.test07.123** %pp
  %f1 = getelementptr %struct.test07.123, %struct.test07.123* %vp, i64 0, i32 0
  %f2 = getelementptr %struct.test07.123, %struct.test07.123* %vp, i64 0, i32 1
  %f3 = getelementptr %struct.test07.123, %struct.test07.123* %vp, i64 0, i32 2
  store %struct.test07.a.45* @globTest07a, %struct.test07.a.45** %f1
  store %struct.test07.b* @globTest07b, %struct.test07.b** %f2
  store %struct.test07.c.67* @globTest07c, %struct.test07.c.67** %f3
  %d = load %struct.test07.a.45*, %struct.test07.a.45** %f1
  %d_addr = getelementptr %struct.test07.a.45, %struct.test07.a.45* %d, i64 0, i32 0
  store %struct.test07.d.8* @globTest07d, %struct.test07.d.8** %d_addr
  call void bitcast (void (%struct.test07*)* @doSomething07
                       to void (%struct.test07.123*)*)(%struct.test07.123* %vp)
  ret void
}
; CHECK: %struct.test07.d = type { i32 }
; CHECK: Safety data: Mismatched argument use
; CHECK: %struct.test07.d.8 = type { i32 }
; CHECK: Safety data: Global instance | Mismatched argument use

