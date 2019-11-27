; RUN: opt -hir-ssa-deconstruction -disable-output -hir-loop-fusion -print-after=hir-loop-fusion < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; <0>          BEGIN REGION { }
; <84>               + DO i1 = 0, 99, 1   <DO_LOOP>
; <85>               |   + DO i2 = 0, 99, 1   <DO_LOOP>
; <8>                |   |   (@A)[0][i1][i2] = i1 + i2;
; <10>               |   |   %2 = (@J)[0][i1][i2];
; <15>               |   |   (@E)[0][i1][i2] = i1 + i2 + %2 + 1;
; <85>               |   + END LOOP
; <85>               |
; <86>               |
; <86>               |   + DO i2 = 0, 99, 1   <DO_LOOP>
; <27>               |   |   %5 = (@A)[0][i1][i2];
; <31>               |   |   (@B)[0][i1][i2] = i2 + %5;
; <86>               |   + END LOOP
; <86>               |
; <87>               |
; <87>               |   + DO i2 = 0, 99, 1   <DO_LOOP>
; <43>               |   |   %7 = (@B)[0][i1][i2];
; <45>               |   |   %8 = (@F)[0][i1][i2];
; <50>               |   |   (@C)[0][i1][i2] = i2 + %7 + %8;
; <87>               |   + END LOOP
; <87>               |
; <88>               |
; <88>               |   + DO i2 = 0, 99, 1   <DO_LOOP>
; <62>               |   |   %10 = (@E)[0][i1][i2];
; <64>               |   |   %11 = (@F)[0][i1][i2];
; <67>               |   |   %12 = (@J)[0][i1][i2];
; <70>               |   |   (@D)[0][i1][i2] = %10 + %11 + %12;
; <88>               |   + END LOOP
; <84>               + END LOOP
; <0>          END REGION

; Fuse Graph before optimization
; Nodes:
; 0:{ 85 } 1:{ 86 } 2:{ 87 } 3:{ 88 }
; Undirected:
; 2:{ 87 } 100--> 3:{ 88 }
; 3:{ 88 } 100--> 2:{ 87 }
; Sucessors Directed:
; 0:{ 85 } 100--> 1:{ 86 }
; 0:{ 85 } 200--> 3:{ 88 }
; 1:{ 86 } 100--> 2:{ 87 }
; Predecessors Directed:
; 1:{ 86 } 100--> 0:{ 85 }
; 2:{ 87 } 100--> 1:{ 86 }
; 3:{ 88 } 200--> 0:{ 85 }

; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   |   (@A)[0][i1][i2] = i1 + i2;
; CHECK:       |   |   %2 = (@J)[0][i1][i2];
; CHECK:       |   |   (@E)[0][i1][i2] = i1 + i2 + %2 + 1;
; CHECK:       |   |   %10 = (@E)[0][i1][i2];
; CHECK:       |   |   %11 = (@F)[0][i1][i2];
; CHECK:       |   |   %12 = (@J)[0][i1][i2];
; CHECK:       |   |   (@D)[0][i1][i2] = %10 + %11 + %12;
; CHECK:       |   |   %5 = (@A)[0][i1][i2];
; CHECK:       |   |   (@B)[0][i1][i2] = i2 + %5;
; CHECK:       |   |   %7 = (@B)[0][i1][i2];
; CHECK:       |   |   %8 = (@F)[0][i1][i2];
; CHECK:       |   |   (@C)[0][i1][i2] = i2 + %7 + %8;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@J = common dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@E = common dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@F = common dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@D = common dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup60, %entry
  %indvars.iv142 = phi i64 [ 0, %entry ], [ %indvars.iv.next143, %for.cond.cleanup60 ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup60
  ret void

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %0 = add nuw nsw i64 %indvars.iv, %indvars.iv142
  %arrayidx6 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %indvars.iv142, i64 %indvars.iv
  %1 = trunc i64 %0 to i32
  store i32 %1, i32* %arrayidx6, align 4
  %arrayidx12 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @J, i64 0, i64 %indvars.iv142, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx12, align 4
  %3 = trunc i64 %0 to i32
  %4 = add i32 %3, 1
  %add13 = add nsw i32 %4, %2
  %arrayidx17 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @E, i64 0, i64 %indvars.iv142, i64 %indvars.iv
  store i32 %add13, i32* %arrayidx17, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.body22.preheader, label %for.body4

for.body22.preheader:                             ; preds = %for.body4
  br label %for.body22

for.body22:                                       ; preds = %for.body22.preheader, %for.body22
  %indvars.iv133 = phi i64 [ %indvars.iv.next134, %for.body22 ], [ 0, %for.body22.preheader ]
  %arrayidx26 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %indvars.iv142, i64 %indvars.iv133
  %5 = load i32, i32* %arrayidx26, align 4
  %6 = trunc i64 %indvars.iv133 to i32
  %add27 = add nsw i32 %5, %6
  %arrayidx31 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @B, i64 0, i64 %indvars.iv142, i64 %indvars.iv133
  store i32 %add27, i32* %arrayidx31, align 4
  %indvars.iv.next134 = add nuw nsw i64 %indvars.iv133, 1
  %exitcond135 = icmp eq i64 %indvars.iv.next134, 100
  br i1 %exitcond135, label %for.body39.preheader, label %for.body22

for.body39.preheader:                             ; preds = %for.body22
  br label %for.body39

for.body39:                                       ; preds = %for.body39.preheader, %for.body39
  %indvars.iv136 = phi i64 [ %indvars.iv.next137, %for.body39 ], [ 0, %for.body39.preheader ]
  %arrayidx43 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @B, i64 0, i64 %indvars.iv142, i64 %indvars.iv136
  %7 = load i32, i32* %arrayidx43, align 4
  %arrayidx47 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @F, i64 0, i64 %indvars.iv142, i64 %indvars.iv136
  %8 = load i32, i32* %arrayidx47, align 4
  %9 = trunc i64 %indvars.iv136 to i32
  %add48 = add i32 %7, %9
  %add49 = add i32 %add48, %8
  %arrayidx53 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @C, i64 0, i64 %indvars.iv142, i64 %indvars.iv136
  store i32 %add49, i32* %arrayidx53, align 4
  %indvars.iv.next137 = add nuw nsw i64 %indvars.iv136, 1
  %exitcond138 = icmp eq i64 %indvars.iv.next137, 100
  br i1 %exitcond138, label %for.body61.preheader, label %for.body39

for.body61.preheader:                             ; preds = %for.body39
  br label %for.body61

for.cond.cleanup60:                               ; preds = %for.body61
  %indvars.iv.next143 = add nuw nsw i64 %indvars.iv142, 1
  %exitcond144 = icmp eq i64 %indvars.iv.next143, 100
  br i1 %exitcond144, label %for.cond.cleanup, label %for.cond1.preheader

for.body61:                                       ; preds = %for.body61.preheader, %for.body61
  %indvars.iv139 = phi i64 [ %indvars.iv.next140, %for.body61 ], [ 0, %for.body61.preheader ]
  %arrayidx65 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @E, i64 0, i64 %indvars.iv142, i64 %indvars.iv139
  %10 = load i32, i32* %arrayidx65, align 4
  %arrayidx69 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @F, i64 0, i64 %indvars.iv142, i64 %indvars.iv139
  %11 = load i32, i32* %arrayidx69, align 4
  %add70 = add nsw i32 %11, %10
  %arrayidx74 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @J, i64 0, i64 %indvars.iv142, i64 %indvars.iv139
  %12 = load i32, i32* %arrayidx74, align 4
  %add75 = add nsw i32 %add70, %12
  %arrayidx79 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @D, i64 0, i64 %indvars.iv142, i64 %indvars.iv139
  store i32 %add75, i32* %arrayidx79, align 4
  %indvars.iv.next140 = add nuw nsw i64 %indvars.iv139, 1
  %exitcond141 = icmp eq i64 %indvars.iv.next140, 100
  br i1 %exitcond141, label %for.cond.cleanup60, label %for.body61
}

