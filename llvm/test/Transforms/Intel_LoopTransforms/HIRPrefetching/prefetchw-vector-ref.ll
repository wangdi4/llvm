; Check that prefetchw is working for vector refs
;
; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -hir-prefetching -hir-prefetching-num-cachelines-threshold=64 -hir-prefetching-skip-non-modified-regions=false -hir-prefetching-skip-num-memory-streams-check=true -hir-prefetching-skip-AVX2-check=true -vplan-force-vf=4 -hir-prefetching-prefetchw=true -print-after=hir-prefetching < %s 2>&1 | FileCheck %s --check-prefix=MERGED-CFG
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,hir-prefetching,print<hir>" -hir-prefetching-num-cachelines-threshold=64 -hir-prefetching-skip-non-modified-regions=false -hir-prefetching-skip-num-memory-streams-check=true -hir-prefetching-skip-AVX2-check=true -vplan-force-vf=4 -hir-prefetching-prefetchw=true < %s 2>&1 | FileCheck %s --check-prefix=MERGED-CFG
;
;*** IR Dump Before HIR Prefetching ***
;
;<0>          BEGIN REGION { modified }
;<14>               %tgu = (zext.i32.i64(%t))/u4;
;<16>               if (0 <u 4 * %tgu)
;<16>               {
;<15>                  + DO i1 = 0, 4 * %tgu + -1, 4   <DO_LOOP>  <MAX_TC_EST = 25000>   <LEGAL_MAX_TC = 536870911> <nounroll> <novectorize>
;<18>                  |   (<4 x i32>*)(@A)[0][i1 + <i64 0, i64 1, i64 2, i64 3>][0] = i1 + <i64 0, i64 1, i64 2, i64 3>;
;<15>                  + END LOOP
;<16>               }
;<11>
;<11>               + DO i1 = 4 * %tgu, zext.i32.i64(%t) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>   <LEGAL_MAX_TC = 3> <nounroll> <novectorize> <max_trip_count = 3>
;<4>                |   (@A)[0][i1][0] = i1;
;<11>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Prefetching ***
;
; TODO: Merged CFG rewiring uses gotos that vector CG cannot convert to proper ifs yet. Merge
; the two checks when this happens.
; MERGED-CFG:        + DO i1 = 0, %loop.ub, 4   <DO_LOOP>  <MAX_TC_EST = 25000>  <LEGAL_MAX_TC = 536870911> <auto-vectorized> <nounroll> <novectorize>
; MERGED-CFG:        |   (<4 x i32>*)(@A)[0][i1 + <i64 0, i64 1, i64 2, i64 3>][0] = i1 + <i64 0, i64 1, i64 2, i64 3>;
; MERGED-CFG:        |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][i1 + 256][0]),  1,  3,  1);
; MERGED-CFG:        |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][i1 + 257][0]),  1,  3,  1);
; MERGED-CFG:        |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][i1 + 258][0]),  1,  3,  1);
; MERGED-CFG:        |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][i1 + 259][0]),  1,  3,  1);
; MERGED-CFG:        + END LOOP

; MERGED-CFG:        + DO i1 = %lb.tmp, zext.i32.i64(%t) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 3>
; MERGED-CFG:        |   (@A)[0][i1][0] = i1;
; MERGED-CFG:        + END LOOP
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100000 x [100000 x i32]] zeroinitializer, align 16
; Function Attrs: nofree norecurse nounwind uwtable writeonly
define dso_local void @foo(i32 %t) local_unnamed_addr #0 {
entry:
  %cmp6 = icmp sgt i32 %t, 0
  br i1 %cmp6, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count8 = zext i32 %t to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx1 = getelementptr inbounds [100000 x [100000 x i32]], [100000 x [100000 x i32]]* @A, i64 0, i64 %indvars.iv, i64 0, !intel-tbaa !2
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx1, align 16, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count8
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

attributes #0 = { nofree norecurse nounwind uwtable writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA100000_A100000_i", !4, i64 0}
!4 = !{!"array@_ZTSA100000_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
