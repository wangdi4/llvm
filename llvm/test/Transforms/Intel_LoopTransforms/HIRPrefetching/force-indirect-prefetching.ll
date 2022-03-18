; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-prefetching -hir-prefetching-skip-non-modified-regions=false -hir-prefetching-skip-num-memory-streams-check=true -hir-prefetching-skip-AVX2-check=true -print-after=hir-prefetching < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,print<hir>" -hir-prefetching-skip-non-modified-regions="false" -hir-prefetching-skip-num-memory-streams-check="true" -hir-prefetching-skip-AVX2-check="true" 2>&1 < %s | FileCheck %s
;
;
;*** IR Dump Before HIR Prefetching (hir-prefetching) ***
;Function: foo
;
;<0>          BEGIN REGION { }
;<18>               + DO i1 = 0, 99999, 1   <DO_LOOP>
;<3>                |   %0 = (@B)[0][i1];
;<5>                |   %1 = (@M)[0][i1];
;<8>                |   %2 = (@C)[0][%1];
;<11>               |   (@A)[0][i1] = %0 + %2;
;<18>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Prefetching (hir-prefetching) ***
;Function: foo
;
; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 99999, 1   <DO_LOOP>
; CHECK:           |   %0 = (@B)[0][i1];
; CHECK:           |   %1 = (@M)[0][i1];
; CHECK:           |   %2 = (@C)[0][%1];
; CHECK:           |   (@A)[0][i1] = %0 + %2;
; CHECK:           |   @llvm.prefetch.p0i8(&((i8*)(@B)[0][i1 + 32]),  0,  3,  1);
; CHECK:           |   @llvm.prefetch.p0i8(&((i8*)(@M)[0][i1 + 32]),  0,  3,  1);
; CHECK:           |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][i1 + 32]),  0,  3,  1);
; CHECK:           + END LOOP
; CHECK:     END REGION
;
; Test the case of enabling indirect prefetching when there is no pragma info
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-prefetching -hir-prefetching-skip-non-modified-regions=false -hir-prefetching-skip-num-memory-streams-check=true -hir-prefetching-skip-AVX2-check=true -hir-prefetching-enable-indirect-prefetching=true -print-after=hir-prefetching < %s 2>&1 | FileCheck %s -check-prefix=INDIRECT
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,print<hir>" -hir-prefetching-skip-non-modified-regions="false" -hir-prefetching-skip-num-memory-streams-check="true" -hir-prefetching-skip-AVX2-check="true" -hir-prefetching-enable-indirect-prefetching=true 2>&1 < %s | FileCheck %s -check-prefix=INDIRECT
;
;*** IR Dump After HIR Prefetching (hir-prefetching) ***
;Function: foo
;
; INDIRECT:  BEGIN REGION { }
; INDIRECT:        + DO i1 = 0, 99999, 1   <DO_LOOP>
; INDIRECT:        |   %0 = (@B)[0][i1];
; INDIRECT:        |   %1 = (@M)[0][i1];
; INDIRECT:        |   %2 = (@C)[0][%1];
; INDIRECT:        |   (@A)[0][i1] = %0 + %2;
; INDIRECT:        |   if (i1 + 32 <=u 99999)
; INDIRECT:        |   {
; INDIRECT:        |      %Load = (@M)[0][i1 + 32];
; INDIRECT:        |      @llvm.prefetch.p0i8(&((i8*)(@C)[0][%Load]),  0,  3,  1);
; INDIRECT:        |   }
; INDIRECT:        |   @llvm.prefetch.p0i8(&((i8*)(@B)[0][i1 + 32]),  0,  3,  1);
; INDIRECT:        |   @llvm.prefetch.p0i8(&((i8*)(@M)[0][i1 + 32]),  0,  3,  1);
; INDIRECT:        |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][i1 + 32]),  0,  3,  1);
; INDIRECT:        + END LOOP
; INDIRECT:  END REGION
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-prefetching -hir-prefetching-skip-non-modified-regions=false -hir-prefetching-skip-num-memory-streams-check=true -hir-prefetching-skip-AVX2-check=true -hir-prefetching-enable-indirect-prefetching=true -hir-cg -force-hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter < %s 2>&1 | FileCheck %s -check-prefix=OPTREPORT
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,hir-cg,simplifycfg,intel-ir-optreport-emitter" -hir-prefetching-skip-non-modified-regions="false" -hir-prefetching-skip-num-memory-streams-check="true" -hir-prefetching-skip-AVX2-check="true" -hir-prefetching-enable-indirect-prefetching=true -intel-opt-report=low -force-hir-cg 2>&1 < %s | FileCheck %s -check-prefix=OPTREPORT
;
; OPTREPORT:  LOOP BEGIN
; OPTREPORT:     remark #25018: Total number of lines prefetched=4
; OPTREPORT:     remark #25019: Number of spatial prefetches=3, default dist=32
; OPTREPORT:     remark #25033: Number of indirect prefetches=1, default dist=32
; OPTREPORT:  LOOP END
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = dso_local local_unnamed_addr global [100000 x i32] zeroinitializer, align 16
@C = dso_local local_unnamed_addr global [100000 x i32] zeroinitializer, align 16
@M = dso_local local_unnamed_addr global [100000 x i32] zeroinitializer, align 16
@A = dso_local local_unnamed_addr global [100000 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [100000 x i32], [100000 x i32]* @B, i64 0, i64 %indvars.iv, !intel-tbaa !3
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %arrayidx2 = getelementptr inbounds [100000 x i32], [100000 x i32]* @M, i64 0, i64 %indvars.iv, !intel-tbaa !3
  %1 = load i32, i32* %arrayidx2, align 4, !tbaa !3
  %idxprom3 = sext i32 %1 to i64
  %arrayidx4 = getelementptr inbounds [100000 x i32], [100000 x i32]* @C, i64 0, i64 %idxprom3, !intel-tbaa !3
  %2 = load i32, i32* %arrayidx4, align 4, !tbaa !3
  %add = add nsw i32 %2, %0
  %arrayidx6 = getelementptr inbounds [100000 x i32], [100000 x i32]* @A, i64 0, i64 %indvars.iv, !intel-tbaa !3
  store i32 %add, i32* %arrayidx6, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100000
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !8

for.end:                                          ; preds = %for.body
  ret i32 undef
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA100000_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
