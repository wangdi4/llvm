; RUN: opt < %s -instcombine -S | FileCheck %s

; CHECK-LABEL: @frem_loop(
; CHECK:         [[PHI:%.*]] = phi double [ 1.415900e+04, %entry ], [ [[RESULT_FP:%.*]], %loop ]
; CHECK:         [[DIVIDEND_FP:%.*]] = fmul double [[PHI:%.*]], 1.680700e+04
; CHECK:         [[DIVIDEND_INT:%.*]] = fptosi double [[DIVIDEND_FP]] to i64
; CHECK-NEXT:    [[RESULT_INT:%.*]] = srem i64 [[DIVIDEND_INT]], 2147483647
; CHECK-NEXT:    [[RESULT_FP]] = sitofp i64 [[RESULT_INT]] to double

define double @frem_loop() {
entry:
  br label %loop

loop:
  %0 = phi double [ 1.415900e+04, %entry ], [ %4, %loop ]
  %1 = phi i32 [ 1, %entry ], [ %9, %loop ]
  %2 = phi double [ 0.000000e+00, %entry ], [ %8, %loop ]
  %3 = fmul double %0, 1.680700e+04
  %4 = frem double %3, 0x41DFFFFFFFC00000
  %5 = fmul double %4, 0x3E00000000200000
  %6 = fptrunc double %5 to float
  %7 = fpext float %6 to double
  %8 = fadd double %2, %7
  %9 = add nuw nsw i32 %1, 1
  %10 = icmp eq i32 %9, 501
  br i1 %10, label %exit, label %loop

exit:
  ret double %8
}

; CHECK-LABEL: @frem_loop_large(
; CHECK:         [[PHI:%.*]] = phi double [ 1.415900e+04, %entry ], [ [[RESULT_FP:%.*]], %loop ]
; CHECK:         [[DIVIDEND_FP:%.*]] = fmul double [[PHI:%.*]], 0x41DFFFFFFFC00000
; CHECK:         [[DIVIDEND_INT:%.*]] = fptosi double [[DIVIDEND_FP]] to i64
; CHECK-NEXT:    [[RESULT_INT:%.*]] = srem i64 [[DIVIDEND_INT]], 2147483647
; CHECK-NEXT:    [[RESULT_FP]] = sitofp i64 [[RESULT_INT]] to double

define double @frem_loop_large() {
entry:
  br label %loop

loop:
  %0 = phi double [ 1.415900e+04, %entry ], [ %4, %loop ]
  %1 = phi i32 [ 1, %entry ], [ %9, %loop ]
  %2 = phi double [ 0.000000e+00, %entry ], [ %8, %loop ]
  %3 = fmul double %0, 0x41DFFFFFFFC00000
  %4 = frem double %3, 0x41DFFFFFFFC00000
  %5 = fmul double %4, 0x3E00000000200000
  %6 = fptrunc double %5 to float
  %7 = fpext float %6 to double
  %8 = fadd double %2, %7
  %9 = add nuw nsw i32 %1, 1
  %10 = icmp eq i32 %9, 501
  br i1 %10, label %exit, label %loop

exit:
  ret double %8
}

; CHECK-LABEL: @frem_loop_larger(
; CHECK:         [[PHI:%.*]] = phi double [ 1.415900e+04, %entry ], [ [[RESULT_FP:%.*]], %loop ]
; CHECK:         [[DIVIDEND:%.*]] = fmul double [[PHI:%.*]], 0x41F0000000000000
; CHECK-NEXT:    [[RESULT:%.*]] = frem double [[DIVIDEND]], 0x41F0000000000000

define double @frem_loop_larger() {
entry:
  br label %loop

loop:
  %0 = phi double [ 1.415900e+04, %entry ], [ %4, %loop ]
  %1 = phi i32 [ 1, %entry ], [ %9, %loop ]
  %2 = phi double [ 0.000000e+00, %entry ], [ %8, %loop ]
  %3 = fmul double %0, 0x41F0000000000000
  %4 = frem double %3, 0x41F0000000000000
  %5 = fmul double %4, 0x3E00000000200000
  %6 = fptrunc double %5 to float
  %7 = fpext float %6 to double
  %8 = fadd double %2, %7
  %9 = add nuw nsw i32 %1, 1
  %10 = icmp eq i32 %9, 501
  br i1 %10, label %exit, label %loop

exit:
  ret double %8
}

; CHECK-LABEL: @frem_loop_zero_divisor(
; CHECK:         [[PHI:%.*]] = phi double [ 1.415900e+04, %entry ], [ [[RESULT_FP:%.*]], %loop ]
; CHECK:         [[DIVIDEND:%.*]] = fmul double [[PHI:%.*]], 1.680700e+04
; CHECK-NEXT:    [[RESULT:%.*]] = frem double [[DIVIDEND]], 0.000000e+00
define double @frem_loop_zero_divisor() {
entry:
  br label %loop

loop:
  %0 = phi double [ 1.415900e+04, %entry ], [ %4, %loop ]
  %1 = phi i32 [ 1, %entry ], [ %9, %loop ]
  %2 = phi double [ 0.000000e+00, %entry ], [ %8, %loop ]
  %3 = fmul double %0, 1.680700e+04
  %4 = frem double %3, 0.0
  %5 = fmul double %4, 0x3E00000000200000
  %6 = fptrunc double %5 to float
  %7 = fpext float %6 to double
  %8 = fadd double %2, %7
  %9 = add nuw nsw i32 %1, 1
  %10 = icmp eq i32 %9, 501
  br i1 %10, label %exit, label %loop

exit:
  ret double %8
}
