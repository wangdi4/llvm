; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans
; RUN: llvm-link -S %s %S/Inputs/intel-dtrans-metadata02a.ll | FileCheck %s

; Test that when merging in !intel_dtrans_type on global variables the types
; referenced in the declaration get remapped based on types already merged
; in; and there are no references to deleted types left.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.network = type { [200 x i8], [200 x i8], i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, double, i64, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i64, i64, i64, i64, i64 }
%struct.node = type { i64, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i64, i64, i32, i32 }
%struct.arc = type { i32, i64, ptr, ptr, i16, ptr, ptr, i64, i64 }

!intel.dtrans.types = !{!0, !7, !9}

!0 = !{!"S", %struct.network zeroinitializer, i32 33, !1, !1, !3, !3, !3, !3, !3, !3, !3, !3, !3, !3, !3, !3, !3, !3, !3, !3, !3, !4, !3, !5, !5, !6, !6, !6, !6, !6, !3, !3, !3, !3, !3}
!1 = !{!"A", i32 200, !2}
!2 = !{i8 0, i32 0}
!3 = !{i64 0, i32 0}
!4 = !{double 0.000000e+00, i32 0}
!5 = !{%struct.node zeroinitializer, i32 1}
!6 = !{%struct.arc zeroinitializer, i32 1}
!7 = !{!"S", %struct.node zeroinitializer, i32 14, !3, !8, !5, !5, !5, !5, !6, !6, !6, !6, !3, !3, !8, !8}
!8 = !{i32 0, i32 0}
!9 = !{!"S", %struct.arc zeroinitializer, i32 9, !8, !3, !5, !5, !10, !6, !6, !3, !3}
!10 = !{i16 0, i32 0}

; CHECK: @stderr = external dso_local local_unnamed_addr global ptr, align 8, !intel_dtrans_type ![[MD:[0-9]+]]
; CHECK-NOT: !{%"type 0x
; CHECK ![[MD]]= !{%struct._IO_FILE zeroinitializer, i32 1}

; end INTEL_FEATURE_SW_DTRANS
