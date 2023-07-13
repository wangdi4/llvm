; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Check that an inttoptr instruction does not get marked as UNHANDLED
; when trying to infer the pointer type of the result.

%struct._ZTS10KernelInfo.KernelInfo = type { i32, i64, i64, i64, i64, ptr, double, double, double, double, double, ptr, i64 }

define void @AcquireKernelBuiltIn(ptr "intel_dtrans_func_index"="1" %i23, i64 %i202) !intel.dtrans.func.type !6 {
  %i217 = add nuw i64 %i202, 71
  %i218 = tail call noalias align 16 ptr @malloc(i64 %i217)
  %i224 = ptrtoint ptr %i218 to i64
  %i225 = add i64 %i224, 71
  %i226 = and i64 %i225, -64

  ; In this case the result gets inferred as being an i8* or a double* because
  ; the pointer gets used as an i8* for the function call, but gets stored into
  ; a structure field member that is a double*.
  %i227 = inttoptr i64 %i226 to ptr

  %i231 = getelementptr inbounds %struct._ZTS10KernelInfo.KernelInfo, ptr %i23, i64 0, i32 5
  %i232 = bitcast ptr %i231 to ptr

  store ptr %i227, ptr %i232
  %i233 = icmp eq ptr %i227, null
  tail call void @llvm.memset.p0.i64(ptr %i227, i8 0, i64 %i217, i1 false)
  ret void
}

; CHECK: %i227 = inttoptr i64 %i226 to ptr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK: Aliased types:
; CHECK:   double*
; CHECK:   i8*
; CHECK: No element pointees.

declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" ptr @malloc(i64)
declare !intel.dtrans.func.type !9 void @llvm.memset.p0.i64(ptr "intel_dtrans_func_index"="1", i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{double 0.0e+00, i32 1}  ; double*
!4 = !{double 0.0e+00, i32 0}  ; double
!5 = !{%struct._ZTS10KernelInfo.KernelInfo zeroinitializer, i32 1}  ; %struct._ZTS10KernelInfo.KernelInfo*
!6 = distinct !{!5}
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7}
!9 = distinct !{!7}
!10 = !{!"S", %struct._ZTS10KernelInfo.KernelInfo zeroinitializer, i32 13, !1, !2, !2, !2, !2, !3, !4, !4, !4, !4, !4, !5, !2} ; { i32, i64, i64, i64, i64, double*, double, double, double, double, double, %struct._ZTS10KernelInfo.KernelInfo*, i64 }

!intel.dtrans.types = !{!10}
