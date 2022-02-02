; RUN: opt %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -disable-output 2>&1 | FileCheck %s

@g = global i32 0
@a = global i32 0

define i32 @main() {
; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        %tgu = (sext.i32.i64(%2) + -1 * sext.i32.i64(%3))/u4;
; CHECK-NEXT:        if (0 <u 4 * %tgu)
; CHECK-NEXT:        {
; CHECK-NEXT:           %red.init = %13;
; CHECK-NEXT:           %red.init1 = -1;
; CHECK-NEXT:           %red.init2 = %10;
; CHECK-NEXT:           %red.init3 = %12;
; CHECK-NEXT:           %red.init4 = -1;
; CHECK-NEXT:           %red.init5 = %9;
; CHECK-NEXT:           %phi.temp = %red.init5;
; CHECK-NEXT:           %phi.temp6 = %red.init3;
; CHECK-NEXT:           %phi.temp8 = %red.init2;
; CHECK-NEXT:           %phi.temp10 = %red.init;
; CHECK-NEXT:           %phi.temp12 = %red.init1;
; CHECK-NEXT:           %phi.temp14 = %red.init4;
; CHECK:                + DO i1 = 0, 4 * %tgu + -1, 4   <DO_LOOP> <auto-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:           |   %.vec = %phi.temp10 > 0.000000e+00;
; CHECK-NEXT:           |   %.vec16 = (%phi.temp10 > 0.000000e+00) ? i1 + <i64 0, i64 1, i64 2, i64 3> : %phi.temp12;
; CHECK-NEXT:           |   %.vec17 = (%phi.temp10 > 0.000000e+00) ? i1 + sext.i32.i64(%3) + <i64 0, i64 1, i64 2, i64 3> : %phi.temp8;
; CHECK-NEXT:           |   %.vec18 = (%phi.temp10 > 0.000000e+00) ? 0.000000e+00 : %phi.temp10;
; CHECK-NEXT:           |   %.vec19 = %phi.temp6 > 0.000000e+00;
; CHECK-NEXT:           |   %.vec20 = (%phi.temp6 > 0.000000e+00) ? i1 + <i64 0, i64 1, i64 2, i64 3> : %phi.temp14;
; CHECK-NEXT:           |   %.vec21 = (%phi.temp6 > 0.000000e+00) ? i1 + sext.i32.i64(%3) + <i64 0, i64 1, i64 2, i64 3> : %phi.temp;
; CHECK-NEXT:           |   %.vec22 = (%phi.temp6 > 0.000000e+00) ? 0.000000e+00 : %phi.temp6;
; CHECK-NEXT:           |   %phi.temp = %.vec21;
; CHECK-NEXT:           |   %phi.temp6 = %.vec22;
; CHECK-NEXT:           |   %phi.temp8 = %.vec17;
; CHECK-NEXT:           |   %phi.temp10 = %.vec18;
; CHECK-NEXT:           |   %phi.temp12 = %.vec16;
; CHECK-NEXT:           |   %phi.temp14 = %.vec20;
; CHECK-NEXT:           + END LOOP
; CHECK:                %13 = @llvm.vector.reduce.fmin.v4f64(%.vec18);
; CHECK-NEXT:           %idx.blend = (%13 == %.vec18) ? %.vec16 : <i64 9223372036854775807, i64 9223372036854775807, i64 9223372036854775807, i64 9223372036854775807>;
; CHECK-NEXT:           %vec.reduce = @llvm.vector.reduce.smin.v4i64(%idx.blend);
; CHECK-NEXT:           %mmidx.cmp. = %vec.reduce == %.vec16;
; CHECK-NEXT:           %bsfintmask = bitcast.<4 x i1>.i4(%mmidx.cmp.);
; CHECK-NEXT:           %bsf = @llvm.cttz.i4(%bsfintmask,  1);
; CHECK-NEXT:           %10 = extractelement %.vec17,  %bsf;
; CHECK-NEXT:           %12 = @llvm.vector.reduce.fmin.v4f64(%.vec22);
; CHECK-NEXT:           %idx.blend30 = (%12 == %.vec22) ? %.vec20 : <i64 9223372036854775807, i64 9223372036854775807, i64 9223372036854775807, i64 9223372036854775807>;
; CHECK-NEXT:           %vec.reduce31 = @llvm.vector.reduce.smin.v4i64(%idx.blend30);
; CHECK-NEXT:           %mmidx.cmp.32 = %vec.reduce31 == %.vec20;
; CHECK-NEXT:           %bsfintmask33 = bitcast.<4 x i1>.i4(%mmidx.cmp.32);
; CHECK-NEXT:           %bsf34 = @llvm.cttz.i4(%bsfintmask33,  1);
; CHECK-NEXT:           %9 = extractelement %.vec21,  %bsf34;
; CHECK-NEXT:        }
;
; CHECK:             + DO i1 = 4 * %tgu, sext.i32.i64(%2) + -1 * sext.i32.i64(%3) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>   <LEGAL_MAX_TC = 3> <nounroll> <novectorize> <max_trip_count = 3>
; CHECK-NEXT:        |   %10 = (%13 > 0.000000e+00) ? i1 + sext.i32.i64(%3) : %10;
; CHECK-NEXT:        |   %13 = (%13 > 0.000000e+00) ? 0.000000e+00 : %13;
; CHECK-NEXT:        |   %9 = (%12 > 0.000000e+00) ? i1 + sext.i32.i64(%3) : %9;
; CHECK-NEXT:        |   %12 = (%12 > 0.000000e+00) ? 0.000000e+00 : %12;
; CHECK-NEXT:        + END LOOP
; CHECK-NEXT:  END REGION
;
  %1 = tail call i32 (i32*, i8*, ...) @fscanf(i32* undef, i8* undef)
  %2 = load i32, i32* @a
  %3 = load i32, i32* @g
  %4 = icmp slt i32 %3, %2
  br i1 %4, label %5, label %27

5:                                                ; preds = %0
  %6 = sext i32 %3 to i64
  %7 = sext i32 %2 to i64
  br label %8

8:                                                ; preds = %8, %5
  %9 = phi i32 [ 1, %5 ], [ %20, %8 ]
  %10 = phi i32 [ 1, %5 ], [ %16, %8 ]
  %11 = phi i64 [ %6, %5 ], [ %22, %8 ]
  %12 = phi double [ undef, %5 ], [ %21, %8 ]
  %13 = phi double [ undef, %5 ], [ %17, %8 ]
  %14 = fcmp fast ogt double %13, 0.000000e+00
  %15 = trunc i64 %11 to i32
  %16 = select i1 %14, i32 %15, i32 %10
  %17 = select i1 %14, double 0.000000e+00, double %13
  %18 = fcmp fast ogt double %12, 0.000000e+00
  %19 = trunc i64 %11 to i32
  %20 = select i1 %18, i32 %19, i32 %9
  %21 = select i1 %18, double 0.000000e+00, double %12
  %22 = add nsw i64 %11, 1
  %23 = icmp eq i64 %22, %7
  br i1 %23, label %24, label %8

24:                                               ; preds = %8
  %25 = phi i32 [ %16, %8 ]
  %26 = phi i32 [ %20, %8 ]
  store i32 %2, i32* @g, align 4
  br label %27

27:
  ret i32 0
}

declare i32 @fscanf(i32*, i8*, ...)
