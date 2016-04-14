; Test for complete unrolling with nine loop levels.

; RUN: opt -hir-ssa-deconstruction -hir-complete-unroll -print-after=hir-complete-unroll 2>&1 < %s | FileCheck %s

; Checks the unrolling of loops from 6th level to 9th level.
; CHECK: BEGIN REGION { modified }
; CHECK: DO i1 = 0, 1, 1
; CHECK: DO i5 = 0, 1, 1
; CHECK-NOT: DO i6 = 0, 1, 1
; CHECK: %0 = {al:8}(%B64)[0][i1][i2][i3][i4][i5][0][0][0][0];
; CHECK: {al:8}(%A64)[0][i1][i2][i3][i4][i5][0][0][0][0] = %0;

; Source Code
;int64_t foo(int M)
;{
;    int64_t A64[5][5][5][5][5][5][5][5][5], B64[5][5][5][5][5][5][5][5][5];
;    int64_t i1, i2, i3, i4, i5, i6, i7, i8, i9;
;    for(i1=0; i1<2; i1++)
;      for(i2=0; i2<2; i2++)
;        for(i3=0;i3<2; i3++)
;        for(i4=0;i4<2; i4++)
;        for(i5=0;i5<2; i5++)
;        for(i6=0;i6<2; i6++)
;        for(i7=0;i7<2; i7++)
;        for(i8=0;i8<2; i8++)
;        for(i9=0;i9<2; i9++)
;                A64[i1][i2][i3][i4][i5][i6][i7][i8][i9] = B64[i1][i2][i3][i4][i5][i6][i7][i8][i9];
;  return A64[M][M][M][M][M][M][M][M][M];
;}



target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i64 @_Z3fooi(i32 %M) #0 {
entry:
  %A64 = alloca [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]]]], align 16
  %B64 = alloca [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]]]], align 16
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc63
  %i1.09 = phi i64 [ 0, %entry ], [ %inc64, %for.inc63 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body, %for.inc60
  %i2.08 = phi i64 [ 0, %for.body ], [ %inc61, %for.inc60 ]
  br label %for.body6

for.body6:                                        ; preds = %for.body3, %for.inc57
  %i3.07 = phi i64 [ 0, %for.body3 ], [ %inc58, %for.inc57 ]
  br label %for.body9

for.body9:                                        ; preds = %for.body6, %for.inc54
  %i4.06 = phi i64 [ 0, %for.body6 ], [ %inc55, %for.inc54 ]
  br label %for.body12

for.body12:                                       ; preds = %for.body9, %for.inc51
  %i5.05 = phi i64 [ 0, %for.body9 ], [ %inc52, %for.inc51 ]
  br label %for.body15

for.body15:                                       ; preds = %for.body12, %for.inc48
  %i6.04 = phi i64 [ 0, %for.body12 ], [ %inc49, %for.inc48 ]
  br label %for.body18

for.body18:                                       ; preds = %for.body15, %for.inc45
  %i7.03 = phi i64 [ 0, %for.body15 ], [ %inc46, %for.inc45 ]
  br label %for.body21

for.body21:                                       ; preds = %for.body18, %for.inc42
  %i8.02 = phi i64 [ 0, %for.body18 ], [ %inc43, %for.inc42 ]
  br label %for.body24

for.body24:                                       ; preds = %for.body21, %for.inc
  %i9.01 = phi i64 [ 0, %for.body21 ], [ %inc, %for.inc ]
  %arrayidx = getelementptr inbounds [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]]]], [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]]]]* %B64, i64 0, i64 %i1.09
  %arrayidx25 = getelementptr inbounds [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]]], [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]]]* %arrayidx, i64 0, i64 %i2.08
  %arrayidx26 = getelementptr inbounds [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]], [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]]* %arrayidx25, i64 0, i64 %i3.07
  %arrayidx27 = getelementptr inbounds [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]], [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]* %arrayidx26, i64 0, i64 %i4.06
  %arrayidx28 = getelementptr inbounds [5 x [5 x [5 x [5 x [5 x i64]]]]], [5 x [5 x [5 x [5 x [5 x i64]]]]]* %arrayidx27, i64 0, i64 %i5.05
  %arrayidx29 = getelementptr inbounds [5 x [5 x [5 x [5 x i64]]]], [5 x [5 x [5 x [5 x i64]]]]* %arrayidx28, i64 0, i64 %i6.04
  %arrayidx30 = getelementptr inbounds [5 x [5 x [5 x i64]]], [5 x [5 x [5 x i64]]]* %arrayidx29, i64 0, i64 %i7.03
  %arrayidx31 = getelementptr inbounds [5 x [5 x i64]], [5 x [5 x i64]]* %arrayidx30, i64 0, i64 %i8.02
  %arrayidx32 = getelementptr inbounds [5 x i64], [5 x i64]* %arrayidx31, i64 0, i64 %i9.01
  %0 = load i64, i64* %arrayidx32, align 8
  %arrayidx33 = getelementptr inbounds [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]]]], [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]]]]* %A64, i64 0, i64 %i1.09
  %arrayidx34 = getelementptr inbounds [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]]], [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]]]* %arrayidx33, i64 0, i64 %i2.08
  %arrayidx35 = getelementptr inbounds [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]], [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]]* %arrayidx34, i64 0, i64 %i3.07
  %arrayidx36 = getelementptr inbounds [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]], [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]* %arrayidx35, i64 0, i64 %i4.06
  %arrayidx37 = getelementptr inbounds [5 x [5 x [5 x [5 x [5 x i64]]]]], [5 x [5 x [5 x [5 x [5 x i64]]]]]* %arrayidx36, i64 0, i64 %i5.05
  %arrayidx38 = getelementptr inbounds [5 x [5 x [5 x [5 x i64]]]], [5 x [5 x [5 x [5 x i64]]]]* %arrayidx37, i64 0, i64 %i6.04
  %arrayidx39 = getelementptr inbounds [5 x [5 x [5 x i64]]], [5 x [5 x [5 x i64]]]* %arrayidx38, i64 0, i64 %i7.03
  %arrayidx40 = getelementptr inbounds [5 x [5 x i64]], [5 x [5 x i64]]* %arrayidx39, i64 0, i64 %i8.02
  %arrayidx41 = getelementptr inbounds [5 x i64], [5 x i64]* %arrayidx40, i64 0, i64 %i9.01
  store i64 %0, i64* %arrayidx41, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body24
  %inc = add nsw i64 %i9.01, 1
  %cmp23 = icmp slt i64 %inc, 2
  br i1 %cmp23, label %for.body24, label %for.end

for.end:                                          ; preds = %for.inc
  br label %for.inc42

for.inc42:                                        ; preds = %for.end
  %inc43 = add nsw i64 %i8.02, 1
  %cmp20 = icmp slt i64 %inc43, 2
  br i1 %cmp20, label %for.body21, label %for.end44

for.end44:                                        ; preds = %for.inc42
  br label %for.inc45

for.inc45:                                        ; preds = %for.end44
  %inc46 = add nsw i64 %i7.03, 1
  %cmp17 = icmp slt i64 %inc46, 2
  br i1 %cmp17, label %for.body18, label %for.end47

for.end47:                                        ; preds = %for.inc45
  br label %for.inc48

for.inc48:                                        ; preds = %for.end47
  %inc49 = add nsw i64 %i6.04, 1
  %cmp14 = icmp slt i64 %inc49, 2
  br i1 %cmp14, label %for.body15, label %for.end50

for.end50:                                        ; preds = %for.inc48
  br label %for.inc51

for.inc51:                                        ; preds = %for.end50
  %inc52 = add nsw i64 %i5.05, 1
  %cmp11 = icmp slt i64 %inc52, 2
  br i1 %cmp11, label %for.body12, label %for.end53

for.end53:                                        ; preds = %for.inc51
  br label %for.inc54

for.inc54:                                        ; preds = %for.end53
  %inc55 = add nsw i64 %i4.06, 1
  %cmp8 = icmp slt i64 %inc55, 2
  br i1 %cmp8, label %for.body9, label %for.end56

for.end56:                                        ; preds = %for.inc54
  br label %for.inc57

for.inc57:                                        ; preds = %for.end56
  %inc58 = add nsw i64 %i3.07, 1
  %cmp5 = icmp slt i64 %inc58, 2
  br i1 %cmp5, label %for.body6, label %for.end59

for.end59:                                        ; preds = %for.inc57
  br label %for.inc60

for.inc60:                                        ; preds = %for.end59
  %inc61 = add nsw i64 %i2.08, 1
  %cmp2 = icmp slt i64 %inc61, 2
  br i1 %cmp2, label %for.body3, label %for.end62

for.end62:                                        ; preds = %for.inc60
  br label %for.inc63

for.inc63:                                        ; preds = %for.end62
  %inc64 = add nsw i64 %i1.09, 1
  %cmp = icmp slt i64 %inc64, 2
  br i1 %cmp, label %for.body, label %for.end65

for.end65:                                        ; preds = %for.inc63
  %idxprom = sext i32 %M to i64
  %idxprom66 = sext i32 %M to i64
  %idxprom67 = sext i32 %M to i64
  %idxprom68 = sext i32 %M to i64
  %idxprom69 = sext i32 %M to i64
  %idxprom70 = sext i32 %M to i64
  %idxprom71 = sext i32 %M to i64
  %idxprom72 = sext i32 %M to i64
  %idxprom73 = sext i32 %M to i64
  %arrayidx74 = getelementptr inbounds [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]]]], [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]]]]* %A64, i64 0, i64 %idxprom73
  %arrayidx75 = getelementptr inbounds [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]]], [5 x [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]]]* %arrayidx74, i64 0, i64 %idxprom72
  %arrayidx76 = getelementptr inbounds [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]], [5 x [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]]* %arrayidx75, i64 0, i64 %idxprom71
  %arrayidx77 = getelementptr inbounds [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]], [5 x [5 x [5 x [5 x [5 x [5 x i64]]]]]]* %arrayidx76, i64 0, i64 %idxprom70
  %arrayidx78 = getelementptr inbounds [5 x [5 x [5 x [5 x [5 x i64]]]]], [5 x [5 x [5 x [5 x [5 x i64]]]]]* %arrayidx77, i64 0, i64 %idxprom69
  %arrayidx79 = getelementptr inbounds [5 x [5 x [5 x [5 x i64]]]], [5 x [5 x [5 x [5 x i64]]]]* %arrayidx78, i64 0, i64 %idxprom68
  %arrayidx80 = getelementptr inbounds [5 x [5 x [5 x i64]]], [5 x [5 x [5 x i64]]]* %arrayidx79, i64 0, i64 %idxprom67
  %arrayidx81 = getelementptr inbounds [5 x [5 x i64]], [5 x [5 x i64]]* %arrayidx80, i64 0, i64 %idxprom66
  %arrayidx82 = getelementptr inbounds [5 x i64], [5 x i64]* %arrayidx81, i64 0, i64 %idxprom
  %1 = load i64, i64* %arrayidx82, align 8
  ret i64 %1
}
