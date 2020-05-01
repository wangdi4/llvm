; We count the number of non-linear streams into the total number of memory streams threshold.
; In this case, (@A) is a non-linear memory stream, (@B) is a linear memory stream and we set up hir-prefetching-num-memory-streams-threshold=2.
; Finally, we only prefetch 1 memory streams for (@B).
;
; Source code
;int A[1000000];
;int B[1000000];
;void foo(int k){
;  int i, j, t;
;  for(i = 0; i < 100000; i++){
;    A[i%5] = i;
;    B[i] = i;
;  }
;}
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-prefetching -hir-prefetching-skip-non-modified-regions=false -hir-prefetching-num-memory-streams-threshold=2 -hir-prefetching-skip-AVX2-check=true -print-after=hir-prefetching < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching,print<hir>" -hir-prefetching-skip-non-modified-regions=false -hir-prefetching-num-memory-streams-threshold=2 -hir-prefetching-skip-AVX2-check=true 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Prefetching ***
;Function: foo
;
;<0>          BEGIN REGION { }
;<15>               + DO i1 = 0, 99999, 1   <DO_LOOP>
;<2>                |   %0 = trunc.i64.i32(i1);
;<6>                |   (@A)[0][i1 + -5 * (zext.i32.i64(%0) /u 5)] = i1;
;<8>                |   (@B)[0][i1] = i1;
;<15>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Prefetching ***
;Function: foo
;
; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 99999, 1   <DO_LOOP>
; CHECK:           |   %0 = trunc.i64.i32(i1);
; CHECK:           |   (@A)[0][i1 + -5 * (zext.i32.i64(%0) /u 5)] = i1;
; CHECK:           |   (@B)[0][i1] = i1;
; CHECK:           |   @llvm.prefetch.p0i8(&((i8*)(@B)[0][i1 + 56]),  0,  3,  1);
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [1000000 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [1000000 x i32] zeroinitializer, align 16
; Function Attrs: nofree norecurse nounwind uwtable writeonly
define dso_local void @foo(i32 %k) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  %rem = urem i32 %0, 5
  %idxprom = zext i32 %rem to i64
  %arrayidx = getelementptr inbounds [1000000 x i32], [1000000 x i32]* @A, i64 0, i64 %idxprom, !intel-tbaa !2
  store i32 %0, i32* %arrayidx, align 4, !tbaa !2
  %arrayidx2 = getelementptr inbounds [1000000 x i32], [1000000 x i32]* @B, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store i32 %0, i32* %arrayidx2, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { nofree norecurse nounwind uwtable writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000000_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
