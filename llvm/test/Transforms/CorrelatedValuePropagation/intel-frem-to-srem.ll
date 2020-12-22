; RUN: opt < %s -correlated-propagation -S | FileCheck %s

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

; CHECK-LABEL: @frem_multiple(
; CHECK:         [[PHI:%.*]] = phi double [ [[RESULT_FP3:%.*]], %for.cond2.preheader ], [ 0x41AC5E4712000000, %entry ]
; CHECK:         [[DIVIDEND_FP1:%.*]] = fmul fast double [[PHI:%.*]], 1.680700e+04
; CHECK:         [[DIVIDEND_INT1:%.*]] = fptosi double [[DIVIDEND_FP1]] to i64
; CHECK-NEXT:    [[RESULT_INT1:%.*]] = srem i64 [[DIVIDEND_INT1]], 2147483647
; CHECK-NEXT:    [[RESULT_FP1:%.*]] = sitofp i64 [[RESULT_INT1]] to double
; CHECK-NEXT:    [[DIVIDEND_FP2:%.*]] = fmul fast double [[RESULT_FP1]], 1.680700e+04
; CHECK-NEXT:    [[DIVIDEND_INT2:%.*]] = fptosi double [[DIVIDEND_FP2]] to i64
; CHECK-NEXT:    [[RESULT_INT2:%.*]] = srem i64 [[DIVIDEND_INT2]], 2147483647
; CHECK-NEXT:    [[RESULT_FP2:%.*]] = sitofp i64 [[RESULT_INT2]] to double
; CHECK-NEXT:    [[DIVIDEND_FP3:%.*]] = fmul fast double [[RESULT_FP2]], 1.680700e+04
; CHECK-NEXT:    [[DIVIDEND_INT3:%.*]] = fptosi double [[DIVIDEND_FP3]] to i64
; CHECK-NEXT:    [[RESULT_INT3:%.*]] = srem i64 [[DIVIDEND_INT3]], 2147483647
; CHECK-NEXT:    [[RESULT_FP3]] = sitofp i64 [[RESULT_INT3]] to double
define double @frem_multiple(i32 %n) {
entry:
  %cmp22 = icmp sgt i32 %n, 0
  br i1 %cmp22, label %for.cond2.preheader, label %for.cond.cleanup

for.cond2.preheader:
  %i1.024 = phi i32 [ %inc10, %for.cond2.preheader ], [ 0, %entry ]
  %v.023 = phi double [ %fmod7.2, %for.cond2.preheader ], [ 0x41AC5E4712000000, %entry ]
  %mul6 = fmul fast double %v.023, 1.680700e+04
  %fmod7 = frem fast double %mul6, 0x41DFFFFFFFC00000
  %mul6.1 = fmul fast double %fmod7, 1.680700e+04
  %fmod7.1 = frem fast double %mul6.1, 0x41DFFFFFFFC00000
  %mul6.2 = fmul fast double %fmod7.1, 1.680700e+04
  %fmod7.2 = frem fast double %mul6.2, 0x41DFFFFFFFC00000
  %inc10 = add nuw nsw i32 %i1.024, 1
  %exitcond = icmp eq i32 %inc10, %n
  br i1 %exitcond, label %for.cond.cleanup, label %for.cond2.preheader

for.cond.cleanup:
  %v.0.lcssa = phi double [ 0x41AC5E4712000000, %entry ], [ %fmod7.2, %for.cond2.preheader ]
  ret double %v.0.lcssa
}

; #include <math.h>

; #define NEXT() ((v = fmod(2401 * v, 8191)) / 8191.0)

; bool unknown_f();

; double frem_cyclic_phis(int n, double l, double p) {
;   double v = 3733;
;   double i = NEXT();

;   for (int i = 0; i < n; ++i) {
;     double t = NEXT();

;     int j = 0;
;     while (true) {
; #pragma nounroll
;       for (int k = 0; k < 4 && !unknown_f(); ++k, ++j) {
;         double t2 = NEXT();
;         if (t2 > p) break;
;       }
;       if (j == 0) break;
;       --j;
;     }

;   }

;   return v;
; }
define double @frem_cyclic_phis(i32 %n, double %p) {
; CHECK-LABEL: @frem_cyclic_phis(
entry:
  %cmp43 = icmp sgt i32 %n, 0
  br i1 %cmp43, label %for.body, label %for.cond.cleanup

for.cond.cleanup:
  %v.0.lcssa = phi double [ 1.979000e+03, %entry ], [ %v.3, %while.end ]
  ret double %v.0.lcssa

for.body:
; CHECK-LABEL: for.body:
; CHECK:         [[PHI1:%.*]] = phi double [ [[PHI4:%.*]], %while.end ], [ 1.979000e+03, %entry ]
; CHECK:         [[DIVIDEND_FP1:%.*]] = fmul fast double [[PHI1]], 2.401000e+03
; CHECK:         [[DIVIDEND_INT1:%.*]] = fptosi double [[DIVIDEND_FP1]] to i64
; CHECK-NEXT:    [[RESULT_INT1:%.*]] = srem i64 [[DIVIDEND_INT1]], 8191
; CHECK-NEXT:    [[RESULT_FP1:%.*]] = sitofp i64 [[RESULT_INT1]] to double
  %v.045 = phi double [ %v.3, %while.end ], [ 1.979000e+03, %entry ]
  %i1.044 = phi i32 [ %inc20, %while.end ], [ 0, %entry ]
  %mul2 = fmul fast double %v.045, 2.401000e+03
  %fmod.i39 = frem fast double %mul2, 8.191000e+03
  br label %while.cond

while.cond:
; CHECK-LABEL: while.cond:
; CHECK:         [[PHI2:%.*]] = phi double [ [[RESULT_FP1]], %for.body ], [ [[PHI4]], %cleanup15 ]
  %j.0 = phi i32 [ 0, %for.body ], [ %dec, %cleanup15 ]
  %v.1 = phi double [ %fmod.i39, %for.body ], [ %v.3, %cleanup15 ]
  %0 = add i32 %j.0, 4
  br label %land.rhs

land.rhs:
; CHECK-LABEL: land.rhs:
; CHECK:         [[PHI3:%.*]] = phi double [ [[PHI2]], %while.cond ], [ [[RESULT_FP2:%.*]], %for.inc ]
  %v.242 = phi double [ %v.1, %while.cond ], [ %fmod.i, %for.inc ]
  %j.140 = phi i32 [ %j.0, %while.cond ], [ %inc14, %for.inc ]
  %call7 = tail call zeroext i1 @_Z9unknown_fv()
  br i1 %call7, label %cleanup15, label %for.body9

for.body9:
; CHECK-LABEL: for.body9:
; CHECK:         [[DIVIDEND_FP2:%.*]] = fmul fast double [[PHI3:%.*]], 2.401000e+03
; CHECK:         [[DIVIDEND_INT2:%.*]] = fptosi double [[DIVIDEND_FP2]] to i64
; CHECK-NEXT:    [[RESULT_INT2:%.*]] = srem i64 [[DIVIDEND_INT2]], 8191
; CHECK-NEXT:    [[RESULT_FP2]] = sitofp i64 [[RESULT_INT2]] to double
  %mul10 = fmul fast double %v.242, 2.401000e+03
  %fmod.i = frem fast double %mul10, 8.191000e+03
  %div12 = fmul fast double %fmod.i, 0x3F20008004002001
  %cmp13 = fcmp fast ogt double %div12, %p
  br i1 %cmp13, label %cleanup15, label %for.inc

for.inc:
  %inc14 = add i32 %j.140, 1
  %exitcond = icmp eq i32 %inc14, %0
  br i1 %exitcond, label %cleanup15, label %land.rhs

cleanup15:
; CHECK-LABEL: cleanup15:
; CHECK:         [[PHI4]] = phi double [ [[PHI3]], %land.rhs ], [ [[RESULT_FP2]], %for.inc ], [ [[RESULT_FP2]], %for.body9 ]
  %j.1.lcssa = phi i32 [ %j.140, %land.rhs ], [ %0, %for.inc ], [ %j.140, %for.body9 ]
  %v.3 = phi double [ %v.242, %land.rhs ], [ %fmod.i, %for.inc ], [ %fmod.i, %for.body9 ]
  %cmp16 = icmp eq i32 %j.1.lcssa, 0
  %dec = add nsw i32 %j.1.lcssa, -1
  br i1 %cmp16, label %while.end, label %while.cond

while.end:
  %inc20 = add nuw nsw i32 %i1.044, 1
  %exitcond46 = icmp eq i32 %inc20, %n
  br i1 %exitcond46, label %for.cond.cleanup, label %for.body
}

declare zeroext i1 @_Z9unknown_fv()
