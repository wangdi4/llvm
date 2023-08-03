; Test the case when they are three index load refs, (@M)[0][i1] and (@M)[0][i1 + -1] have constant distance,
; and (@M)[0][i1 + sext.i32.i64(%K)] does not have constant distance with other two index load refs.
; We will generate two HLIfs in this case.
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,print<hir>" 2>&1 < %s | FileCheck %s
;
; Source code
;#pragma  prefetch A
;#pragma  prefetch B:2:20
;  for (i=0; i< N; i++) {
;      A[i] = B[M[i]] + B[M[i-1]] + B[M[i+K]];
; }
;
;
;*** IR Dump Before HIR Prefetching (hir-prefetching) ***
;Function: foo
;
;<0>          BEGIN REGION { }
;<45>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
;<14>               |   %2 = (@M)[0][i1];
;<20>               |   %5 = (@M)[0][i1 + -1];
;<24>               |   %add = (@B)[0][%5]  +  (@B)[0][%2];
;<27>               |   %8 = (@M)[0][i1 + sext.i32.i64(%K)];
;<31>               |   %add12 = %add  +  (@B)[0][%8];
;<33>               |   (@A)[0][i1] = %add12;
;<45>               + END LOOP
;<45>
;<44>               ret ;
;<0>          END REGION
;
;*** IR Dump After HIR Prefetching (hir-prefetching) ***
;Function: foo
;
; CHECK:           BEGIN REGION { modified }
; CHECK-NEXT:           + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
; CHECK-NEXT:           |   %2 = (@M)[0][i1];
; CHECK-NEXT:           |   %5 = (@M)[0][i1 + -1];
; CHECK-NEXT:           |   %add = (@B)[0][%5]  +  (@B)[0][%2];
; CHECK-NEXT:           |   %8 = (@M)[0][i1 + sext.i32.i64(%K)];
; CHECK-NEXT:           |   %add12 = %add  +  (@B)[0][%8];
; CHECK-NEXT:           |   (@A)[0][i1] = %add12;
; CHECK-NEXT:           |   if (i1 + 20 <=u zext.i32.i64(%N) + -1)
; CHECK-NEXT:           |   {
; CHECK-NEXT:           |      %Load = (@M)[0][i1 + 20];
; CHECK-NEXT:           |      %Load2 = (@M)[0][i1 + 19];
; CHECK-NEXT:           |      @llvm.prefetch.p0(&((i8*)(@B)[0][%Load]),  0,  1,  1);
; CHECK-NEXT:           |      @llvm.prefetch.p0(&((i8*)(@B)[0][%Load2]),  0,  1,  1);
; CHECK-NEXT:           |   }
; CHECK-NEXT:           |   if (i1 + sext.i32.i64(%K) + 20 <=u zext.i32.i64(%N) + sext.i32.i64(%K) + -1)
; CHECK-NEXT:           |   {
; CHECK-NEXT:           |      %Load3 = (@M)[0][i1 + sext.i32.i64(%K) + 20];
; CHECK-NEXT:           |      @llvm.prefetch.p0(&((i8*)(@B)[0][%Load3]),  0,  1,  1);
; CHECK-NEXT:           |   }
; CHECK-NEXT:           |   @llvm.prefetch.p0(&((i8*)(@A)[0][i1 + 16]),  0,  3,  1);
; CHECK-NEXT:           + END LOOP
;
; CHECK:                ret ;
; CHECK:          END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local global [1000 x float] zeroinitializer, align 16
@B = dso_local global [1000 x float] zeroinitializer, align 16
@M = dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32 %N, i32 %K) local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(ptr @A), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(ptr @B), "QUAL.PRAGMA.HINT"(i32 2), "QUAL.PRAGMA.DISTANCE"(i32 20) ]
  %cmp21 = icmp sgt i32 %N, 0
  br i1 %cmp21, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %1 = sext i32 %K to i64
  %wide.trip.count25 = zext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @M, i64 0, i64 %indvars.iv, !intel-tbaa !3
  %2 = load i32, ptr %arrayidx, align 4, !tbaa !3
  %idxprom1 = sext i32 %2 to i64
  %arrayidx2 = getelementptr inbounds [1000 x float], ptr @B, i64 0, i64 %idxprom1, !intel-tbaa !8
  %3 = load float, ptr %arrayidx2, align 4, !tbaa !8
  %4 = add nsw i64 %indvars.iv, -1
  %arrayidx4 = getelementptr inbounds [1000 x i32], ptr @M, i64 0, i64 %4, !intel-tbaa !3
  %5 = load i32, ptr %arrayidx4, align 4, !tbaa !3
  %idxprom5 = sext i32 %5 to i64
  %arrayidx6 = getelementptr inbounds [1000 x float], ptr @B, i64 0, i64 %idxprom5, !intel-tbaa !8
  %6 = load float, ptr %arrayidx6, align 4, !tbaa !8
  %add = fadd fast float %6, %3
  %7 = add nsw i64 %indvars.iv, %1
  %arrayidx9 = getelementptr inbounds [1000 x i32], ptr @M, i64 0, i64 %7, !intel-tbaa !3
  %8 = load i32, ptr %arrayidx9, align 4, !tbaa !3
  %idxprom10 = sext i32 %8 to i64
  %arrayidx11 = getelementptr inbounds [1000 x float], ptr @B, i64 0, i64 %idxprom10, !intel-tbaa !8
  %9 = load float, ptr %arrayidx11, align 4, !tbaa !8
  %add12 = fadd fast float %add, %9
  %arrayidx14 = getelementptr inbounds [1000 x float], ptr @A, i64 0, i64 %indvars.iv, !intel-tbaa !8
  store float %add12, ptr %arrayidx14, align 4, !tbaa !8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count25
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !11

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.PREFETCH_LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA1000_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !10, i64 0}
!9 = !{!"array@_ZTSA1000_f", !10, i64 0}
!10 = !{!"float", !6, i64 0}
!11 = distinct !{!11, !12}
!12 = !{!"llvm.loop.mustprogress"}
