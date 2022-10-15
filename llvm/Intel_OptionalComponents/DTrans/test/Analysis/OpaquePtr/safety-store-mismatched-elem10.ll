; REQUIRES: asserts

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test a variant of the special case code used for safety-store-safe06.ll,
; where the pattern of tracing the value to the allocation is not matched,
; causing the structure to be marked as "Mismatched element access".

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS10KernelInfo.KernelInfo = type { i32, i64, i64, i64, i64, double*, double, double, double, double, double, %struct._ZTS10KernelInfo.KernelInfo*, i64 }

define void @test(i64 %in) {
  %i3 = call i8* @malloc(i64 104)
  %i23 = bitcast i8* %i3 to %struct._ZTS10KernelInfo.KernelInfo*
  %i217 = add nuw i64 %in, 71

  ; Allocate memory that will be stored in the structure field.
  %i218 = call i8* @malloc(i64 %i217)
  %i220 = ptrtoint i8* %i218 to i64
  %i221 = add i64 %i220, 71
  %i222 = and i64 %i221, -64
  %i226 = shl i64 %i222, 1
  %i227 = inttoptr i64 %i226 to i8*

  %i231 = getelementptr inbounds %struct._ZTS10KernelInfo.KernelInfo, %struct._ZTS10KernelInfo.KernelInfo* %i23, i64 0, i32 5
  %i232 = bitcast double** %i231 to i8**
  ; Store %i227 into a structure field that was declared as a double*
  store i8* %i227, i8** %i232

  ; Use %i227 as an i8* type to create a type alias.
  %cmp = icmp eq i8* %i227, null
  tail call void @llvm.memset.p0i8.i64(i8* %i227, i8 0, i64 %i217, i1 false)

  ; Use %i227 as the expected field type to index into the allocated memory
  %i406 = bitcast i8* %i227 to double*
  %i407 = getelementptr double, double* %i406, i64 8

  ret void
}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct._ZTS10KernelInfo.KernelInfo
; CHECK: Safety data: Bad casting | Mismatched element access{{ *}}
; CHECK: End LLVMType: %struct._ZTS10KernelInfo.KernelInfo

declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" i8* @malloc(i64)
declare !intel.dtrans.func.type !8 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1 immarg)

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
