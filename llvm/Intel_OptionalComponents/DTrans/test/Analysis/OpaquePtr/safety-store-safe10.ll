; REQUIRES: asserts
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that the DTransSafetyAnalyzer recognizes a special pattern where:
; - A pointer is allocated
; - An offset of the pointer is stored into a field member defined to contain
;   'double*'
; - The pointer type is collected as an i8* or i8** from the allocation and
;   storing the original pointer to the allocated memory.
;
; Verify the structure is not marked with a "Mismatched element access" safety
; flag.

%struct.KernelInfo = type { i32, i64, i64, i64, i64, ptr, double, double, double, double, double, ptr, i64 }

define void @test(i64 %i63) {
 ; Allocate %struct.KernelInfo
  %i41 = tail call ptr @malloc(i64 104)

  ; Allocate double* for field member 5 of %struct.KernelInfo
  %i78 = add nuw i64 %i63, 71
  %i79 = tail call noalias ptr @malloc(i64 %i78)

  ; Store original pointer into store of allocated memory, and
  ; adjust pointer to use for field member for double*.
  %i84 = ptrtoint ptr %i79 to i64
  %i85 = add i64 %i84, 71
  %i86 = and i64 %i85, -64
  %i87 = inttoptr i64 %i86 to ptr

  ; This causes the type to also be seen as 'i8**'. This will be
  ; handled as a special case for the analysis.
  %i88 = getelementptr inbounds ptr, ptr %i87, i64 -1
  store ptr %i79, ptr %i88, align 8

  ; Store the pointer that will be used for the array of 'double' into the structure.
  %i89 = getelementptr inbounds %struct.KernelInfo, ptr %i41, i64 0, i32 5
  store ptr %i87, ptr %i89

  ; Perform loads and stores from element zero of the allocated array
  store double -1.000000e+00, ptr %i87
  %val = load double, ptr %i87

  ret void
}

declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.KernelInfo
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.KernelInfo

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{double 0.0e+00, i32 1}  ; double*
!4 = !{double 0.0e+00, i32 0}  ; double
!5 = !{%struct.KernelInfo zeroinitializer, i32 1}  ; %struct.KernelInfo*
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.KernelInfo zeroinitializer, i32 13, !1, !2, !2, !2, !2, !3, !4, !4, !4, !4, !4, !5, !2} ; { i32, i64, i64, i64, i64, double*, double, double, double, double, double, %struct.KernelInfo*, i64 }

!intel.dtrans.types = !{!8}
