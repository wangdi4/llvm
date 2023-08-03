; RUN: opt < %s -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-annotator-cleaner 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the DTrans annotator cleaner removes the
; DTrans specific metadata and ptr annotations.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%__SOA_struct.test01 = type { ptr, ptr }
%__SOADT_struct.test01dep = type { i32, i32 }
%__SOA_struct.test02 = type { ptr, ptr }

@__intel_dtrans_aostosoa_alloc = private constant [38 x i8] c"{dtrans} AOS-to-SOA allocation {id:0}\00"
@__intel_dtrans_aostosoa_index = private constant [41 x i8] c"{dtrans} AOS-to-SOA peeling index {id:0}\00"
@__intel_dtrans_aostosoa_index.1 = private constant [41 x i8] c"{dtrans} AOS-to-SOA peeling index {id:1}\00"
@0 = private constant [1 x i8] zeroinitializer

@__unknown_annot = private constant [27 x i8] c"Unknown pointer annotation\00"
@1 = private constant [1 x i8] zeroinitializer

@__soa_struct.test01 = internal global %__SOA_struct.test01 zeroinitializer
@__soa_struct.test02 = internal global %__SOA_struct.test02 zeroinitializer
@g_test01depptr = internal unnamed_addr global %__SOADT_struct.test01dep zeroinitializer

define i32 @main(i32 %argc, ptr %argv) {
  call void @test01()
  call void @test02()
  ret i32 0
}

; Verify the llvm.ptr.annotation calls and metadata tags are removed.
; Verify that non-DTrans annotations and metadata are not removed.
define internal void @test01() {
; CHECK-LABEL: define internal void @test01
  %mem = call ptr @malloc(i64 32)

  ; This is a DTrans annotation, and should be removed.
  %annot_alloc = call ptr @llvm.ptr.annotation.p0.p0(
     ptr %mem,
     ptr getelementptr inbounds ([38 x i8], ptr @__intel_dtrans_aostosoa_alloc, i32 0, i32 0),
     ptr getelementptr inbounds ([1 x i8], ptr @0, i32 0, i32 0), i32 0, ptr null)
; CHECK-NOT:call ptr @llvm.ptr.annotation.p0.p0(ptr %mem, ptr @__intel_dtrans_aostosoa_alloc

   ; This is not a DTrans annotation, and should not be removed.
  %tmp_annot = call ptr @llvm.ptr.annotation.p0.p0(
     ptr %mem,
     ptr getelementptr inbounds ([27 x i8], ptr @__unknown_annot, i32 0, i32 0),
     ptr getelementptr inbounds ([1 x i8], ptr @1, i32 0, i32 0), i32 0, ptr null)
; CHECK: call ptr @llvm.ptr.annotation.p0.p0(ptr %mem, ptr @__unknown_annot

  %f1 = getelementptr i8, ptr %mem, i64 0, !dtrans-type !0, !unknown-md !3
; CHECK: %f1 = getelementptr i8, ptr %mem, i64 0
; CHECK-SAME: !unknown-md
; CHECK-NOT: !dtrans-type

  %f2 = getelementptr i8, ptr %mem, i64 8, !dtrans-type !1
; CHECK: %f2 = getelementptr i8, ptr %mem, i64 8
; CHECK-NOT: !dtrans-type
  ret void
}

; Verify the __intel_dtrans_aostosoa_index annotations (with and without
; suffix extensions) are removed.
define internal void @test02() {
; CHECK-LABEL: define internal void @test02

  ; Use of an annotation on a constant expression.
  %alloc_idx = call ptr @llvm.ptr.annotation.p0.p0(
     ptr getelementptr inbounds (%__SOADT_struct.test01dep, ptr @g_test01depptr, i64 0, i32 0),
     ptr getelementptr inbounds ([41 x i8], ptr @__intel_dtrans_aostosoa_index, i32 0, i32 0),
     ptr getelementptr inbounds ([1 x i8], ptr @0, i32 0, i32 0), i32 0, ptr null)

  %ptr1_to_st01 = load i32, ptr getelementptr inbounds (%__SOADT_struct.test01dep, ptr @g_test01depptr, i64 0, i32 0)
  %tmp = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 0
  %tmp1 = load ptr, ptr %tmp
  %tmp2 = zext i32 %ptr1_to_st01 to i64
  %ptr1_to_st02 = getelementptr i32, ptr %tmp1, i64 %tmp2

  ; Use of an annotation that has an extension.
  %alloc_idx1 = call ptr @llvm.ptr.annotation.p0.p0(
      ptr %ptr1_to_st02,
      ptr getelementptr inbounds ([41 x i8], ptr @__intel_dtrans_aostosoa_index.1, i32 0, i32 0),
      ptr getelementptr inbounds ([1 x i8], ptr @0, i32 0, i32 0), i32 0, ptr null)

  %st02 = load i32, ptr %ptr1_to_st02
  %tmp3 = getelementptr %__SOA_struct.test02, ptr @__soa_struct.test02, i64 0, i32 0
  %tmp4 = load ptr, ptr %tmp3
  %tmp5 = zext i32 %st02 to i64
  %ptr2_to_st01 = getelementptr i32, ptr %tmp4, i64 %tmp5

  ; Use of an annotation with a local pointer.
  %alloc_idx2 = call ptr @llvm.ptr.annotation.p0.p0(
      ptr %ptr2_to_st01,
      ptr getelementptr inbounds ([41 x i8], ptr @__intel_dtrans_aostosoa_index, i32 0, i32 0),
      ptr getelementptr inbounds ([1 x i8], ptr @0, i32 0, i32 0), i32 0, ptr null)

  %st01 = load i32, ptr %ptr2_to_st01
  %ptr3_to_st01 = getelementptr %__SOADT_struct.test01dep, ptr @g_test01depptr, i64 0, i32 0

  ; Use the result of the annotation in another instruction.
  %alloc_idx3 = call ptr @llvm.ptr.annotation.p0.p0(
      ptr %ptr3_to_st01,
      ptr getelementptr inbounds ([41 x i8], ptr @__intel_dtrans_aostosoa_index, i32 0, i32 0),
      ptr getelementptr inbounds ([1 x i8], ptr @0, i32 0, i32 0), i32 0, ptr null)
  store i32 %st01, ptr %alloc_idx3

; All calls to llvm.ptr.annotation should have been removed.
; CHECK-NOT: call ptr @llvm.ptr.annotation

; The use of the annotation result should be replaced with the 1st argument.
; CHECK: store i32 %st01, ptr %ptr3_to_st01

  ret void
}

declare ptr @malloc(i64)
declare ptr @llvm.ptr.annotation.p0.p0(ptr, ptr, ptr, i32, ptr)

!0 = !{ptr null}
!1 = !{ptr null}
!3 = !{i64 0}
