; Test multiple indirect prefetching with assigned prefetch dist and hint.
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,print<hir>" 2>&1 < %s | FileCheck %s
;
;#pragma  prefetch A
;#pragma  prefetch B
;#pragma  prefetch C:3:40
;  for (i=0; i< N; i++) {
;      A[i] = B[i] + C[M[i]];
;      D[i] = D[i] + C[M[i+10]];
;  }
;
;*** IR Dump Before HIR Prefetching (hir-prefetching) ***
;Function: sub
;
;<0>          BEGIN REGION { }
;<43>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
;<15>               |   %2 = (%M)[i1];
;<19>               |   %add = (@C)[0][%2]  +  (@B)[0][i1];
;<21>               |   (@A)[0][i1] = %add;
;<26>               |   %6 = (%M)[i1 + 10];
;<30>               |   %add13 = (@C)[0][%6]  +  (@D)[0][i1];
;<31>               |   (@D)[0][i1] = %add13;
;<43>               + END LOOP
;<43>
;<42>               ret &((undef)[0]);
;<0>          END REGION
;
;*** IR Dump After HIR Prefetching (hir-prefetching) ***
;Function: sub
;
; CHECK:          BEGIN REGION { modified }
; CHECK-NEXT:           + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
; CHECK-NEXT:           |   %2 = (%M)[i1];
; CHECK-NEXT:           |   %add = (@C)[0][%2]  +  (@B)[0][i1];
; CHECK-NEXT:           |   (@A)[0][i1] = %add;
; CHECK-NEXT:           |   %6 = (%M)[i1 + 10];
; CHECK-NEXT:           |   %add13 = (@C)[0][%6]  +  (@D)[0][i1];
; CHECK-NEXT:           |   (@D)[0][i1] = %add13;
; CHECK-NEXT:           |   if (i1 + 50 <=u zext.i32.i64(%N) + 9)
; CHECK-NEXT:           |   {
; CHECK-NEXT:           |      %Load = (%M)[i1 + 40];
; CHECK-NEXT:           |      %Load2 = (%M)[i1 + 50];
; CHECK-NEXT:           |      @llvm.prefetch.p0(&((i8*)(@C)[0][%Load]),  0,  0,  1);
; CHECK-NEXT:           |      @llvm.prefetch.p0(&((i8*)(@C)[0][%Load2]),  0,  0,  1);
; CHECK-NEXT:           |   }
; CHECK-NEXT:           |   @llvm.prefetch.p0(&((i8*)(@A)[0][i1 + 15]),  0,  3,  1);
; CHECK-NEXT:           |   @llvm.prefetch.p0(&((i8*)(@B)[0][i1 + 15]),  0,  3,  1);
; CHECK-NEXT:           + END LOOP
;
; CHECK:                ret &((undef)[0]);
; CHECK:          END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local global [1000 x float] zeroinitializer, align 16
@B = dso_local global [1000 x float] zeroinitializer, align 16
@C = dso_local global [1000 x float] zeroinitializer, align 16
@D = dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local noalias ptr @sub(ptr nocapture readonly %M, i32 %N) local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(ptr @A), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(ptr @B), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(ptr @C), "QUAL.PRAGMA.HINT"(i32 3), "QUAL.PRAGMA.DISTANCE"(i32 40) ]
  %cmp25 = icmp sgt i32 %N, 0
  br i1 %cmp25, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count28 = zext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x float], ptr @B, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %1 = load float, ptr %arrayidx, align 4, !tbaa !2
  %ptridx = getelementptr inbounds i32, ptr %M, i64 %indvars.iv
  %2 = load i32, ptr %ptridx, align 4, !tbaa !7
  %idxprom2 = sext i32 %2 to i64
  %arrayidx3 = getelementptr inbounds [1000 x float], ptr @C, i64 0, i64 %idxprom2, !intel-tbaa !2
  %3 = load float, ptr %arrayidx3, align 4, !tbaa !2
  %add = fadd fast float %3, %1
  %arrayidx5 = getelementptr inbounds [1000 x float], ptr @A, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store float %add, ptr %arrayidx5, align 4, !tbaa !2
  %arrayidx7 = getelementptr inbounds [1000 x float], ptr @D, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %4 = load float, ptr %arrayidx7, align 4, !tbaa !2
  %5 = add nuw nsw i64 %indvars.iv, 10
  %ptridx10 = getelementptr inbounds i32, ptr %M, i64 %5
  %6 = load i32, ptr %ptridx10, align 4, !tbaa !7
  %idxprom11 = sext i32 %6 to i64
  %arrayidx12 = getelementptr inbounds [1000 x float], ptr @C, i64 0, i64 %idxprom11, !intel-tbaa !2
  %7 = load float, ptr %arrayidx12, align 4, !tbaa !2
  %add13 = fadd fast float %7, %4
  store float %add13, ptr %arrayidx7, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count28
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !9

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
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"int", !5, i64 0}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
