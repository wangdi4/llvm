; RUN: opt < %s -S -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 -dtrans-usecrulecompat -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt < %s -S -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 -dtrans-usecrulecompat -whole-program-assume 2>&1 | FileCheck %s


; This test verifies that function attributes on the function signatures and
; call sites get modified appropriately when a call parameter is changed from
; a pointer type to an index into the peeled structure.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
%struct.test01 = type { i32, i64, %struct.test01*, %struct.test02*, i16** }
%struct.test02 = type { %struct.test01*, %struct.test01*, %struct.test01** }

define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 40)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  %alloc02 = call i8* @calloc(i64 10, i64 24)
  %struct02_mem = bitcast i8* %alloc02 to %struct.test02*

  call void @test01caller();
  call void @test02caller();
  call void @test03caller();
  call void @test04caller();
  call void @test05caller();
  ret i32 0
}


; Verify the pointer parameters at the call site get their attributes
; updated for the arguments that are changed from pointers to the structure
; to being integer index values. Also, verify the return value attributes
; are updated.
define internal void @test01caller() {
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
; CHECK-LABEL: define internal void @test01caller
; CHECK: %res = call i64 @test01callee.1(i64 %local1_val, i64 %local2_val, i8* nonnull %local3_val)


; Verify that pointer-to-pointer parameter types at the call site of a
; structure being transformed do not get their attributes stripped when
; the parameter type is converted to an integer index.
define internal void @test02caller() {
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
; CHECK-LABEL: define internal void @test02caller
; CHECK: call nonnull i64* @test02callee.2(i64* noalias %local1_val, i64* nocapture nonnull %local2_val, i8* nonnull %local3_val)


; Verify changes to attributes do not occur on the pointers of dependent
; data types that get renamed.
define internal void @test03caller() {
  %local1 = alloca %struct.test02*
  %local2 = alloca %struct.test02*
  %local1_val = load %struct.test02*, %struct.test02** %local1
  %local2_val = load %struct.test02*, %struct.test02** %local2
  %res = call nonnull %struct.test02* @test03callee(%struct.test02* noalias %local1_val, %struct.test02* nocapture nonnull %local2_val)
  ret void
}
; CHECK-LABEL: define internal void @test03caller
; CHECK: call nonnull %__SOADT_struct.test02* @test03callee.3(%__SOADT_struct.test02* noalias %local1_val, %__SOADT_struct.test02* nocapture nonnull %local2_val)


; Verify changes to attributes do not occur on non-cloned routines.
define internal void @test04caller() {
  %local1 = alloca i8*
  %local2 = alloca i8*
  %local1_val = load i8*, i8** %local1
  %local2_val = load i8*, i8** %local2
  %res = call nonnull i8* @test04callee(i8* noalias %local1_val, i8* nocapture nonnull %local2_val)
  ret void
}
; CHECK-LABEL: define internal void @test04caller
; CHECK: call nonnull i8* @test04callee(i8* noalias %local1_val, i8* nocapture nonnull %local2_val)

; Verify changes to attributes when using an indirect function call.
%struct.test05 = type { i32, void (%struct.test01*)* }
@g_test05 = global %struct.test05 { i32 1, void (%struct.test01*)* @test05callee }
define internal void @test05caller() {
  %local1 = alloca %struct.test01*
  %local1_val = load %struct.test01*, %struct.test01** %local1
  %field_addr = getelementptr inbounds %struct.test05, %struct.test05* @g_test05, i32 0, i32 1
  %func_addr = load void (%struct.test01*)*, void (%struct.test01*)** %field_addr
  call void %func_addr(%struct.test01* nocapture nonnull %local1_val)
  ret void
}
; CHECK-LABEL: void @test05caller
; CHECK: call void %func_addr(i64 %local1_val)


; Test with non-cloned call. None of the attributes should be changed.
define nonnull i8* @test04callee(i8* noalias nonnull readnone %in1, i8* nocapture %in2) {
  %sel = select i1 undef, i8* %in1, i8* %in2
  ret i8* %sel
}
; CHECK: define internal nonnull i8* @test04callee(i8* noalias nonnull readnone %in1, i8* nocapture %in2)



; The following routines will be cloned. These get printed after the
;non-cloned routines.

; Uses of the pointers to the type will be converted to pointers to the index
; type. Verify the attributes get updated on the return value, and only on
; the pointers to the transform types.
define nonnull %struct.test01* @test01callee(%struct.test01* noalias nonnull readonly dereferenceable(8) %in1, %struct.test01* nocapture nonnull readnone %in2, i8* noalias nonnull %in3) {
  %sel = select i1 undef, %struct.test01* %in1, %struct.test01* %in2
  ret %struct.test01* %sel
}
; CHECK: define internal i64 @test01callee.1(i64 %in1, i64 %in2, i8* noalias nonnull %in3) {


; Test with pointer to pointer of type being converted.
; In this case the attributes should not be changed because the
; parameters are still pointers, but are now to different types.
define nonnull %struct.test01** @test02callee(%struct.test01** noalias nonnull readnone %in1, %struct.test01** nocapture %in2, i8* nonnull %in3) {
  %sel = select i1 undef, %struct.test01** %in1, %struct.test01** %in2
  ret %struct.test01** %sel
}
; CHECK define internal nonnull i64* @test02callee.2(i64* noalias nonnull readnone %in1, i64* nocapture %in2, i8* nonnull %in3) {


; Verify changes to attributes do not occur on the pointers of dependent
; data types that get renamed
define internal nonnull %struct.test02* @test03callee(%struct.test02* noalias nonnull readnone %in1, %struct.test02* nocapture %in2) {
  %sel = select i1 undef, %struct.test02* %in1, %struct.test02* %in2
  ret %struct.test02* %sel
}
; CHECK define internal nonnull %__SOADT_struct.test02* @test03callee.3(%__SOADT_struct.test02* noalias nonnull readnone %in1, %__SOADT_struct.test02* nocapture %in2)

; This function is called for testing indirect calls.
; The checks for test01callee already cover the verification of the
; function signature attributes, so no checks needed on this one.
define internal void @test05callee(%struct.test01* noalias nonnull %in1) {
  ret void
}

declare i8* @calloc(i64, i64)
