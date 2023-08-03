; Check that only indirect prefetch A and B will not be indirect prefetched
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,print<hir>" -hir-prefetching-skip-non-modified-regions="false" -hir-prefetching-skip-num-memory-streams-check="true" -hir-prefetching-skip-AVX2-check="true" 2>&1 < %s | FileCheck %s
;
; Source code:
;  #pragma prefetch A
;  for (i=0; i< 100000; i++) {
;      A[M[i]] = B[M[i]];
;  }
;
;*** IR Dump Before HIR Prefetching (hir-prefetching) ***
;Function: foo
;
;<0>          BEGIN REGION { }
;<22>               + DO i1 = 0, 99999, 1   <DO_LOOP>
;<7>                |   %1 = (@M)[0][i1];
;<12>               |   (@A)[0][%1] = (@B)[0][%1];
;<22>               + END LOOP
;<22>
;<21>               ret undef;
;<0>          END REGION
;
;*** IR Dump After HIR Prefetching (hir-prefetching) ***
;Function: foo
;
; CHECK:           BEGIN REGION { modified }
; CHECK-NEXT:            + DO i1 = 0, 99999, 1   <DO_LOOP>
; CHECK-NEXT:            |   %1 = (@M)[0][i1];
; CHECK-NEXT:            |   (@A)[0][%1] = (@B)[0][%1];
; CHECK-NEXT:            |   if (i1 + 42 <=u 99999)
; CHECK-NEXT:            |   {
; CHECK-NEXT:            |      %Load = (@M)[0][i1 + 42];
; CHECK-NEXT:            |      @llvm.prefetch.p0(&((i8*)(@A)[0][%Load]),  0,  3,  1);
; CHECK-NEXT:            |   }
; CHECK-NEXT:            + END LOOP
;
; CHECK:            ret undef;
; CHECK:      END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local global [100000 x i32] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [100000 x i32] zeroinitializer, align 16
@M = dso_local local_unnamed_addr global [100000 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local i32 @foo() local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(ptr @A), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [100000 x i32], ptr @M, i64 0, i64 %indvars.iv, !intel-tbaa !3
  %1 = load i32, ptr %arrayidx, align 4, !tbaa !3
  %idxprom1 = sext i32 %1 to i64
  %arrayidx2 = getelementptr inbounds [100000 x i32], ptr @B, i64 0, i64 %idxprom1, !intel-tbaa !3
  %2 = load i32, ptr %arrayidx2, align 4, !tbaa !3
  %arrayidx6 = getelementptr inbounds [100000 x i32], ptr @A, i64 0, i64 %idxprom1, !intel-tbaa !3
  store i32 %2, ptr %arrayidx6, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100000
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !8

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.PREFETCH_LOOP"() ]
  ret i32 undef
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
!4 = !{!"array@_ZTSA100000_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
