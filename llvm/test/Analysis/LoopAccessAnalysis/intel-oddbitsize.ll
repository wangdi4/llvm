; RUN: opt -S -loop-load-elim < %s 2>&1 | FileCheck %s
;
; 8547: The odd bitsize types (created by Intel FPGA extensions)
; were crashing LoopAccessAnalysis which assumes allocation size and typesize
; are the same.

; CHECK: zext i7
; CHECK: icmp slt i33

source_filename = "llvm-link"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

define void @_Z3qrdPfii() {
  %1 = alloca [64 x [64 x float]], align 16
  %2 = sext i32 undef to i33
  br label %3

; <label>:3:                                      ; preds = %0
  br label %5

; <label>:4:                                      ; preds = %13
  unreachable

; <label>:5:                                      ; preds = %13, %3
  %6 = phi i7 [ undef, %3 ], [ %14, %13 ]
  %7 = add i7 %6, -1
  %8 = zext i7 %7 to i64
  %9 = getelementptr inbounds [64 x [64 x float]], [64 x [64 x float]]* %1, i64 0, i64 undef, i64 %8
  %10 = load float, float* %9, align 4, !tbaa !0
  %11 = zext i7 %7 to i64
  %12 = getelementptr inbounds [64 x [64 x float]], [64 x [64 x float]]* %1, i64 0, i64 0, i64 %11
  store float undef, float* %12, align 4, !tbaa !0
  br label %13

; <label>:13:                                     ; preds = %5
  %14 = add i7 %6, 1
  %15 = zext i7 %14 to i33
  %16 = icmp slt i33 %15, %2
  br i1 %16, label %5, label %4
}

!0 = !{!1, !1, i64 0}
!1 = !{!"float", !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C++ TBAA"}
