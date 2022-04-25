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
;   if (p == 0) return 0;
;   NEXT();

;   for (int i = 0; i < n; ++i) {
;     double t = NEXT();

;     if (t > l) NEXT();
;     int j = 0;
;     while (j < p) {
; #pragma nounroll
;       for (int k = 0; k < 4 && !unknown_f(); ++k, ++j) {
;         double t2 = NEXT();
;         if (t2 > p) break;
;       }
;       if (j == 0) NEXT();
;     }

;   }

;   return v;
; }
define noundef double @frem_cyclic_phis(i32 noundef %n, double noundef %l, i32 noundef %p) {
; CHECK-LABEL: @frem_cyclic_phis(
entry:
  %cmp = icmp eq i32 %p, 0
  br i1 %cmp, label %cleanup37, label %for.cond.preheader

for.cond.preheader:
  %cmp258 = icmp sgt i32 %n, 0
  br i1 %cmp258, label %for.body.lr.ph, label %cleanup37

for.body.lr.ph:
  %conv = uitofp i32 %p to double
  br label %for.body

for.body:
; CHECK-LABEL: for.body:
; CHECK:         [[PHI1:%.*]] = phi double [ 1.979000e+03, %for.body.lr.ph ], [ [[PHI4:%.*]], %while.cond.while.end_crit_edge ]
; CHECK:         [[DIVIDEND_FP1:%.*]] = fmul fast double [[PHI1]], 2.401000e+03
; CHECK:         [[DIVIDEND_INT1:%.*]] = fptosi double [[DIVIDEND_FP1]] to i64
; CHECK-NEXT:    [[RESULT_INT1:%.*]] = srem i64 [[DIVIDEND_INT1]], 8191
; CHECK-NEXT:    [[RESULT_FP1:%.*]] = sitofp i64 [[RESULT_INT1]] to double
  %v.060 = phi double [ 1.979000e+03, %for.body.lr.ph ], [ %v.5, %while.cond.while.end_crit_edge ]
  %i1.059 = phi i32 [ 0, %for.body.lr.ph ], [ %inc33, %while.cond.while.end_crit_edge ]
  %mul3 = fmul fast double %v.060, 2.401000e+03
  %fmod.i = frem fast double %mul3, 8.191000e+03
  %div5 = fmul fast double %fmod.i, 0x3F20008004002001
  %cmp6 = fcmp fast ogt double %div5, %l
  br i1 %cmp6, label %if.then7, label %for.cond13.preheader.preheader

if.then7:
; CHECK-LABEL: if.then7:
; CHECK:         [[DIVIDEND_FP2:%.*]] = fmul fast double [[RESULT_FP1]], 2.401000e+03
; CHECK:         [[DIVIDEND_INT2:%.*]] = fptosi double [[DIVIDEND_FP2]] to i64
; CHECK-NEXT:    [[RESULT_INT2:%.*]] = srem i64 [[DIVIDEND_INT2]], 8191
; CHECK-NEXT:    [[RESULT_FP2:%.*]] = sitofp i64 [[RESULT_INT2]] to double
  %mul8 = fmul fast double %fmod.i, 2.401000e+03
  %fmod.i49 = frem fast double %mul8, 8.191000e+03
  br label %for.cond13.preheader.preheader

for.cond13.preheader.preheader:
; CHECK-LABEL: for.cond13.preheader.preheader:
; CHECK:         [[PHI2:%.*]] = phi double [ [[RESULT_FP1]], %for.body ], [ [[RESULT_FP2:%.*]], %if.then7 ]
  %v.256.ph = phi double [ %fmod.i, %for.body ], [ %fmod.i49, %if.then7 ]
  br label %for.cond13.preheader

for.cond13.preheader:
; CHECK-LABEL: for.cond13.preheader:
; CHECK:         [[PHI3:%.*]] = phi double [ [[PHI6:%.*]], %if.end31 ], [ [[PHI2]], %for.cond13.preheader.preheader ]
  %j.057 = phi i32 [ %j.1.lcssa, %if.end31 ], [ 0, %for.cond13.preheader.preheader ]
  %v.256 = phi double [ %v.5, %if.end31 ], [ %v.256.ph, %for.cond13.preheader.preheader ]
  %0 = add i32 %j.057, 4
  br label %land.rhs

land.rhs:
; CHECK-LABEL: land.rhs:
; CHECK:         [[PHI4:%.*]] = phi double [ [[PHI3]], %for.cond13.preheader ], [ [[RESULT_FP2:%.*]], %for.inc ]
  %j.153 = phi i32 [ %j.057, %for.cond13.preheader ], [ %inc24, %for.inc ]
  %v.352 = phi double [ %v.256, %for.cond13.preheader ], [ %fmod.i50, %for.inc ]
  %call15 = tail call noundef zeroext i1 @_Z9unknown_fv()
  br i1 %call15, label %cleanup25, label %for.body17

for.body17:
; CHECK-LABEL: for.body17:
; CHECK:         [[DIVIDEND_FP3:%.*]] = fmul fast double [[PHI4:%.*]], 2.401000e+03
; CHECK:         [[DIVIDEND_INT3:%.*]] = fptosi double [[DIVIDEND_FP3]] to i64
; CHECK-NEXT:    [[RESULT_INT3:%.*]] = srem i64 [[DIVIDEND_INT3]], 8191
; CHECK-NEXT:    [[RESULT_FP3:%.*]] = sitofp i64 [[RESULT_INT3]] to double
  %mul18 = fmul fast double %v.352, 2.401000e+03
  %fmod.i50 = frem fast double %mul18, 8.191000e+03
  %div20 = fmul fast double %fmod.i50, 0x3F20008004002001
  %cmp21 = fcmp fast ogt double %div20, %conv
  br i1 %cmp21, label %cleanup25, label %for.inc

for.inc:
  %inc24 = add i32 %j.153, 1
  %exitcond.not = icmp eq i32 %inc24, %0
  br i1 %exitcond.not, label %cleanup25, label %land.rhs

cleanup25:
; CHECK-LABEL: cleanup25:
; CHECK:         [[PHI5:%.*]] = phi double [ [[PHI4]], %land.rhs ], [ [[RESULT_FP3]], %for.inc ], [ [[RESULT_FP3]], %for.body17 ]
  %j.1.lcssa = phi i32 [ %j.153, %land.rhs ], [ %0, %for.inc ], [ %j.153, %for.body17 ]
  %v.4 = phi double [ %v.352, %land.rhs ], [ %fmod.i50, %for.inc ], [ %fmod.i50, %for.body17 ]
  %cmp26 = icmp eq i32 %j.1.lcssa, 0
  br i1 %cmp26, label %if.then27, label %if.end31

if.then27:
; CHECK-LABEL: if.then27:
; CHECK:         [[DIVIDEND_FP4:%.*]] = fmul fast double [[PHI5]], 2.401000e+03
; CHECK:         [[DIVIDEND_INT4:%.*]] = fptosi double [[DIVIDEND_FP4]] to i64
; CHECK-NEXT:    [[RESULT_INT4:%.*]] = srem i64 [[DIVIDEND_INT4]], 8191
; CHECK-NEXT:    [[RESULT_FP4:%.*]] = sitofp i64 [[RESULT_INT4]] to double
  %mul28 = fmul fast double %v.4, 2.401000e+03
  %fmod.i51 = frem fast double %mul28, 8.191000e+03
  br label %if.end31

if.end31:
; CHECK-LABEL: if.end31:
; CHECK:         [[PHI6]] = phi double [ [[RESULT_FP4]], %if.then27 ], [ [[PHI5]], %cleanup25 ]
  %v.5 = phi double [ %fmod.i51, %if.then27 ], [ %v.4, %cleanup25 ]
  %cmp12 = icmp ult i32 %j.1.lcssa, %p
  br i1 %cmp12, label %for.cond13.preheader, label %while.cond.while.end_crit_edge

while.cond.while.end_crit_edge:
  %inc33 = add nuw nsw i32 %i1.059, 1
  %exitcond62.not = icmp eq i32 %inc33, %n
  br i1 %exitcond62.not, label %cleanup37, label %for.body

cleanup37:
  %retval.0 = phi double [ 0.000000e+00, %entry ], [ 1.979000e+03, %for.cond.preheader ], [ %v.5, %while.cond.while.end_crit_edge ]
  ret double %retval.0
}

declare zeroext i1 @_Z9unknown_fv()
