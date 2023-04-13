;RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-memrec,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Check that we successfully distribute sparse array reduction in combination
; with extra stripmine option that normalizes the IV bounds ref. %lb is used
; in some IV based refs like (%"bilsc2_$IFABOR")[i2 + %lb] but we don't want
; that to exist for temparrays like (%.TempArray)[0][i2].

; HIR Dump Before

;          BEGIN REGION { }
;                + DO i1 = 0, zext.i32.i64(%"bilsc2_$NFABOR_fetch.361"), 1   <DO_LOOP>
;                |   %"bilsc2_$IFABOR[]_fetch.364" = (%"bilsc2_$IFABOR")[i1];
;                |   %"bilsc2_$RA[]_fetch.368" = (%"bilsc2_$NFABOR")[i1 + %"bilsc2_$NFABOR_fetch.361"];
;                |   %add.46 = 1.000000e+00  +  1.000000e+00;
;                |   %"bilsc2_$DPDXA[]_fetch.390" = (%"bilsc2_$NFABOR")[%"bilsc2_$IFABOR[]_fetch.364"];
;                |   %"bilsc2_$SURFBO[][]_fetch.395" = (%"bilsc2_$NFABOR")[0];
;                |   %mul.49 = %add.46  *  1.000000e+00;
;                |   %add.47 = %mul.49  +  %"bilsc2_$DPDXA[]_fetch.390";
;                |   (%"bilsc2_$NFABOR")[%"bilsc2_$IFABOR[]_fetch.364"] = %add.47;
;                + END LOOP
;          END REGION

; HIR After
; CHECK:   BEGIN REGION { modified }
; CHECK:         + DO i1 = 0, (zext.i32.i64(%"bilsc2_$NFABOR_fetch.361"))/u64, 1   <DO_LOOP>
;                |   %min = (-64 * i1 + zext.i32.i64(%"bilsc2_$NFABOR_fetch.361") <= 63) ? -64 * i1 + zext.i32.i64(%"bilsc2_$NFABOR_fetch.361") : 63;
;                |
;                |   %lb = 64 * i1;
; CHECK:         |   + DO i2 = 0, 64 * i1 + %min + -1 * %lb
;                |   |   %"bilsc2_$IFABOR[]_fetch.364" = (%"bilsc2_$IFABOR")[i2 + %lb];
;                |   |   %add.46 = 1.000000e+00  +  1.000000e+00;
;                |   |   %mul.49 = %add.46  *  1.000000e+00;
;                |   |   (%.TempArray)[0][i2] = %mul.49;
; CHECK:         |   + END LOOP
;                |
;                |
; CHECK:         |   + DO i2 = 0, 64 * i1 + %min + -1 * %lb
;                |   |   %"bilsc2_$IFABOR[]_fetch.364" = (%"bilsc2_$IFABOR")[i2 + %lb];
; CHECK:         |   |   %mul.49 = (%.TempArray)[0][i2];
;                |   |   %"bilsc2_$RA[]_fetch.368" = (%"bilsc2_$NFABOR")[i2 + %"bilsc2_$NFABOR_fetch.361" + trunc.i64.i32(%lb)];
;                |   |   %"bilsc2_$DPDXA[]_fetch.390" = (%"bilsc2_$NFABOR")[%"bilsc2_$IFABOR[]_fetch.364"];
;                |   |   %"bilsc2_$SURFBO[][]_fetch.395" = (%"bilsc2_$NFABOR")[0];
;                |   |   %add.47 = %mul.49  +  %"bilsc2_$DPDXA[]_fetch.390";
;                |   |   (%"bilsc2_$NFABOR")[%"bilsc2_$IFABOR[]_fetch.364"] = %add.47;
; CHECK:         |   + END LOOP
; CHECK:         + END LOOP
;          END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @bilsc2_(ptr noalias %"bilsc2_$NFABOR", ptr %"bilsc2_$IFABOR", i32 %"bilsc2_$NFABOR_fetch.361") {
alloca_0:
  %wide.trip.count3065 = zext i32 %"bilsc2_$NFABOR_fetch.361" to i64
  br label %bb35

bb35:                                             ; preds = %bb35, %alloca_0
  %indvars.iv3063 = phi i64 [ 0, %alloca_0 ], [ %indvars.iv.next3064, %bb35 ]
  %"bilsc2_$IFABOR[]322" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr elementtype(i32) %"bilsc2_$IFABOR", i64 %indvars.iv3063)
  %"bilsc2_$IFABOR[]_fetch.364" = load i32, ptr %"bilsc2_$IFABOR[]322", align 1
  %0 = trunc i64 %indvars.iv3063 to i32
  %mul.43 = add i32 %"bilsc2_$NFABOR_fetch.361", %0
  %int_sext323 = sext i32 %mul.43 to i64
  %"bilsc2_$RA[]324" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr elementtype(double) %"bilsc2_$NFABOR", i64 %int_sext323)
  %"bilsc2_$RA[]_fetch.368" = load double, ptr %"bilsc2_$RA[]324", align 1
  %int_sext333 = sext i32 %"bilsc2_$IFABOR[]_fetch.364" to i64
  %add.46 = fadd double 1.000000e+00, 1.000000e+00
  %"bilsc2_$DPDXA[]342" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr elementtype(double) %"bilsc2_$NFABOR", i64 %int_sext333)
  %"bilsc2_$DPDXA[]_fetch.390" = load double, ptr %"bilsc2_$DPDXA[]342", align 1
  %"bilsc2_$SURFBO[][]_fetch.395" = load double, ptr %"bilsc2_$NFABOR", align 1
  %mul.49 = fmul double %add.46, 1.000000e+00
  %add.47 = fadd double %mul.49, %"bilsc2_$DPDXA[]_fetch.390"
  store double %add.47, ptr %"bilsc2_$DPDXA[]342", align 1
  %indvars.iv.next3064 = add i64 %indvars.iv3063, 1
  %exitcond3066 = icmp eq i64 %indvars.iv3063, %wide.trip.count3065
  br i1 %exitcond3066, label %bb39_endif.loopexit3146, label %bb35

bb39_endif.loopexit3146:                          ; preds = %bb35
  ret void
}

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; uselistorder directives
uselistorder ptr @llvm.intel.subscript.p0.i64.i64.p0.i64, { 2, 1, 0 }

attributes #0 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
