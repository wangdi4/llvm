;Test the prefetching pragma info is in the main loop rather than in the remainder loop
;
; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -hir-prefetching -hir-prefetching-num-cachelines-threshold=64 -hir-prefetching-skip-non-modified-regions=false -hir-prefetching-skip-num-memory-streams-check=true -hir-prefetching-skip-AVX2-check=true -vplan-force-vf=4 -print-after=hir-prefetching -hir-details -disable-output < %s 2>&1 | FileCheck %s --check-prefix=MERGED-CFG
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,hir-prefetching,print<hir>" -hir-prefetching-num-cachelines-threshold=64 -hir-prefetching-skip-non-modified-regions=false -hir-prefetching-skip-num-memory-streams-check=true -hir-prefetching-skip-AVX2-check=true -vplan-force-vf=4 -hir-details -disable-output < %s 2>&1 | FileCheck %s --check-prefix=MERGED-CFG
;
;*** IR Dump Before HIR Prefetching ***
;Function: sub
;
;<0>          BEGIN REGION { modified }
;<4>                if (%N > 0)
;<4>                {
;<30>                  %tgu = (zext.i32.i64(%N))/u4;
;<32>                  if (0 <u 4 * %tgu)
;<32>                  {
;<31>                     + DO i1 = 0, 4 * %tgu + -1, 4   <DO_LOOP>  <MAX_TC_EST = 2500>  <LEGAL_MAX_TC = 536870911> <auto-vectorized> <nounroll> <novectorize>
;<34>                     |   %.vec = sitofp.<4 x i32>.<4 x float>(i1 + <i64 0, i64 1, i64 2, i64 3>);
;<35>                     |   (<4 x float>*)(@A)[0][i1 + <i64 0, i64 1, i64 2, i64 3>][0] = %.vec;
;<31>                     + END LOOP
;<32>                  }
;<27>
;<27>                  + DO i1 = 4 * %tgu, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>   <LEGAL_MAX_TC = 3> <nounroll> <novectorize> <max_trip_count = 3>
;<13>                  |   %conv = sitofp.i32.float(i1);
;<15>                  |   (@A)[0][i1][0] = %conv;
;<27>                  + END LOOP
;<4>                }
;<26>               ret ;
;<0>          END REGION
;
; *** IR Dump After HIR Prefetching ***
;Function: sub
;
; TODO: Merged CFG rewiring uses gotos that vector CG cannot convert to proper ifs yet. Merge
; the two checks when this happens.
; MERGED-CFG:         + Loop metadata: !llvm.loop
; MERGED-CFG:         + Prefetching directives:{&((@A)[0]):1:40}
; MERGED-CFG:         + DO i64 i1 = 0, %loop.ub, 4   <DO_LOOP>  <MAX_TC_EST = 2500>  <LEGAL_MAX_TC = 536870911> <auto-vectorized> <nounroll> <novectorize>
; MERGED-CFG:         |   %.vec4 = sitofp.<4 x i32>.<4 x float>(i1 + <i64 0, i64 1, i64 2, i64 3>);
; MERGED-CFG:         |   (<4 x float>*)(@A)[0][i1 + <i64 0, i64 1, i64 2, i64 3>][0] = %.vec4;
; MERGED-CFG:         |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][i1 + 160][0]),  0,  2,  1);
; MERGED-CFG:         |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][i1 + 161][0]),  0,  2,  1);
; MERGED-CFG:         |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][i1 + 162][0]),  0,  2,  1);
; MERGED-CFG:         |   @llvm.prefetch.p0i8(&((i8*)(@A)[0][i1 + 163][0]),  0,  2,  1);
; MERGED-CFG:         + END LOOP

; MERGED-CFG:         + Loop metadata: !llvm.loop
; MERGED-CFG:         + DO i64 i1 = %lb.tmp, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 3>
; MERGED-CFG:         |   %conv = sitofp.i32.float(i1);
; MERGED-CFG:         |   (@A)[0][i1][0] = %conv;
; MERGED-CFG:         + END LOOP
;
;
;Module Before HIR
; ModuleID = 't1.c'
source_filename = "t1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local global [10000 x [10000 x float]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @sub(i32* nocapture readnone %M, i32 %N) local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"([10000 x [10000 x float]]* @A), "QUAL.PRAGMA.HINT"(i32 1), "QUAL.PRAGMA.DISTANCE"(i32 40) ]
  %cmp6 = icmp sgt i32 %N, 0
  br i1 %cmp6, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count8 = zext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %1 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %1 to float
  %arrayidx1 = getelementptr inbounds [10000 x [10000 x float]], [10000 x [10000 x float]]* @A, i64 0, i64 %indvars.iv, i64 0, !intel-tbaa !2
  store float %conv, float* %arrayidx1, align 16, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count8
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !8

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

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="broadwell" "target-features"="+64bit,+adx,+aes,+avx,+avx2,+bmi,+bmi2,+cmov,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+prfchw,+rdrnd,+rdseed,+rtm,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt,-amx-bf16,-amx-int8,-amx-tile,-avx512bf16,-avx512bitalg,-avx512bw,-avx512cd,-avx512dq,-avx512er,-avx512f,-avx512ifma,-avx512pf,-avx512vbmi,-avx512vbmi2,-avx512vl,-avx512vnni,-avx512vp2intersect,-avx512vpopcntdq,-avxvnni,-cldemote,-clflushopt,-clwb,-clzero,-enqcmd,-fma4,-gfni,-hreset,-kl,-lwp,-movdir64b,-movdiri,-mwaitx,-pconfig,-pku,-prefetchwt1,-ptwrite,-rdpid,-serialize,-sgx,-sha,-shstk,-sse4a,-tbm,-tsxldtrk,-uintr,-vaes,-vpclmulqdq,-waitpkg,-wbnoinvd,-widekl,-xop,-xsavec,-xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (2021.2.0.YYYYMMDD)"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA10000_A10000_f", !4, i64 0}
!4 = !{!"array@_ZTSA10000_f", !5, i64 0}
!5 = !{!"float", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
