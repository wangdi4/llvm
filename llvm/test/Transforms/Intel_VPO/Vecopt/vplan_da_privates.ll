; This test makes sure that the divergence/uniformity property of privates is
; correctly identified and the replacement of the memory created via
; createPrivateMemory() and corresponding aliases happens in the loop as well
; as in the preheader.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

;const int N=1024;
;int arr1[N];
;int arr2[N];
;int i = 0, j = 0;

;extern int helper(int);
;extern int helperPtr(int*);
;int getElement(int RetIdx) {
;#pragma omp simd private(arr1)
;  for (i = 0; i < N; ++i)
;    for (j = 0; j < N; ++j) {
;      arr1[j] = helper(arr1[j] + arr1[i] + arr2[i] + arr2[j]);
;    }
;  return arr1[RetIdx];
;}

;RUN: opt -S -VPlanDriver -enable-vp-value-codegen=true -disable-vplan-codegen -disable-output -debug-only=vplan-divergence-analysis -vplan-print-after-linearization %s 2>& 1| FileCheck %s

; REQUIRES:asserts

; CHECK: Memory entity = [1024 x i32]* [[ARR_PRIV:%.*]]
; CHECK: Divergent: [Shape: Unit Stride, Stride: i32 1] i32 [[PHI2:%.*]] = phi  [ i32 {{.*}}, {{.*}} ],  [ i32 {{.*}}, {{.*}} ]
; CHECK: Uniform: [Shape: Uniform] i32 [[PHI1:%.*]] = phi  [ i32 0, {{.*}} ],  [ i32 {{.*}}, {{.*}} ]
; CHECK-NEXT: Uniform: [Shape: Uniform] i64 [[SEXT1:%.*]] = sext i32 [[PHI1]] to i64
; CHECK-NEXT: Divergent: [Shape: Random] i32* [[PRIV_GEP1:%.*]] = getelementptr inbounds [1024 x i32]* [[ARR_PRIV]] i64 0 i64 [[SEXT1]]
; CHECK: Divergent: [Shape: Random] i8* [[IV_IDX:%.*]] = bitcast i32* %inv.arrayidx
; CHECK-NEXT: Divergent: [Shape: Random] i8 [[BC1:%.*]] = load i8* %bc.1
; CHECK-NEXT: Divergent: [Shape: Random] i82 [[BC2:%.*]] = load i82* %bc.2
; CHECK: Divergent: [Shape: Random] i32 [[L1:%.*]] = load i32* [[PRIV_GEP1]]
; CHECK: Divergent: [Shape: Random] i32* [[PRIV_GEP2:%.*]] = getelementptr inbounds [1024 x i32]* [[ARR_PRIV]] i64 0 i64 [[SEXT2:%.*]]
; CHECK: Divergent: [Shape: Random] i32 [[VAL_TO_STORE:%.*]] = call i32 {{.*}} i32 (i32)* @helper
; CHECK: Divergent: [Shape: Random] i64 [[L_BC3:%.*]] = load i64* %bc.3
; CHECK-NEXT: Divergent: [Shape: Random] i64 [[L_BC4:%.*]] = load i64* %bc.gep
; CHECK-NEXT: Uniform: [Shape: Uniform] i32 [[JVAL:%.*]] = load i32* @j
; CHECK-NEXT: Uniform: [Shape: Uniform] i64 [[SEXT3:%.*]] = sext i32 [[JVAL]] to i64
; CHECK-NEXT: Divergent: [Shape: Random] i32* [[PRIV_GEP3:%.*]] = getelementptr inbounds [1024 x i32]* [[ARR_PRIV]] i64 0 i64 [[SEXT3]]
; CHECK-NEXT: Divergent: [Shape: Random] store i32 [[VAL_TO_STORE]] i32* [[PRIV_GEP3]]

; CHECK: After predication and linearization
; CHECK:  [DA: Divergent] [1024 x i32]* [[PRIV1:%.*]] = allocate-priv [1024 x i32]*
; CHECK-NEXT:  [DA: Divergent] i32* [[GEP1:%.*]]  = getelementptr inbounds [1024 x i32]* [[PRIV1]] i64 0 i64 0
; CHECK-NEXT:  [DA: Divergent] i8* [[BC1:%.*]] = bitcast i32* [[GEP1]]
; CHECK-NEXT:  [DA: Divergent] i82* [[BC2:%.*]] = bitcast i8* [[BC1]]
; CHECK-NEXT:  [DA: Divergent] i64* [[BC3:%.*]] = bitcast i82* [[BC2]]
; CHECK-NEXT:  [DA: Divergent] i64* [[GEP2:%.*]] = getelementptr inbounds i64* [[BC3]] i64 6
; CHECK-NEXT:  [DA: Divergent] i32* [[PRIV2:%.*]] = allocate-priv i32*
; CHECK-NEXT:  [DA: Divergent] i32 [[IND1:%.*]] = induction-init{add} i32 0 i32 1
; CHECK-NEXT:  [DA: Uniform]   i32 [[IND2:%.*]] = induction-init-step{add} i32 1

; CHECK: [DA: Uniform] i64 [[IDX1:%.*]] = sext i32 {{.*}} to i64
; CHECK-NEXT:  [DA: Divergent] i32* [[GEP3:%.*]] = getelementptr inbounds [1024 x i32]* [[PRIV1]] i64 0 i64 [[IDX1]]
; CHECK-NEXT:  [DA: Divergent] i32* [[BC4:%.*]] = bitcast [1024 x i32]* [[PRIV1]]
; CHECK-NEXT:  [DA: Divergent] i8*  [[BC5:%.*]] = bitcast i32* [[GEP1]]
; CHECK-NEXT:  [DA: Divergent] i8  [[L1:%.*]] = load i8* [[BC1]]
; CHECK-NEXT:  [DA: Divergent] i82 [[L2:%.*]] = load i82* [[BC2]]
; CHECK-NEXT:  [DA: Divergent] i32 [[L3:%.*]] = load i32* [[GEP3]]
; CHECK:       [DA: Divergent] i32* [[GEP4:%.*]] = getelementptr inbounds [1024 x i32]* [[PRIV1]] i64 0 i64 {{.*}}
; CHECK-NEXT:  [DA: Divergent] i32 [[L4:%.*]] = load i32* [[GEP4]]
; CHECK:       [DA: Divergent] i32 [[R1:%.*]] = call i32 {{.*}} i32 (i32)* @helper
; CHECK-NEXT:  [DA: Divergent] i64 [[L5:%.*]] = load i64* [[BC3]]
; CHECK-NEXT:  [DA: Divergent] i64 [[L6:%.*]] = load i64* [[GEP2]]
; CHECK:  [DA: Divergent] i32* [[GEP5:%.*]] = getelementptr inbounds [1024 x i32]* [[PRIV1]] i64 0 i64 {{.*}}
; CHECK-NEXT:  [DA: Divergent] store i32 {{.*}} i32* [[GEP5]]


@N = dso_local local_unnamed_addr constant i32 1024, align 4
@i = dso_local local_unnamed_addr global i32 0, align 4
@j = dso_local local_unnamed_addr global i32 0, align 4
@arr1 = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr2 = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local i32 @getElement(i32 %RetIdx) local_unnamed_addr {
omp.inner.for.body.lr.ph:
  %arr1.priv = alloca [1024 x i32], align 4
  %inv.arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr1.priv, i64 0, i64 0
  %bc.1 = bitcast i32* %inv.arrayidx to i8*
  %bc.2 = bitcast i8* %bc.1 to i82*
  %bc.3 = bitcast i82* %bc.2 to i64*
  %bc.gep = getelementptr inbounds i64, i64* %bc.3, i64 6
  %inv.arrayidx1 = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr1.priv, i64 0, i64 6
  %i.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"([1024 x i32]* %arr1.priv), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LASTPRIVATE"(i32* %i.lpriv) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %DIR.OMP.SIMD.1
  %.omp.iv.local.0 = phi i32 [ 0, %DIR.OMP.SIMD.1 ], [ %add13, %omp.inner.for.inc ]
  store i32 %.omp.iv.local.0, i32* %i.lpriv, align 4
  store i32 0, i32* @j, align 4
  br label %for.body

for.body:                                         ; preds = %for.body.for.body_crit_edge, %omp.inner.for.body
  %1 = phi i32 [ %.omp.iv.local.0, %omp.inner.for.body ], [ %.pre, %for.body.for.body_crit_edge ]
  %storemerge22 = phi i32 [ 0, %omp.inner.for.body ], [ %inc, %for.body.for.body_crit_edge ]
  %idxprom = sext i32 %storemerge22 to i64
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr1.priv, i64 0, i64 %idxprom
  %bc = bitcast [1024 x i32]* %arr1.priv to i32*
  %bc.inv = bitcast i32* %inv.arrayidx to i8*
  %bc1.load = load i8, i8* %bc.1
  %bc2.load = load i82, i82* %bc.2
  %2 = load i32, i32* %arrayidx, align 4
  %idxprom2 = sext i32 %1 to i64
  %arrayidx3 = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr1.priv, i64 0, i64 %idxprom2
  %3 = load i32, i32* %arrayidx3, align 4
  %add4 = add nsw i32 %3, %2
  %arrayidx6 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr2, i64 0, i64 %idxprom2
  %4 = load i32, i32* %arrayidx6, align 4
  %add7 = add nsw i32 %add4, %4
  %arrayidx9 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr2, i64 0, i64 %idxprom
  %5 = load i32, i32* %arrayidx9, align 4
  %add10 = add nsw i32 %add7, %5
  %call = call i32 @helper(i32 %add10)
  %bc3.load = load i64, i64* %bc.3
  %bc4.load = load i64, i64* %bc.gep
  %6 = load i32, i32* @j, align 4
  %idxprom11 = sext i32 %6 to i64
  %arrayidx12 = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr1.priv, i64 0, i64 %idxprom11
  store i32 %call, i32* %arrayidx12, align 4
  %inc = add nsw i32 %6, 1
  store i32 %inc, i32* @j, align 4
  %cmp1 = icmp slt i32 %inc, 1024
  br i1 %cmp1, label %for.body.for.body_crit_edge, label %omp.inner.for.inc

for.body.for.body_crit_edge:                      ; preds = %for.body
  %.pre = load i32, i32* %i.lpriv, align 4
  br label %for.body

omp.inner.for.inc:                                ; preds = %for.body
  %add13 = add nuw nsw i32 %.omp.iv.local.0, 1
  %exitcond = icmp eq i32 %add13, 1024
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.inc
  %7 = load i32, i32* %i.lpriv, align 4
  store i32 %7, i32* @i, align 4
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  %idxprom14 = sext i32 %RetIdx to i64
  %arrayidx15 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr1, i64 0, i64 %idxprom14
  %8 = load i32, i32* %arrayidx15, align 4
  ret i32 %8
}



; This test makes sure that a scalar-private, which is passes onto a helper function is marked as Divergent.
;int scalPrivate(int RetIdx) {
;#pragma omp simd private(j)
;  for (i = 0; i < N; ++i) {
;    j += i;
;    arr1[i] += helper(j) + helperPtr(&j);
;  }
;  return arr1[RetIdx];
;}


; CHECK: Memory entity = i32* [[J:%.*]]
; CHECK-NEXT: Memory entity = i32* %i.lpriv


; CHECK: Divergent: [Shape: Unit Stride, Stride: i32 1] i32 [[PHI1:%.*]] = phi  [ i32 0, {{.*}} ],  [ i32 {{.*}}, {{.*}} ]
; CHECK: Divergent: [Shape: Random] store i32 [[PHI1]] i32* %i.lpriv
; CHECK: Divergent: [Shape: Random] i32 [[J1:%.*]] = load i32* [[J]]
; CHECK: Divergent: [Shape: Random] i32 [[ADD4:%.*]] = add i32 [[J1]] i32 [[PHI1]]
; CHECK: Divergent: [Shape: Random] store i32 [[ADD4]] i32* [[J]]
; CHECK: Divergent: [Shape: Random] i32 [[H1:%.*]] = call i32 [[ADD4]] i32 (i32)* @helper
; CHECK: Divergent: [Shape: Random] i32 [[H2:%.*]] = call i32* [[J]] i32 (i32*)* @helperPtr
; CHECK: Divergent: [Shape: Random] i32 [[ADD2:%.*]] = add i32 [[H2]] i32 [[H1]]

; CHECK: After predication and linearization
; CHECK:      [DA: Divergent] i32* [[PRIV2:%.*]] = allocate-priv i32*
; CHECK-NEXT: [DA: Divergent] i32* [[L_PRIV:%.*]] = allocate-priv i32*


; CHECK:      [DA: Divergent] i32 [[PHI5:%.*]] = phi  [ i32 {{.*}}, {{.*}} ],  [ i32 [[ADD1:%.*]], {{.*}}]
; CHECK:      [DA: Divergent] store i32 [[PHI5]] i32* [[L_PRIV]]
; CHECK:      [DA: Divergent] store i32 {{.*}} i32* [[PRIV2]]
; CHECK-NEXT: [DA: Divergent] i32 {{.*}} = call i32 {{.*}} i32 (i32)* @helper
; CHECK-NEXT: [DA: Divergent] i32 {{.*}} = call i32* [[PRIV2]] i32 (i32*)* @helperPtr
; CHECK:      [DA: Divergent] i32 [[ADD1]] = add i32 [[PHI5]] i32 {{.*}}


; Function Attrs: nounwind uwtable
define dso_local i32 @scalPrivate(i32 %RetIdx) local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %j.priv = alloca i32, align 4
  %i.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"(i32* %j.priv), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LASTPRIVATE"(i32* %i.lpriv) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %.omp.iv.local.0 = phi i32 [ 0, %DIR.OMP.SIMD.1 ], [ %add5, %omp.inner.for.body ]
  store i32 %.omp.iv.local.0, i32* %i.lpriv, align 4
  %1 = load i32, i32* %j.priv, align 4
  %add1 = add nsw i32 %1, %.omp.iv.local.0
  store i32 %add1, i32* %j.priv, align 4
  %call = call i32 @helper(i32 %add1)
  %call2 = call i32 @helperPtr(i32* nonnull %j.priv)
  %add3 = add nsw i32 %call2, %call
  %2 = load i32, i32* %i.lpriv, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr1, i64 0, i64 %idxprom
  %3 = load i32, i32* %arrayidx, align 4
  %add4 = add nsw i32 %add3, %3
  store i32 %add4, i32* %arrayidx, align 4
  %add5 = add nuw nsw i32 %.omp.iv.local.0, 1
  %exitcond = icmp eq i32 %add5, 1024
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  %.lcssa = phi i32 [ %2, %omp.inner.for.body ]
  store i32 %.lcssa, i32* @i, align 4
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  %idxprom6 = sext i32 %RetIdx to i64
  %arrayidx7 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr1, i64 0, i64 %idxprom6
  %4 = load i32, i32* %arrayidx7, align 4
  ret i32 %4
}


; Function Attrs: nounwind
declare token @llvm.directive.region.entry()
; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0)

declare dso_local i32 @helper(i32 %0) local_unnamed_addr
declare dso_local i32 @helperPtr(i32* %0) local_unnamed_addr #2
