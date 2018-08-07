; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-unroll-and-jam -print-after=hir-unroll-and-jam 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; HIR-
; + DO i1 = 0, 49, 1   <DO_LOOP>
; |   %b.037 = %0;
; |
; |   + DO i2 = 0, 49, 1   <DO_LOOP>
; |   |   %indvars.iv = i2 + %0  +  1;
; |   |   %smax = (i2 + %0 > 10) ? i2 + %0 : 10;
; |   |   if (i2 + %0 < 11)
; |   |   {
; |   |      %12 = (@B)[0][i2];
; |   |      %arrayidx10.promoted = (@A)[0][i1][(%12 * %b.037)];
; |   |      (@A)[0][i1][(%12 * %b.037)] = i2 + %0 + %arrayidx10.promoted + trunc.i33.i32(((zext.i32.i33(((-1 * %b.037) + %smax)) * zext.i32.i33((-1 + (-1 * %b.037) + %smax))) /u 2)) + (((-1 * %b.037) + %smax) * %indvars.iv);
; |   |   }
; |   |   %b.037 = i2 + %0 + 1;
; |   + END LOOP
; + END LOOP

; NOTE: This test case is unrolled using unroll pragma as we may stop unrolling it automatically in the future. This is because conditional temporal locality ((@B)[0][i2] under if conditon) does not provide any benefit with unroll & jam.

; Verify that all occurences of %b.037 are replaced by %temp in the first unrolled iteration inside th i2 loop. This was not happening for the linear definition of %b.037 in the last instruction of i2 loop.

; CHECK: + DO i1 = 0, 24, 1   <DO_LOOP>
; CHECK: |   %temp = %0;
; CHECK: |   %b.037 = %0;
; CHECK: |
; CHECK: |   + DO i2 = 0, 49, 1   <DO_LOOP>
; CHECK: |   |   %indvars.iv = i2 + %0  +  1;
; CHECK: |   |   %smax = (i2 + %0 > 10) ? i2 + %0 : 10;
; CHECK: |   |   if (i2 + %0 < 11)
; CHECK: |   |   {
; CHECK: |   |      %12 = (@B)[0][i2];
; CHECK: |   |      %arrayidx10.promoted = (@A)[0][2 * i1][(%12 * %temp)];
; CHECK: |   |      (@A)[0][2 * i1][(%12 * %temp)] = i2 + %0 + %arrayidx10.promoted + trunc.i33.i32(((zext.i32.i33(((-1 * %temp) + %smax)) * zext.i32.i33((-1 + (-1 * %temp) + %smax))) /u 2)) + (((-1 * %temp) + %smax) * %indvars.iv);
; CHECK: |   |   }
; CHECK: |   |   %temp = i2 + %0 + 1;


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = external dso_local local_unnamed_addr global [50 x i32], align 16
@A = external dso_local local_unnamed_addr global [50 x [50 x i32]], align 16

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr {
entry:
  %0 = load i32, i32* getelementptr inbounds ([50 x i32], [50 x i32]* @B, i64 0, i64 2), align 8
  %1 = sub i32 0, %0
  %2 = xor i32 %0, -1
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc15, %entry
  %indvars.iv45 = phi i64 [ 0, %entry ], [ %indvars.iv.next46, %for.inc15 ]
  br label %for.body3

for.body3:                                        ; preds = %for.inc11, %for.cond1.preheader
  %indvars.iv43 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next44, %for.inc11 ]
  %indvars.iv41 = phi i32 [ %2, %for.cond1.preheader ], [ %indvars.iv.next42, %for.inc11 ]
  %indvars.iv39 = phi i32 [ %1, %for.cond1.preheader ], [ %indvars.iv.next40, %for.inc11 ]
  %indvars.iv.in = phi i32 [ %0, %for.cond1.preheader ], [ %indvars.iv, %for.inc11 ]
  %b.037 = phi i32 [ %0, %for.cond1.preheader ], [ %inc13, %for.inc11 ]
  %indvars.iv = add i32 %indvars.iv.in, 1
  %3 = icmp sgt i32 %b.037, 10
  %smax = select i1 %3, i32 %b.037, i32 10
  %4 = add i32 %smax, %indvars.iv39
  %cmp534 = icmp slt i32 %b.037, 11
  br i1 %cmp534, label %for.body6.lr.ph, label %for.inc11

for.body6.lr.ph:                                  ; preds = %for.body3
  %5 = zext i32 %4 to i33
  %6 = add i32 %smax, %indvars.iv41
  %7 = zext i32 %6 to i33
  %8 = mul i33 %5, %7
  %9 = lshr i33 %8, 1
  %10 = trunc i33 %9 to i32
  %11 = mul i32 %indvars.iv, %4
  %arrayidx = getelementptr inbounds [50 x i32], [50 x i32]* @B, i64 0, i64 %indvars.iv43
  %12 = load i32, i32* %arrayidx, align 4
  %mul = mul nsw i32 %12, %b.037
  %idxprom9 = sext i32 %mul to i64
  %arrayidx10 = getelementptr inbounds [50 x [50 x i32]], [50 x [50 x i32]]* @A, i64 0, i64 %indvars.iv45, i64 %idxprom9
  %arrayidx10.promoted = load i32, i32* %arrayidx10, align 4
  %13 = add i32 %arrayidx10.promoted, %b.037
  %14 = add i32 %13, %11
  %15 = add i32 %14, %10
  store i32 %15, i32* %arrayidx10, align 4
  br label %for.inc11

for.inc11:                                        ; preds = %for.body6.lr.ph, %for.body3
  %indvars.iv.next44 = add nuw nsw i64 %indvars.iv43, 1
  %inc13 = add nsw i32 %b.037, 1
  %indvars.iv.next40 = add i32 %indvars.iv39, -1
  %indvars.iv.next42 = add i32 %indvars.iv41, -1
  %exitcond = icmp eq i64 %indvars.iv.next44, 50
  br i1 %exitcond, label %for.inc15, label %for.body3

for.inc15:                                        ; preds = %for.inc11
  %indvars.iv.next46 = add nuw nsw i64 %indvars.iv45, 1
  %exitcond47 = icmp eq i64 %indvars.iv.next46, 50
  br i1 %exitcond47, label %for.end17, label %for.cond1.preheader, !llvm.loop !0

for.end17:                                        ; preds = %for.inc15
  ret i32 0
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll_and_jam.count", i32 2}

