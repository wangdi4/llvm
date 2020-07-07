; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser | FileCheck %s

; Check parsing output for the loop verifying that header phi %indvars.iv38 with unusual access pattern is parsed conservatively as a blob.

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   %.pre = (@kh)[0][i1 + 5];
; CHECK: |   %1 = %.pre;
; CHECK: |
; CHECK: |   + DO i2 = 0, 0, 1   <DO_LOOP>
; CHECK: |   |   %3 = (@kh)[0][i1 + i2 + 8];
; CHECK: |   |   %1 = %1  &  %3 + 9;
; CHECK: |   |   (@kh)[0][i1 + 5] = %1;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %4 = (%indvars.iv38)[0][0];
; CHECK: |   (@t3)[0][i1 + 5] = %4;
; CHECK: |   %indvars.iv38 = &(([20 x i32]*)(%indvars.iv38)[0][1]);
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@n = local_unnamed_addr global i32 0, align 4
@g = local_unnamed_addr global i8 0, align 1
@kh = local_unnamed_addr global [20 x i32] zeroinitializer, align 16
@t3 = local_unnamed_addr global [20 x i32] zeroinitializer, align 16
@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: nounwind uwtable
define i32 @main() local_unnamed_addr #0 {
entry:
  store i32 0, i32* @n, align 4
  br label %for.end

for.end:                                          ; preds = %for.end
  store i32 20, i32* @n, align 4
  store i8 5, i8* @g, align 1
  br label %for.body4

for.body4:                                        ; preds = %for.end24, %for.end
  %indvars.iv40 = phi i64 [ 5, %for.end ], [ %indvars.iv.next41, %for.end24 ]
  %indvars.iv38 = phi [20 x i32]* [ bitcast (i32* getelementptr inbounds ([20 x i32], [20 x i32]* @kh, i64 0, i64 8) to [20 x i32]*), %for.end ], [ %5, %for.end24 ]
  %arrayidx16 = getelementptr inbounds [20 x i32], [20 x i32]* @kh, i64 0, i64 %indvars.iv40
  %0 = and i64 %indvars.iv40, 4294967295
  %.pre = load i32, i32* %arrayidx16, align 4
  br label %for.body11

for.body11:                                       ; preds = %for.body11, %for.body4
  %1 = phi i32 [ %.pre, %for.body4 ], [ %and, %for.body11 ]
  %indvars.iv35 = phi i64 [ %indvars.iv40, %for.body4 ], [ %indvars.iv.next36, %for.body11 ]
  %2 = add nuw nsw i64 %indvars.iv35, 3
  %arrayidx13 = getelementptr inbounds [20 x i32], [20 x i32]* @kh, i64 0, i64 %2
  %3 = load i32, i32* %arrayidx13, align 4
  %add14 = add i32 %3, 9
  %and = and i32 %1, %add14
  store i32 %and, i32* %arrayidx16, align 4
  %indvars.iv.next36 = add nuw nsw i64 %indvars.iv35, 1
  %cmp9 = icmp ult i64 %indvars.iv35, %0
  br i1 %cmp9, label %for.body11, label %for.end24

for.end24:                                        ; preds = %for.body11
  %indvars.iv3839 = getelementptr inbounds [20 x i32], [20 x i32]* %indvars.iv38, i64 0, i64 0
  %arrayidx21 = getelementptr inbounds [20 x i32], [20 x i32]* @t3, i64 0, i64 %indvars.iv40
  %4 = load i32, i32* %indvars.iv3839, align 4
  store i32 %4, i32* %arrayidx21, align 4
  %indvars.iv.next41 = add nuw nsw i64 %indvars.iv40, 1
  %scevgep = getelementptr [20 x i32], [20 x i32]* %indvars.iv38, i64 0, i64 1
  %5 = bitcast i32* %scevgep to [20 x i32]*
  %exitcond = icmp eq i64 %indvars.iv.next41, 10
  br i1 %exitcond, label %for.end27, label %for.body4

for.end27:                                        ; preds = %for.end24
  store i8 10, i8* @g, align 1
  %6 = load i32, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @t3, i64 0, i64 8), align 16
  ret i32 0
}


