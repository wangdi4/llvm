; UNSUPPORTED: enable-opaque-pointers

; This set of tests is to verify the functionality of the DTransOptBase class
; via the DTransOptBaseTest derivation to verify that global variables
; that need to be remapped to new types due to type replacement get
; remapped correctly.

; Test with legacy pass manager
; RUN: opt  < %s -whole-program-assume -internalize -S -dtrans-optbasetest \
; RUN: -dtrans-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a,struct.test04a,struct.test05a,struct.test06a,struct.test07a,struct.test08a,struct.test09a \
; RUN: 2>&1 | FileCheck %s

; Test with new pass manager
; RUN: opt  < %s -whole-program-assume -S -passes='internalize,dtrans-optbasetest' \
; RUN: -dtrans-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a,struct.test04a,struct.test05a,struct.test06a,struct.test07a,struct.test08a,struct.test09a \
; RUN: 2>&1 | FileCheck %s

; Run tests without supplying dtrans analysis info to the base class
; RUN: opt  < %s -whole-program-assume -internalize -S -dtrans-optbasetest \
; RUN: -dtrans-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a,struct.test04a,struct.test05a,struct.test06a,struct.test07a,struct.test08a,struct.test09a -dtrans-optbasetest-use-analysis=false \
; RUN: 2>&1 | FileCheck %s

; Test with new pass manager
; RUN: opt  < %s -whole-program-assume -S -passes='internalize,dtrans-optbasetest' \
; RUN: -dtrans-optbasetest-typelist=struct.test01a,struct.test02a,struct.test03a,struct.test04a,struct.test05a,struct.test06a,struct.test07a,struct.test08a,struct.test09a -dtrans-optbasetest-use-analysis=false \
; RUN: 2>&1 | FileCheck %s


; Test with a global that should not be converted
%struct.noconvert_test00a = type { i32, i32, i64 }
@g_noconvert_test00a = internal global %struct.noconvert_test00a zeroinitializer

; Globals whose types do not change are output first.

; CHECK: @g_noconvert_test00a = internal global %struct.noconvert_test00a zeroinitializer
; CHECK: @g_test09b = internal constant %struct.test09b { i32 9, void (i8*)* bitcast (void (%__DTT_struct.test09a*)* @test09a_func1.4 to void (i8*)*) }
; CHECK: @g_test09c = private constant i8* bitcast (void (%__DTT_struct.test09a*)* @test09a_alias to i8*)

; Test with just converting a type without other types referencing it
; that does not have initializers to verify that type gets converted,
; and the attributes, initializers, and alignment information is preserved
; on the replacement variable.
%struct.test01a = type { i32, i32, i64 }
@g_test01a = internal global %struct.test01a zeroinitializer, align 8
; CHECK: @g_test01a = internal global %__DTT_struct.test01a zeroinitializer, align 8


; Test with just converting a type without other types referencing it
; that has initializers to verify the replacement gets initialized with the
; same values.
%struct.test02a = type { i8, i16, i32 }
@g_test02a = private global %struct.test02a { i8 100, i16 1000, i32 10000 }, align 4
; CHECK: @g_test02a = private global %__DTT_struct.test02a { i8 100, i16 1000, i32 10000 }, align 4


; Test that a variable marked 'constant' and containing a section keeps those
; properties when the remapper replaces the type.
%struct.test03a = type { i8, i16, i32 }
@g_test03a = constant %struct.test02a { i8 100, i16 1000, i32 10000 }, section "readonly", align 4
; CHECK: @g_test03a = internal constant %__DTT_struct.test02a { i8 100, i16 1000, i32 10000 }, section "readonly", align 4


; Test with converting a type that has another type referencing it, which
; will trigger the replacement of a global variable.
%struct.test04a = type { i32, i32, i32 }
%struct.test04b = type { i32, %struct.test04a* }
@g_test04b = global %struct.test04b zeroinitializer
; CHECK: @g_test04b = internal global %__DDT_struct.test04b zeroinitializer


; Test with converting a type that causes another type to need to be mapped,
; which then causes another type to be remapped, where there are global
; initializers that will depend on each other.
%struct.test05a = type { i32, i32, i32 }
%struct.test05b = type { i32, %struct.test05a*, %struct.test05c* }
%struct.test05c = type { i32, %struct.test05b*, %struct.test05c* }
@g_test05a = global %struct.test05a { i32 1, i32 2, i32 3 }
@g_test05b = global %struct.test05b { i32 0, %struct.test05a* @g_test05a, %struct.test05c* @g_test05c }
@g_test05c = global %struct.test05c { i32 1, %struct.test05b* @g_test05b, %struct.test05c* @g_test05c }
; CHECK: @g_test05a = internal global %__DTT_struct.test05a { i32 1, i32 2, i32 3 }
; CHECK: @g_test05b = internal global %__DDT_struct.test05b { i32 0, %__DTT_struct.test05a* @g_test05a, %__DDT_struct.test05c* @g_test05c }
; CHECK: @g_test05c = internal global %__DDT_struct.test05c { i32 1, %__DDT_struct.test05b* @g_test05b, %__DDT_struct.test05c* @g_test05c }


; Test with converting a dependent type that contains an initialized array of
; function pointer addresses, where the function pointer type itself is also
; changed.
%struct.test06a = type { i32, i32, i32 }
%struct.test06b = type { i32, [2 x void (%struct.test06a*)*] }
@g_test06b = global %struct.test06b { i32 6, [2 x void (%struct.test06a*)*] [ void (%struct.test06a*)* @test06a_func1, void (%struct.test06a*)* @test06a_func2 ] }
define void @test06a_func1(%struct.test06a* %in) {
  ret void
}
define void @test06a_func2(%struct.test06a* %in) {
  ret void
}
; CHECK: @g_test06b = internal global %__DDT_struct.test06b { i32 6, [2 x void (%__DTT_struct.test06a*)*] [void (%__DTT_struct.test06a*)* @test06a_func1.1, void (%__DTT_struct.test06a*)* @test06a_func2.2] }


; Test for converting using a global variable that gets remapped in a
; non-cloned function.
%struct.test07a = type { i32, i32, i32 }
@g_test07a = global %struct.test07a zeroinitializer
define void @test07a_func() {
  %b_addr = getelementptr inbounds %struct.test07a, %struct.test07a* @g_test07a, i32 0, i32 1
  store i32 48, i32* %b_addr
  ret void
}
;CHECK: @g_test07a = internal global %__DTT_struct.test07a zeroinitializer


; Test for converting using a global variable that gets remapped in a
; cloned function.
%struct.test08a = type { i32, i32, i32 }
@g_test08a = global %struct.test08a zeroinitializer
define void @test08a_func(%struct.test08a *%in) {
  %in_c_addr = getelementptr inbounds %struct.test08a, %struct.test08a* %in, i32 0, i32 2
  %in_c_val = load i32, i32* %in_c_addr
  %c_addr = getelementptr inbounds %struct.test08a, %struct.test08a* @g_test08a, i32 0, i32 2
  store i32 %in_c_val, i32* %c_addr
  ret void
}
;CHECK: @g_test08a = internal global %__DTT_struct.test08a zeroinitializer

; Test with converting a dependent type that is initialized with the address
; of function that is going to be cloned, but the type of the global variable
; is unchanged because the initialization uses a bitcast on the function type.
%struct.test09a = type { i32, i32, i32 }
%struct.test09b = type { i32, void (i8*)* }
@g_test09b = internal constant %struct.test09b { i32 9, void (i8*)* bitcast (void (%struct.test09a*)* @test09a_func1 to void (i8*)* ) }
define void @test09a_func1(%struct.test09a* %in) {
  ret void
}

; Test to verify aliases get remapped when their aliasee changes.
@test09a_alias = internal alias void (%struct.test09a*), void (%struct.test09a*)* @test09a_func1
; CHECK: @test09a_alias = internal alias void (%__DTT_struct.test09a*), void (%__DTT_struct.test09a*)* @test09a_func1.4

; Test to verify that a global with an initializer that uses an alias gets
; converted.
@g_test09c = private constant i8* bitcast (void (%struct.test09a*)* @test09a_alias to i8*)

; Verify that the types of the variables were converted within the function bodies.
; CHECK: define internal void @test07a_func()
; CHECK:   %b_addr = getelementptr inbounds %__DTT_struct.test07a, %__DTT_struct.test07a* @g_test07a, i32 0, i32 1

; CHECK: define internal void @test08a_func.3(%__DTT_struct.test08a* %in)
; CHECK:  %in_c_addr = getelementptr inbounds %__DTT_struct.test08a, %__DTT_struct.test08a* %in, i32 0, i32 2
; CHECK:  %c_addr = getelementptr inbounds %__DTT_struct.test08a, %__DTT_struct.test08a* @g_test08a, i32 0, i32 2
