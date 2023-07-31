; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-minmax-recognition" -print-after=hir-minmax-recognition -aa-pipeline="basic-aa" -disable-output 2>&1 < %s  | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-minmax-recognition" -print-changed -disable-output 2>&1 < %s  | FileCheck %s --check-prefix=CHECK-CHANGED

; Test checks that following code is recognized as maxnum.f64() pattern:
;   BEGIN REGION { }
;     + DO i1 = 0, sext.i32.i64(%NY_BLOCK) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;     |   + DO i2 = 0, sext.i32.i64(%NX_BLOCK) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;     |   |   %X = (%ARRAY)[i1][i2];
;     |   |   if (%X > (%BUF_3D)[-1][%BLOCK][%K][i1][i2])
;     |   |   {
;     |   |      (%BUF_3D)[-1][%BLOCK][%K][i1][i2] = %X;
;     |   |   }
;     |   + END LOOP
;     + END LOOP
;   END REGION

;*** IR Dump After HIR MinMax Recognition (hir-minmax-recognition) ***

; CHECK:  BEGIN REGION { modified }
;           + DO i1 = 0, sext.i32.i64(%NY_BLOCK) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;           |   + DO i2 = 0, sext.i32.i64(%NX_BLOCK) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;           |   |   %X = (%ARRAY)[i1][i2];
; CHECK:    |   |   %call = @llvm.maxnum.f64(%X,  (%BUF_3D)[-1][%BLOCK][%K][i1][i2]);
;           |   |   (%BUF_3D)[-1][%BLOCK][%K][i1][i2] = %call;
;           |   + END LOOP
;           + END LOOP
;         END REGION

; Verify that pass is dumped with print-changed when it triggers.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED: Dump After HIRMinMaxRecognition

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind uwtable
define void @tf_(ptr noalias nocapture readonly dereferenceable(8) %ARRAY, ptr noalias nocapture readonly dereferenceable(4) %"tf_$BLOCK", ptr noalias nocapture readonly dereferenceable(4) %"tf_$K", ptr noalias nocapture readonly dereferenceable(4) %"tf_$NX_BLOCK", ptr noalias nocapture readonly dereferenceable(4) %"tf_$NY_BLOCK", ptr noalias nocapture dereferenceable(8) %BUF_3D) local_unnamed_addr {
alloca_0:
  %NX_BLOCK = load i32, ptr %"tf_$NX_BLOCK", align 1
  %NY_BLOCK = load i32, ptr %"tf_$NY_BLOCK", align 1
  %int_sext = sext i32 %NX_BLOCK to i64
  %mul.1 = shl nsw i64 %int_sext, 3
  %int_sext2 = sext i32 %NY_BLOCK to i64
  %mul.2 = mul nsw i64 %mul.1, %int_sext2
  %mul.3 = mul nsw i64 %mul.2, 50
  %mul.4 = mul i64 %mul.2, 2500
  %K = load i32, ptr %"tf_$K", align 1
  %int_sext15 = sext i32 %K to i64
  %BLOCK = load i32, ptr %"tf_$BLOCK", align 1
  %int_sext16 = sext i32 %BLOCK to i64
  %"tf_$TAVG_BUF_3D[]8" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 4, i64 1, i64 %mul.4, ptr nonnull elementtype(double) %BUF_3D, i64 0)
  %"tf_$TAVG_BUF_3D[][]9" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul.3, ptr nonnull elementtype(double) %"tf_$TAVG_BUF_3D[]8", i64 %int_sext16)
  %"tf_$TAVG_BUF_3D[][][]10" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul.2, ptr nonnull elementtype(double) %"tf_$TAVG_BUF_3D[][]9", i64 %int_sext15)
  %rel.10.not61 = icmp slt i32 %NX_BLOCK, 1
  %rel.11.not63 = icmp slt i32 %NY_BLOCK, 1
  br i1 %rel.11.not63, label %loop_exit19, label %loop_test13.preheader.preheader

loop_test13.preheader.preheader:                  ; preds = %alloca_0
  %0 = add nsw i64 %int_sext, 1
  %1 = add nsw i64 %int_sext2, 1
  br label %loop_test13.preheader

where_mask_then11:                                ; preds = %loop_body14
  store double %X, ptr %"tf_$TAVG_BUF_3D[][][][][]12", align 1 
  br label %where_mask_endif12

where_mask_endif12:                               ; preds = %where_mask_then11, %loop_body14
  %add.6 = add nuw nsw i64 %"$loop_ctr.062", 1
  %exitcond = icmp eq i64 %add.6, %0
  br i1 %exitcond, label %loop_exit15.loopexit, label %loop_body14

loop_body14:                                      ; preds = %loop_body14.preheader, %where_mask_endif12
  %"$loop_ctr.062" = phi i64 [ %add.6, %where_mask_endif12 ], [ 1, %loop_body14.preheader ]
  %"tf_$ARRAY[][]7" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"tf_$ARRAY[]6", i64 %"$loop_ctr.062")
  %X = load double, ptr %"tf_$ARRAY[][]7", align 1 
  %"tf_$TAVG_BUF_3D[][][][][]12" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"tf_$TAVG_BUF_3D[][][][]11", i64 %"$loop_ctr.062")
  %"tf_$TAVG_BUF_3D[][][][][]_fetch.50" = load double, ptr %"tf_$TAVG_BUF_3D[][][][][]12", align 1
  %rel.7 = fcmp reassoc ninf nsz arcp contract afn ogt double %X, %"tf_$TAVG_BUF_3D[][][][][]_fetch.50"
  br i1 %rel.7, label %where_mask_then11, label %where_mask_endif12

loop_exit15.loopexit:                             ; preds = %where_mask_endif12
  br label %loop_exit15

loop_exit15:                                      ; preds = %loop_exit15.loopexit, %loop_test13.preheader
  %add.7 = add nuw nsw i64 %"$loop_ctr1.064", 1
  %exitcond65 = icmp eq i64 %add.7, %1
  br i1 %exitcond65, label %loop_exit19.loopexit, label %loop_test13.preheader

loop_test13.preheader:                            ; preds = %loop_test13.preheader.preheader, %loop_exit15
  %"$loop_ctr1.064" = phi i64 [ %add.7, %loop_exit15 ], [ 1, %loop_test13.preheader.preheader ]
  %"tf_$ARRAY[]6" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr nonnull elementtype(double) %ARRAY, i64 %"$loop_ctr1.064")
  %"tf_$TAVG_BUF_3D[][][][]11" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr nonnull elementtype(double) %"tf_$TAVG_BUF_3D[][][]10", i64 %"$loop_ctr1.064")
  br i1 %rel.10.not61, label %loop_exit15, label %loop_body14.preheader

loop_body14.preheader:                            ; preds = %loop_test13.preheader
  br label %loop_body14

loop_exit19.loopexit:                             ; preds = %loop_exit15
  br label %loop_exit19

loop_exit19:                                      ; preds = %loop_exit19.loopexit, %alloca_0
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)

