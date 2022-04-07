; RUN: opt  -hir-ssa-deconstruction -enable-new-pm=0 -analyze -hir-framework  <%s  2>&1 | FileCheck %s
; RUN: opt  -passes="hir-ssa-deconstruction,print<hir>"  <%s 2>&1 | FileCheck %s

; Test checks that '%x | %y == 0' is not converted to 'umax (%x, %y) == 0'
; in unknown loop bottomtest when one of the operands is AddRec.

; CHECK:           + UNKNOWN LOOP i1
; CHECK:           |   <i1 = 0>
; CHECK:           |   bb8:
; CHECK:           |   %I.0.out = -4 * i1 + 2;
; CHECK:           |   %rel.16.out = -1 * i1;
; CHECK:           |   %rel.13 = -48 * i1 + 24 == 0;
; CHECK:           |   %brmerge = %rel.13  |  -1 * i1;
; CHECK:           |   if (%brmerge == 0)
; CHECK:           |   {
; CHECK:           |      <i1 = i1 + 1>
; CHECK:           |      goto bb8;
; CHECK:           |   }
; CHECK:           + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @MAIN__() local_unnamed_addr {
bb8.preheader:
  br label %bb8

bb8:                                              ; preds = %bb8.preheader, %bb8
  %rel.16 = phi i1 [ true, %bb8 ], [ false, %bb8.preheader ]
  %"I.0" = phi i32 [ -2, %bb8 ], [ 2, %bb8.preheader ]
  %mul.10 = mul nsw i32 %"I.0", 12
  %rel.13 = icmp eq i32 %mul.10, 0
  %brmerge = or i1 %rel.13, %rel.16
  br i1 %brmerge, label %bb8.loopexit, label %bb8

bb8.loopexit:                                     ; preds = %bb8
  %rel.16.lcssa = phi i1 [ %rel.16, %bb8 ]
  %"I.0.lcssa" = phi i32 [ %"I.0", %bb8 ]
  br label %bb5

bb5:                                              ; preds = %bb8.loopexit, %bb8_then
  ret void
}


