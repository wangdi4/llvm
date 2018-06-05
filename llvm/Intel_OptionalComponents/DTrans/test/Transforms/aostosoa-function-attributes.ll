; RUN: opt < %s -S -dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -passes=dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s


; This test verifies that function attributes on the function signatures and
; call sites get modified appropriately when a call parameter is changed from
; a pointer type to an index into the peeled structure.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
%struct.test01 = type { i32, i64, %struct.test01*, %struct.test02*, i16** }
%struct.test02 = type { %struct.test01*, %struct.test01*, %struct.test01** }


; Verify the pointer parameters at the call site get their attributes
; updated for the arguments that are changed from pointers to the structure
; to being integer index values. Also, verify the return value attributes
; are updated.
define void @test01caller() {
  %local1 = alloca %struct.test01*
  %local2 = alloca %struct.test01*
  %local3 = alloca i8*
  %local1_val = load %struct.test01*, %struct.test01** %local1
  %local2_val = load %struct.test01*, %struct.test01** %local2
  %local3_val = load i8*, i8** %local3

  ; Note: this test does not actually guarantee these attributes are valid
  ; for the above LIT code, it's just checking the AOS-To-SOA effects.
  %res = call nonnull %struct.test01* @test01callee(%struct.test01* noalias %local1_val, %struct.test01* nocapture nonnull %local2_val, i8* nonnull %local3_val)
  ret void
}
; CHECK: %res = call i64 @test01callee.1(i64 %local1_val, i64 %local2_val, i8* nonnull %local3_val)


; Verify that pointer-to-pointer parameter types at the call site of a
; structure being transformed do not get their attributes stripped when
; the parameter type is converted to an integer index.
define void @test02caller() {
  %local1 = alloca %struct.test01**
  %local2 = alloca %struct.test01**
  %local3 = alloca i8*
  %local1_val = load %struct.test01**, %struct.test01*** %local1
  %local2_val = load %struct.test01**, %struct.test01*** %local2
  %local3_val = load i8*, i8** %local3

  ; Note: this test does not actually guarantee these attributes are valid
  ; for the above LIT code, it's just checking the AOS-To-SOA effects.
  %res = call nonnull %struct.test01** @test02callee(%struct.test01** noalias %local1_val, %struct.test01** nocapture nonnull %local2_val, i8* nonnull %local3_val)
  ret void
}
; CHECK: call nonnull i64* @test02callee.2(i64* noalias %local1_val, i64* nocapture nonnull %local2_val, i8* nonnull %local3_val)


; Verify changes to attributes do not occur on the pointers of dependent
; data types that get renamed.
define void @test03caller() {
  %local1 = alloca %struct.test02*
  %local2 = alloca %struct.test02*
  %local1_val = load %struct.test02*, %struct.test02** %local1
  %local2_val = load %struct.test02*, %struct.test02** %local2
  %res = call nonnull %struct.test02* @test03callee(%struct.test02* noalias %local1_val, %struct.test02* nocapture nonnull %local2_val)
  ret void
}
; CHECK: call nonnull %__SOADT_struct.test02* @test03callee.3(%__SOADT_struct.test02* noalias %local1_val, %__SOADT_struct.test02* nocapture nonnull %local2_val)


; Verify changes to attributes do not occur on non-cloned routines.
define void @test04caller() {
  %local1 = alloca i8*
  %local2 = alloca i8*
  %local1_val = load i8*, i8** %local1
  %local2_val = load i8*, i8** %local2
  %res = call nonnull i8* @test04callee(i8* noalias %local1_val, i8* nocapture nonnull %local2_val)
  ret void
}
; CHECK: call nonnull i8* @test04callee(i8* noalias %local1_val, i8* nocapture nonnull %local2_val)

; Verify changes to attributes when 'invoke' is used instead of 'call'.
; The behavior for updating attributes should be the same, but be sure
; the 'invoke' instruction is handled properly.
define void @test05caller() personality i32 (...)* @__gxx_personality_v0 {
  %local1 = alloca %struct.test01*
  %local2 = alloca %struct.test01*
  %local3 = alloca i8*
  %local1_val = load %struct.test01*, %struct.test01** %local1
  %local2_val = load %struct.test01*, %struct.test01** %local2
  %local3_val = load i8*, i8** %local3
  %res = invoke nonnull %struct.test01* @test05callee(%struct.test01* noalias %local1_val, %struct.test01* nocapture nonnull %local2_val, i8* nonnull %local3_val)
    to label %invoke.cont05 unwind label %invoke.lpad
invoke.cont05:
  ret void
invoke.lpad:
  %exn = landingpad {i8*, i32} cleanup
  unreachable
}
; CHECK: invoke i64 @test05callee.4(i64 %local1_val, i64 %local2_val, i8* nonnull %local3_val)


; Verify changes to attributes when using an indirect function call.
%struct.test06 = type { i32, [2 x void (%struct.test01*)*] }
@g_test06 = global %struct.test06 { i32 2, [2 x void (%struct.test01*)*] [ void (%struct.test01*)* @test06callee, void (%struct.test01*)* @test06callee ] }
define void @test06caller(i32 %in1) {
  %local1 = alloca %struct.test01*
  %local1_val = load %struct.test01*, %struct.test01** %local1
  %field_addr = getelementptr inbounds %struct.test06, %struct.test06* @g_test06, i32 0, i32 1, i32 %in1
  %func_addr = load void (%struct.test01*)*, void (%struct.test01*)** %field_addr
  call void %func_addr(%struct.test01* nocapture nonnull %local1_val)
  ret void
}
; CHECK: call void %func_addr(i64 %local1_val)


; Test with non-cloned call. None of the attributes should be changed.
define nonnull i8* @test04callee(i8* noalias nonnull readnone %in1, i8* nocapture %in2) {
  %sel = select i1 undef, i8* %in1, i8* %in2
  ret i8* %sel
}
; CHECK: define nonnull i8* @test04callee(i8* noalias nonnull readnone %in1, i8* nocapture %in2)



; The following routines will be cloned. These get printed after the
;non-cloned routines.

; Uses of the pointers to the type will be converted to pointers to the index
; type. Verify the attributes get updated on the return value, and only on
; the pointers to the transform types.
define nonnull %struct.test01* @test01callee(%struct.test01* noalias nonnull readonly dereferenceable(8) %in1, %struct.test01* nocapture nonnull readnone %in2, i8* noalias nonnull %in3) {
  %sel = select i1 undef, %struct.test01* %in1, %struct.test01* %in2
  ret %struct.test01* %sel
}
; CHECK: define i64 @test01callee.1(i64 %in1, i64 %in2, i8* noalias nonnull %in3) {


; Test with pointer to pointer of type being converted.
; In this case the attributes should not be changed because the
; parameters are still pointers, but are now to different types.
define nonnull %struct.test01** @test02callee(%struct.test01** noalias nonnull readnone %in1, %struct.test01** nocapture %in2, i8* nonnull %in3) {
  %sel = select i1 undef, %struct.test01** %in1, %struct.test01** %in2
  ret %struct.test01** %sel
}
; CHECK: define nonnull i64* @test02callee.2(i64* noalias nonnull readnone %in1, i64* nocapture %in2, i8* nonnull %in3) {


; Verify changes to attributes do not occur on the pointers of dependent
; data types that get renamed
define nonnull %struct.test02* @test03callee(%struct.test02* noalias nonnull readnone %in1, %struct.test02* nocapture %in2) {
  %sel = select i1 undef, %struct.test02* %in1, %struct.test02* %in2
  ret %struct.test02* %sel
}
; CHECK: define nonnull %__SOADT_struct.test02* @test03callee.3(%__SOADT_struct.test02* noalias nonnull readnone %in1, %__SOADT_struct.test02* nocapture %in2)

; This function is called for testing the usage of 'invoke'.
; The checks for test01callee already cover the verification of the
; function signature attributes, so no checks needed on this one.
define nonnull %struct.test01* @test05callee(%struct.test01* noalias nonnull readonly dereferenceable(8) %in1, %struct.test01* nocapture nonnull readnone %in2, i8* noalias nonnull %in3) {
  %sel = select i1 undef, %struct.test01* %in1, %struct.test01* %in2
  ret %struct.test01* %sel
}

; This function is called for testing indirect calls.
; The checks for test01callee already cover the verification of the
; function signature attributes, so no checks needed on this one.
define void @test06callee(%struct.test01* noalias nonnull %in1) {
  ret void
}

declare noalias i8* @malloc(i64)
declare i32 @__gxx_personality_v0(...)
