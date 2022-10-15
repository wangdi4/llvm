; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that the DTrans safety analyzer handles global variables that are
; initialized using global objects that do not require metadata.
; (CMPLRLLVM-32674)

%struct.FuncDesc = type { ptr, ptr, i32 }

@.str.67.1599 = private unnamed_addr constant [5 x i8] c"acos\00", align 1
@.str.68.1600 = private unnamed_addr constant [5 x i8] c"asin\00", align 1

@_ZN12MathFunction9functableE = internal global [3 x %struct.FuncDesc]
  [%struct.FuncDesc { ptr getelementptr inbounds ([5 x i8], ptr @.str.67.1599, i32 0, i32 0), ptr @acos, i32 1 },
  %struct.FuncDesc { ptr getelementptr inbounds ([5 x i8], ptr @.str.68.1600, i32 0, i32 0), ptr @asin, i32 1 },
  %struct.FuncDesc zeroinitializer], align 16, !intel_dtrans_type !2

declare dso_local double @acos(double)
declare dso_local double @asin(double)

!intel.dtrans.types = !{!7}

!0 = !{i8 0, i32 1}
!1 = !{i32 0, i32 0}
!2 = !{!"A", i32 3, !3}
!3 = !{%struct.FuncDesc zeroinitializer, i32 0}
!4 = !{double 0.000000e+00, i32 0}
!5 = !{!6, i32 1}
!6 = !{!"F", i1 true, i32 0, !4}
!7 = !{!"S", %struct.FuncDesc zeroinitializer, i32 3, !0, !5, !1}

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.FuncDesc
; CHECK: Safety data: Global instance | Has initializer list | Global array | Has function ptr{{ *$}}
