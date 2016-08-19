; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the we are able to handle IV multiplications resulting in polynomial SCEVs by looking ahead.

; CHECK: + DO i1 = 0, 47, 1   <DO_LOOP>
; CHECK: |   %n.addr.038.out = %n.addr.038;
; CHECK: |   %qb.041.out1 = %qb.041;
; CHECK: |   %indvars.iv42.out = %indvars.iv42;
; CHECK: |   %indvars.iv44.out = %indvars.iv44;
; CHECK: |
; CHECK: |      %2 = i1 + -1  *  i1 + -2;
; CHECK: |      %qb.135 = %qb.041.out1;
; CHECK: |   + DO i2 = 0, (i1 + -1)/u3, 1   <DO_LOOP>
; CHECK: |   |   %qb.135.out = %qb.135;
; CHECK: |   |   %9 = i1 + -1  *  (-1 + (-1 * %indvars.iv54)) * i2 + %n.addr.038.out + -1;
; CHECK: |   |   %qb.135 = (-1 + (-1 * %indvars.iv54)) * i2 + -1 * trunc.i33.i32((%2 /u 2)) + %indvars.iv44.out + %n.addr.038.out + %qb.135.out  +  %9;
; CHECK: |   |   %11 = -1 * i1  +  %indvars.iv52 * i2 + %n.addr.038.out;
; CHECK: |   + END LOOP
; CHECK: |      %qb.041 = %qb.135;
; CHECK: |      %n.addr.038 = %11;
; CHECK: |
; CHECK: |   %qb.041.out = %qb.041;
; CHECK: |   %indvars.iv42 = %indvars.iv42  +  -6 * i1 + -12;
; CHECK: |   %indvars.iv44 = %indvars.iv44  +  %indvars.iv42.out;
; CHECK: |   %indvars.iv52 = -1 * i1 + -1;
; CHECK: |   %indvars.iv54 = i1;
; CHECK: + END LOOP


;Module Before HIR; ModuleID = 'non-affine.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readnone uwtable
define i32 @foo(i32 %n) #0 {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.12, %entry
  %indvars.iv54 = phi i32 [ -1, %entry ], [ %indvars.iv.next55, %for.inc.12 ]
  %indvars.iv52 = phi i32 [ 0, %entry ], [ %indvars.iv.next53, %for.inc.12 ]
  %indvars.iv48 = phi i32 [ -2, %entry ], [ %indvars.iv.next49, %for.inc.12 ]
  %indvars.iv44 = phi i32 [ -1, %entry ], [ %indvars.iv.next45, %for.inc.12 ]
  %indvars.iv42 = phi i32 [ -7, %entry ], [ %indvars.iv.next43, %for.inc.12 ]
  %indvars.iv = phi i32 [ -12, %entry ], [ %indvars.iv.next, %for.inc.12 ]
  %qb.041 = phi i32 [ undef, %entry ], [ %qb.1.lcssa, %for.inc.12 ]
  %i.039 = phi i32 [ 1, %entry ], [ %inc, %for.inc.12 ]
  %n.addr.038 = phi i32 [ %n, %entry ], [ %n.addr.1.lcssa, %for.inc.12 ]
  %cmp2.32 = icmp sgt i32 %i.039, 1
  br i1 %cmp2.32, label %for.body.3.lr.ph, label %for.inc.12

for.body.3.lr.ph:                                 ; preds = %for.cond.1.preheader
  %0 = zext i32 %indvars.iv54 to i33
  %1 = zext i32 %indvars.iv48 to i33
  %2 = mul i33 %0, %1
  %3 = lshr i33 %2, 1
  %4 = trunc i33 %3 to i32
  %5 = sub i32 %indvars.iv44, %4
  br label %for.inc.9

for.inc.9:                                        ; preds = %for.body.3.lr.ph, %for.inc.9
  %qb.135 = phi i32 [ %qb.041, %for.body.3.lr.ph ], [ %10, %for.inc.9 ]
  %k.034 = phi i32 [ 1, %for.body.3.lr.ph ], [ %add10, %for.inc.9 ]
  %n.addr.133 = phi i32 [ %n.addr.038, %for.body.3.lr.ph ], [ %11, %for.inc.9 ]
  %6 = add i32 %5, %qb.135
  %7 = add i32 %6, %n.addr.133
  %8 = add i32 %n.addr.133, -1
  %9 = mul i32 %indvars.iv54, %8
  %10 = add i32 %7, %9
  %11 = add i32 %indvars.iv52, %n.addr.133
  %add10 = add nuw nsw i32 %k.034, 3
  %cmp2 = icmp slt i32 %add10, %i.039
  br i1 %cmp2, label %for.inc.9, label %for.inc.12.loopexit

for.inc.12.loopexit:                              ; preds = %for.inc.9
  %.lcssa57 = phi i32 [ %11, %for.inc.9 ]
  %.lcssa = phi i32 [ %10, %for.inc.9 ]
  br label %for.inc.12

for.inc.12:                                       ; preds = %for.inc.12.loopexit, %for.cond.1.preheader
  %qb.1.lcssa = phi i32 [ %qb.041, %for.cond.1.preheader ], [ %.lcssa, %for.inc.12.loopexit ]
  %n.addr.1.lcssa = phi i32 [ %n.addr.038, %for.cond.1.preheader ], [ %.lcssa57, %for.inc.12.loopexit ]
  %inc = add nuw nsw i32 %i.039, 1
  %indvars.iv.next = add nsw i32 %indvars.iv, -6
  %indvars.iv.next43 = add i32 %indvars.iv42, %indvars.iv
  %indvars.iv.next45 = add i32 %indvars.iv44, %indvars.iv42
  %indvars.iv.next55 = add nsw i32 %indvars.iv54, 1
  %indvars.iv.next49 = add nsw i32 %indvars.iv48, 1
  %indvars.iv.next53 = add nsw i32 %indvars.iv52, -1
  %exitcond = icmp eq i32 %indvars.iv.next55, 47
  br i1 %exitcond, label %for.end.13, label %for.cond.1.preheader

for.end.13:                                       ; preds = %for.inc.12
  %qb.1.lcssa.lcssa = phi i32 [ %qb.1.lcssa, %for.inc.12 ]
  ret i32 %qb.1.lcssa.lcssa
}

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind readnone uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind argmemonly }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1686) (llvm/branches/loopopt 1974)"}
