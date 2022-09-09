; UNSUPPORTED: enable-opaque-pointers

; This test verifies that type mapping occurs properly in the DTrans base.
; The DTrans Transform Test class is used to establish a set of types that
; should be remapped (__DTT_<name>). Then the DTrans Transform class should
; identify and create remapped types for all the dependent types (__DDT_<name>)

; Test with legacy pass manager
; RUN: opt  < %s -whole-program-assume -internalize -S -dtrans-optbasetest \
; RUN: -dtrans-optbasetest-typelist=struct.test01a,struct.test02a,\
; RUN:struct.test03a,struct.test04a,struct.test05a,struct.test06a,\
; RUN:struct.test07a,struct.test08a,struct.test09a,struct.test10a,\
; RUN:struct.test11a,struct.test12a,struct.test13a,struct.test14a,\
; RUN:struct.test15a,struct.test16a 2>&1 | FileCheck %s

; Test with new pass manager
; RUN: opt  < %s -whole-program-assume -S \
; RUN: -passes='internalize,dtrans-optbasetest' \
; RUN: -dtrans-optbasetest-typelist=struct.test01a,struct.test02a,\
; RUN:struct.test03a,struct.test04a,struct.test05a,struct.test06a,\
; RUN:struct.test07a,struct.test08a,struct.test09a,struct.test10a,\
; RUN:struct.test11a,struct.test12a,struct.test13a,struct.test14a,\
; RUN:struct.test15a,struct.test16a 2>&1 | FileCheck %s

; Test when base class is used without dtrans analysis parameter to
; be sure all the types and dependent types are found without relying
; on the analysis pass.
; RUN: opt  < %s -whole-program-assume -internalize -S -dtrans-optbasetest \
; RUN: -dtrans-optbasetest-typelist=struct.test01a,struct.test02a,\
; RUN:struct.test03a,struct.test04a,struct.test05a,struct.test06a,\
; RUN:struct.test07a,struct.test08a,struct.test09a,struct.test10a,\
; RUN:struct.test11a,struct.test12a,struct.test13a,struct.test14a,\
; RUN:struct.test15a,struct.test16a -dtrans-optbasetest-use-analysis=false 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -S \
; RUN: -passes='internalize,dtrans-optbasetest' \
; RUN: -dtrans-optbasetest-typelist=struct.test01a,struct.test02a,\
; RUN:struct.test03a,struct.test04a,struct.test05a,struct.test06a,\
; RUN:struct.test07a,struct.test08a,struct.test09a,struct.test10a,\
; RUN:struct.test11a,struct.test12a,struct.test13a,struct.test14a,\
; RUN:struct.test15a,struct.test16a -dtrans-optbasetest-use-analysis=false 2>&1 | FileCheck %s

; CHECK: %__DTT_struct.test01a = type { i32, i32, i32 }
; CHECK: %__DDT_struct.test02b = type { i32, %__DTT_struct.test02a* }
; CHECK: %__DTT_struct.test02a = type { i32, i32, i32 }
; CHECK: %__DDT_struct.test03c = type { i32, %__DDT_struct.test03b*, %__DDT_struct.test03c* }
; CHECK: %__DDT_struct.test03b = type { i32, %__DTT_struct.test03a*, %__DDT_struct.test03c* }
; CHECK: %__DTT_struct.test03a = type { i32, i32, i32 }
; CHECK: %__DDT_struct.test04b = type { i32, %__DTT_struct.test04a** }
; CHECK: %__DTT_struct.test04a = type { i32, i32, i32 }
; CHECK: %__DDT_struct.test05b = type { i32, %__DTT_struct.test05a* }
; CHECK: %__DTT_struct.test05a = type { i32, i32, i32, %__DTT_struct.test05a*, %__DTT_struct.test05a* }
; CHECK: %__DDT_struct.test06b = type { i32, [5 x %__DTT_struct.test06a] }
; CHECK: %__DTT_struct.test06a = type { i32, i32, i32 }
; CHECK: %__DDT_struct.test07b = type { i32, [5 x %__DTT_struct.test07a*] }
; CHECK: %__DTT_struct.test07a = type { i32, i32, i32 }
; CHECK: %__DDT_struct.test08b = type { i32, void (%__DTT_struct.test08a*)* }
; CHECK: %__DTT_struct.test08a = type { i32, i32, i32 }
; CHECK: %__DDT_struct.test09b = type { i32, void (%__DTT_struct.test09a*, i32, ...)* }
; CHECK: %__DTT_struct.test09a = type { i32, i32, i32 }
; CHECK: %__DDT_struct.test10b = type <{ i32, %__DTT_struct.test10a* }>
; CHECK: %__DTT_struct.test10a = type <{ i32, i32, i32 }>
; CHECK: %__DTT_struct.test11a = type { i32, i32, i32 }
; CHECK: %__DTT_struct.test12a = type { i32, i32, i32 }
; CHECK: %__DTT_struct.test13a = type { i32, i32, i32 }
; CHECK: %__DDT_struct.test14b = type { i32, [2 x [3 x [4 x %__DTT_struct.test14a**]]]* }
; CHECK: %__DTT_struct.test14a = type { i32, i32, i32 }
; CHECK: %__DDT_struct.test15b = type { i32, %__DTT_struct.test15a* (i8, [16 x %__DTT_struct.test15a*])* }
; CHECK: %__DTT_struct.test15a = type { i32, i32, i32 }
; CHECK: %__DDT_struct.test16b = type { <2 x %__DTT_struct.test16a*> }
; CHECK: %__DTT_struct.test16a = type { i32, i32, i32 }

; Test with just converting a type without other types referencing it.
%struct.test01a = type { i32, i32, i32 }
define void @test01() {
  %local = alloca %struct.test01a
  ret void;
}
; CHECK: define internal void @test01()
; CHECK: %local = alloca %__DTT_struct.test01a

; Test with converting a type that has another type referencing it.
%struct.test02a = type { i32, i32, i32 }
%struct.test02b = type { i32, %struct.test02a* }
define void @test02() {
  %local = alloca %struct.test02b
  ret void;
}
; CHECK: define internal void @test02()
; CHECK: %local = alloca %__DDT_struct.test02b

; Test with converting a type that causes another type to need to be mapped,
; which then causes another type to be remapped.
%struct.test03a = type { i32, i32, i32 }
%struct.test03b = type { i32, %struct.test03a*, %struct.test03c* }
%struct.test03c = type { i32, %struct.test03b*, %struct.test03c* }
define void @test03() {
  %local = alloca %struct.test03c
  ret void;
}
; CHECK: define internal void @test03()
; CHECK: %local = alloca %__DDT_struct.test03c

; Test with converting a type that has a pointer-to-pointer reference
; to it.
%struct.test04a = type { i32, i32, i32 }
%struct.test04b = type { i32, %struct.test04a** }
define void @test04() {
  %local = alloca %struct.test04b
  ret void;
}
; CHECK: define internal void @test04()
; CHECK: %local = alloca %__DDT_struct.test04b

; Test with converting a type that contains self-pointer references.
%struct.test05a = type { i32, i32, i32, %struct.test05a*, %struct.test05a* }
%struct.test05b = type { i32, %struct.test05a* }
define void @test05() {
  %local = alloca %struct.test05b
  ret void;
}
; CHECK: define internal void @test05()
; CHECK: %local = alloca %__DDT_struct.test05b

; Test with converting a type contained within an array
%struct.test06a = type { i32, i32, i32 }
%struct.test06b = type { i32, [5 x %struct.test06a] }
define void @test06() {
  %local = alloca %struct.test06b
  ret void;
}
; CHECK: define internal void @test06()
; CHECK: %local = alloca %__DDT_struct.test06b

; Test with converting a type with a pointer to the type contained in an array
%struct.test07a = type { i32, i32, i32 }
%struct.test07b = type { i32, [5 x %struct.test07a*] }
define void @test07() {
  %local = alloca %struct.test07b
  ret void;
}
; CHECK: define internal void @test07()
; CHECK: %local = alloca %__DDT_struct.test07b

; Test with converting a type that contains a function type.
%struct.test08a = type { i32, i32, i32 }
%struct.test08b = type { i32, void (%struct.test08a*)* }
define void @test08() {
  %local = alloca %struct.test08b
  ret void;
}
; CHECK: define internal void @test08()
; CHECK: %local = alloca %__DDT_struct.test08b

; Test with converting a type that contains a vararg function type.
%struct.test09a = type { i32, i32, i32 }
%struct.test09b = type { i32, void (%struct.test09a*, i32, ...)* }
define void @test09() {
  %local = alloca %struct.test09b
  ret void;
}
; CHECK: define internal void @test09()
; CHECK: %local = alloca %__DDT_struct.test09b

; Test with converting a type that has another type referencing it, when using
; packed structures to verify packed attribute is maintained.
%struct.test10a = type <{ i32, i32, i32 }>
%struct.test10b = type <{ i32, %struct.test10a* }>
define void @test10() {
  %local = alloca %struct.test10b
  ret void;
}
; CHECK: define internal void @test10()
; CHECK: %local = alloca %__DDT_struct.test10b


; Test that array types get converted when they reference a converted structure
%struct.test11a = type { i32, i32, i32 }
define void @test11() {
  %local = alloca [10 x %struct.test11a]
  ret void;
}
; CHECK: define internal void @test11() {
; CHECK: %local = alloca [10 x %__DTT_struct.test11a]

; Test that pointers to array types get converted when they reference a pointers to
; pointers of a converted structure
%struct.test12a = type { i32, i32, i32 }
define void @test12() {
  %local = alloca [10 x %struct.test12a**]*
  ret void;
}
; CHECK: define internal void @test12() {
; CHECK: %local = alloca [10 x %__DTT_struct.test12a**]*

; Test with multi-dimimensional array
%struct.test13a = type { i32, i32, i32 }
define void @test13() {
  %local = alloca [2 x [3 x [4 x %struct.test13a**]]]*
  ret void;
}
; CHECK: define internal void @test13() {
; CHECK: %local = alloca [2 x [3 x [4 x %__DTT_struct.test13a**]]]*

; Test with multi-dimimensional array that triggers a dependent type conversion.
%struct.test14a = type { i32, i32, i32 }
%struct.test14b = type { i32, [2 x [3 x [4 x %struct.test14a**]]]* }
define void @test14() {
  %local = alloca %struct.test14b
  ret void;
}
; CHECK: define internal void @test14() {
; CHECK: %local = alloca %__DDT_struct.test14b

; Test with array of function pointers which use a dependent type, and
; returns dependent type.
%struct.test15a = type { i32, i32, i32 }
%struct.test15b = type { i32, %struct.test15a* (i8, [16 x %struct.test15a*])* }
define void @test15() {
  %local = alloca %struct.test15b
  ret void;
}
; CHECK: define internal void @test15() {
; CHECK: %local = alloca %__DDT_struct.test15b

; Test to verify that vector types containing pointers to a structure
; type being converted get remapped to a new type.
%struct.test16a = type { i32, i32, i32}
%struct.test16b = type { <2 x %struct.test16a*> }
define void @test16() {
  %local1 = alloca %struct.test16b*
  %local2 = alloca <2 x %struct.test16a*>
  %ins = insertelement <2 x %struct.test16a*> undef, %struct.test16a* undef, i32 0
  ret void;
}
; CHECK: define internal void @test16() {
; CHECK: %local1 = alloca %__DDT_struct.test16b*
; CHECK: %local2 = alloca <2 x %__DTT_struct.test16a*>
; CHECK: %ins = insertelement <2 x %__DTT_struct.test16a*> undef, %__DTT_struct.test16a* undef, i32 0
