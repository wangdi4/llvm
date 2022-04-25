; RUN: opt -analyze -enable-new-pm=0 -hir-region-identification < %s 2>&1 | FileCheck %s
; RUN: opt -passes="print<hir-region-identification>" < %s 2>&1 | FileCheck %s

; CHECK-NOT: Region 1

; Test checks that HIRRegionIdentification does not allow constant
; out-of-bounds access for arrays.


; int (*int_arr_ptr)[20];
; int test1( int n) {
;   int i;
;   for (i = 0; i < 2; i++)
;        int_arr_ptr[i][31] += n;
;
;   return 0;
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S2 = type { double, [20 x %struct.S1], i16 }
%struct.S1 = type { float, [20 x i32], i64 }

@int_arr_ptr = dso_local local_unnamed_addr global [20 x i32]* null, align 8
@int_arr = dso_local local_unnamed_addr global [20 x [20 x [20 x i32]]] zeroinitializer, align 16
@s2_arr_arr = dso_local local_unnamed_addr global [20 x [20 x %struct.S2]] zeroinitializer, align 16
@s2_arr = dso_local local_unnamed_addr global [20 x %struct.S2] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @test1(i32 %n) local_unnamed_addr {
entry:
  %0 = load [20 x i32]*, [20 x i32]** @int_arr_ptr, align 8
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %cmp = phi i1 [ true, %entry ], [ false, %for.body ]
  %indvars.iv = phi i64 [ 0, %entry ], [ 1, %for.body ]
  %arrayidx1 = getelementptr inbounds [20 x i32], [20 x i32]* %0, i64 %indvars.iv, i64 31
  %1 = load i32, i32* %arrayidx1, align 4
  %add = add nsw i32 %1, %n
  store i32 %add, i32* %arrayidx1, align 4
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  ret i32 0
}



; int int_arr[20][20][20];
; int test2(int n) {
;   int i, j;
;   for (i = 0; i < 2; i++)
;     for (j = 0; j < 3; j++)
;       int_arr[i][25][j] += n;
;
;   return 0;
; }

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @test2(i32 %n) local_unnamed_addr {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc7
  %cmp = phi i1 [ true, %entry ], [ false, %for.inc7 ]
  %indvars.iv18 = phi i64 [ 0, %entry ], [ 1, %for.inc7 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx6 = getelementptr inbounds [20 x [20 x [20 x i32]]], [20 x [20 x [20 x i32]]]* @int_arr, i64 0, i64 %indvars.iv18, i64 25, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx6, align 4
  %add = add nsw i32 %0, %n
  store i32 %add, i32* %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 3
  br i1 %exitcond.not, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %for.body3
  br i1 %cmp, label %for.cond1.preheader, label %for.end9

for.end9:                                         ; preds = %for.inc7
  ret i32 0
}


; struct S1 {
;   float f1;
;   int arr1[20];
;   long l1;
; };
;
; struct S2 {
;   double d2;
;   struct S1 s1_2[20];
;   short sh2;
; } s2_arr[20], s2_arr_arr[20][20];
;
; int test3(int n) {
;   int i, j;
;   for (i = 0; i < 10; i++)
;     for (j = 0; j < 10; j++)
;         s2_arr_arr[i][21].s1_2[j].arr1[i] += n;
;
;   return 0;
; }

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @test3(i32 %n) local_unnamed_addr {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc9
  %indvars.iv21 = phi i64 [ 0, %entry ], [ %indvars.iv.next22, %for.inc9 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx8 = getelementptr inbounds [20 x [20 x %struct.S2]], [20 x [20 x %struct.S2]]* @s2_arr_arr, i64 0, i64 %indvars.iv21, i64 21, i32 1, i64 %indvars.iv, i32 1, i64 %indvars.iv21
  %0 = load i32, i32* %arrayidx8, align 4
  %add = add nsw i32 %0, %n
  store i32 %add, i32* %arrayidx8, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond.not, label %for.inc9, label %for.body3

for.inc9:                                         ; preds = %for.body3
  %indvars.iv.next22 = add nuw nsw i64 %indvars.iv21, 1
  %exitcond23.not = icmp eq i64 %indvars.iv.next22, 10
  br i1 %exitcond23.not, label %for.end11, label %for.cond1.preheader

for.end11:                                        ; preds = %for.inc9
  ret i32 0
}


; int test4(int n) {
;   int i, k;
;   struct S2 s2_local_arr[20];
;   for (i = 0; i < 2; i++)
;       for (k = 0; k < 3; k++)
;         s2_local_arr[k].s1_2[24].arr1[i] += n;
;
;   return 0;
; }

; Function Attrs: nofree nosync nounwind readnone uwtable
define dso_local i32 @test4(i32 %n) local_unnamed_addr {
entry:
  %s2_local_arr = alloca [20 x %struct.S2], align 16
  %0 = bitcast [20 x %struct.S2]* %s2_local_arr to i8*
  call void @llvm.lifetime.start.p0i8(i64 38720, i8* nonnull %0)
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc7
  %cmp = phi i1 [ true, %entry ], [ false, %for.inc7 ]
  %indvars.iv18 = phi i64 [ 0, %entry ], [ 1, %for.inc7 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx6 = getelementptr inbounds [20 x %struct.S2], [20 x %struct.S2]* %s2_local_arr, i64 0, i64 %indvars.iv, i32 1, i64 24, i32 1, i64 %indvars.iv18
  %1 = load i32, i32* %arrayidx6, align 4
  %add = add nsw i32 %1, %n
  store i32 %add, i32* %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 3
  br i1 %exitcond.not, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %for.body3
  br i1 %cmp, label %for.cond1.preheader, label %for.end9

for.end9:                                         ; preds = %for.inc7
  call void @llvm.lifetime.end.p0i8(i64 38720, i8* nonnull %0)
  ret i32 0
}


; int test5(struct S2 *ptr_s2, int n) {
;   int j, k;
;     for (j = 0; j < 2; j++)
;       for (k = 0; k < 3; k++)
;         ptr_s2[k].s1_2[j].arr1[25] += n;
;
;   return 0;
; }

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @test5(%struct.S2* nocapture %ptr_s2, i32 %n) local_unnamed_addr {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc7
  %cmp = phi i1 [ true, %entry ], [ false, %for.inc7 ]
  %indvars.iv18 = phi i64 [ 0, %entry ], [ 1, %for.inc7 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx6 = getelementptr inbounds %struct.S2, %struct.S2* %ptr_s2, i64 %indvars.iv, i32 1, i64 %indvars.iv18, i32 1, i64 25
  %0 = load i32, i32* %arrayidx6, align 4
  %add = add nsw i32 %0, %n
  store i32 %add, i32* %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 3
  br i1 %exitcond.not, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %for.body3
  br i1 %cmp, label %for.cond1.preheader, label %for.end9

for.end9:                                         ; preds = %for.inc7
  ret i32 0
}



; int test6(struct S2 *ptr_s2, int n) {
;   int i;
;   for (i = 0; i < 3; i++)
;        ptr_s2->s1_2[26].arr1[i] += n;
;
;   return 0;
; }

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @test6(%struct.S2* nocapture %ptr_s2, i32 %n) local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx1 = getelementptr inbounds %struct.S2, %struct.S2* %ptr_s2, i64 0, i32 1, i64 26, i32 1, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx1, align 4
  %add = add nsw i32 %0, %n
  store i32 %add, i32* %arrayidx1, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 3
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 0
}

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)
