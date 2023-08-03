; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-details -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; We form an SCC by combining values (%conv9.lcssa24 -> %conv9.lcssa -> %conv9).
; %conv9 which is the operand of single operand phi %conv9.lcssa was not marked
;   liveout of i2 loop.
; The logic was extended to handle this case

; Formed HIR-
; + DO i1 = 0, (-1 * %.pr + -2)/u2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483648>
; |   %conv9.lcssa24.out = %conv9.lcssa24;
; |   %.pre29 = %.pre;
; |   %c.promoted = %.pre;
; |   %4 = %.pre;
; |   %conv921 = %conv9.lcssa24.out;
; |
; |   + DO i2 = 0, 4, 1   <DO_LOOP>
; |   |   %sub = %conv9.lcssa24.out  -  %4;
; |   |   %.pre28 = %.pre29;
; |   |   %c.promoted26 = %c.promoted;
; |   |   %9 = 0;
; |   |   if (%4 != 0)
; |   |   {
; |   |      (@b)[0] = %conv921;
; |   |      (@c)[0] = %c.promoted + 2 * ((-2 + (-1 * %c.promoted)) /u 2) + 2;
; |   |      %.pre28 = %c.promoted + 2 * ((-2 + (-1 * %c.promoted)) /u 2) + 2;
; |   |      %c.promoted26 = %c.promoted + 2 * ((-2 + (-1 * %c.promoted)) /u 2) + 2;
; |   |      %9 = %c.promoted + 2 * ((-2 + (-1 * %c.promoted)) /u 2) + 2;
; |   |   }
; |   |   %conv9.lcssa24 = %0  &&  %conv9.lcssa24.out + -1 * %4;
; |   |   %conv9.lcssa24.out2 = %conv9.lcssa24;
; |   |   %.pre29 = %.pre28;
; |   |   %c.promoted = %c.promoted26;
; |   |   %4 = %9;
; |   |   %conv921 = %conv9.lcssa24.out2;
; |   + END LOOP
; |
; |   %conv9.lcssa24.out1 = %conv9.lcssa24;
; |   %.pre = %.pre28;
; + END LOOP

; CHECK: <RVAL-REG> NON-LINEAR i32 %conv9.lcssa24 {sb:[[LIVEOUTSYMBASE:.*]]}
; CHECK: LiveOut symbases:
; CHECK-SAME: [[LIVEOUTSYMBASE]]
; CHECK: DO i32 i2

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@e = common dso_local local_unnamed_addr global i32 0, align 4
@f = common dso_local local_unnamed_addr global i32 0, align 4
@d = common dso_local local_unnamed_addr global i32 0, align 4
@c = common dso_local local_unnamed_addr global i32 0, align 4
@g = common dso_local local_unnamed_addr global i64 0, align 8
@b = common dso_local local_unnamed_addr global i32 0, align 4
@a = common dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: norecurse nounwind uwtable
define dso_local void @h() local_unnamed_addr #0 {
entry:
  %.pr = load i32, ptr @e, align 4
  %tobool22 = icmp eq i32 %.pr, 0
  br i1 %tobool22, label %for.end15, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %0 = load i32, ptr @a, align 4
  %f.promoted23 = load i32, ptr @f, align 4
  %1 = sub i32 -2, %.pr
  %2 = and i32 %1, -2
  %3 = add i32 %.pr, %2
  %.pre.pre = load i32, ptr @c, align 4
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.end12
  %.pre = phi i32 [ %.pre.pre, %for.body.lr.ph ], [ %.pre28.lcssa, %for.end12 ]
  %add1425 = phi i32 [ %.pr, %for.body.lr.ph ], [ %add14, %for.end12 ]
  %conv9.lcssa24 = phi i32 [ %f.promoted23, %for.body.lr.ph ], [ %conv9.lcssa, %for.end12 ]
  %conv = sext i32 %conv9.lcssa24 to i64
  br label %for.body3

for.body3:                                        ; preds = %for.body, %for.end
  %.pre29 = phi i32 [ %.pre, %for.body ], [ %.pre28, %for.end ]
  %c.promoted = phi i32 [ %.pre, %for.body ], [ %c.promoted26, %for.end ]
  %4 = phi i32 [ %.pre, %for.body ], [ %9, %for.end ]
  %conv921 = phi i32 [ %conv9.lcssa24, %for.body ], [ %conv9, %for.end ]
  %storemerge20 = phi i32 [ 0, %for.body ], [ %add11, %for.end ]
  %conv4 = sext i32 %4 to i64
  %sub = sub nsw i64 %conv, %conv4
  %tobool618 = icmp eq i32 %4, 0
  br i1 %tobool618, label %for.end, label %for.body7.lr.ph

for.body7.lr.ph:                                  ; preds = %for.body3
  store i32 %conv921, ptr @b, align 4
  %5 = sub i32 -2, %c.promoted
  %6 = and i32 %5, -2
  %7 = add i32 %c.promoted, 2
  %8 = add i32 %7, %6
  store i32 %8, ptr @c, align 4
  br label %for.end

for.end:                                          ; preds = %for.body7.lr.ph, %for.body3
  %.pre28 = phi i32 [ %8, %for.body7.lr.ph ], [ %.pre29, %for.body3 ]
  %c.promoted26 = phi i32 [ %8, %for.body7.lr.ph ], [ %c.promoted, %for.body3 ]
  %9 = phi i32 [ %8, %for.body7.lr.ph ], [ 0, %for.body3 ]
  %10 = trunc i64 %sub to i32
  %conv9 = and i32 %0, %10
  %add11 = add nuw nsw i32 %storemerge20, 3
  %cmp = icmp ult i32 %add11, 13
  br i1 %cmp, label %for.body3, label %for.end12

for.end12:                                        ; preds = %for.end
  %.pre28.lcssa = phi i32 [ %.pre28, %for.end ]
  %conv9.lcssa = phi i32 [ %conv9, %for.end ]
  %sub.lcssa = phi i64 [ %sub, %for.end ]
  %add14 = add nsw i32 %add1425, 2
  %tobool = icmp eq i32 %add14, 0
  br i1 %tobool, label %for.cond.for.end15_crit_edge, label %for.body

for.cond.for.end15_crit_edge:                     ; preds = %for.end12
  %conv9.lcssa.lcssa = phi i32 [ %conv9.lcssa, %for.end12 ]
  %sub.lcssa.lcssa = phi i64 [ %sub.lcssa, %for.end12 ]
  %11 = add i32 %3, 2
  store i32 %conv9.lcssa.lcssa, ptr @f, align 4
  store i64 %sub.lcssa.lcssa, ptr @g, align 8
  store i32 15, ptr @d, align 4
  store i32 %11, ptr @e, align 4
  br label %for.end15

for.end15:                                        ; preds = %for.cond.for.end15_crit_edge, %entry
  ret void
}
