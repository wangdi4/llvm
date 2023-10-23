; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,print<hir>" -disable-output -hir-cost-model-throttling=0 %s 2>&1 | FileCheck %s

; Check that we distribute the loop without recomputation in presence of unsafe usercalls.
; Previously, distribution was  incorrectly trying to recompute rel.167 by reloading (%"fm413_$IRECN")[0] which was being modified by a call via fake ref, &((%argblock2628)[0]))

; HIR Before:
; + DO i1 = 0, 98, 1   <DO_LOOP>
; |   (null)[0] = 0;
; |   (null)[0] = 0;
; |   (null)[0] = 0;
; |   (null)[0] = 0;
; |   (null)[0] = 0;
; |   (null)[0] = null;
; |   %func_result2611 = @for_read_dir(&((%"$io_ctx")[0]),  0);
; |   (%argblock2628)[0].0 = &((%"fm413_$IRECN")[0]);
; |   %func_result2631 = @for_read_dir_xmit(&((%"$io_ctx")[0]),  &((%"(&)val$2627")[0]),  &((%argblock2628)[0]));
; |   (null)[0] = 0;
; |   (null)[0] = 0;
; |   (null)[0] = 0;
; |   (null)[0] = 0;
; |   (null)[0] = null;
; |   (null)[0] = 0;
; |   (null)[0] = 0;
; |   (null)[0] = 0;
; |   (null)[0] = 0;
; |   (null)[0] = null;
; |   (null)[0] = 0;
; |   (null)[0] = 0;
; |   %rel.167 = (%"fm413_$IRECN")[0] == 2 * i1 + 15;
; |   %"fm413_$IVCOMP.133.root" = %"fm413_$IVCOMP.133.root"  +  %rel.167;
; + END LOOP


; CHECK: modified

; CHECK: + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK: |   %min = (-64 * i1 + 98 <= 63) ? -64 * i1 + 98 : 63;
; CHECK: |
; CHECK: |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK: |   |   (null)[0] = 0;
; CHECK: |   |   (null)[0] = 0;
; CHECK: |   |   (null)[0] = 0;
; CHECK: |   |   (null)[0] = 0;
; CHECK: |   |   (null)[0] = 0;
; CHECK: |   |   (null)[0] = null;
; CHECK: |   |   %func_result2611 = @for_read_dir(&((%"$io_ctx")[0]),  0);
; CHECK: |   |   (%argblock2628)[0].0 = &((%"fm413_$IRECN")[0]);
; CHECK: |   |   %func_result2631 = @for_read_dir_xmit(&((%"$io_ctx")[0]),  &((%"(&)val$2627")[0]),  &((%argblock2628)[0]));
; CHECK: |   |   (null)[0] = 0;
; CHECK: |   |   (null)[0] = 0;
; CHECK: |   |   (null)[0] = 0;
; CHECK: |   |   (null)[0] = 0;
; CHECK: |   |   (null)[0] = null;
; CHECK: |   |   (null)[0] = 0;
; CHECK: |   |   (null)[0] = 0;
; CHECK: |   |   (null)[0] = 0;
; CHECK: |   |   (null)[0] = 0;
; CHECK: |   |   (null)[0] = null;
; CHECK: |   |   (null)[0] = 0;
; CHECK: |   |   (null)[0] = 0;
; CHECK: |   |   %rel.167 = (%"fm413_$IRECN")[0] == 128 * i1 + 2 * i2 + 15;
; CHECK: |   |   (%.TempArray)[0][i2] = %rel.167;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |
; CHECK: |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK: |   |   %rel.167 = (%.TempArray)[0][i2];
; CHECK: |   |   %"fm413_$IVCOMP.133.root" = %"fm413_$IVCOMP.133.root"  +  %rel.167;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @MAIN__(ptr %"fm413_$IRECN", ptr %"(&)val$2627") {
bb12:
  %"$io_ctx" = alloca [8 x i64], i32 0, align 16
  %argblock2628 = alloca <{ ptr }>, i32 0, align 8
  br label %do.body1773

do.body1773:                                      ; preds = %do.body1773, %bb12
  %"fm413_$IVCOMP.133" = phi i32 [ %spec.select5805, %do.body1773 ], [ 0, %bb12 ]
  %"fm413_$I.2" = phi i32 [ %add.138, %do.body1773 ], [ 1, %bb12 ]
  %"fm413_$IREC.2" = phi i32 [ %add.135, %do.body1773 ], [ 13, %bb12 ]
  %add.135 = add i32 %"fm413_$IREC.2", 2
  store i64 0, ptr null, align 16
  store i8 0, ptr null, align 1
  store i8 0, ptr null, align 1
  store i8 0, ptr null, align 1
  store i8 0, ptr null, align 1
  store ptr null, ptr null, align 8
  %func_result2611 = call i32 (ptr, i32, ...) @for_read_dir(ptr %"$io_ctx", i32 0)
  store ptr %"fm413_$IRECN", ptr %argblock2628, align 8
  %func_result2631 = call i32 @for_read_dir_xmit(ptr %"$io_ctx", ptr %"(&)val$2627", ptr %argblock2628)
  store i8 0, ptr null, align 1
  store i8 0, ptr null, align 1
  store i8 0, ptr null, align 1
  store i8 0, ptr null, align 1
  store ptr null, ptr null, align 8
  store i8 0, ptr null, align 1
  store i8 0, ptr null, align 1
  store i8 0, ptr null, align 1
  store i8 0, ptr null, align 1
  store ptr null, ptr null, align 8
  store i8 0, ptr null, align 1
  store i8 0, ptr null, align 1
  %"fm413_$IRECN_fetch.757" = load i32, ptr %"fm413_$IRECN", align 4
  %rel.167 = icmp eq i32 %"fm413_$IRECN_fetch.757", %add.135
  %add.137 = zext i1 %rel.167 to i32
  %spec.select5805 = add nsw i32 %"fm413_$IVCOMP.133", %add.137
  %add.138 = add i32 %"fm413_$I.2", 1
  %exitcond5813.not = icmp eq i32 %add.138, 100
  br i1 %exitcond5813.not, label %do.epilog1776, label %do.body1773

do.epilog1776:                                    ; preds = %do.body1773
  %spec.select5805.lcssa = phi i32 [ %spec.select5805, %do.body1773 ]
  ret void
}

declare i32 @for_read_dir(ptr, i32, ...)

declare i32 @for_read_dir_xmit(ptr, ptr, ptr)
