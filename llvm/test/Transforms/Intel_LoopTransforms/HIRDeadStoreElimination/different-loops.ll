; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Test checkes that the first store to @arr[] is not eliminated by HIR DSE pass
; because the load in the second loop serves as an intervening load.  

;      BEGIN REGION { }
;         + DO i1 = 0, 95, 1   <DO_LOOP>
;         |   %y.037.out = %y.037;
;         |
;         |   + DO i2 = 0, 35, 1   <DO_LOOP>
;         |   |   (@arr)[0][-1 * i2 + 38] = -1 * i2 + %y.037.out + -1;
;         |   + END LOOP
;         |
;         |   %y.037 = %y.037  +  -36;
;         |
;         |   + DO i2 = 0, 35, 1   <DO_LOOP>
;         |   |   %2 = (@arr)[0][-1 * i2 + 36];
;         |   |   %y.037 = %2  +  %y.037;
;         |   |   (@arr)[0][-1 * i2 + 38] = %y.037 * i1 + %y.037;
;         |   + END LOOP
;         + END LOOP
;      END REGION

; CHECK-NOT: modified
; CHECK: (@arr)[0][-1 * i2 + 38]
; CHECK: (@arr)[0][-1 * i2 + 38]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @main() local_unnamed_addr {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc17
  %y.037 = phi i32 [ 95, %entry ], [ %add10.lcssa, %for.inc17 ]
  %x.036 = phi i32 [ 1, %entry ], [ %inc, %for.inc17 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 37, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %y.133 = phi i32 [ %y.037, %for.cond1.preheader ], [ %dec, %for.body3 ]
  %dec = add i32 %y.133, -1
  %0 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @arr, i64 0, i64 %0
  store i32 %dec, i32* %arrayidx, align 4
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %cmp2 = icmp ugt i64 %indvars.iv.next, 1
  br i1 %cmp2, label %for.body3, label %for.body7.preheader

for.body7.preheader:                              ; preds = %for.body3
  %1 = add i32 %y.037, -36
  br label %for.body7

for.body7:                                        ; preds = %for.body7.preheader, %for.body7
  %indvars.iv39 = phi i64 [ 37, %for.body7.preheader ], [ %indvars.iv.next40, %for.body7 ]
  %y.234 = phi i32 [ %1, %for.body7.preheader ], [ %add10, %for.body7 ]
  %indvars.iv.next40 = add nsw i64 %indvars.iv39, -1
  %arrayidx9 = getelementptr inbounds [100 x i32], [100 x i32]* @arr, i64 0, i64 %indvars.iv.next40
  %2 = load i32, i32* %arrayidx9, align 4
  %add10 = add i32 %2, %y.234
  %mul = mul i32 %add10, %x.036
  %3 = add nuw nsw i64 %indvars.iv39, 1
  %arrayidx13 = getelementptr inbounds [100 x i32], [100 x i32]* @arr, i64 0, i64 %3
  store i32 %mul, i32* %arrayidx13, align 4
  %cmp6 = icmp ugt i64 %indvars.iv.next40, 1
  br i1 %cmp6, label %for.body7, label %for.inc17

for.inc17:                                        ; preds = %for.body7
  %add10.lcssa = phi i32 [ %add10, %for.body7 ]
  %inc = add nuw nsw i32 %x.036, 1
  %exitcond.not = icmp eq i32 %inc, 97
  br i1 %exitcond.not, label %for.end18, label %for.cond1.preheader

for.end18:                                        ; preds = %for.inc17
  ret i32 0
}
