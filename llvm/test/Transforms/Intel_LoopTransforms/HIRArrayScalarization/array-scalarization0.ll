; [Note]
; This LIT test is otherwise identical to array-scalarization1.ll, except the array data type used in this one is f64
; while those in array-scalarization1.ll is i64.

; vector of memref using HIRLMM's API:
; RUN: opt -hir-create-function-level-region -hir-ssa-deconstruction -hir-pre-vec-complete-unroll -hir-arrayscalarization-test-launcher -disable-hir-arrayscalarization-test-launcher=false -run-hir-arrayscalarization-test-launcher-array-scalarization-memrefs -print-before=hir-arrayscalarization-test-launcher -print-after=hir-arrayscalarization-test-launcher -S < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll,print<hir>,hir-arrayscalarization-test-launcher,print<hir>" -hir-create-function-level-region -disable-hir-arrayscalarization-test-launcher=false -run-hir-arrayscalarization-test-launcher-array-scalarization-memrefs -S < %s 2>&1  | FileCheck %s
;
; set of symbase using HIRTransformUtil's public API:
; RUN: opt -hir-create-function-level-region -hir-ssa-deconstruction -hir-pre-vec-complete-unroll -hir-arrayscalarization-test-launcher -disable-hir-arrayscalarization-test-launcher=false -run-hir-arrayscalarization-test-launcher-array-scalarization-symbases -hir-arrayscalarization-test-launcher-array-scalarization-symbases=42,43,44,45,46,47,48,49,50,59,60,61,62,64,65,67,68,70,74 -print-before=hir-arrayscalarization-test-launcher -print-after=hir-arrayscalarization-test-launcher -S < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll,print<hir>,hir-arrayscalarization-test-launcher,print<hir>" -hir-create-function-level-region -disable-hir-arrayscalarization-test-launcher=false -run-hir-arrayscalarization-test-launcher-array-scalarization-symbases -hir-arrayscalarization-test-launcher-array-scalarization-symbases=42,43,44,45,46,47,48,49,50,59,60,61,62,64,65,67,68,70,74 -S < %s 2>&1  | FileCheck %s
;
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
; The full list includes: (%AA)[0][1], (%AA)[0][2], (%AA)[0][3], (%AA)[0][4].
;

;*** IR Dump Before HIR Array Scalarization Test Launcher ***
;
;       BEGIN REGION { modified }
;             + DO i1 = 0, 99, 1   <DO_LOOP>
;             |   %1 = (@x1)[0];
;             |   %2 = (@B1)[0][i1 + 1];
;             |   (%AA)[0][1] = %1 + %2;
;             |   %3 = (@x2)[0];
;             |   %4 = (@B2)[0][i1 + 1];
;             |   (%AA)[0][2] = %3 + %4;
;             |   %5 = (@x3)[0];
;             |   %6 = (@B3)[0][i1 + 1];
;             |   (%AA)[0][3] = %5 + %6;
;             |   %7 = (@x4)[0];
;             |   %8 = (@B4)[0][i1 + 1];
;             |   (%AA)[0][4] = %7 + %8;
;             |   %9 = (@D)[0][1][i1 + 1];
;             |   %10 = (%AA)[0][1];
;             |   (@D)[0][1][i1 + 1] = %9 + %10;
;             |   %9 = (@D)[0][2][i1 + 1];
;             |   %10 = (%AA)[0][2];
;             |   (@D)[0][2][i1 + 1] = %9 + %10;
;             |   %9 = (@D)[0][3][i1 + 1];
;             |   %10 = (%AA)[0][3];
;             |   (@D)[0][3][i1 + 1] = %9 + %10;
;             |   %9 = (@D)[0][4][i1 + 1];
;             |   %10 = (%AA)[0][4];
;             |   (@D)[0][4][i1 + 1] = %9 + %10;
;             + END LOOP
;
;             @llvm.lifetime.end.p0i8(400,  &((i8*)(%AA)[0]));
;             ret ;
;       END REGION

;
; *** IR Dump After HIR Array Scalarization Test Launcher ***

;CHECK:      BEGIN REGION { modified }
;CHECK:            + DO i1 = 0, 99, 1   <DO_LOOP>
;CHECK:            |   %1 = (@x1)[0];
;CHECK:            |   %2 = (@B1)[0][i1 + 1];
;CHECK:            |   %[[ARRSC:array-scalarize[0-9]*]] = %1 + %2;
;CHECK:            |   %3 = (@x2)[0];
;CHECK:            |   %4 = (@B2)[0][i1 + 1];
;CHECK:            |   %[[ARRSC2:array-scalarize[0-9]*]] = %3 + %4;
;CHECK:            |   %5 = (@x3)[0];
;CHECK:            |   %6 = (@B3)[0][i1 + 1];
;CHECK:            |   %[[ARRSC3:array-scalarize[0-9]*]] = %5 + %6;
;CHECK:            |   %7 = (@x4)[0];
;CHECK:            |   %8 = (@B4)[0][i1 + 1];
;CHECK:            |   %[[ARRSC4:array-scalarize[0-9]*]] = %7 + %8;
;CHECK:            |   %9 = (@D)[0][1][i1 + 1];
;CHECK:            |   %10 = %[[ARRSC]];
;CHECK:            |   (@D)[0][1][i1 + 1] = %9 + %10;
;CHECK:            |   %9 = (@D)[0][2][i1 + 1];
;CHECK:            |   %10 = %[[ARRSC2]];
;CHECK:            |   (@D)[0][2][i1 + 1] = %9 + %10;
;CHECK:            |   %9 = (@D)[0][3][i1 + 1];
;CHECK:            |   %10 = %[[ARRSC3]];
;CHECK:            |   (@D)[0][3][i1 + 1] = %9 + %10;
;CHECK:            |   %9 = (@D)[0][4][i1 + 1];
;CHECK:            |   %10 = %[[ARRSC4]];
;CHECK:            |   (@D)[0][4][i1 + 1] = %9 + %10;
;CHECK:            + END LOOP
;
;CHECK:            @llvm.lifetime.end.p0i8(400,  &((i8*)(%AA)[0]));
;CHECK:            ret ;
;CHECK:      END REGION


;Module Before HIR
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
  %0 = bitcast [100 x i32]* %AA to i8*
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %0) #2
  %arrayidx1 = getelementptr inbounds [100 x i32], [100 x i32]* %AA, i64 0, i64 1, !intel-tbaa !2
  %arrayidx5 = getelementptr inbounds [100 x i32], [100 x i32]* %AA, i64 0, i64 2, !intel-tbaa !2
  %arrayidx9 = getelementptr inbounds [100 x i32], [100 x i32]* %AA, i64 0, i64 3, !intel-tbaa !2
  %arrayidx13 = getelementptr inbounds [100 x i32], [100 x i32]* %AA, i64 0, i64 4, !intel-tbaa !2
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup16
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %0) #2
  ret void

for.body:                                         ; preds = %entry, %for.cond.cleanup16
  %indvars.iv47 = phi i64 [ 1, %entry ], [ %indvars.iv.next48, %for.cond.cleanup16 ]
  %1 = load i32, i32* @x1, align 4, !tbaa !7
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @B1, i64 0, i64 %indvars.iv47, !intel-tbaa !2
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %add = add nsw i32 %2, %1
  store i32 %add, i32* %arrayidx1, align 4, !tbaa !2
  %3 = load i32, i32* @x2, align 4, !tbaa !7
  %arrayidx3 = getelementptr inbounds [100 x i32], [100 x i32]* @B2, i64 0, i64 %indvars.iv47, !intel-tbaa !2
  %4 = load i32, i32* %arrayidx3, align 4, !tbaa !2
  %add4 = add nsw i32 %4, %3
  store i32 %add4, i32* %arrayidx5, align 8, !tbaa !2
  %5 = load i32, i32* @x3, align 4, !tbaa !7
  %arrayidx7 = getelementptr inbounds [100 x i32], [100 x i32]* @B3, i64 0, i64 %indvars.iv47, !intel-tbaa !2
  %6 = load i32, i32* %arrayidx7, align 4, !tbaa !2
  %add8 = add nsw i32 %6, %5
  store i32 %add8, i32* %arrayidx9, align 4, !tbaa !2
  %7 = load i32, i32* @x4, align 4, !tbaa !7
  %arrayidx11 = getelementptr inbounds [100 x i32], [100 x i32]* @B4, i64 0, i64 %indvars.iv47, !intel-tbaa !2
  %8 = load i32, i32* %arrayidx11, align 4, !tbaa !2
  %add12 = add nsw i32 %8, %7
  store i32 %add12, i32* %arrayidx13, align 16, !tbaa !2
  br label %for.body17

for.cond.cleanup16:                               ; preds = %for.body17
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %exitcond49 = icmp eq i64 %indvars.iv.next48, 101
  br i1 %exitcond49, label %for.cond.cleanup, label %for.body

for.body17:                                       ; preds = %for.body, %for.body17
  %indvars.iv = phi i64 [ 1, %for.body ], [ %indvars.iv.next, %for.body17 ]
  %arrayidx21 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @D, i64 0, i64 %indvars.iv, i64 %indvars.iv47, !intel-tbaa !8
  %9 = load i32, i32* %arrayidx21, align 4, !tbaa !8
  %arrayidx23 = getelementptr inbounds [100 x i32], [100 x i32]* %AA, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %10 = load i32, i32* %arrayidx23, align 4, !tbaa !2
  %add24 = add nsw i32 %10, %9
  store i32 %add24, i32* %arrayidx21, align 4, !tbaa !8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.cond.cleanup16, label %for.body17
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

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
