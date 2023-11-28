; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that removeRedundantNodes() gets rid of the entire loopnest after
; DSE eliminates the dead store (%t61)[i1][i2] as the loopnest itself becomes
; dead code.

; (%t61)[i1][i2] is dead because %t61 is an alloca which isn't used anywhere
; else in the function.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, %t6024 + -2, 1   <DO_LOOP>
; CHECK: |   %t6052 = (%t147)[4];
; CHECK: |
; CHECK: |   + DO i2 = 0, %t906 + -2, 1   <DO_LOOP>
; CHECK: |   |   %t6032 = (%t76)[i2];
; CHECK: |   |   %t6036 = (%t15)[i2];
; CHECK: |   |   if (trunc.i32.i1(%t6032) != 0 & %t6036 > 0.000000e+00)
; CHECK: |   |   {
; CHECK: |   |      if (i1 + 1 <= %t6052)
; CHECK: |   |      {
; CHECK: |   |         %t6041 = (%t127)[i2];
; CHECK: |   |         %t6043 = (%t81)[i1][i2];
; CHECK: |   |         %t6045 = (%t106)[i2];
; CHECK: |   |         %t6046 = %t6041  *  %t3;
; CHECK: |   |         %t6047 = %t6046  *  %t6043;
; CHECK: |   |         %t6048 = %t6047  *  %t6045;
; CHECK: |   |         (%t61)[i1][i2] = %t6048;
; CHECK: |   |      }
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK:      BEGIN REGION { modified }
; CHECK-NEXT: END REGION


define void @foo(ptr %t106, ptr %t127, ptr %t147, i64 %t35, ptr %t15, i64 %t150, double %t3, i64 %t6024, ptr %t76, i64 %t906) {
entry:
  %t61 = alloca double, i64 %t35, align 8
  %t81 = alloca double, i64 %t35, align 8
  br label %outer.loop

outer.loop:                                             ; preds = %outer.latch, %entry
  %t6026 = phi i64 [ 1, %entry ], [ %t6059, %outer.latch ]
  %t6027 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %t150, ptr nonnull elementtype(double) %t81, i64 %t6026)
  %t6028 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %t150, ptr nonnull elementtype(double) %t61, i64 %t6026)
  %t6051 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %t147, i64 5)
  %t6052 = load i32, ptr %t6051, align 4
  br label %inner.loop

inner.loop:                                             ; preds = %inner.latch, %outer.loop
  %t6030 = phi i64 [ 1, %outer.loop ], [ %t6056, %inner.latch ]
  %t6031 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %t76, i64 %t6030)
  %t6032 = load i32, ptr %t6031, align 4
  %t6033 = and i32 %t6032, 1
  %t6034 = icmp ne i32 %t6033, 0
  %t6035 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %t15, i64 %t6030)
  %t6036 = load double, ptr %t6035, align 1
  %t6037 = fcmp fast ogt double %t6036, 0.000000e+00
  %t6038 = and i1 %t6034, %t6037
  br i1 %t6038, label %t6050, label %inner.latch

t6050:                                             ; preds = %inner.loop
  %t6053 = sext i32 %t6052 to i64
  %t6054 = icmp sgt i64 %t6026, %t6053
  br i1 %t6054, label %inner.latch, label %t6039

t6039:                                             ; preds = %t6050
  %t6040 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %t127, i64 %t6030)
  %t6041 = load double, ptr %t6040, align 8
  %t6042 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %t6027, i64 %t6030)
  %t6043 = load double, ptr %t6042, align 8
  %t6044 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %t106, i64 %t6030)
  %t6045 = load double, ptr %t6044, align 8
  %t6046 = fmul fast double %t6041, %t3
  %t6047 = fmul fast double %t6046, %t6043
  %t6048 = fmul fast double %t6047, %t6045
  %t6049 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %t6028, i64 %t6030)
  store double %t6048, ptr %t6049, align 8
  br label %inner.latch

inner.latch:                                             ; preds = %t6050, %t6039, %inner.loop
  %t6056 = add nuw nsw i64 %t6030, 1
  %t6057 = icmp eq i64 %t6056, %t906
  br i1 %t6057, label %outer.latch, label %inner.loop

outer.latch:                                             ; preds = %inner.latch
  %t6059 = add nuw nsw i64 %t6026, 1
  %t6060 = icmp eq i64 %t6059, %t6024
  br i1 %t6060, label %exit, label %outer.loop

exit:
  ret void
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
