; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Check that all mem refs are linear (no store of "address of" to a temp)

; FORTRAN Input:
;
;       SUBROUTINE INTERP(Z,M,U,N)
;       REAL*8 Z(M,M,M),U(N,N,N)
;       INTEGER M, N
;       INTEGER I3, I2, I1
; C
;       DO 400 I3=2,M-1
;          DO 200 I2=2,M-1
;             DO 100 I1=2,M-1
;             U(2*I1-1,2*I2-1,2*I3-1)=U(2*I1-1,2*I2-1,2*I3-1)
;      >      +Z(I1,I2,I3)
;  100        CONTINUE
;             DO 200 I1=2,M-1
;             U(2*I1-2,2*I2-1,2*I3-1)=U(2*I1-2,2*I2-1,2*I3-1)
;      >      +0.5D0*(Z(I1-1,I2,I3)+Z(I1,I2,I3))
;  200     CONTINUE
;  400  CONTINUE
; C
;       RETURN
;       END

; Correct HIR:
; BEGIN REGION { }
; + DO i1 = 0, zext.i32.i64(%"interp_$M5") + -3, 1   <DO_LOOP>
; |   + DO i2 = 0, zext.i32.i64(%"interp_$M5") + -3, 1   <DO_LOOP>
; |   |   + DO i3 = 0, zext.i32.i64(%"interp_$M5") + -3, 1   <DO_LOOP>
; |   |   |   %11 = (%"interp_$U")[2 * i1 + 3][2 * i2 + 3][2 * i3 + 3];
; |   |   |   %13 = (%"interp_$Z")[i1 + 2][i2 + 2][i3 + 2];
; |   |   |   %add86 = %11  +  %13;
; |   |   |   (%"interp_$U")[2 * i1 + 3][2 * i2 + 3][2 * i3 + 3] = %add86;
; |   |   + END LOOP
; |   |
; |   |
; |   |   + DO i3 = 0, zext.i32.i64(%"interp_$M5") + -3, 1   <DO_LOOP>
; |   |   |   %21 = (%"interp_$U")[2 * i1 + 3][2 * i2 + 3][2 * i3 + 2];
; |   |   |   %24 = (%"interp_$Z")[i1 + 2][i2 + 2][i3 + 1];
; |   |   |   %26 = (%"interp_$Z")[i1 + 2][i2 + 2][i3 + 2];
; |   |   |   %add162 = %24  +  %26;
; |   |   |   %mul163 = %add162  *  5.000000e-01;
; |   |   |   %add164 = %21  +  %mul163;
; |   |   |   (%"interp_$U")[2 * i1 + 3][2 * i2 + 3][2 * i3 + 2] = %add164;
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP
; END REGION

; CHECK: BEGIN REGION

; We should not have an estimated trip count of 1.
; CHECK-NOT: MAX_TC_EST = 1
; CHECK-NOT: = &((%"interp_$U")

; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @interp_(ptr noalias nocapture readonly %"interp_$Z", ptr noalias nocapture readonly %"interp_$M", ptr noalias %"interp_$U", ptr noalias %"interp_$N") local_unnamed_addr {
alloca:
  %"interp_$M5" = load i32, ptr %"interp_$M", align 4
  %"interp_$N6" = load i32, ptr %"interp_$N", align 4
  %int_sext = sext i32 %"interp_$N6" to i64
  %mul = shl nsw i64 %int_sext, 3
  %mul41 = mul nsw i64 %mul, %int_sext
  %int_sext49 = sext i32 %"interp_$M5" to i64
  %mul50 = shl nsw i64 %int_sext49, 3
  %mul54 = mul nsw i64 %mul50, %int_sext49
  %rel = icmp slt i32 %"interp_$M5", 3
  br i1 %rel, label %bb77, label %bb8.preheader

bb8.preheader:                                    ; preds = %alloca
  %wide.trip.count = zext i32 %"interp_$M5" to i64
  %wide.trip.count282 = zext i32 %"interp_$M5" to i64
  %wide.trip.count282.le = zext i32 %"interp_$M5" to i64
  %wide.trip.count282.le.le = zext i32 %"interp_$M5" to i64
  br label %bb12.preheader

bb12.preheader:                                   ; preds = %bb8.preheader, %bb3
  %indvars.iv292 = phi i64 [ 2, %bb8.preheader ], [ %indvars.iv.next293, %bb3 ]
  %indvars.iv292.tr = trunc i64 %indvars.iv292 to i32
  %0 = shl i32 %indvars.iv292.tr, 1
  %1 = add i32 %0, -1
  %int_sext75 = sext i32 %1 to i64
  %2 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul41, ptr elementtype(double) %"interp_$U", i64 %int_sext75)
  %3 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul54, ptr elementtype(double) %"interp_$Z", i64 %indvars.iv292)
  br label %bb16.preheader

bb16.preheader:                                   ; preds = %bb12.preheader, %bb44
  %indvars.iv284 = phi i64 [ 2, %bb12.preheader ], [ %indvars.iv.next285, %bb44 ]
  %indvars.iv284.tr = trunc i64 %indvars.iv284 to i32
  %4 = shl i32 %indvars.iv284.tr, 1
  %5 = add i32 %4, -1
  %int_sext70 = sext i32 %5 to i64
  %6 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) %2, i64 %int_sext70)
  %7 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul50, ptr elementtype(double) %3, i64 %indvars.iv284)
  br label %bb16

bb16:                                             ; preds = %bb16, %bb16.preheader
  %indvars.iv = phi i64 [ 2, %bb16.preheader ], [ %indvars.iv.next, %bb16 ]
  %indvars.iv.tr = trunc i64 %indvars.iv to i32
  %8 = shl i32 %indvars.iv.tr, 1
  %9 = add i32 %8, -1
  %int_sext65 = sext i32 %9 to i64
  %10 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %6, i64 %int_sext65)
  %11 = load double, ptr %10, align 8
  %12 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %7, i64 %indvars.iv)
  %13 = load double, ptr %12, align 8
  %add86 = fadd double %11, %13
  store double %add86, ptr %10, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb43.preheader, label %bb16

bb43.preheader:                                   ; preds = %bb16
  %indvars.iv284.tr301 = trunc i64 %indvars.iv284 to i32
  %14 = shl i32 %indvars.iv284.tr301, 1
  %15 = add i32 %14, -1
  %int_sext136 = sext i32 %15 to i64
  %16 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %2, i64 %int_sext136)
  %17 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul50, ptr elementtype(double) nonnull %3, i64 %indvars.iv284)
  br label %bb43

bb43:                                             ; preds = %bb43, %bb43.preheader
  %indvars.iv277 = phi i64 [ 2, %bb43.preheader ], [ %indvars.iv.next278, %bb43 ]
  %indvars.iv277.tr = trunc i64 %indvars.iv277 to i32
  %18 = shl i32 %indvars.iv277.tr, 1
  %19 = add i32 %18, -2
  %int_sext131 = sext i32 %19 to i64
  %20 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %16, i64 %int_sext131)
  %21 = load double, ptr %20, align 8
  %22 = add nsw i64 %indvars.iv277, -1
  %23 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %17, i64 %22)
  %24 = load double, ptr %23, align 8
  %25 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %17, i64 %indvars.iv277)
  %26 = load double, ptr %25, align 8
  %add162 = fadd double %24, %26
  %mul163 = fmul double %add162, 5.000000e-01
  %add164 = fadd double %21, %mul163
  store double %add164, ptr %20, align 8
  %indvars.iv.next278 = add nuw nsw i64 %indvars.iv277, 1
  %exitcond283 = icmp eq i64 %indvars.iv.next278, %wide.trip.count282
  br i1 %exitcond283, label %bb44, label %bb43

bb44:                                             ; preds = %bb43
  %indvars.iv.next285 = add nuw nsw i64 %indvars.iv284, 1
  %exitcond291 = icmp eq i64 %indvars.iv.next285, %wide.trip.count282.le
  br i1 %exitcond291, label %bb3, label %bb16.preheader

bb3:                                              ; preds = %bb44
  %indvars.iv.next293 = add nuw nsw i64 %indvars.iv292, 1
  %exitcond299 = icmp eq i64 %indvars.iv.next293, %wide.trip.count282.le.le
  br i1 %exitcond299, label %bb77.loopexit, label %bb12.preheader

bb77.loopexit:                                    ; preds = %bb3
  br label %bb77

bb77:                                             ; preds = %bb77.loopexit, %alloca
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)

