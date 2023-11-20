; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir-dd-analysis>,hir-unroll-and-jam" -hir-dd-analysis-verify=Region -print-before=hir-unroll-and-jam -print-after=hir-unroll-and-jam < %s 2>&1 | FileCheck %s

; Verify that we don't give up on unroll and jam when all the dependencies are
; well-known (no (*) dependence). Previously locality analysis was considering
; them as possible aliasing issue.

; CHECK: (%RES)[-1 * i1 + %II + -3][-1 * i2 + %JJ + -3][-1 * i3 + %KK + -3] --> (%RES)[-1 * i1 + %II + -3][-1 * i2 + %JJ + -2][-1 * i3 + %KK + -3] FLOW (= < =) (0 1 0)
; CHECK: (%RES)[-1 * i1 + %II + -3][-1 * i2 + %JJ + -3][-1 * i3 + %KK + -3] --> (%RES)[-1 * i1 + %II + -3][-1 * i2 + %JJ + -3][-1 * i3 + %KK + -2] FLOW (= = <) (0 0 1)
; CHECK: (%RES)[-1 * i1 + %II + -3][-1 * i2 + %JJ + -3][-1 * i3 + %KK + -3] --> (%RES)[-1 * i1 + %II + -2][-1 * i2 + %JJ + -3][-1 * i3 + %KK + -3] FLOW (< = =)

; CHECK: Dump Before

; CHECK: + DO i1 = 0, %II + -5, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, %JJ + -5, 1   <DO_LOOP>
; CHECK: |   |   + DO i3 = 0, %KK + -5, 1   <DO_LOOP>
; CHECK: |   |   |   %"sipiter2_$RES[][][]_fetch.18" = (%RES)[-1 * i1 + %II + -3][-1 * i2 + %JJ + -3][-1 * i3 + %KK + -3];
; CHECK: |   |   |   %"sipiter2_$UN[][][]_fetch.30" = (%UN)[-1 * i1 + %II + -3][-1 * i2 + %JJ + -3][-1 * i3 + %KK + -3];
; CHECK: |   |   |   %"sipiter2_$RES[][][]_fetch.37" = (%RES)[-1 * i1 + %II + -3][-1 * i2 + %JJ + -2][-1 * i3 + %KK + -3];
; CHECK: |   |   |   %"sipiter2_$UT[][][]_fetch.49" = (%UT)[-1 * i1 + %II + -3][-1 * i2 + %JJ + -3][-1 * i3 + %KK + -3];
; CHECK: |   |   |   %"sipiter2_$RES[][][]_fetch.56" = (%RES)[-1 * i1 + %II + -3][-1 * i2 + %JJ + -3][-1 * i3 + %KK + -2];
; CHECK: |   |   |   %"sipiter2_$UE[][][]_fetch.68" = (%UE)[-1 * i1 + %II + -3][-1 * i2 + %JJ + -3][-1 * i3 + %KK + -3];
; CHECK: |   |   |   %"sipiter2_$RES[][][]_fetch.75" = (%RES)[-1 * i1 + %II + -2][-1 * i2 + %JJ + -3][-1 * i3 + %KK + -3];
; CHECK: |   |   |   %mul.9.neg = %"sipiter2_$RES[][][]_fetch.37"  *  %"sipiter2_$UN[][][]_fetch.30";
; CHECK: |   |   |   %mul.14.neg = %"sipiter2_$RES[][][]_fetch.56"  *  %"sipiter2_$UT[][][]_fetch.49";
; CHECK: |   |   |   %mul.19.neg = %"sipiter2_$RES[][][]_fetch.75"  *  %"sipiter2_$UE[][][]_fetch.68";
; CHECK: |   |   |   %reass.add = %mul.14.neg  +  %mul.9.neg;
; CHECK: |   |   |   %reass.add106 = %reass.add  +  %mul.19.neg;
; CHECK: |   |   |   %sub.6 = %"sipiter2_$RES[][][]_fetch.18"  -  %reass.add106;
; CHECK: |   |   |   (%RES)[-1 * i1 + %II + -3][-1 * i2 + %JJ + -3][-1 * i3 + %KK + -3] = %sub.6;
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: + DO i1 = 0, %tgu{{.*}} + -1, 1   <DO_LOOP> <nounroll and jam>

; CHECK: + DO i2 = 0, %tgu{{.*}} + -1, 1   <DO_LOOP> <nounroll and jam>


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @sipiter2_(ptr noalias nocapture readonly dereferenceable(4) %KK.in, ptr noalias nocapture readonly dereferenceable(4) %JJ.in, ptr noalias nocapture readonly dereferenceable(4) %II.in, ptr noalias nocapture dereferenceable(4) %RES, ptr noalias nocapture readonly dereferenceable(4) %UE, ptr noalias nocapture readonly dereferenceable(4) %UN, ptr noalias nocapture readonly dereferenceable(4) %UT) {
alloca_0:
  %KK = load i64, ptr %KK.in, align 1
  %JJ = load i64, ptr %JJ.in, align 1
  %mul.1 = shl nsw i64 %KK, 2
  %mul.2 = mul nsw i64 %mul.1, %JJ
  %II = load i64, ptr %II.in, align 1
  %rel.1 = icmp slt i64 %II, 5
  br i1 %rel.1, label %do.end_do6, label %do.body5.preheader

do.body5.preheader:                               ; preds = %alloca_0
  %sub.2 = add i64 %JJ, -2
  %rel.2 = icmp slt i64 %JJ, 5
  %sub.3 = add i64 %KK, -2
  %rel.3 = icmp slt i64 %KK, 5
  %t3 = add nsw i64 %II, -2
  br label %do.body5

do.body5:                                         ; preds = %do.body5.preheader, %do.end_do10
  %indvars.iv111 = phi i64 [ %t3, %do.body5.preheader ], [ %indvars.iv.next112, %do.end_do10 ]
  br i1 %rel.2, label %do.end_do10, label %do.body9.preheader

do.body9.preheader:                               ; preds = %do.body5
  %"sipiter2_$RES[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul.2, ptr nonnull elementtype(float) %RES, i64 %indvars.iv111)
  %"sipiter2_$UN[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul.2, ptr nonnull elementtype(float) %UN, i64 %indvars.iv111)
  %"sipiter2_$UT[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul.2, ptr nonnull elementtype(float) %UT, i64 %indvars.iv111)
  %"sipiter2_$UE[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul.2, ptr nonnull elementtype(float) %UE, i64 %indvars.iv111)
  %t4 = add nuw nsw i64 %indvars.iv111, 1
  %"sipiter2_$RES[]39" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul.2, ptr nonnull elementtype(float) %RES, i64 %t4)
  br label %do.body9

do.body9:                                         ; preds = %do.body9.preheader, %do.end_do14
  %indvars.iv108 = phi i64 [ %sub.2, %do.body9.preheader ], [ %indvars.iv.next109, %do.end_do14 ]
  br i1 %rel.3, label %do.end_do14, label %do.body13.preheader

do.body13.preheader:                              ; preds = %do.body9
  %"sipiter2_$RES[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr nonnull elementtype(float) %"sipiter2_$RES[]", i64 %indvars.iv108)
  %"sipiter2_$UN[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr nonnull elementtype(float) %"sipiter2_$UN[]", i64 %indvars.iv108)
  %t5 = add nuw nsw i64 %indvars.iv108, 1
  %"sipiter2_$RES[][]22" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr nonnull elementtype(float) %"sipiter2_$RES[]", i64 %t5)
  %"sipiter2_$UT[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr nonnull elementtype(float) %"sipiter2_$UT[]", i64 %indvars.iv108)
  %"sipiter2_$UE[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr nonnull elementtype(float) %"sipiter2_$UE[]", i64 %indvars.iv108)
  %"sipiter2_$RES[][]40" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr nonnull elementtype(float) %"sipiter2_$RES[]39", i64 %indvars.iv108)
  br label %do.body13

do.body13:                                        ; preds = %do.body13.preheader, %do.body13
  %indvars.iv = phi i64 [ %sub.3, %do.body13.preheader ], [ %indvars.iv.next, %do.body13 ]
  %"sipiter2_$RES[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sipiter2_$RES[][]", i64 %indvars.iv)
  %"sipiter2_$RES[][][]_fetch.18" = load float, ptr %"sipiter2_$RES[][][]", align 1
  %"sipiter2_$UN[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sipiter2_$UN[][]", i64 %indvars.iv)
  %"sipiter2_$UN[][][]_fetch.30" = load float, ptr %"sipiter2_$UN[][][]", align 1
  %"sipiter2_$RES[][][]23" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sipiter2_$RES[][]22", i64 %indvars.iv)
  %"sipiter2_$RES[][][]_fetch.37" = load float, ptr %"sipiter2_$RES[][][]23", align 1
  %"sipiter2_$UT[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sipiter2_$UT[][]", i64 %indvars.iv)
  %"sipiter2_$UT[][][]_fetch.49" = load float, ptr %"sipiter2_$UT[][][]", align 1
  %t6 = add nuw nsw i64 %indvars.iv, 1
  %"sipiter2_$RES[][][]32" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sipiter2_$RES[][]", i64 %t6)
  %"sipiter2_$RES[][][]_fetch.56" = load float, ptr %"sipiter2_$RES[][][]32", align 1
  %"sipiter2_$UE[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sipiter2_$UE[][]", i64 %indvars.iv)
  %"sipiter2_$UE[][][]_fetch.68" = load float, ptr %"sipiter2_$UE[][][]", align 1
  %"sipiter2_$RES[][][]41" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sipiter2_$RES[][]40", i64 %indvars.iv)
  %"sipiter2_$RES[][][]_fetch.75" = load float, ptr %"sipiter2_$RES[][][]41", align 1
  %mul.9.neg = fmul reassoc ninf nsz arcp contract afn float %"sipiter2_$RES[][][]_fetch.37", %"sipiter2_$UN[][][]_fetch.30"
  %mul.14.neg = fmul reassoc ninf nsz arcp contract afn float %"sipiter2_$RES[][][]_fetch.56", %"sipiter2_$UT[][][]_fetch.49"
  %mul.19.neg = fmul reassoc ninf nsz arcp contract afn float %"sipiter2_$RES[][][]_fetch.75", %"sipiter2_$UE[][][]_fetch.68"
  %reass.add = fadd reassoc ninf nsz arcp contract afn float %mul.14.neg, %mul.9.neg
  %reass.add106 = fadd reassoc ninf nsz arcp contract afn float %reass.add, %mul.19.neg
  %sub.6 = fsub reassoc ninf nsz arcp contract afn float %"sipiter2_$RES[][][]_fetch.18", %reass.add106
  store float %sub.6, ptr %"sipiter2_$RES[][][]", align 1
  %rel.4 = icmp ugt i64 %indvars.iv, 3
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  br i1 %rel.4, label %do.body13, label %do.end_do14.loopexit

do.end_do14.loopexit:                             ; preds = %do.body13
  br label %do.end_do14

do.end_do14:                                      ; preds = %do.end_do14.loopexit, %do.body9
  %indvars.iv.next109 = add nsw i64 %indvars.iv108, -1
  %rel.5 = icmp sgt i64 %indvars.iv108, 3
  br i1 %rel.5, label %do.body9, label %do.end_do10.loopexit

do.end_do10.loopexit:                             ; preds = %do.end_do14
  br label %do.end_do10

do.end_do10:                                      ; preds = %do.end_do10.loopexit, %do.body5
  %indvars.iv.next112 = add nsw i64 %indvars.iv111, -1
  %rel.6 = icmp sgt i64 %indvars.iv111, 3
  br i1 %rel.6, label %do.body5, label %do.end_do6.loopexit

do.end_do6.loopexit:                              ; preds = %do.end_do10
  br label %do.end_do6

do.end_do6:                                       ; preds = %do.end_do6.loopexit, %alloca_0
  ret void
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #1 = { mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }

