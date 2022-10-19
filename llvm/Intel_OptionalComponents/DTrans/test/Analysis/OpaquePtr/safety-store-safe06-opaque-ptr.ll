; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that a special case of storing allocated memory to a structure field
; where the field is a pointer to a scalar type does not get marked as a
; "Mismatched element access".

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS10KernelInfo.KernelInfo = type { i32, i64, i64, i64, i64, ptr, double, double, double, double, double, ptr, i64 }

define void @test(i64 %in) {
  %i3 = call ptr @malloc(i64 104)
  %i217 = add nuw i64 %in, 71

  ; Allocate memory that will be stored in the structure field.
  %i218 = call ptr @malloc(i64 %i217)
  %i224 = ptrtoint ptr %i218 to i64
  %i225 = add i64 %i224, 71
  %i226 = and i64 %i225, -64
  %i227 = inttoptr i64 %i226 to ptr

  %i231 = getelementptr inbounds %struct._ZTS10KernelInfo.KernelInfo, ptr %i3, i64 0, i32 5
  ; Store %i227 into a structure field that was declared as a double*
  store ptr %i227, ptr %i231

  ; Use %i227 as an i8* type to create a type alias.
  %cmp = icmp eq ptr %i227, null
  tail call void @llvm.memset.p0i8.i64(ptr %i227, i8 0, i64 %i217, i1 false)
  ; Use %i227 as the expected field type to index into the allocated memory
  %i407 = getelementptr double, ptr %i227, i64 8

  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct._ZTS10KernelInfo.KernelInfo
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct._ZTS10KernelInfo.KernelInfo

declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0
declare !intel.dtrans.func.type !8 void @llvm.memset.p0i8.i64(ptr "intel_dtrans_func_index"="1", i8, i64, i1 immarg)

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{double 0.0e+00, i32 1}  ; double*
!4 = !{double 0.0e+00, i32 0}  ; double
!5 = !{%struct._ZTS10KernelInfo.KernelInfo zeroinitializer, i32 1}  ; %struct._ZTS10KernelInfo.KernelInfo*
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = distinct !{!6}
!9 = !{!"S", %struct._ZTS10KernelInfo.KernelInfo zeroinitializer, i32 13, !1, !2, !2, !2, !2, !3, !4, !4, !4, !4, !4, !5, !2} ; { i32, i64, i64, i64, i64, double*, double, double, double, double, double, %struct._ZTS10KernelInfo.KernelInfo*, i64 }

!intel.dtrans.types = !{!9}

