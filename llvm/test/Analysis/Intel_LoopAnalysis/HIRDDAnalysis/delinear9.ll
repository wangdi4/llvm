; RUN: opt -hir-ssa-deconstruction -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 <%s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output <%s 2>&1 | FileCheck %s

; Check that linear DD refs were successfully delinearized.

;#define MATDAT  short int *__restrict__
;void matrix_mul_matrix(unsigned int  N,
;           MATDAT C, MATDAT A, MATDAT B) {
;  unsigned int  i,j,k;
;  for (i=0; i<N; i++) {
;    for (j=0; j<N; j++) {
;      C[i*N+j]=0;
;      for(k=0;k<N;k++) {
;         C[i*N+j]+= A[i*N+k] *  B[k*N+j];
;      }
;    }
;  }
;}

; HIR dump
;    BEGIN REGION { }
;          + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
;          |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
;          |   |   %1 = 0;
;          |   |
;          |   |   + DO i3 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
;          |   |   |   %3 = (%A)[%N * i1 + i3];
;          |   |   |   %4 = (%B)[i2 + %N * i3];
;          |   |   |   %1 = (%3 * %4)  +  %1;
;          |   |   + END LOOP
;          |   |
;          |   |   (%C)[%N * i1 + i2] = %1;
;          |   + END LOOP
;          + END LOOP
;    END REGION

; CHECK-NOT: (%C)[%N * i1 + i2] --> (%C)[%N * i1 + i2] OUTPUT (* *) (? ?)



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @matrix_mul_matrix(i32 %N, i16* noalias nocapture %C, i16* noalias nocapture readonly %A, i16* noalias nocapture readonly %B) local_unnamed_addr {
entry:
  %cmp551.not = icmp eq i32 %N, 0
  br i1 %cmp551.not, label %for.end29, label %for.cond1.preheader.preheader

for.cond1.preheader.preheader:                    ; preds = %entry
  %wide.trip.count60 = zext i32 %N to i64
  br label %for.body3.preheader

for.body3.preheader:                              ; preds = %for.inc27, %for.cond1.preheader.preheader
  %i.057 = phi i32 [ %inc28, %for.inc27 ], [ 0, %for.cond1.preheader.preheader ]
  %mul = mul i32 %i.057, %N
  br label %for.body6.preheader

for.body6.preheader:                              ; preds = %for.inc24, %for.body3.preheader
  %indvars.iv58 = phi i64 [ 0, %for.body3.preheader ], [ %indvars.iv.next59, %for.inc24 ]
  %0 = trunc i64 %indvars.iv58 to i32
  %add = add i32 %mul, %0
  %idxprom = zext i32 %add to i64
  %arrayidx = getelementptr inbounds i16, i16* %C, i64 %idxprom
  br label %for.body6

for.body6:                                        ; preds = %for.body6.preheader, %for.body6
  %indvars.iv = phi i64 [ 0, %for.body6.preheader ], [ %indvars.iv.next, %for.body6 ]
  %1 = phi i16 [ 0, %for.body6.preheader ], [ %add22, %for.body6 ]
  %2 = trunc i64 %indvars.iv to i32
  %add8 = add i32 %mul, %2
  %idxprom9 = zext i32 %add8 to i64
  %arrayidx10 = getelementptr inbounds i16, i16* %A, i64 %idxprom9
  %3 = load i16, i16* %arrayidx10, align 2
  %mul11 = mul i32 %2, %N
  %add12 = add i32 %mul11, %0
  %idxprom13 = zext i32 %add12 to i64
  %arrayidx14 = getelementptr inbounds i16, i16* %B, i64 %idxprom13
  %4 = load i16, i16* %arrayidx14, align 2
  %mul16 = mul i16 %4, %3
  %add22 = add i16 %mul16, %1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count60
  br i1 %exitcond.not, label %for.inc24, label %for.body6

for.inc24:                                        ; preds = %for.body6
  %add22.lcssa = phi i16 [ %add22, %for.body6 ]
  store i16 %add22.lcssa, i16* %arrayidx, align 2
  %indvars.iv.next59 = add nuw nsw i64 %indvars.iv58, 1
  %exitcond61.not = icmp eq i64 %indvars.iv.next59, %wide.trip.count60
  br i1 %exitcond61.not, label %for.inc27, label %for.body6.preheader

for.inc27:                                        ; preds = %for.inc24
  %inc28 = add nuw i32 %i.057, 1
  %exitcond62.not = icmp eq i32 %inc28, %N
  br i1 %exitcond62.not, label %for.end29.loopexit, label %for.body3.preheader

for.end29.loopexit:                               ; preds = %for.inc27
  br label %for.end29

for.end29:                                        ; preds = %for.end29.loopexit, %entry
  ret void
}

