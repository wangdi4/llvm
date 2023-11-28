; RUN: opt -passes='vec-clone,print<inline-report>' -mtriple=x86_64 -vec-clone-legalize-enabled -inline-report=0xf847 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup,vec-clone,inlinereportemitter' -mtriple=x86_64 -vec-clone-legalize-enabled -inline-report=0xf8c6 -disable-output < %s 2>&1 | FileCheck %s

; Check the inlining report after vec cloning. This test is based on
; clone_cmdtarget_variants.ll.

; CHECK: COMPILE FUNC: _Z4funcPi
; CHECK: COMPILE FUNC: _ZGVxN4u__Z4funcPi
; CHECK: llvm.directive.region.entry
; CHECK: llvm.directive.region.exit
; CHECK: COMPILE FUNC: _ZGVYN8u__Z4funcPi
; CHECK: llvm.directive.region.entry
; CHECK: llvm.directive.region.exit
; CHECK: COMPILE FUNC: _ZGVZN16u__Z4funcPi
; CHECK: llvm.directive.region.entry
; CHECK: llvm.directive.region.exit
; CHECK: COMPILE FUNC: _ZGVxN4l4__Z4funcPi
; CHECK: llvm.directive.region.entry
; CHECK: llvm.directive.region.exit
; CHECK: COMPILE FUNC: _ZGVYN8l4__Z4funcPi
; CHECK: llvm.directive.region.entry
; CHECK: llvm.directive.region.exit
; CHECK: COMPILE FUNC: _ZGVZN16l4__Z4funcPi
; CHECK: llvm.directive.region.entry
; CHECK: llvm.directive.region.exit
; CHECK: COMPILE FUNC: _ZGVxN4v__Z4funcPi
; CHECK: llvm.directive.region.entry
; CHECK: llvm.directive.region.exit
; CHECK: COMPILE FUNC: _ZGVYN8v__Z4funcPi
; CHECK: llvm.directive.region.entry
; CHECK: llvm.directive.region.exit
; CHECK: COMPILE FUNC: _ZGVZN16v__Z4funcPi
; CHECK: llvm.directive.region.entry
; CHECK: llvm.directive.region.exit

; Test to check vec-clone vector variants specialization for -vecabi=cmdtarget
; LLVM IR produced from source:
;#pragma omp declare simd ompx_processor(skylake_avx512) notinbranch
;#pragma omp declare simd ompx_processor(tigerlake) notinbranch
;#pragma omp declare simd linear(p) notinbranch
;#pragma omp declare simd uniform(p) notinbranch
;int func(int* p)
;{
;  return *p+*p;
;}


; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define noundef i32 @_Z4funcPi(ptr nocapture noundef readonly %p) local_unnamed_addr #0 {
entry:
  %0 = load i32, ptr %p, align 4, !tbaa !0
  %add = shl nsw i32 %0, 1
  ret i32 %add
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7" "target-features"="+crc32,+cx16,+cx8,+fxsr,+cmov,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="true" "vector-dispatch"="_ZGVxN4u__Z4funcPi:core_i7_sse4_2;_ZGVYN8u__Z4funcPi:haswell;_ZGVZN16u__Z4funcPi:skylake_avx512;_ZGVxN4l4__Z4funcPi:core_i7_sse4_2;_ZGVYN8l4__Z4funcPi:haswell;_ZGVZN16l4__Z4funcPi:skylake_avx512;_ZGVxN4v__Z4funcPi:core_i7_sse4_2;_ZGVYN8v__Z4funcPi:haswell;_ZGVZN16v__Z4funcPi:skylake_avx512,tigerlake" "vector-variants"="_ZGVxN4u__Z4funcPi,_ZGVYN8u__Z4funcPi,_ZGVZN16u__Z4funcPi,_ZGVxN4l4__Z4funcPi,_ZGVYN8l4__Z4funcPi,_ZGVZN16l4__Z4funcPi,_ZGVxN4v__Z4funcPi,_ZGVYN8v__Z4funcPi,_ZGVZN16v__Z4funcPi" }

!0 = !{!1, !1, i64 0}
!1 = !{!"int", !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C++ TBAA"}
