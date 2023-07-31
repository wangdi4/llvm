; Test the case when two index load refs do not have non-constant distance, we will generate two HLIfs for the indirect prefetch
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,print<hir>" 2>&1 < %s | FileCheck %s
;
; Source code
;#pragma  prefetch A
;#pragma  prefetch B:2:20
;  for (i=0; i< N; i++) {
;      A[i] = B[M[i]] + B[M[i+K]];
; }
;
;*** IR Dump Before HIR Prefetching (hir-prefetching) ***
;Function: sub
;
;<0>          BEGIN REGION { }
;<38>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10000>
;<14>               |   %2 = (%M)[i1];
;<17>               |   %3 = (@B)[0][%2];
;<20>               |   %5 = (%M)[i1 + sext.i32.i64(%K)];
;<23>               |   %6 = (@B)[0][%5];
;<26>               |   (@A)[0][i1] = %3 + %6;
;<38>               + END LOOP
;<38>
;<37>               ret &((undef)[0]);
;<0>          END REGION
;
;*** IR Dump After HIR Prefetching (hir-prefetching) ***
;Function: sub
;
; CHECK:        BEGIN REGION { modified }
; CHECK-NEXT:           + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10000>
; CHECK-NEXT:           |   %2 = (%M)[i1];
; CHECK-NEXT:           |   %3 = (@B)[0][%2];
; CHECK-NEXT:           |   %5 = (%M)[i1 + sext.i32.i64(%K)];
; CHECK-NEXT:           |   %6 = (@B)[0][%5];
; CHECK-NEXT:           |   (@A)[0][i1] = %3 + %6;
; CHECK-NEXT:           |   if (i1 + 20 <=u zext.i32.i64(%N) + -1)
; CHECK-NEXT:           |   {
; CHECK-NEXT:           |      %Load = (%M)[i1 + 20];
; CHECK-NEXT:           |      @llvm.prefetch.p0(&((i8*)(@B)[0][%Load]),  0,  1,  1);
; CHECK-NEXT:           |   }
; CHECK-NEXT:           |   if (i1 + sext.i32.i64(%K) + 20 <=u zext.i32.i64(%N) + sext.i32.i64(%K) + -1)
; CHECK-NEXT:           |   {
; CHECK-NEXT:           |      %Load2 = (%M)[i1 + sext.i32.i64(%K) + 20];
; CHECK-NEXT:           |      @llvm.prefetch.p0(&((i8*)(@B)[0][%Load2]),  0,  1,  1);
; CHECK-NEXT:           |   }
; CHECK-NEXT:           |   @llvm.prefetch.p0(&((i8*)(@A)[0][i1 + 24]),  0,  3,  1);
; CHECK-NEXT:           + END LOOP
;
; CHECK:                ret &((undef)[0]);
; CHECK:       END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local global [10000 x i32] zeroinitializer, align 16
@B = dso_local global [10000 x i32] zeroinitializer, align 16
@C = dso_local local_unnamed_addr global [10000 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local noalias ptr @sub(ptr nocapture readonly %M, i32 %N, i32 %K) local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(ptr @A), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(ptr @B), "QUAL.PRAGMA.HINT"(i32 2), "QUAL.PRAGMA.DISTANCE"(i32 20) ]
  %cmp15 = icmp sgt i32 %N, 0
  br i1 %cmp15, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %1 = sext i32 %K to i64
  %wide.trip.count18 = zext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32, ptr %M, i64 %indvars.iv
  %2 = load i32, ptr %ptridx, align 4, !tbaa !2
  %idxprom1 = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds [10000 x i32], ptr @B, i64 0, i64 %idxprom1, !intel-tbaa !6
  %3 = load i32, ptr %arrayidx, align 4, !tbaa !6
  %4 = add nsw i64 %indvars.iv, %1
  %ptridx3 = getelementptr inbounds i32, ptr %M, i64 %4
  %5 = load i32, ptr %ptridx3, align 4, !tbaa !2
  %idxprom4 = sext i32 %5 to i64
  %arrayidx5 = getelementptr inbounds [10000 x i32], ptr @B, i64 0, i64 %idxprom4, !intel-tbaa !6
  %6 = load i32, ptr %arrayidx5, align 4, !tbaa !6
  %add6 = add nsw i32 %6, %3
  %arrayidx8 = getelementptr inbounds [10000 x i32], ptr @A, i64 0, i64 %indvars.iv, !intel-tbaa !6
  store i32 %add6, ptr %arrayidx8, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count18
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !8

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.PREFETCH_LOOP"() ]
  ret ptr undef
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA10000_i", !3, i64 0}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
