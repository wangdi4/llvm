; This test has indirect prefetching for @C. We have pragma prefetch for %M here, we can still
; prefetch %1, because %1 is the lval of load %M inst in the preheader.
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,print<hir>" 2>&1 < %s | FileCheck %s
;
; Source code
;#pragma  prefetch A:1:40
;#pragma  prefetch B
;#pragma  prefetch C
;#pragma  prefetch M
;  for (i=0; i< N; i++) {
;      A[i] = B[i] + C[M[i]];
;  }
;
;*** IR Dump Before HIR Prefetching (hir-prefetching) ***
;Function: sub
;
;<0>          BEGIN REGION { }
;<8>                   %1 = (%M.addr)[0];
;<34>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10000>
;<14>               |   %2 = (@B)[0][i1];
;<16>               |   %3 = (%1)[i1];
;<19>               |   %4 = (@C)[0][%3];
;<22>               |   (@A)[0][i1] = %2 + %4;
;<34>               + END LOOP
;<34>
;<33>               ret &((undef)[0]);
;<0>          END REGION
;
;*** IR Dump After HIR Prefetching (hir-prefetching) ***
;Function: sub
;
;CHECK:          BEGIN REGION { modified }
;CHECK-NEXT:               %1 = (%M.addr)[0];
;CHECK-NEXT:            + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10000>
;CHECK-NEXT:            |   %2 = (@B)[0][i1];
;CHECK-NEXT:            |   %3 = (%1)[i1];
;CHECK-NEXT:            |   %4 = (@C)[0][%3];
;CHECK-NEXT:            |   (@A)[0][i1] = %2 + %4;
;CHECK-NEXT:            |   if (i1 + 32 <=u zext.i32.i64(%N) + -1)
;CHECK-NEXT:            |   {
;CHECK-NEXT:            |      %Load = (%1)[i1 + 32];
;CHECK-NEXT:            |      @llvm.prefetch.p0(&((i8*)(@C)[0][%Load]),  0,  3,  1);
;CHECK-NEXT:            |   }
;CHECK-NEXT:            |   @llvm.prefetch.p0(&((i8*)(@A)[0][i1 + 40]),  0,  2,  1);
;CHECK-NEXT:            |   @llvm.prefetch.p0(&((i8*)(@B)[0][i1 + 32]),  0,  3,  1);
;CHECK-NEXT:            |   @llvm.prefetch.p0(&((i8*)(%1)[i1 + 32]),  0,  3,  1);
;CHECK-NEXT:            + END LOOP
;
;CHECK:                 ret &((undef)[0]);
;CHECK:           END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local global [10000 x i32] zeroinitializer, align 16
@B = dso_local global [10000 x i32] zeroinitializer, align 16
@C = dso_local global [10000 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local noalias ptr @sub(ptr %M, i32 %N) local_unnamed_addr #0 {
entry:
  %M.addr = alloca ptr, align 8
  store ptr %M, ptr %M.addr, align 8, !tbaa !2
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(ptr @A), "QUAL.PRAGMA.HINT"(i32 1), "QUAL.PRAGMA.DISTANCE"(i32 40), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(ptr @B), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(ptr @C), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(ptr %M.addr), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1) ]
  %cmp11 = icmp sgt i32 %N, 0
  br i1 %cmp11, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %1 = load ptr, ptr %M.addr, align 8, !tbaa !2
  %wide.trip.count13 = zext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [10000 x i32], ptr @B, i64 0, i64 %indvars.iv, !intel-tbaa !6
  %2 = load i32, ptr %arrayidx, align 4, !tbaa !6
  %ptridx = getelementptr inbounds i32, ptr %1, i64 %indvars.iv
  %3 = load i32, ptr %ptridx, align 4, !tbaa !9
  %idxprom2 = sext i32 %3 to i64
  %arrayidx3 = getelementptr inbounds [10000 x i32], ptr @C, i64 0, i64 %idxprom2, !intel-tbaa !6
  %4 = load i32, ptr %arrayidx3, align 4, !tbaa !6
  %add = add nsw i32 %4, %2
  %arrayidx5 = getelementptr inbounds [10000 x i32], ptr @A, i64 0, i64 %indvars.iv, !intel-tbaa !6
  store i32 %add, ptr %arrayidx5, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count13
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !10

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
!3 = !{!"pointer@_ZTSPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !8, i64 0}
!7 = !{!"array@_ZTSA10000_i", !8, i64 0}
!8 = !{!"int", !4, i64 0}
!9 = !{!8, !8, i64 0}
!10 = distinct !{!10, !11}
!11 = !{!"llvm.loop.mustprogress"}
