; Test that nontemporal streams get excluded from prefetches. This is based on
; non-constant-trip-count, with one of the stores being marked !nontemporal.
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-prefetching -hir-prefetching-num-cachelines-threshold=64 -hir-prefetching-skip-non-modified-regions=false -hir-prefetching-skip-num-memory-streams-check=true -hir-prefetching-skip-AVX2-check=true -print-after=hir-prefetching < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,print<hir>" -hir-prefetching-num-cachelines-threshold=64 -hir-prefetching-skip-non-modified-regions="false" -hir-prefetching-skip-num-memory-streams-check="true" -hir-prefetching-skip-AVX2-check="true" 2>&1 < %s | FileCheck %s
;
; Source code
;int A[50000];
;int B[50000];
;int t = 10000;
;void foo(int t){
;  int i;
;  for(i = 0; i < t; i++){
;    A[i] = A[i+1] + A[2 * i+2] + A[i+10000] +  A[i+40000] + A[i+8000]  + A[i+24] + A[20];
;    B[i] = B[i+1] + B[i+2] + B[2 * i+4] + B[i+8] + B[i+12] + B[i+16];
;  }
;}
;
; *** IR Dump Before HIR Prefetching ***
;
;<0>          BEGIN REGION { }
;<59>               + DO i1 = 0, sext.i32.i64(%t) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10000>
;<4>                |   %0 = (@A)[0][i1 + 1];
;<8>                |   %3 = (@A)[0][2 * i1 + 2];
;<12>               |   %5 = (@A)[0][i1 + 10000];
;<16>               |   %7 = (@A)[0][i1 + 40000];
;<20>               |   %9 = (@A)[0][i1 + 8000];
;<24>               |   %11 = (@A)[0][i1 + 24];
;<26>               |   %12 = (@A)[0][20];
;<29>               |   (@A)[0][i1] = %0 + %3 + %5 + %7 + %9 + %11 + %12;
;<31>               |   %13 = (@B)[0][i1 + 1];
;<34>               |   %15 = (@B)[0][i1 + 2];
;<38>               |   %17 = (@B)[0][2 * i1 + 4];
;<42>               |   %19 = (@B)[0][i1 + 8];
;<46>               |   %21 = (@B)[0][i1 + 12];
;<50>               |   %23 = (@B)[0][i1 + 16];
;<53>               |   (@B)[0][i1] = %13 + %15 + %17 + %19 + %21 + %23;
;<59>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Prefetching ***
;
; CHECK:    BEGIN REGION { }
; CHECK:           + DO i1 = 0, sext.i32.i64(%t) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10000>
; CHECK:           |   %0 = (@A)[0][i1 + 1];
; CHECK:           |   %3 = (@A)[0][2 * i1 + 2];
; CHECK:           |   %5 = (@A)[0][i1 + 10000];
; CHECK:           |   %7 = (@A)[0][i1 + 40000];
; CHECK:           |   %9 = (@A)[0][i1 + 8000];
; CHECK:           |   %11 = (@A)[0][i1 + 24];
; CHECK:           |   %12 = (@A)[0][20];
; CHECK:           |   (@A)[0][i1] = %0 + %3 + %5 + %7 + %9 + %11 + %12;
; CHECK:           |   %13 = (@B)[0][i1 + 1];
; CHECK:           |   %15 = (@B)[0][i1 + 2];
; CHECK:           |   %17 = (@B)[0][2 * i1 + 4];
; CHECK:           |   %19 = (@B)[0][i1 + 8];
; CHECK:           |   %21 = (@B)[0][i1 + 12];
; CHECK:           |   %23 = (@B)[0][i1 + 16];
; CHECK:           |   (@B)[0][i1] = %13 + %15 + %17 + %19 + %21 + %23;
; CHECK:           |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][i1 + 7]),  0,  3,  1);
; CHECK:           |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][i1 + 10007]),  0,  3,  1);
; CHECK:           |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][i1 + 40007]),  0,  3,  1);
; CHECK:           |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][2 * i1 + 16]),  0,  3,  1);
; CHECK:           |   @llvm.prefetch.p0i8(&((i8*)(@B)[0][2 * i1 + 18]),  0,  3,  1);
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@t = dso_local local_unnamed_addr global i32 10000, align 4
@A = common dso_local local_unnamed_addr global [50000 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [50000 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(i32 %t) local_unnamed_addr #0 {
entry:
  %cmp66 = icmp sgt i32 %t, 0
  br i1 %cmp66, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %t to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds [50000 x i32], [50000 x i32]* @A, i64 0, i64 %indvars.iv.next, !intel-tbaa !2
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %1 = shl nuw nsw i64 %indvars.iv, 1
  %2 = add nuw nsw i64 %1, 2
  %arrayidx3 = getelementptr inbounds [50000 x i32], [50000 x i32]* @A, i64 0, i64 %2, !intel-tbaa !2
  %3 = load i32, i32* %arrayidx3, align 8, !tbaa !2
  %add4 = add nsw i32 %3, %0
  %4 = add nuw nsw i64 %indvars.iv, 10000
  %arrayidx7 = getelementptr inbounds [50000 x i32], [50000 x i32]* @A, i64 0, i64 %4, !intel-tbaa !2
  %5 = load i32, i32* %arrayidx7, align 4, !tbaa !2
  %add8 = add nsw i32 %add4, %5
  %6 = add nuw nsw i64 %indvars.iv, 40000
  %arrayidx11 = getelementptr inbounds [50000 x i32], [50000 x i32]* @A, i64 0, i64 %6, !intel-tbaa !2
  %7 = load i32, i32* %arrayidx11, align 4, !tbaa !2
  %add12 = add nsw i32 %add8, %7
  %8 = add nuw nsw i64 %indvars.iv, 8000
  %arrayidx15 = getelementptr inbounds [50000 x i32], [50000 x i32]* @A, i64 0, i64 %8, !intel-tbaa !2
  %9 = load i32, i32* %arrayidx15, align 4, !tbaa !2
  %add16 = add nsw i32 %add12, %9
  %10 = add nuw nsw i64 %indvars.iv, 24
  %arrayidx19 = getelementptr inbounds [50000 x i32], [50000 x i32]* @A, i64 0, i64 %10, !intel-tbaa !2
  %11 = load i32, i32* %arrayidx19, align 4, !tbaa !2
  %add20 = add nsw i32 %add16, %11
  %12 = load i32, i32* getelementptr inbounds ([50000 x i32], [50000 x i32]* @A, i64 0, i64 20), align 16, !tbaa !2
  %add21 = add nsw i32 %add20, %12
  %arrayidx23 = getelementptr inbounds [50000 x i32], [50000 x i32]* @A, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store i32 %add21, i32* %arrayidx23, align 4, !tbaa !2
  %arrayidx26 = getelementptr inbounds [50000 x i32], [50000 x i32]* @B, i64 0, i64 %indvars.iv.next, !intel-tbaa !2
  %13 = load i32, i32* %arrayidx26, align 4, !tbaa !2
  %14 = add nuw nsw i64 %indvars.iv, 2
  %arrayidx29 = getelementptr inbounds [50000 x i32], [50000 x i32]* @B, i64 0, i64 %14, !intel-tbaa !2
  %15 = load i32, i32* %arrayidx29, align 4, !tbaa !2
  %add30 = add nsw i32 %15, %13
  %16 = add nuw nsw i64 %1, 4
  %arrayidx34 = getelementptr inbounds [50000 x i32], [50000 x i32]* @B, i64 0, i64 %16, !intel-tbaa !2
  %17 = load i32, i32* %arrayidx34, align 8, !tbaa !2
  %add35 = add nsw i32 %add30, %17
  %18 = add nuw nsw i64 %indvars.iv, 8
  %arrayidx38 = getelementptr inbounds [50000 x i32], [50000 x i32]* @B, i64 0, i64 %18, !intel-tbaa !2
  %19 = load i32, i32* %arrayidx38, align 4, !tbaa !2
  %add39 = add nsw i32 %add35, %19
  %20 = add nuw nsw i64 %indvars.iv, 12
  %arrayidx42 = getelementptr inbounds [50000 x i32], [50000 x i32]* @B, i64 0, i64 %20, !intel-tbaa !2
  %21 = load i32, i32* %arrayidx42, align 4, !tbaa !2
  %add43 = add nsw i32 %add39, %21
  %22 = add nuw nsw i64 %indvars.iv, 16
  %arrayidx46 = getelementptr inbounds [50000 x i32], [50000 x i32]* @B, i64 0, i64 %22, !intel-tbaa !2
  %23 = load i32, i32* %arrayidx46, align 4, !tbaa !2
  %add47 = add nsw i32 %add43, %23
  %arrayidx49 = getelementptr inbounds [50000 x i32], [50000 x i32]* @B, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store i32 %add47, i32* %arrayidx49, align 4, !tbaa !2, !nontemporal !7
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA50000_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{i32 1}
