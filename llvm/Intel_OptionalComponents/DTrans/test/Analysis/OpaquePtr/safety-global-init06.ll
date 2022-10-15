; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test initializing a global variable that is an array of structures, which
; has a field that is a pointer to a function type.

%struct._ResizeFilter = type { float (float, %struct._ResizeFilter*)*, float (float, %struct._ResizeFilter*)*, float, float, float, float, [7 x float], i32, i32, i64 }
%struct.anon.0 = type { float (float, %struct._ResizeFilter*)*, double, double, double, double, i32 }

@AcquireResizeFilter.filters = internal unnamed_addr constant [2 x %struct.anon.0] [
  %struct.anon.0 { float (float, %struct._ResizeFilter*)* @Box, double 5.000000e-01, double 5.000000e-01, double 0.000000e+00, double 0.000000e+00, i32 0 },
  %struct.anon.0 { float (float, %struct._ResizeFilter*)* @Triangle, double 1.000000e+00, double 1.000000e+00, double 0.000000e+00, double 0.000000e+00, i32 1 }]

define float @Box(float %in0, %struct._ResizeFilter* "intel_dtrans_func_index"="1" %in1) !intel.dtrans.func.type !9 {
  ret float 1.000000e+00
}

define float @Triangle(float %in0, %struct._ResizeFilter* "intel_dtrans_func_index"="1" %in1) !intel.dtrans.func.type !10 {
  %test = fcmp fast olt float %in0, 1.000000e+00
  %sub = fsub fast float 1.000000e+00, %in0
  %res = select i1 %test, float %sub, float 0.000000e+00
  ret float %res
}

; CHECK-LABEL: LLVMType: %struct._ResizeFilter
; CHECK: Safety data: Has function ptr{{ *}}
; CHECK: End LLVMType: %struct._ResizeFilter

; CHECK-LABEL: LLVMType: %struct.anon.0
; CHECK: Safety data: Global instance | Has initializer list | Global array | Has function ptr{{ *}}
; CHECK: End LLVMType: %struct.anon.0

!intel.dtrans.types = !{!11, !12}

!1 = !{!"F", i1 false, i32 2, !2, !2, !3}  ; float (float, %struct._ResizeFilter*)
!2 = !{float 0.0e+00, i32 0}  ; float
!3 = !{%struct._ResizeFilter zeroinitializer, i32 1}  ; %struct._ResizeFilter*
!4 = !{!1, i32 1}  ; float (float, %struct._ResizeFilter*)*
!5 = !{!"A", i32 7, !2}  ; [7 x float]
!6 = !{i32 0, i32 0}  ; i32
!7 = !{i64 0, i32 0}  ; i64
!8 = !{double 0.0e+00, i32 0}  ; double
!9 = distinct !{!3}
!10 = distinct !{!3}
!11 = !{!"S", %struct._ResizeFilter zeroinitializer, i32 10, !4, !4, !2, !2, !2, !2, !5, !6, !6, !7} ; { float (float, %struct._ResizeFilter*)*, float (float, %struct._ResizeFilter*)*, float, float, float, float, [7 x float], i32, i32, i64 }
!12 = !{!"S", %struct.anon.0 zeroinitializer, i32 6, !4, !8, !8, !8, !8, !6} ; { float (float, %struct._ResizeFilter*)*, double, double, double, double, i32 }
