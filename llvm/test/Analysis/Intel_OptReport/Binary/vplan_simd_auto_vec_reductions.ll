; RUN: llc < %s -O3 -intel-opt-report=high -intel-opt-report-emitter=mir -opt-report-embed -enable-protobuf-opt-report=true -debug-only=opt-report-support-utils 2>&1 | FileCheck %s
; REQUIRES: asserts, proto_bor

; CHECK:      Global Mloop optimization report for : vec_reductions
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN
; CHECK:          remark #15301: SIMD LOOP WAS VECTORIZED
; CHECK:          remark #25588: Loop has SIMD reduction
; CHECK:      LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN
; CHECK:          remark #15300: LOOP WAS VECTORIZED
; CHECK:          remark #25587: Loop has reduction
; CHECK:      LOOP END
; CHECK-NEXT: =================================================================

; CHECK-LABEL:    --- Start Protobuf Binary OptReport Printer ---
; CHECK-NEXT:     Version: 1.5
; CHECK-NEXT:     Property Message Map:
; CHECK-DAG:        C_LOOP_VEC_HAS_REDUCTION --> Loop has reduction
; CHECK-DAG:        C_LOOP_VEC_SIMD --> SIMD LOOP WAS VECTORIZED
; CHECK-DAG:        C_LOOP_VECTORIZED --> LOOP WAS VECTORIZED
; CHECK-DAG:        C_LOOP_HAS_SIMD_REDUCTION --> Loop has SIMD reduction
; CHECK-DAG:        C_LOOP_VEC_VL --> vectorization support: vector length %s
; CHECK-NEXT:     Number of reports: 2

; CHECK-DAG:      === Loop Begin ===
; CHECK-DAG:      Anchor ID: e35f65e74729759c0c4d00b9f62bbf93
; CHECK-DAG:      Number of remarks: 3
; CHECK-DAG:        Property: C_LOOP_VECTORIZED, Remark ID: 15300, Remark Args:
; CHECK-DAG:        Property: C_LOOP_VEC_VL, Remark ID: 15305, Remark Args: 4
; CHECK-DAG:        Property: C_LOOP_VEC_HAS_REDUCTION, Remark ID: 25587, Remark Args:
; CHECK-DAG:      ==== Loop End ====

; CHECK-DAG:      === Loop Begin ===
; CHECK-DAG:      Anchor ID: dd6fe9abf9686209fe1121a4e330f436
; CHECK-DAG:      Number of remarks: 3
; CHECK-DAG:        Property: C_LOOP_VEC_SIMD, Remark ID: 15301, Remark Args:
; CHECK-DAG:        Property: C_LOOP_VEC_VL, Remark ID: 15305, Remark Args: 4
; CHECK-DAG:        Property: C_LOOP_HAS_SIMD_REDUCTION, Remark ID: 25588, Remark Args:
; CHECK-DAG:      ==== Loop End ====
; CHECK:          --- End Protobuf Binary OptReport Printer ---

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define float @vec_reductions(float* nocapture %a) {
entry:
  %x = alloca float, align 4
  store float 0.000000e+00, float* %x, align 4
  %gepload = load float, float* %x, align 4
  br label %loop.35

loop.35:                                          ; preds = %loop.35, %entry
  %i1.i64.0 = phi i64 [ 0, %entry ], [ %nextivloop.35, %loop.35 ]
  %t17.0 = phi <4 x float> [ <float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00>, %entry ], [ %0, %loop.35 ]
  %0 = fadd <4 x float> %t17.0, <float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01>
  %nextivloop.35 = add nuw nsw i64 %i1.i64.0, 4
  %condloop.35 = icmp ult i64 %i1.i64.0, 996
  br i1 %condloop.35, label %loop.35, label %afterloop.35, !llvm.loop !0

afterloop.35:                                     ; preds = %loop.35
  %1 = call float @llvm.vector.reduce.fadd.v4f32(float %gepload, <4 x float> %0)
  store float %1, float* %x, align 4
  br label %loop.50

loop.50:                                          ; preds = %loop.50, %afterloop.35
  %t20.0 = phi <4 x float> [ zeroinitializer, %afterloop.35 ], [ %2, %loop.50 ]
  %i1.i6410.0 = phi i64 [ 0, %afterloop.35 ], [ %nextivloop.50, %loop.50 ]
  %2 = fadd fast <4 x float> %t20.0, <float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01>
  %nextivloop.50 = add nuw nsw i64 %i1.i6410.0, 4
  %condloop.50 = icmp ult i64 %i1.i6410.0, 996
  br i1 %condloop.50, label %loop.50, label %afterloop.50, !llvm.loop !33

afterloop.50:                                     ; preds = %loop.50
  %3 = call fast float @llvm.vector.reduce.fadd.v4f32(float 0.000000e+00, <4 x float> %2)
  %res = fadd float %1, %3
  ret float %res
}

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare float @llvm.vector.reduce.fadd.v4f32(float, <4 x float>) #1

attributes #0 = { nounwind }

!0 = distinct !{!0, !1, !2, !3, !4, !5}
!1 = !{!"llvm.loop.vectorize.enable", i32 1}
!2 = !{!"llvm.loop.vectorize.width", i32 1}
!3 = !{!"llvm.loop.interleave.count", i32 1}
!4 = !{!"llvm.loop.isvectorized", i32 1}
!5 = distinct !{!"intel.optreport", !7}
!7 = !{!"intel.optreport.remarks", !8, !9, !10, !11, !12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32}
!8 = !{!"intel.optreport.remark", i32 15301}
!9 = !{!"intel.optreport.remark", i32 15305, !"4"}
!10 = !{!"intel.optreport.remark", i32 15475}
!11 = !{!"intel.optreport.remark", i32 15476, !"4.000000"}
!12 = !{!"intel.optreport.remark", i32 15477, !"5.000000"}
!13 = !{!"intel.optreport.remark", i32 15478, !"0.796875"}
!14 = !{!"intel.optreport.remark", i32 15482, !"0"}
!15 = !{!"intel.optreport.remark", i32 15484, !"0"}
!16 = !{!"intel.optreport.remark", i32 15485, !"0"}
!17 = !{!"intel.optreport.remark", i32 15488}
!18 = !{!"intel.optreport.remark", i32 15447}
!19 = !{!"intel.optreport.remark", i32 15450, !"0"}
!20 = !{!"intel.optreport.remark", i32 15451, !"0"}
!21 = !{!"intel.optreport.remark", i32 15456, !"0"}
!22 = !{!"intel.optreport.remark", i32 15457, !"0"}
!23 = !{!"intel.optreport.remark", i32 15458, !"0"}
!24 = !{!"intel.optreport.remark", i32 15459, !"0"}
!25 = !{!"intel.optreport.remark", i32 15462, !"0"}
!26 = !{!"intel.optreport.remark", i32 15463, !"0"}
!27 = !{!"intel.optreport.remark", i32 15554, !"0"}
!28 = !{!"intel.optreport.remark", i32 15555, !"0"}
!29 = !{!"intel.optreport.remark", i32 15556, !"0"}
!30 = !{!"intel.optreport.remark", i32 15557, !"0"}
!31 = !{!"intel.optreport.remark", i32 15474}
!32 = !{!"intel.optreport.remark", i32 25588}
!33 = distinct !{!33, !2, !3, !34}
!34 = distinct !{!"intel.optreport", !36}
!36 = !{!"intel.optreport.remarks", !37, !9, !10, !11, !12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !38}
!37 = !{!"intel.optreport.remark", i32 15300}
!38 = !{!"intel.optreport.remark", i32 25587}
