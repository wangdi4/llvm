; Source code
;
;int A[1000000];
;int B[1000000];
;void foo(){
;  int i;
;  for(i = 0; i < 100000; i++){
;    A[i] = A[i+1] + A[2 * i+2] + A[i+100000] +  A[i+400000] + A[i+80000]  + A[i+24] + A[20];
;    B[i] = B[i+1] + B[i+2] + B[2 * i+4] + B[i+8] + B[i+12] + B[i+16];
;  }
;}
;
; After locality analysis, the memrefs will be grouped like this-
; Group1: A[20]  // we will not prefetch invariant
; Group2: A[i], A[i+1], A[i+24],A[i+80000], A[i+100000], A[i+400000] //separated into 3 data streams
; Group3: A[2*i + 2] //only 1 data stream
; Group4: B[i], B[i+2], B[i+8], B[i+12], B[i+16] //only 1 data stream
; Group5: B[2 * i+4] //only 1 data stream
;
; Note:The analysis is by no means perfect. The memory streams in Group2 and Group3 may overlap.
; The spatial groups are currently formed based on constant distance between refs.
; Prefetching can probably refine this by using more analysis/heuristics
; but it is always hard to know whether prefetching for one of them is sufficient.
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-prefetching -hir-prefetching-skip-non-modified-regions=false -hir-prefetching-skip-num-memory-streams-check=true -hir-prefetching-skip-AVX2-check=true -print-after=hir-prefetching < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,print<hir>" -hir-prefetching-skip-non-modified-regions="false" -hir-prefetching-skip-num-memory-streams-check="true" -hir-prefetching-skip-AVX2-check="true" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Prefetching ***
;
;<0>          BEGIN REGION { }
;<59>               + DO i1 = 0, 99999, 1   <DO_LOOP>
;<4>                |   %0 = (@A)[0][i1 + 1];
;<8>                |   %3 = (@A)[0][2 * i1 + 2];
;<12>               |   %5 = (@A)[0][i1 + 100000];
;<16>               |   %7 = (@A)[0][i1 + 400000];
;<20>               |   %9 = (@A)[0][i1 + 80000];
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
; CHECK:     BEGIN REGION { }
; CHECK-NEXT:      + DO i1 = 0, 99999, 1   <DO_LOOP>
; CHECK-NEXT:      |   %0 = (@A)[0][i1 + 1];
; CHECK-NEXT:      |   %3 = (@A)[0][2 * i1 + 2];
; CHECK-NEXT:      |   %5 = (@A)[0][i1 + 100000];
; CHECK-NEXT:      |   %7 = (@A)[0][i1 + 400000];
; CHECK-NEXT:      |   %9 = (@A)[0][i1 + 80000];
; CHECK-NEXT:      |   %11 = (@A)[0][i1 + 24];
; CHECK-NEXT:      |   %12 = (@A)[0][20];
; CHECK-NEXT:      |   (@A)[0][i1] = %0 + %3 + %5 + %7 + %9 + %11 + %12;
; CHECK-NEXT:      |   %13 = (@B)[0][i1 + 1];
; CHECK-NEXT:      |   %15 = (@B)[0][i1 + 2];
; CHECK-NEXT:      |   %17 = (@B)[0][2 * i1 + 4];
; CHECK-NEXT:      |   %19 = (@B)[0][i1 + 8];
; CHECK-NEXT:      |   %21 = (@B)[0][i1 + 12];
; CHECK-NEXT:      |   %23 = (@B)[0][i1 + 16];
; CHECK-NEXT:      |   (@B)[0][i1] = %13 + %15 + %17 + %19 + %21 + %23;
; CHECK-NEXT:      |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][i1 + 6]),  0,  3,  1);
; CHECK-NEXT:      |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][i1 + 100006]),  0,  3,  1);
; CHECK-NEXT:      |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][i1 + 400006]),  0,  3,  1);
; CHECK-NEXT:      |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][2 * i1 + 14]),  0,  3,  1);
; CHECK-NEXT:      |   @llvm.prefetch.p0i8(&((i8*)(@B)[0][i1 + 6]),  0,  3,  1);
; CHECK-NEXT:      |   @llvm.prefetch.p0i8(&((i8*)(@B)[0][2 * i1 + 16]),  0,  3,  1);
; CHECK-NEXT:      + END LOOP
; CHECK:     END  REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [1000000 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [1000000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds [1000000 x i32], [1000000 x i32]* @A, i64 0, i64 %indvars.iv.next, !intel-tbaa !2
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %1 = shl nuw nsw i64 %indvars.iv, 1
  %2 = add nuw nsw i64 %1, 2
  %arrayidx3 = getelementptr inbounds [1000000 x i32], [1000000 x i32]* @A, i64 0, i64 %2, !intel-tbaa !2
  %3 = load i32, i32* %arrayidx3, align 8, !tbaa !2
  %add4 = add nsw i32 %3, %0
  %4 = add nuw nsw i64 %indvars.iv, 100000
  %arrayidx7 = getelementptr inbounds [1000000 x i32], [1000000 x i32]* @A, i64 0, i64 %4, !intel-tbaa !2
  %5 = load i32, i32* %arrayidx7, align 4, !tbaa !2
  %add8 = add nsw i32 %add4, %5
  %6 = add nuw nsw i64 %indvars.iv, 400000
  %arrayidx11 = getelementptr inbounds [1000000 x i32], [1000000 x i32]* @A, i64 0, i64 %6, !intel-tbaa !2
  %7 = load i32, i32* %arrayidx11, align 4, !tbaa !2
  %add12 = add nsw i32 %add8, %7
  %8 = add nuw nsw i64 %indvars.iv, 80000
  %arrayidx15 = getelementptr inbounds [1000000 x i32], [1000000 x i32]* @A, i64 0, i64 %8, !intel-tbaa !2
  %9 = load i32, i32* %arrayidx15, align 4, !tbaa !2
  %add16 = add nsw i32 %add12, %9
  %10 = add nuw nsw i64 %indvars.iv, 24
  %arrayidx19 = getelementptr inbounds [1000000 x i32], [1000000 x i32]* @A, i64 0, i64 %10, !intel-tbaa !2
  %11 = load i32, i32* %arrayidx19, align 4, !tbaa !2
  %add20 = add nsw i32 %add16, %11
  %12 = load i32, i32* getelementptr inbounds ([1000000 x i32], [1000000 x i32]* @A, i64 0, i64 20), align 16, !tbaa !2
  %add21 = add nsw i32 %add20, %12
  %arrayidx23 = getelementptr inbounds [1000000 x i32], [1000000 x i32]* @A, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store i32 %add21, i32* %arrayidx23, align 4, !tbaa !2
  %arrayidx26 = getelementptr inbounds [1000000 x i32], [1000000 x i32]* @B, i64 0, i64 %indvars.iv.next, !intel-tbaa !2
  %13 = load i32, i32* %arrayidx26, align 4, !tbaa !2
  %14 = add nuw nsw i64 %indvars.iv, 2
  %arrayidx29 = getelementptr inbounds [1000000 x i32], [1000000 x i32]* @B, i64 0, i64 %14, !intel-tbaa !2
  %15 = load i32, i32* %arrayidx29, align 4, !tbaa !2
  %add30 = add nsw i32 %15, %13
  %16 = add nuw nsw i64 %1, 4
  %arrayidx34 = getelementptr inbounds [1000000 x i32], [1000000 x i32]* @B, i64 0, i64 %16, !intel-tbaa !2
  %17 = load i32, i32* %arrayidx34, align 8, !tbaa !2
  %add35 = add nsw i32 %add30, %17
  %18 = add nuw nsw i64 %indvars.iv, 8
  %arrayidx38 = getelementptr inbounds [1000000 x i32], [1000000 x i32]* @B, i64 0, i64 %18, !intel-tbaa !2
  %19 = load i32, i32* %arrayidx38, align 4, !tbaa !2
  %add39 = add nsw i32 %add35, %19
  %20 = add nuw nsw i64 %indvars.iv, 12
  %arrayidx42 = getelementptr inbounds [1000000 x i32], [1000000 x i32]* @B, i64 0, i64 %20, !intel-tbaa !2
  %21 = load i32, i32* %arrayidx42, align 4, !tbaa !2
  %add43 = add nsw i32 %add39, %21
  %22 = add nuw nsw i64 %indvars.iv, 16
  %arrayidx46 = getelementptr inbounds [1000000 x i32], [1000000 x i32]* @B, i64 0, i64 %22, !intel-tbaa !2
  %23 = load i32, i32* %arrayidx46, align 4, !tbaa !2
  %add47 = add nsw i32 %add43, %23
  %arrayidx49 = getelementptr inbounds [1000000 x i32], [1000000 x i32]* @B, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store i32 %add47, i32* %arrayidx49, align 4, !tbaa !2
  %exitcond = icmp eq i64 %indvars.iv.next, 100000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000000_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
