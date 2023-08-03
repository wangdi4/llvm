; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-memory-reduction-sinking,print<hir>" 2>&1 < %s | FileCheck %s

; This test case checks that memory reduction sinking was applied since
; the sinking refs and the store refs have the same base and shape, even if
; the alignment is lower than the size of type (float).

; HIR before transformation

;  <53>  + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;  <54>  |   + DO i2 = 0, sext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;  <14>  |   |   %Bi2 = (%B)[i2];
;  <16>  |   |   %add.1 = (%A)[i1 + 1][0]  +  1.000000e+00;
;  <17>  |   |   (%A)[i1 + 1][0] = %add.1;
;  <19>  |   |   %add.2 = (%A)[i1 + 1][1]  +  2.000000e+00;
;  <20>  |   |   (%A)[i1 + 1][1] = %add.2;
;  <22>  |   |   %add.3 = (%A)[i1 + 1][2]  +  3.000000e+00;
;  <23>  |   |   (%A)[i1 + 1][2] = %add.3;
;  <28>  |   |   %sub.1 = (%A)[%Bi2][0]  +  -4.000000e+00;
;  <29>  |   |   (%A)[%Bi2][0] = %sub.1;
;  <32>  |   |   %sub.2 = (%A)[%Bi2][1]  +  -5.000000e+00;
;  <33>  |   |   (%A)[%Bi2][1] = %sub.2;
;  <36>  |   |   %sub.3 = (%A)[%Bi2][2]  +  -6.000000e+00;
;  <37>  |   |   (%A)[%Bi2][2] = %sub.3;
;  <54>  |   + END LOOP
;  <53>  + END LOOP

; The analysis process will check the following edge:

; 16:29 (%A)[i1 + 1][0] --> (%A)[%Bi2][0] ANTI (* *) (? ?)

;  The load instructions in the IR are marked as `align 1` and the size if 4
; (float). Since both refs have the same shape and base, then we can omit the
; alignment checks.

; HIR after transformation:

; CHECK:  + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:  |      %tmp = 0.000000e+00;
; CHECK:  |      %tmp2 = 0.000000e+00;
; CHECK:  |      %tmp4 = 0.000000e+00;
; CHECK:  |   + DO i2 = 0, sext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:  |   |   %Bi2 = (%B)[i2];
; CHECK:  |   |   %tmp4 = %tmp4  +  1.000000e+00;
; CHECK:  |   |   %tmp2 = %tmp2  +  2.000000e+00;
; CHECK:  |   |   %tmp = %tmp  +  3.000000e+00;
; CHECK:  |   |   %sub.1 = (%A)[%Bi2][0]  +  -4.000000e+00;
; CHECK:  |   |   (%A)[%Bi2][0] = %sub.1;
; CHECK:  |   |   %sub.2 = (%A)[%Bi2][1]  +  -5.000000e+00;
; CHECK:  |   |   (%A)[%Bi2][1] = %sub.2;
; CHECK:  |   |   %sub.3 = (%A)[%Bi2][2]  +  -6.000000e+00;
; CHECK:  |   |   (%A)[%Bi2][2] = %sub.3;
; CHECK:  |   + END LOOP
; CHECK:  |      %add.1 = (%A)[i1 + 1][0]  +  %tmp4;
; CHECK:  |      (%A)[i1 + 1][0] = %add.1;
; CHECK:  |      %add.2 = (%A)[i1 + 1][1]  +  %tmp2;
; CHECK:  |      (%A)[i1 + 1][1] = %add.2;
; CHECK:  |      %add.3 = (%A)[i1 + 1][2]  +  %tmp;
; CHECK:  |      (%A)[i1 + 1][2] = %add.3;
; CHECK:  + END LOOP

;Module Before HIR
; ModuleID = 'special.f90'
source_filename = "special.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define void @specialred_(ptr noalias nocapture dereferenceable(4) %A, ptr noalias nocapture readonly dereferenceable(4) %"specialred_$II", ptr noalias nocapture readonly dereferenceable(4) %"specialred_$MM", ptr noalias nocapture readonly dereferenceable(4) %B) local_unnamed_addr #0 {
alloca_0:
  %n = load i32, ptr %"specialred_$II", align 1
  %rel.1 = icmp slt i32 %n, 1
  br i1 %rel.1, label %do.end_do3, label %do.body2.preheader

do.body2.preheader:                               ; preds = %alloca_0
  %m = load i32, ptr %"specialred_$MM", align 1
  %rel.2 = icmp slt i32 %m, 1
  %0 = add nuw nsw i32 %m, 1
  %1 = add nuw nsw i32 %n, 1
  %wide.trip.count54 = zext i32 %1 to i64
  %wide.trip.count = sext i32 %0 to i64
  br label %do.body2

do.body2:                                         ; preds = %do.body2.preheader, %do.end_do7
  %indvars.iv52 = phi i64 [ 1, %do.body2.preheader ], [ %indvars.iv.next53, %do.end_do7 ]
  br i1 %rel.2, label %do.end_do7, label %do.body6.preheader

do.body6.preheader:                               ; preds = %do.body2
  %"specialred_$XNP[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 400, ptr nonnull elementtype(float) %A, i64 %indvars.iv52)
  %"specialred_$XNP[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"specialred_$XNP[]", i64 1)
  %"specialred_$XNP[][]8" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"specialred_$XNP[]", i64 2)
  %"specialred_$XNP[][]14" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"specialred_$XNP[]", i64 3)
  br label %do.body6

do.body6:                                         ; preds = %do.body6.preheader, %do.body6
  %indvars.iv = phi i64 [ 1, %do.body6.preheader ], [ %indvars.iv.next, %do.body6 ]
  %"specialred_$INDEX[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %B, i64 %indvars.iv)
  %Bi2 = load i32, ptr %"specialred_$INDEX[]", align 1
  %"specialred_$XNP[][]_fetch.8" = load float, ptr %"specialred_$XNP[][]", align 1
  %add.1 = fadd reassoc ninf nsz arcp contract afn float %"specialred_$XNP[][]_fetch.8", 1.000000e+00
  store float %add.1, ptr %"specialred_$XNP[][]", align 1
  %"specialred_$XNP[][]_fetch.11" = load float, ptr %"specialred_$XNP[][]8", align 1
  %add.2 = fadd reassoc ninf nsz arcp contract afn float %"specialred_$XNP[][]_fetch.11", 2.000000e+00
  store float %add.2, ptr %"specialred_$XNP[][]8", align 1
  %"specialred_$XNP[][]_fetch.14" = load float, ptr %"specialred_$XNP[][]14", align 1
  %add.3 = fadd reassoc ninf nsz arcp contract afn float %"specialred_$XNP[][]_fetch.14", 3.000000e+00
  store float %add.3, ptr %"specialred_$XNP[][]14", align 1
  %int_sext18 = sext i32 %Bi2 to i64
  %"specialred_$XNP[]19" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 400, ptr nonnull elementtype(float) %A, i64 %int_sext18)
  %"specialred_$XNP[][]20" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"specialred_$XNP[]19", i64 1)
  %"specialred_$XNP[][]_fetch.17" = load float, ptr %"specialred_$XNP[][]20", align 1
  %sub.1 = fadd reassoc ninf nsz arcp contract afn float %"specialred_$XNP[][]_fetch.17", -4.000000e+00
  store float %sub.1, ptr %"specialred_$XNP[][]20", align 1
  %"specialred_$XNP[][]26" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"specialred_$XNP[]19", i64 2)
  %"specialred_$XNP[][]_fetch.20" = load float, ptr %"specialred_$XNP[][]26", align 1
  %sub.2 = fadd reassoc ninf nsz arcp contract afn float %"specialred_$XNP[][]_fetch.20", -5.000000e+00
  store float %sub.2, ptr %"specialred_$XNP[][]26", align 1
  %"specialred_$XNP[][]32" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"specialred_$XNP[]19", i64 3)
  %"specialred_$XNP[][]_fetch.23" = load float, ptr %"specialred_$XNP[][]32", align 1
  %sub.3 = fadd reassoc ninf nsz arcp contract afn float %"specialred_$XNP[][]_fetch.23", -6.000000e+00
  store float %sub.3, ptr %"specialred_$XNP[][]32", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %do.end_do7.loopexit, label %do.body6

do.end_do7.loopexit:                              ; preds = %do.body6
  br label %do.end_do7

do.end_do7:                                       ; preds = %do.end_do7.loopexit, %do.body2
  %indvars.iv.next53 = add nuw nsw i64 %indvars.iv52, 1
  %exitcond55 = icmp eq i64 %indvars.iv.next53, %wide.trip.count54
  br i1 %exitcond55, label %do.end_do3.loopexit, label %do.body2

do.end_do3.loopexit:                              ; preds = %do.end_do7
  br label %do.end_do3

do.end_do3:                                       ; preds = %do.end_do3.loopexit, %alloca_0
  ret void
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
