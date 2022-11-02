
; Test to check that we don't crash on in-memory linear which needs a close form when
; we generate masked remainder.
; RUN: opt -disable-output -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -vplan-masked-remainder-gain-threshold=0 -vplan-enable-peel-rem-strip=0 %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

define hidden void @_ZN9LAMMPS_NS21ImproperHarmonicIntel4evalILi0ELi1ELi0EfdEEviPNS_12IntelBuffersIT2_T3_EERKNS0_10ForceConstIS3_EE.DIR.OMP.PARALLEL.2(i32 %N) #1 {
; CHECK:             + DO i1 = 0, [[LOOP_UB0:%.*]], 16   <DO_LOOP> <simd-vectorized> <nounroll> <novectorize>
; CHECK:             + DO i1 = [[PHI_TEMP0:%.*]], [[LOOP_UB200:%.*]], 16   <DO_LOOP>  <MAX_TC_EST = 1>  <LEGAL_MAX_TC = 1> <vector-remainder> <nounroll> <novectorize> <max_trip_count = 1>
; CHECK:             |   [[DOTVEC210:%.*]] = i1 + <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15> <u [[N0:%.*]];
newFuncRoot:
  %n.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %newFuncRoot
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %n.linear.iv, i32 1) ]
  br label %DIR.OMP.ORDERED.2351

DIR.OMP.ORDERED.2351:                             ; preds = %DIR.OMP.ORDERED.2351, %DIR.OMP.SIMD.1
  %.omp.iv.local.0720 = phi i32 [ 0, %DIR.OMP.SIMD.1 ], [ %add438, %DIR.OMP.ORDERED.2351 ]
  store i32 %.omp.iv.local.0720, i32* %n.linear.iv, align 4
  %add438 = add nuw i32 %.omp.iv.local.0720, 1
  %exitcond.not = icmp eq i32 %add438, %N
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.7, label %DIR.OMP.ORDERED.2351

DIR.OMP.END.SIMD.7:                               ; preds = %DIR.OMP.ORDERED.2351
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  unreachable
}

attributes #0 = { nounwind }
attributes #1 = { "target-features"="+64bit,+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+avx512vnni,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+rtm,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves,-amx-bf16,-amx-int8,-amx-tile,-avx512bf16,-avx512bitalg,-avx512er,-avx512fp16,-avx512ifma,-avx512pf,-avx512vbmi,-avx512vbmi2,-avx512vp2intersect,-avx512vpopcntdq,-avxvnni,-cldemote,-clzero,-enqcmd,-fma4,-gfni,-hreset,-kl,-lwp,-movdir64b,-movdiri,-mwaitx,-pconfig,-prefetchwt1,-ptwrite,-rdpid,-rdpru,-serialize,-sgx,-sha,-shstk,-sse4a,-tbm,-tsxldtrk,-uintr,-vaes,-vpclmulqdq,-waitpkg,-wbnoinvd,-widekl,-xop" }

!nvvm.annotations = !{}
