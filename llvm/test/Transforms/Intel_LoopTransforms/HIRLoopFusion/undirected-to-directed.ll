; RUN: opt -hir-ssa-deconstruction -disable-output -hir-loop-fusion -print-after=hir-loop-fusion < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that un-directed edges will be converted to directed edges.

; int A[100][100];
; int B[100][100];
; int C[100][100];
; int D[100][100];
; int E[100][100];
; int F[100][100];
; int J[100][100];
;
; void foo() {
;   for(int i=0;i<100;++i) {
;     for(int j=0;j<100;++j) {
;       D[i][j] = A[i][j] + B[i][j];
;     }
;     for(int j=0;j<100;++j) {
;       E[i][j] = A[i][j] + C[i][j];
;     }
;     for(int j=0;j<100;++j) {
;       F[i][j] = D[i][j] + E[i][j];
;     }
;     for(int j=0;j<100;++j) {
;       J[i][j] = B[i][j] + C[i][j] + F[i][j] + D[i][j];
;     }
;   }
; }
;
; <0>          BEGIN REGION { }
; <82>               + DO i1 = 0, 99, 1   <DO_LOOP>
; <83>               |   + DO i2 = 0, 99, 1   <DO_LOOP>
; <6>                |   |   %0 = (@A)[0][i1][i2];
; <8>                |   |   %1 = (@B)[0][i1][i2];
; <11>               |   |   (@D)[0][i1][i2] = %0 + %1;
; <83>               |   + END LOOP
; <83>               |
; <84>               |
; <84>               |   + DO i2 = 0, 99, 1   <DO_LOOP>
; <23>               |   |   %2 = (@A)[0][i1][i2];
; <25>               |   |   %3 = (@C)[0][i1][i2];
; <28>               |   |   (@E)[0][i1][i2] = %2 + %3;
; <84>               |   + END LOOP
; <84>               |
; <85>               |
; <85>               |   + DO i2 = 0, 99, 1   <DO_LOOP>
; <40>               |   |   %4 = (@D)[0][i1][i2];
; <42>               |   |   %5 = (@E)[0][i1][i2];
; <45>               |   |   (@F)[0][i1][i2] = %4 + %5;
; <85>               |   + END LOOP
; <85>               |
; <86>               |
; <86>               |   + DO i2 = 0, 99, 1   <DO_LOOP>
; <57>               |   |   %6 = (@B)[0][i1][i2];
; <59>               |   |   %7 = (@C)[0][i1][i2];
; <62>               |   |   %8 = (@F)[0][i1][i2];
; <65>               |   |   %9 = (@D)[0][i1][i2];
; <68>               |   |   (@J)[0][i1][i2] = %6 + %7 + %8 + %9;
; <86>               |   + END LOOP
; <82>               + END LOOP
; <0>          END REGION
;
; Fuse Graph before optimization
; Nodes:
; 0:{ 83 } 1:{ 84 } 2:{ 85 } 3:{ 86 }
; Undirected:
; 0:{ 83 } 100--> 1:{ 84 }
; 1:{ 84 } 100--> 0:{ 83 }
; Sucessors Directed:
; 0:{ 83 } 100--> 2:{ 85 }
; 0:{ 83 } 200--> 3:{ 86 }
; 1:{ 84 } 100--> 2:{ 85 }
; 1:{ 84 } 100--> 3:{ 86 }
; 2:{ 85 } 200--> 3:{ 86 }
; Predecessors Directed:
; 2:{ 85 } 100--> 0:{ 83 }
; 2:{ 85 } 100--> 1:{ 84 }
; 3:{ 86 } 200--> 0:{ 83 }
; 3:{ 86 } 200--> 2:{ 85 }
; 3:{ 86 } 100--> 1:{ 84 }

; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK-NOT:   |   | DO i2
; CHECK:       |   |   %2 = (@A)[0][i1][i2];
; CHECK:       |   |   %3 = (@C)[0][i1][i2];
; CHECK:       |   |   (@E)[0][i1][i2] = %2 + %3;
; CHECK:       |   |   %0 = (@A)[0][i1][i2];
; CHECK:       |   |   %1 = (@B)[0][i1][i2];
; CHECK:       |   |   (@D)[0][i1][i2] = %0 + %1;
; CHECK:       |   |   %4 = (@D)[0][i1][i2];
; CHECK:       |   |   %5 = (@E)[0][i1][i2];
; CHECK:       |   |   (@F)[0][i1][i2] = %4 + %5;
; CHECK:       |   |   %6 = (@B)[0][i1][i2];
; CHECK:       |   |   %7 = (@C)[0][i1][i2];
; CHECK:       |   |   %8 = (@F)[0][i1][i2];
; CHECK:       |   |   %9 = (@D)[0][i1][i2];
; CHECK:       |   |   (@J)[0][i1][i2] = %6 + %7 + %8 + %9;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK:  END REGION

source_filename = "undirectional-to-directional.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@D = common dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@E = common dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@F = common dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@J = common dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup60, %entry
  %indvars.iv143 = phi i64 [ 0, %entry ], [ %indvars.iv.next144, %for.cond.cleanup60 ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup60
  ret void

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx6 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %indvars.iv143, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx6, align 4
  %arrayidx10 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @B, i64 0, i64 %indvars.iv143, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx10, align 4
  %add = add nsw i32 %1, %0
  %arrayidx14 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @D, i64 0, i64 %indvars.iv143, i64 %indvars.iv
  store i32 %add, i32* %arrayidx14, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.body19.preheader, label %for.body4

for.body19.preheader:                             ; preds = %for.body4
  br label %for.body19

for.body19:                                       ; preds = %for.body19.preheader, %for.body19
  %indvars.iv134 = phi i64 [ %indvars.iv.next135, %for.body19 ], [ 0, %for.body19.preheader ]
  %arrayidx23 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %indvars.iv143, i64 %indvars.iv134
  %2 = load i32, i32* %arrayidx23, align 4
  %arrayidx27 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @C, i64 0, i64 %indvars.iv143, i64 %indvars.iv134
  %3 = load i32, i32* %arrayidx27, align 4
  %add28 = add nsw i32 %3, %2
  %arrayidx32 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @E, i64 0, i64 %indvars.iv143, i64 %indvars.iv134
  store i32 %add28, i32* %arrayidx32, align 4
  %indvars.iv.next135 = add nuw nsw i64 %indvars.iv134, 1
  %exitcond136 = icmp eq i64 %indvars.iv.next135, 100
  br i1 %exitcond136, label %for.body40.preheader, label %for.body19

for.body40.preheader:                             ; preds = %for.body19
  br label %for.body40

for.body40:                                       ; preds = %for.body40.preheader, %for.body40
  %indvars.iv137 = phi i64 [ %indvars.iv.next138, %for.body40 ], [ 0, %for.body40.preheader ]
  %arrayidx44 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @D, i64 0, i64 %indvars.iv143, i64 %indvars.iv137
  %4 = load i32, i32* %arrayidx44, align 4
  %arrayidx48 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @E, i64 0, i64 %indvars.iv143, i64 %indvars.iv137
  %5 = load i32, i32* %arrayidx48, align 4
  %add49 = add nsw i32 %5, %4
  %arrayidx53 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @F, i64 0, i64 %indvars.iv143, i64 %indvars.iv137
  store i32 %add49, i32* %arrayidx53, align 4
  %indvars.iv.next138 = add nuw nsw i64 %indvars.iv137, 1
  %exitcond139 = icmp eq i64 %indvars.iv.next138, 100
  br i1 %exitcond139, label %for.body61.preheader, label %for.body40

for.body61.preheader:                             ; preds = %for.body40
  br label %for.body61

for.cond.cleanup60:                               ; preds = %for.body61
  %indvars.iv.next144 = add nuw nsw i64 %indvars.iv143, 1
  %exitcond145 = icmp eq i64 %indvars.iv.next144, 100
  br i1 %exitcond145, label %for.cond.cleanup, label %for.cond1.preheader

for.body61:                                       ; preds = %for.body61.preheader, %for.body61
  %indvars.iv140 = phi i64 [ %indvars.iv.next141, %for.body61 ], [ 0, %for.body61.preheader ]
  %arrayidx65 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @B, i64 0, i64 %indvars.iv143, i64 %indvars.iv140
  %6 = load i32, i32* %arrayidx65, align 4
  %arrayidx69 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @C, i64 0, i64 %indvars.iv143, i64 %indvars.iv140
  %7 = load i32, i32* %arrayidx69, align 4
  %add70 = add nsw i32 %7, %6
  %arrayidx74 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @F, i64 0, i64 %indvars.iv143, i64 %indvars.iv140
  %8 = load i32, i32* %arrayidx74, align 4
  %add75 = add nsw i32 %add70, %8
  %arrayidx79 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @D, i64 0, i64 %indvars.iv143, i64 %indvars.iv140
  %9 = load i32, i32* %arrayidx79, align 4
  %add80 = add nsw i32 %add75, %9
  %arrayidx84 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @J, i64 0, i64 %indvars.iv143, i64 %indvars.iv140
  store i32 %add80, i32* %arrayidx84, align 4
  %indvars.iv.next141 = add nuw nsw i64 %indvars.iv140, 1
  %exitcond142 = icmp eq i64 %indvars.iv.next141, 100
  br i1 %exitcond142, label %for.cond.cleanup60, label %for.body61
}

