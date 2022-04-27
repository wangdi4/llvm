; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-prefetching -print-after=hir-prefetching < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,print<hir>" 2>&1 < %s | FileCheck %s
;
; Source code
;#pragma  prefetch A:1:40
;#pragma  prefetch B
;#pragma  prefetch C
;  for (i=0; i< N; i++) {
;      A[i] = B[i] + C[M[i]];
;  }
;
;*** IR Dump Before HIR Prefetching (hir-prefetching) ***
;Function: sub
;
;<0>          BEGIN REGION { }
;<33>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
;<15>               |   %2 = (%M)[i1];
;<19>               |   %add = (@C)[0][%2]  +  (@B)[0][i1];
;<21>               |   (@A)[0][i1] = %add;
;<33>               + END LOOP
;<33>
;<32>               ret &((undef)[0]);
;<0>          END REGION
;
;*** IR Dump After HIR Prefetching (hir-prefetching) ***
;Function: sub
;
; CHECK:         BEGIN REGION { modified }
; CHECK-NEXT:           + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
; CHECK-NEXT:           |   %2 = (%M)[i1];
; CHECK-NEXT:           |   %add = (@C)[0][%2]  +  (@B)[0][i1];
; CHECK-NEXT:           |   (@A)[0][i1] = %add;
; CHECK-NEXT:           |   if (i1 + 31 <=u zext.i32.i64(%N) + -1)
; CHECK-NEXT:           |   {
; CHECK-NEXT:           |      %Load = (%M)[i1 + 31];
; CHECK-NEXT:           |      @llvm.prefetch.p0i8(&((i8*)(@C)[0][%Load]),  0,  3,  1);
; CHECK-NEXT:           |   }
; CHECK-NEXT:           |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][i1 + 40]),  0,  2,  1);
; CHECK-NEXT:           |   @llvm.prefetch.p0i8(&((i8*)(@B)[0][i1 + 31]),  0,  3,  1);
; CHECK-NEXT:           + END LOOP
;
; CHECK:                ret &((undef)[0]);
; CHECK:          END REGION
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-prefetching -print-after=hir-prefetching -hir-cg -force-hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter < %s 2>&1 | FileCheck %s -check-prefix=OPTREPORT
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,hir-cg,simplifycfg,intel-ir-optreport-emitter" -intel-opt-report=low -force-hir-cg 2>&1 < %s | FileCheck %s -check-prefix=OPTREPORT
;
; OPTREPORT: LOOP BEGIN
; OPTREPORT:    remark #25018: Total number of lines prefetched=3
; OPTREPORT:    remark #25019: Number of spatial prefetches=2,  default dist=31
; OPTREPORT:    remark #25033: Number of indirect prefetches=1, default dist=31
; OPTREPORT:    remark #25147: Using directive-based hint=1, distance=40 for prefetching spatial memory reference
; OPTREPORT: LOOP END
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local global [1000 x float] zeroinitializer, align 16
@B = dso_local global [1000 x float] zeroinitializer, align 16
@C = dso_local global [1000 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local noalias i8* @sub(i32* nocapture readonly %M, i32 %N) local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"([1000 x float]* @A), "QUAL.PRAGMA.HINT"(i32 1), "QUAL.PRAGMA.DISTANCE"(i32 40), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"([1000 x float]* @B), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"([1000 x float]* @C), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1) ]
  %cmp11 = icmp sgt i32 %N, 0
  br i1 %cmp11, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count13 = zext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x float], [1000 x float]* @B, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %1 = load float, float* %arrayidx, align 4, !tbaa !2
  %ptridx = getelementptr inbounds i32, i32* %M, i64 %indvars.iv
  %2 = load i32, i32* %ptridx, align 4, !tbaa !7
  %idxprom2 = sext i32 %2 to i64
  %arrayidx3 = getelementptr inbounds [1000 x float], [1000 x float]* @C, i64 0, i64 %idxprom2, !intel-tbaa !2
  %3 = load float, float* %arrayidx3, align 4, !tbaa !2
  %add = fadd fast float %3, %1
  %arrayidx5 = getelementptr inbounds [1000 x float], [1000 x float]* @A, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store float %add, float* %arrayidx5, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count13
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !9

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.PREFETCH_LOOP"() ]
  ret i8* undef
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
