; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-sinking-for-perfect-loopnest -hir-pragma-loop-blocking -print-after=hir-pragma-loop-blocking -disable-output < %s 2>&1 | FileCheck %s

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-pragma-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -disable-output 2>&1 < %s | FileCheck %s

; Check that blocking for integer sizes occurs correctly for pragma loop_block directives

; Source code

; #define SZ 8192
; void foo1() {
;   int i,j,k;
;   #pragma block_loop factor (16) level (1)
;   #pragma block_loop factor (64) level (3)
;   for(i=0; i<SZ; i++)
;   #pragma block_loop factor (32) level (1)
;     for(j=0; j<SZ; j++)
;       for(k=0; k<SZ; k++)
;         C[i][j] = C[i][j] + A[i][k] * B[k][j];
; }

; void foo2() {
;   int i,j,k;
;   for(i=0; i<SZ; i++)
;     for(j=0; j<SZ; j++)
;     #pragma block_loop factor (64)
;       for(k=0; k<SZ; k++)
;         C[i][j] = C[i][j] + A[i][k] * B[k][j];
; }

; void foo3() {
;   int i,j,k;
;   #pragma block_loop factor (16)
;   for(i=0; i<SZ; i++)
;     for(j=0; j<SZ; j++)
;       for(k=0; k<SZ; k++)
;         C[i][j] = C[i][j] + A[i][k] * B[k][j];
; }

; Before Transformation:
; BEGIN REGION { }
;      + DO i1 = 0, 8191, 1   <DO_LOOP>
;      |   + DO i2 = 0, 8191, 1   <DO_LOOP>
;      |   |   + DO i3 = 0, 8191, 1   <DO_LOOP>
;      |   |   |   %1 = (@C)[0][i1][i2];
;      |   |   |   %2 = (@A)[0][i1][i3];
;      |   |   |   %3 = (@B)[0][i3][i2];
;      |   |   |   %1 = (%2 * %3)  +  %1;
;      |   |   |   (@C)[0][i1][i2] = %1;
;      |   |   + END LOOP
;      |   + END LOOP
;      + END LOOP
; END REGION

; Check that Permutation is [L1-BS,L3-BS,L1,L2-BS,L2,L3] (BS = By-strip Loop)
; CHECK: Function: foo1
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 511, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 127, 1   <DO_LOOP>
; CHECK:        |   |   + DO i3 = 0, 15, 1   <DO_LOOP>
; CHECK:        |   |   |   + DO i4 = 0, 255, 1   <DO_LOOP>
; CHECK:        |   |   |   |   + DO i5 = 0, 31, 1   <DO_LOOP>
; CHECK:        |   |   |   |   |   + DO i6 = 0, 63, 1   <DO_LOOP>
; CHECK-DAG:    |   |   |   |   |   |   %2 = (@C)[0][16 * i1 + i3][32 * i4 + i5];
; CHECK-DAG:    |   |   |   |   |   |   %3 = (@A)[0][16 * i1 + i3][64 * i2 + i6];
; CHECK-DAG:    |   |   |   |   |   |   %4 = (@B)[0][64 * i2 + i6][32 * i4 + i5];
; CHECK:        |   |   |   |   |   |   %2 = (%3 * %4)  +  %2;
; CHECK:        |   |   |   |   |   |   (@C)[0][16 * i1 + i3][32 * i4 + i5] = %2;
; CHECK:        |   |   |   |   |   + END LOOP
; CHECK:        |   |   |   |   + END LOOP
; CHECK:        |   |   |   + END LOOP
; CHECK:        |   |   + END LOOP
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION

; Check that we force blocking for inner loop
; CHECK:  Function: foo2
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 8191, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 8191, 1   <DO_LOOP>
; CHECK:        |   |   + DO i3 = 0, 127, 1   <DO_LOOP>
; CHECK:        |   |   |   + DO i4 = 0, 63, 1   <DO_LOOP>
; CHECK-DAG:    |   |   |   |   %1 = (@C)[0][i1][i2];
; CHECK-DAG:    |   |   |   |   %2 = (@A)[0][i1][64 * i3 + i4];
; CHECK-DAG:    |   |   |   |   %3 = (@B)[0][64 * i3 + i4][i2];
; CHECK:        |   |   |   |   %1 = (%2 * %3)  +  %1;
; CHECK:        |   |   |   |   (@C)[0][i1][i2] = %1;
; CHECK:        |   |   |   + END LOOP
; CHECK:        |   |   + END LOOP
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION

; Check that all loop levels under outer loop are blocked by 16
; CHECK:  Function: foo3
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 511, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 511, 1   <DO_LOOP>
; CHECK:        |   |   + DO i3 = 0, 511, 1   <DO_LOOP>
; CHECK:        |   |   |   + DO i4 = 0, 15, 1   <DO_LOOP>
; CHECK:        |   |   |   |   + DO i5 = 0, 15, 1   <DO_LOOP>
; CHECK:        |   |   |   |   |   + DO i6 = 0, 15, 1   <DO_LOOP>
; CHECK-DAG:    |   |   |   |   |   |   %1 = (@C)[0][16 * i1 + i4][16 * i2 + i5];
; CHECK-DAG:    |   |   |   |   |   |   %2 = (@A)[0][16 * i1 + i4][16 * i3 + i6];
; CHECK-DAG:    |   |   |   |   |   |   %3 = (@B)[0][16 * i3 + i6][16 * i2 + i5];
; CHECK:        |   |   |   |   |   |   %1 = (%2 * %3)  +  %1;
; CHECK:        |   |   |   |   |   |   (@C)[0][16 * i1 + i4][16 * i2 + i5] = %1;
; CHECK:        |   |   |   |   |   + END LOOP
; CHECK:        |   |   |   |   + END LOOP
; CHECK:        |   |   |   + END LOOP
; CHECK:        |   |   + END LOOP
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
; CHECK:  END REGION


;Module Before HIR
; ModuleID = 'm.c'
source_filename = "m.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@C = dso_local local_unnamed_addr global [8192 x [8192 x i32]] zeroinitializer, align 16
@A = dso_local local_unnamed_addr global [8192 x [8192 x i32]] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [8192 x [8192 x i32]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo1() local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(), "QUAL.PRAGMA.LEVEL"(i32 1), "QUAL.PRAGMA.FACTOR"(i32 16), "QUAL.PRAGMA.LEVEL"(i32 3), "QUAL.PRAGMA.FACTOR"(i32 64) ]
  br label %for.body

for.body:                                         ; preds = %for.end23, %entry
  %indvars.iv47 = phi i64 [ 0, %entry ], [ %indvars.iv.next48, %for.end23 ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(), "QUAL.PRAGMA.LEVEL"(i32 1), "QUAL.PRAGMA.FACTOR"(i32 32) ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc21, %for.body
  %indvars.iv44 = phi i64 [ 0, %for.body ], [ %indvars.iv.next45, %for.inc21 ]
  %arrayidx8 = getelementptr inbounds [8192 x [8192 x i32]], [8192 x [8192 x i32]]* @C, i64 0, i64 %indvars.iv47, i64 %indvars.iv44, !intel-tbaa !2
  %arrayidx8.promoted = load i32, i32* %arrayidx8, align 4, !tbaa !2
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %2 = phi i32 [ %arrayidx8.promoted, %for.cond4.preheader ], [ %add, %for.body6 ]
  %arrayidx12 = getelementptr inbounds [8192 x [8192 x i32]], [8192 x [8192 x i32]]* @A, i64 0, i64 %indvars.iv47, i64 %indvars.iv, !intel-tbaa !2
  %3 = load i32, i32* %arrayidx12, align 4, !tbaa !2
  %arrayidx16 = getelementptr inbounds [8192 x [8192 x i32]], [8192 x [8192 x i32]]* @B, i64 0, i64 %indvars.iv, i64 %indvars.iv44, !intel-tbaa !2
  %4 = load i32, i32* %arrayidx16, align 4, !tbaa !2
  %mul = mul nsw i32 %4, %3
  %add = add nsw i32 %mul, %2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 8192
  br i1 %exitcond, label %for.inc21, label %for.body6

for.inc21:                                        ; preds = %for.body6
  %add.lcssa = phi i32 [ %add, %for.body6 ]
  store i32 %add.lcssa, i32* %arrayidx8, align 4, !tbaa !2
  %indvars.iv.next45 = add nuw nsw i64 %indvars.iv44, 1
  %exitcond46 = icmp eq i64 %indvars.iv.next45, 8192
  br i1 %exitcond46, label %for.end23, label %for.cond4.preheader

for.end23:                                        ; preds = %for.inc21
  call void @llvm.directive.region.exit(token %1) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %exitcond49 = icmp eq i64 %indvars.iv.next48, 8192
  br i1 %exitcond49, label %for.end26, label %for.body

for.end26:                                        ; preds = %for.end23
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind uwtable
define dso_local void @foo2() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc24, %entry
  %indvars.iv47 = phi i64 [ 0, %entry ], [ %indvars.iv.next48, %for.inc24 ]
  br label %for.body3

for.body3:                                        ; preds = %for.end, %for.cond1.preheader
  %indvars.iv44 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next45, %for.end ]
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(), "QUAL.PRAGMA.LEVEL"(i32 -1), "QUAL.PRAGMA.FACTOR"(i32 64) ]
  %arrayidx8 = getelementptr inbounds [8192 x [8192 x i32]], [8192 x [8192 x i32]]* @C, i64 0, i64 %indvars.iv47, i64 %indvars.iv44, !intel-tbaa !2
  %arrayidx8.promoted = load i32, i32* %arrayidx8, align 4, !tbaa !2
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.body3
  %indvars.iv = phi i64 [ 0, %for.body3 ], [ %indvars.iv.next, %for.body6 ]
  %1 = phi i32 [ %arrayidx8.promoted, %for.body3 ], [ %add, %for.body6 ]
  %arrayidx12 = getelementptr inbounds [8192 x [8192 x i32]], [8192 x [8192 x i32]]* @A, i64 0, i64 %indvars.iv47, i64 %indvars.iv, !intel-tbaa !2
  %2 = load i32, i32* %arrayidx12, align 4, !tbaa !2
  %arrayidx16 = getelementptr inbounds [8192 x [8192 x i32]], [8192 x [8192 x i32]]* @B, i64 0, i64 %indvars.iv, i64 %indvars.iv44, !intel-tbaa !2
  %3 = load i32, i32* %arrayidx16, align 4, !tbaa !2
  %mul = mul nsw i32 %3, %2
  %add = add nsw i32 %mul, %1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 8192
  br i1 %exitcond, label %for.end, label %for.body6

for.end:                                          ; preds = %for.body6
  %add.lcssa = phi i32 [ %add, %for.body6 ]
  store i32 %add.lcssa, i32* %arrayidx8, align 4, !tbaa !2
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  %indvars.iv.next45 = add nuw nsw i64 %indvars.iv44, 1
  %exitcond46 = icmp eq i64 %indvars.iv.next45, 8192
  br i1 %exitcond46, label %for.inc24, label %for.body3

for.inc24:                                        ; preds = %for.end
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %exitcond49 = icmp eq i64 %indvars.iv.next48, 8192
  br i1 %exitcond49, label %for.end26, label %for.cond1.preheader

for.end26:                                        ; preds = %for.inc24
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @foo3() local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(), "QUAL.PRAGMA.LEVEL"(i32 -1), "QUAL.PRAGMA.FACTOR"(i32 16) ]
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc24, %entry
  %indvars.iv47 = phi i64 [ 0, %entry ], [ %indvars.iv.next48, %for.inc24 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc21, %for.cond1.preheader
  %indvars.iv44 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next45, %for.inc21 ]
  %arrayidx8 = getelementptr inbounds [8192 x [8192 x i32]], [8192 x [8192 x i32]]* @C, i64 0, i64 %indvars.iv47, i64 %indvars.iv44, !intel-tbaa !2
  %arrayidx8.promoted = load i32, i32* %arrayidx8, align 4, !tbaa !2
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %1 = phi i32 [ %arrayidx8.promoted, %for.cond4.preheader ], [ %add, %for.body6 ]
  %arrayidx12 = getelementptr inbounds [8192 x [8192 x i32]], [8192 x [8192 x i32]]* @A, i64 0, i64 %indvars.iv47, i64 %indvars.iv, !intel-tbaa !2
  %2 = load i32, i32* %arrayidx12, align 4, !tbaa !2
  %arrayidx16 = getelementptr inbounds [8192 x [8192 x i32]], [8192 x [8192 x i32]]* @B, i64 0, i64 %indvars.iv, i64 %indvars.iv44, !intel-tbaa !2
  %3 = load i32, i32* %arrayidx16, align 4, !tbaa !2
  %mul = mul nsw i32 %3, %2
  %add = add nsw i32 %mul, %1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 8192
  br i1 %exitcond, label %for.inc21, label %for.body6

for.inc21:                                        ; preds = %for.body6
  %add.lcssa = phi i32 [ %add, %for.body6 ]
  store i32 %add.lcssa, i32* %arrayidx8, align 4, !tbaa !2
  %indvars.iv.next45 = add nuw nsw i64 %indvars.iv44, 1
  %exitcond46 = icmp eq i64 %indvars.iv.next45, 8192
  br i1 %exitcond46, label %for.inc24, label %for.cond4.preheader

for.inc24:                                        ; preds = %for.inc21
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %exitcond49 = icmp eq i64 %indvars.iv.next48, 8192
  br i1 %exitcond49, label %for.end26, label %for.cond1.preheader

for.end26:                                        ; preds = %for.inc24
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  ret void
}

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA8192_A8192_i", !4, i64 0}
!4 = !{!"array@_ZTSA8192_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
