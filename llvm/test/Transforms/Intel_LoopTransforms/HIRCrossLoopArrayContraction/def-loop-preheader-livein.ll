; RUN: opt -intel-libirc-allowed -hir-details -hir-create-function-level-region -passes="hir-ssa-deconstruction,require<hir-loop-statistics>,hir-cross-loop-array-contraction" -print-before=hir-cross-loop-array-contraction -print-after=hir-cross-loop-array-contraction -disable-output < %s 2>&1 | FileCheck %s

; Verify that %val's symbase is added as livein to i2 loop after we perform
; array contraction inside i2 loop. %val is defined in the def loop's (first i2
; loop) preheader.


; CHECK: Dump Before

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   do.outer:
; CHECK: |
; CHECK: |      %div = %val  /  5;
; CHECK: |      <RVAL-REG> LINEAR i32 %val {sb:[[VALSB:.*]]}
; CHECK: |   + DO i32 i2 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |   |   + DO i32 i3 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   |   + DO i32 i4 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   |   |   + DO i32 i5 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   |   |   |   + DO i32 i6 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   |   |   |   |   (%A)[0][i2][i3][i4][i5][i6] = %div;
; CHECK: |   |   |   |   |   + END LOOP
; CHECK: |   |   |   |   + END LOOP
; CHECK: |   |   |   + END LOOP
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |
; CHECK: |   + UNKNOWN LOOP i2
; CHECK: |   |   <i2 = 0>
; CHECK: |   |   do.body:
; CHECK: |   |
; CHECK: |   |   + DO i32 i3 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |   |   |   + DO i32 i4 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   |   |   + DO i32 i5 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   |   |   |   + DO i32 i6 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   |   |   |   |   + DO i32 i7 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   |   |   |   |   |   %0 = (%A)[0][i3][i4][i5][i7][i6];
; CHECK: |   |   |   |   |   |   |   (@B)[0][i3][i4][i5][i6][i7] = %0 + 1;
; CHECK: |   |   |   |   |   |   + END LOOP
; CHECK: |   |   |   |   |   + END LOOP
; CHECK: |   |   |   |   + END LOOP
; CHECK: |   |   |   + END LOOP
; CHECK: |   |   + END LOOP
; CHECK: |   |
; CHECK: |   |   %ld1 = (%b)[0];
; CHECK: |   |   if (%ld1 != 0)
; CHECK: |   |   {
; CHECK: |   |      <i2 = i2 + 1>
; CHECK: |   |      goto do.body;
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %ld2 = (%b)[0];
; CHECK: |   if (%ld2 != 0)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto do.outer;
; CHECK: |   }
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   do.outer:
; CHECK: |   if (%n > 0)
; CHECK: |   {
; CHECK: |      %div = %val  /  5;
; CHECK: |   }
; CHECK: |
; CHECK: |   + LiveIn symbases:{{.*}}[[VALSB]]
; CHECK: |   + UNKNOWN LOOP i2
; CHECK: |   |   <i2 = 0>
; CHECK: |   |   do.body:
; CHECK: |   |
; CHECK: |   |      %temp = %val  /  5;
; CHECK: |   |   + DO i32 i3 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |   |   |   + DO i32 i4 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   |   |   + DO i32 i5 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   |   |   |   + DO i32 i6 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   |   |   |   |   + DO i32 i7 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   |   |   |   |   |   (%ContractedArray)[0][i6][i7] = %temp;
; CHECK: |   |   |   |   |   |   + END LOOP
; CHECK: |   |   |   |   |   + END LOOP
; CHECK: |   |   |   |   |
; CHECK: |   |   |   |   |
; CHECK: |   |   |   |   |   + DO i32 i6 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   |   |   |   |   + DO i32 i7 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   |   |   |   |   |   %0 = (%ContractedArray)[0][i7][i6];
; CHECK: |   |   |   |   |   |   |   (@B)[0][i3][i4][i5][i6][i7] = %0 + 1;
; CHECK: |   |   |   |   |   |   + END LOOP
; CHECK: |   |   |   |   |   + END LOOP
; CHECK: |   |   |   |   + END LOOP
; CHECK: |   |   |   + END LOOP
; CHECK: |   |   + END LOOP
; CHECK: |   |
; CHECK: |   |   %ld1 = (%b)[0];
; CHECK: |   |   if (%ld1 != 0)
; CHECK: |   |   {
; CHECK: |   |      <i2 = i2 + 1>
; CHECK: |   |      goto do.body;
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %ld2 = (%b)[0];
; CHECK: |   if (%ld2 != 0)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto do.outer;
; CHECK: |   }
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = dso_local global [100 x [100 x [100 x [10 x [10 x i32]]]]] zeroinitializer, align 16

; Function Attrs: noinline nounwind uwtable
define dso_local void @shell(ptr %b, i32 %n, i32 %val){
entry:
  %A = alloca [100 x [100 x [100 x [10 x [10 x i32]]]]], align 16
 br label %do.outer

do.outer:
  %ncmp = icmp sgt i32 %n, 0
  br i1 %ncmp, label %for.body.pre, label %for.exit

for.body.pre:
  %div = sdiv i32 %val, 5
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc30
  %m.052 = phi i32 [ 0, %for.body.pre ], [ %inc31, %for.inc30 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body, %for.inc27
  %l.051 = phi i32 [ 0, %for.body ], [ %inc28, %for.inc27 ]
  br label %for.body6

for.body6:                                        ; preds = %for.body3, %for.inc24
  %k.050 = phi i32 [ 0, %for.body3 ], [ %inc25, %for.inc24 ]
  br label %for.body9

for.body9:                                        ; preds = %for.body6, %for.inc21
  %i.049 = phi i32 [ 0, %for.body6 ], [ %inc22, %for.inc21 ]
  br label %for.body12

for.body12:                                       ; preds = %for.body9, %for.inc
  %j.048 = phi i32 [ 0, %for.body9 ], [ %inc, %for.inc ]
  %idxprom = sext i32 %m.052 to i64
  %arrayidx = getelementptr inbounds [100 x [100 x [100 x [10 x [10 x i32]]]]], ptr %A, i64 0, i64 %idxprom
  %idxprom13 = sext i32 %l.051 to i64
  %arrayidx14 = getelementptr inbounds [100 x [100 x [10 x [10 x i32]]]], ptr %arrayidx, i64 0, i64 %idxprom13
  %idxprom15 = sext i32 %k.050 to i64
  %arrayidx16 = getelementptr inbounds [100 x [10 x [10 x i32]]], ptr %arrayidx14, i64 0, i64 %idxprom15
  %idxprom17 = sext i32 %i.049 to i64
  %arrayidx18 = getelementptr inbounds [10 x [10 x i32]], ptr %arrayidx16, i64 0, i64 %idxprom17
  %idxprom19 = sext i32 %j.048 to i64
  %arrayidx20 = getelementptr inbounds [10 x i32], ptr %arrayidx18, i64 0, i64 %idxprom19
  store i32 %div, ptr %arrayidx20, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body12
  %inc = add nsw i32 %j.048, 1
  %cmp11 = icmp slt i32 %inc, 10
  br i1 %cmp11, label %for.body12, label %for.end

for.end:                                          ; preds = %for.inc
  br label %for.inc21

for.inc21:                                        ; preds = %for.end
  %inc22 = add nsw i32 %i.049, 1
  %cmp8 = icmp slt i32 %inc22, 10
  br i1 %cmp8, label %for.body9, label %for.end23

for.end23:                                        ; preds = %for.inc21
  br label %for.inc24

for.inc24:                                        ; preds = %for.end23
  %inc25 = add nsw i32 %k.050, 1
  %cmp5 = icmp slt i32 %inc25, 100
  br i1 %cmp5, label %for.body6, label %for.end26

for.end26:                                        ; preds = %for.inc24
  br label %for.inc27

for.inc27:                                        ; preds = %for.end26
  %inc28 = add nsw i32 %l.051, 1
  %cmp2 = icmp slt i32 %inc28, 100
  br i1 %cmp2, label %for.body3, label %for.end29

for.end29:                                        ; preds = %for.inc27
  br label %for.inc30

for.inc30:                                        ; preds = %for.end29
  %inc31 = add nsw i32 %m.052, 1
  %cmp = icmp slt i32 %inc31, %n
  br i1 %cmp, label %for.body, label %for.end32

for.end32:                                        ; preds = %for.inc30
  br label %for.exit

for.exit:
  br label %do.body
   
do.body:                                          ; preds = %do.cond, %for.end32
  %ncmp2 = icmp sgt i32 %n, 0
  br i1 %ncmp2, label %for.body81.pre, label %for.body81.exit

for.body81.pre:
  br label %for.body81

for.body81:                                       ; preds = %for.end77, %for.inc131
  %m78.047 = phi i32 [ 0, %for.body81.pre ], [ %inc132, %for.inc131 ]
  br label %for.body85

for.body85:                                       ; preds = %for.body81, %for.inc128
  %l82.046 = phi i32 [ 0, %for.body81 ], [ %inc129, %for.inc128 ]
  br label %for.body89

for.body89:                                       ; preds = %for.body85, %for.inc125
  %k86.045 = phi i32 [ 0, %for.body85 ], [ %inc126, %for.inc125 ]
  br label %for.body93

for.body93:                                       ; preds = %for.body89, %for.inc122
  %i90.044 = phi i32 [ 0, %for.body89 ], [ %inc123, %for.inc122 ]
  br label %for.body97

for.body97:                                       ; preds = %for.body93, %for.inc119
  %j94.043 = phi i32 [ 0, %for.body93 ], [ %inc120, %for.inc119 ]
  %idxprom98 = sext i32 %m78.047 to i64
  %arrayidx99 = getelementptr inbounds [100 x [100 x [100 x [10 x [10 x i32]]]]], ptr %A, i64 0, i64 %idxprom98
  %idxprom100 = sext i32 %l82.046 to i64
  %arrayidx101 = getelementptr inbounds [100 x [100 x [10 x [10 x i32]]]], ptr %arrayidx99, i64 0, i64 %idxprom100
  %idxprom102 = sext i32 %k86.045 to i64
  %arrayidx103 = getelementptr inbounds [100 x [10 x [10 x i32]]], ptr %arrayidx101, i64 0, i64 %idxprom102
  %idxprom104 = sext i32 %j94.043 to i64
  %arrayidx105 = getelementptr inbounds [10 x [10 x i32]], ptr %arrayidx103, i64 0, i64 %idxprom104
  %idxprom106 = sext i32 %i90.044 to i64
  %arrayidx107 = getelementptr inbounds [10 x i32], ptr %arrayidx105, i64 0, i64 %idxprom106
  %0 = load i32, ptr %arrayidx107, align 4
  %add108 = add nsw i32 %0, 1
  %idxprom109 = sext i32 %m78.047 to i64
  %arrayidx110 = getelementptr inbounds [100 x [100 x [100 x [10 x [10 x i32]]]]], ptr @B, i64 0, i64 %idxprom109
  %idxprom111 = sext i32 %l82.046 to i64
  %arrayidx112 = getelementptr inbounds [100 x [100 x [10 x [10 x i32]]]], ptr %arrayidx110, i64 0, i64 %idxprom111
  %idxprom113 = sext i32 %k86.045 to i64
  %arrayidx114 = getelementptr inbounds [100 x [10 x [10 x i32]]], ptr %arrayidx112, i64 0, i64 %idxprom113
  %idxprom115 = sext i32 %i90.044 to i64
  %arrayidx116 = getelementptr inbounds [10 x [10 x i32]], ptr %arrayidx114, i64 0, i64 %idxprom115
  %idxprom117 = sext i32 %j94.043 to i64
  %arrayidx118 = getelementptr inbounds [10 x i32], ptr %arrayidx116, i64 0, i64 %idxprom117
  store i32 %add108, ptr %arrayidx118, align 4
  br label %for.inc119

for.inc119:                                       ; preds = %for.body97
  %inc120 = add nsw i32 %j94.043, 1
  %cmp96 = icmp slt i32 %inc120, 10
  br i1 %cmp96, label %for.body97, label %for.end121

for.end121:                                       ; preds = %for.inc119
  br label %for.inc122

for.inc122:                                       ; preds = %for.end121
  %inc123 = add nsw i32 %i90.044, 1
  %cmp92 = icmp slt i32 %inc123, 10
  br i1 %cmp92, label %for.body93, label %for.end124

for.end124:                                       ; preds = %for.inc122
  br label %for.inc125

for.inc125:                                       ; preds = %for.end124
  %inc126 = add nsw i32 %k86.045, 1
  %cmp88 = icmp slt i32 %inc126, 100
  br i1 %cmp88, label %for.body89, label %for.end127

for.end127:                                       ; preds = %for.inc125
  br label %for.inc128

for.inc128:                                       ; preds = %for.end127
  %inc129 = add nsw i32 %l82.046, 1
  %cmp84 = icmp slt i32 %inc129, 100
  br i1 %cmp84, label %for.body85, label %for.end130

for.end130:                                       ; preds = %for.inc128
  br label %for.inc131

for.inc131:                                       ; preds = %for.end130
  %inc132 = add nsw i32 %m78.047, 1
  %cmp80 = icmp slt i32 %inc132, %n
  br i1 %cmp80, label %for.body81, label %for.end133

for.end133:                                       ; preds = %for.inc131
  br label %for.body81.exit

for.body81.exit:
  br label %do.cond

do.cond:                                          ; preds = %for.end133
  %ld1 = load i32, ptr %b, align 4
  %tobool = icmp ne i32 %ld1, 0
  br i1 %tobool, label %do.body, label %do.outer.latch

do.outer.latch:                                          ; preds = %for.end133
  %ld2 = load i32, ptr %b, align 4
  %tobool1 = icmp ne i32 %ld2, 0
  br i1 %tobool1, label %do.outer, label %do.end

do.end:                                           ; preds = %do.cond
  ret void
}

