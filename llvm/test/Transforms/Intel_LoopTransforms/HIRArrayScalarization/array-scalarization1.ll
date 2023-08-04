; [Note]
; This LIT test is otherwise identical to array-scalarization0.ll, except the array data type used in this one is i64
; while those in array-scalarization0.ll is f64.

; set of symbases using HIRTransformUtil's public API:
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll,hir-arrayscalarization-test-launcher,print<hir>" -hir-create-function-level-region -disable-hir-arrayscalarization-test-launcher=false -hir-arrayscalarization-test-launcher-array-scalarization-symbases=63,73 -disable-output < %s 2>&1  | FileCheck %s
;

; This LIT test is very similar to array-scalarization0.ll, except it has additional coverage.
; This LIT models a testcase where array scalarization opportunities are available after loop unrolling and array contraction.
; E.g.
;   (%AA)[0][1] = .
;   ...
;               =  (%AA)[0][1]
;
; This forms a group of 2 items {(%AA)[0][1], (%AA)[0][1]}
;                                 W            R
; This is a suitable group for array scalarization, thus the respective (%AA)[0][1] are replaced with a temp.
; E.g.
;   array-scalarize = .
;   ...
;               =  array-scalarize
; Note:
; - since leading ref is a store, there is no need to initialize the temp (generate a load) in the prehdr.
; - since the ref is dead after the loop, there is no need to generate a store in the postexit.
; - this is also true for a few other refs.
; The full list includes: (%AA)[0][1], (%AA)[0][2], (%AA)[0][3], (%AA)[0][4],
;                         (%AM)[0][1], (%AM)[0][2], (%AM)[0][3], (%AM)[0][4],
;

;*** IR Dump Before HIR Array Scalarization Test Launcher ***
;
;<0>          BEGIN REGION { modified }
;<73>               + DO i1 = 0, 99, 1   <DO_LOOP>
;<3>                |   %2 = (@x1)[0];
;<5>                |   %3 = (@B1)[0][i1 + 1];
;<7>                |   (%AA)[0][1] = %2 + %3;
;<8>                |   %4 = (@x2)[0];
;<10>               |   %5 = (@B2)[0][i1 + 1];
;<12>               |   (%AA)[0][2] = %4 + %5;
;<13>               |   %6 = (@x3)[0];
;<15>               |   %7 = (@B3)[0][i1 + 1];
;<17>               |   (%AA)[0][3] = %6 + %7;
;<18>               |   %8 = (@x4)[0];
;<20>               |   %9 = (@B4)[0][i1 + 1];
;<22>               |   (%AA)[0][4] = %8 + %9;
;<23>               |   %10 = (@x1)[0];
;<26>               |   %12 = (@B1)[0][i1];
;<28>               |   (%AM1)[0][1] = %10 + %12;
;<29>               |   %13 = (@x2)[0];
;<31>               |   %14 = (@B2)[0][i1];
;<33>               |   (%AM1)[0][2] = %13 + %14;
;<34>               |   %15 = (@x3)[0];
;<36>               |   %16 = (@B3)[0][i1];
;<38>               |   (%AM1)[0][3] = %15 + %16;
;<39>               |   %17 = (@x4)[0];
;<41>               |   %18 = (@B4)[0][i1];
;<43>               |   (%AM1)[0][4] = %17 + %18;
;<76>               |   %19 = (@D)[0][1][i1 + 1];
;<77>               |   %20 = (%AA)[0][1];
;<78>               |   %21 = (%AM1)[0][1];
;<79>               |   (@D)[0][1][i1 + 1] = %19 + %20 + %21;
;<80>               |   %19 = (@D)[0][2][i1 + 1];
;<81>               |   %20 = (%AA)[0][2];
;<82>               |   %21 = (%AM1)[0][2];
;<83>               |   (@D)[0][2][i1 + 1] = %19 + %20 + %21;
;<84>               |   %19 = (@D)[0][3][i1 + 1];
;<85>               |   %20 = (%AA)[0][3];
;<86>               |   %21 = (%AM1)[0][3];
;<87>               |   (@D)[0][3][i1 + 1] = %19 + %20 + %21;
;<48>               |   %19 = (@D)[0][4][i1 + 1];
;<50>               |   %20 = (%AA)[0][4];
;<53>               |   %21 = (%AM1)[0][4];
;<55>               |   (@D)[0][4][i1 + 1] = %19 + %20 + %21;
;<73>               + END LOOP
;
;<70>               @llvm.lifetime.end.p0(400,  &((i8*)(%AM1)[0]));
;<71>               @llvm.lifetime.end.p0(400,  &((i8*)(%AA)[0]));
;<72>               ret ;
;<0>          END REGION


;*** IR Dump After HIR Array Scalarization Test Launcher ***

;CHECK:      BEGIN REGION { modified }
;CHECK:            + DO i1 = 0, 99, 1   <DO_LOOP>
;CHECK:            |   %0 = (@x1)[0];
;CHECK:            |   %1 = (@B1)[0][i1 + 1];
;CHECK:            |   %[[ARRSC:array-scalarize[0-9]*]] = %0 + %1;
;CHECK:            |   %2 = (@x2)[0];
;CHECK:            |   %3 = (@B2)[0][i1 + 1];
;CHECK:            |   %[[ARRSC2:array-scalarize[0-9]*]] = %2 + %3;
;CHECK:            |   %4 = (@x3)[0];
;CHECK:            |   %5 = (@B3)[0][i1 + 1];
;CHECK:            |   %[[ARRSC3:array-scalarize[0-9]*]] = %4 + %5;
;CHECK:            |   %6 = (@x4)[0];
;CHECK:            |   %7 = (@B4)[0][i1 + 1];
;CHECK:            |   %[[ARRSC4:array-scalarize[0-9]*]] = %6 + %7;
;CHECK:            |   %8 = (@x1)[0];
;CHECK:            |   %10 = (@B1)[0][i1];
;CHECK:            |   %[[ARRSC5:array-scalarize[0-9]*]] = %8 + %10;
;CHECK:            |   %11 = (@x2)[0];
;CHECK:            |   %12 = (@B2)[0][i1];
;CHECK:            |   %[[ARRSC6:array-scalarize[0-9]*]] = %11 + %12;
;CHECK:            |   %13 = (@x3)[0];
;CHECK:            |   %14 = (@B3)[0][i1];
;CHECK:            |   %[[ARRSC7:array-scalarize[0-9]*]] = %13 + %14;
;CHECK:            |   %15 = (@x4)[0];
;CHECK:            |   %16 = (@B4)[0][i1];
;CHECK:            |   %[[ARRSC8:array-scalarize[0-9]*]] = %15 + %16;
;CHECK:            |   %17 = (@D)[0][1][i1 + 1];
;CHECK:            |   %18 = %[[ARRSC]];
;CHECK:            |   %19 = %[[ARRSC5]];
;CHECK:            |   (@D)[0][1][i1 + 1] = %17 + %18 + %19;
;CHECK:            |   %17 = (@D)[0][2][i1 + 1];
;CHECK:            |   %18 = %[[ARRSC2]];
;CHECK:            |   %19 = %[[ARRSC6]];
;CHECK:            |   (@D)[0][2][i1 + 1] = %17 + %18 + %19;
;CHECK:            |   %17 = (@D)[0][3][i1 + 1];
;CHECK:            |   %18 = %[[ARRSC3]];
;CHECK:            |   %19 = %[[ARRSC7]];
;CHECK:            |   (@D)[0][3][i1 + 1] = %17 + %18 + %19;
;CHECK:            |   %17 = (@D)[0][4][i1 + 1];
;CHECK:            |   %18 = %[[ARRSC4]];
;CHECK:            |   %19 = %[[ARRSC8]];
;CHECK:            |   (@D)[0][4][i1 + 1] = %17 + %18 + %19;
;CHECK:            + END LOOP
;
;CHECK:            @llvm.lifetime.end.p0(400,  &((%AM1)[0]));
;CHECK:            @llvm.lifetime.end.p0(400,  &((%AA)[0]));
;CHECK:            ret ;
;CHECK:      END REGION


;Module Before HIR
; ModuleID = 'test-arrayscalarization1.c'
source_filename = "test-arrayscalarization1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x1 = dso_local global i32 0, align 4
@B1 = dso_local global [100 x i32] zeroinitializer, align 16
@x2 = dso_local global i32 0, align 4
@B2 = dso_local global [100 x i32] zeroinitializer, align 16
@x3 = dso_local global i32 0, align 4
@B3 = dso_local global [100 x i32] zeroinitializer, align 16
@x4 = dso_local global i32 0, align 4
@B4 = dso_local global [100 x i32] zeroinitializer, align 16
@D = dso_local global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  %AA = alloca [100 x i32], align 16
  %AM1 = alloca [100 x i32], align 16
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %AA) #2
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %AM1) #2
  %arrayidx1 = getelementptr inbounds [100 x i32], ptr %AA, i64 0, i64 1, !intel-tbaa !2
  %arrayidx5 = getelementptr inbounds [100 x i32], ptr %AA, i64 0, i64 2, !intel-tbaa !2
  %arrayidx9 = getelementptr inbounds [100 x i32], ptr %AA, i64 0, i64 3, !intel-tbaa !2
  %arrayidx13 = getelementptr inbounds [100 x i32], ptr %AA, i64 0, i64 4, !intel-tbaa !2
  %arrayidx17 = getelementptr inbounds [100 x i32], ptr %AM1, i64 0, i64 1, !intel-tbaa !2
  %arrayidx22 = getelementptr inbounds [100 x i32], ptr %AM1, i64 0, i64 2, !intel-tbaa !2
  %arrayidx27 = getelementptr inbounds [100 x i32], ptr %AM1, i64 0, i64 3, !intel-tbaa !2
  %arrayidx32 = getelementptr inbounds [100 x i32], ptr %AM1, i64 0, i64 4, !intel-tbaa !2
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup35
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %AM1) #2
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %AA) #2
  ret void

for.body:                                         ; preds = %entry, %for.cond.cleanup35
  %indvars.iv74 = phi i64 [ 1, %entry ], [ %indvars.iv.next75, %for.cond.cleanup35 ]
  %0 = load  i32, ptr @x1, align 4, !tbaa !7
  %arrayidx = getelementptr inbounds [100 x i32], ptr @B1, i64 0, i64 %indvars.iv74, !intel-tbaa !2
  %1 = load  i32, ptr %arrayidx, align 4, !tbaa !2
  %add = add nsw i32 %1, %0
  store  i32 %add, ptr %arrayidx1, align 4, !tbaa !2
  %2 = load  i32, ptr @x2, align 4, !tbaa !7
  %arrayidx3 = getelementptr inbounds [100 x i32], ptr @B2, i64 0, i64 %indvars.iv74, !intel-tbaa !2
  %3 = load  i32, ptr %arrayidx3, align 4, !tbaa !2
  %add4 = add nsw i32 %3, %2
  store  i32 %add4, ptr %arrayidx5, align 8, !tbaa !2
  %4 = load  i32, ptr @x3, align 4, !tbaa !7
  %arrayidx7 = getelementptr inbounds [100 x i32], ptr @B3, i64 0, i64 %indvars.iv74, !intel-tbaa !2
  %5 = load  i32, ptr %arrayidx7, align 4, !tbaa !2
  %add8 = add nsw i32 %5, %4
  store  i32 %add8, ptr %arrayidx9, align 4, !tbaa !2
  %6 = load  i32, ptr @x4, align 4, !tbaa !7
  %arrayidx11 = getelementptr inbounds [100 x i32], ptr @B4, i64 0, i64 %indvars.iv74, !intel-tbaa !2
  %7 = load  i32, ptr %arrayidx11, align 4, !tbaa !2
  %add12 = add nsw i32 %7, %6
  store  i32 %add12, ptr %arrayidx13, align 16, !tbaa !2
  %8 = load  i32, ptr @x1, align 4, !tbaa !7
  %9 = add nsw i64 %indvars.iv74, -1
  %arrayidx15 = getelementptr inbounds [100 x i32], ptr @B1, i64 0, i64 %9, !intel-tbaa !2
  %10 = load  i32, ptr %arrayidx15, align 4, !tbaa !2
  %add16 = add nsw i32 %10, %8
  store  i32 %add16, ptr %arrayidx17, align 4, !tbaa !2
  %11 = load  i32, ptr @x2, align 4, !tbaa !7
  %arrayidx20 = getelementptr inbounds [100 x i32], ptr @B2, i64 0, i64 %9, !intel-tbaa !2
  %12 = load  i32, ptr %arrayidx20, align 4, !tbaa !2
  %add21 = add nsw i32 %12, %11
  store  i32 %add21, ptr %arrayidx22, align 8, !tbaa !2
  %13 = load  i32, ptr @x3, align 4, !tbaa !7
  %arrayidx25 = getelementptr inbounds [100 x i32], ptr @B3, i64 0, i64 %9, !intel-tbaa !2
  %14 = load  i32, ptr %arrayidx25, align 4, !tbaa !2
  %add26 = add nsw i32 %14, %13
  store  i32 %add26, ptr %arrayidx27, align 4, !tbaa !2
  %15 = load  i32, ptr @x4, align 4, !tbaa !7
  %arrayidx30 = getelementptr inbounds [100 x i32], ptr @B4, i64 0, i64 %9, !intel-tbaa !2
  %16 = load  i32, ptr %arrayidx30, align 4, !tbaa !2
  %add31 = add nsw i32 %16, %15
  store  i32 %add31, ptr %arrayidx32, align 16, !tbaa !2
  br label %for.body36

for.cond.cleanup35:                               ; preds = %for.body36
  %indvars.iv.next75 = add nuw nsw i64 %indvars.iv74, 1
  %exitcond77 = icmp eq i64 %indvars.iv.next75, 101
  br i1 %exitcond77, label %for.cond.cleanup, label %for.body

for.body36:                                       ; preds = %for.body, %for.body36
  %indvars.iv = phi i64 [ 1, %for.body ], [ %indvars.iv.next, %for.body36 ]
  %arrayidx40 = getelementptr inbounds [100 x [100 x i32]], ptr @D, i64 0, i64 %indvars.iv, i64 %indvars.iv74, !intel-tbaa !8
  %17 = load  i32, ptr %arrayidx40, align 4, !tbaa !8
  %arrayidx42 = getelementptr inbounds [100 x i32], ptr %AA, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %18 = load  i32, ptr %arrayidx42, align 4, !tbaa !2
  %add43 = add nsw i32 %18, %17
  %arrayidx45 = getelementptr inbounds [100 x i32], ptr %AM1, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %19 = load  i32, ptr %arrayidx45, align 4, !tbaa !2
  %add46 = add nsw i32 %add43, %19
  store  i32 %add46, ptr %arrayidx40, align 4, !tbaa !8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.cond.cleanup35, label %for.body36
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler Pro 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!4, !4, i64 0}
!8 = !{!9, !4, i64 0}
!9 = !{!"array@_ZTSA100_A100_i", !3, i64 0}
