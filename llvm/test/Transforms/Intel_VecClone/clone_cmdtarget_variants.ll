; RUN: opt -passes=vec-clone -mtriple=x86_64 -vec-clone-legalize-enabled -S < %s  | FileCheck %s

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
; CHECK:  define noundef i32 @_Z4funcPi(ptr nocapture noundef readonly [[P0:%.*]]) local_unnamed_addr #[[ORIG:.*]] {
; CHECK:  define x86_regcallcc noundef <4 x i32> @_ZGVxN4u__Z4funcPi(ptr nocapture noundef readonly [[P0]]) local_unnamed_addr #[[COREI7:.*]] {
; CHECK:  define x86_regcallcc noundef <8 x i32> @_ZGVYN8u__Z4funcPi(ptr nocapture noundef readonly [[P0]]) local_unnamed_addr #[[HSW:.*]] {
; CHECK:  define x86_regcallcc noundef <16 x i32> @_ZGVZN16u__Z4funcPi(ptr nocapture noundef readonly [[P0]]) local_unnamed_addr #[[AVX512:.*]] {
; CHECK:  define x86_regcallcc noundef <4 x i32> @_ZGVxN4l4__Z4funcPi(ptr nocapture noundef readonly [[P0]]) local_unnamed_addr #[[COREI7]] {
; CHECK:  define x86_regcallcc noundef <8 x i32> @_ZGVYN8l4__Z4funcPi(ptr nocapture noundef readonly [[P0]]) local_unnamed_addr #[[HSW]] {
; CHECK:  define x86_regcallcc noundef <16 x i32> @_ZGVZN16l4__Z4funcPi(ptr nocapture noundef readonly [[P0]]) local_unnamed_addr #[[AVX512]] {
; CHECK:  define x86_regcallcc noundef <4 x i32> @_ZGVxN4v__Z4funcPi(<2 x ptr> nocapture noundef readonly [[P00:%.*]], <2 x ptr> nocapture noundef readonly [[P01:%.*]]) local_unnamed_addr #[[COREI7]] {
; CHECK:  define x86_regcallcc noundef <8 x i32> @_ZGVYN8v__Z4funcPi(<4 x ptr> nocapture noundef readonly [[P00]], <4 x ptr> nocapture noundef readonly [[P01]]) local_unnamed_addr #[[HSW]] {
; CHECK:  define x86_regcallcc noundef <16 x i32> @_ZGVZN16v__Z4funcPi(<8 x ptr> nocapture noundef readonly [[P00]], <8 x ptr> nocapture noundef readonly [[P01]]) local_unnamed_addr #[[DISP:.*]] !llvm.vec.auto.cpu.dispatch ![[MD1:.*]] {
;
entry:
  %0 = load i32, ptr %p, align 4, !tbaa !0
  %add = shl nsw i32 %0, 1
  ret i32 %add
}

; CHECK:  attributes #[[ORIG]] = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7" "target-features"="+crc32,+cx16,+cx8,+fxsr,+cmov,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="true" "vector-variants"="_ZGVxN4u__Z4funcPi,_ZGVYN8u__Z4funcPi,_ZGVZN16u__Z4funcPi,_ZGVxN4l4__Z4funcPi,_ZGVYN8l4__Z4funcPi,_ZGVZN16l4__Z4funcPi,_ZGVxN4v__Z4funcPi,_ZGVYN8v__Z4funcPi,_ZGVZN16v__Z4funcPi" }
; CHECK:  attributes #[[COREI7]] = { mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core_i7_sse4_2" "target-features"="+cmov,+mmx,+popcnt,+sse,+sse2,+sse3,+ssse3,+sse4.1,+sse4.2,+cx16,+cx8,+crc32,+fxsr,+sahf,+x87" "tune-cpu"="core_i7_sse4_2" "unsafe-fp-math"="true" }
; CHECK:  attributes #[[HSW]] = { mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="haswell" "target-features"="+cmov,+mmx,+popcnt,+sse,+sse2,+sse3,+ssse3,+sse4.1,+sse4.2,+avx,+avx2,+fma,+bmi,+bmi2,+pclmul,+cx16,+cx8,+crc32,+f16c,+fsgsbase,+fxsr,+invpcid,+lzcnt,+movbe,+rdrnd,+sahf,+x87,+xsave,+xsaveopt" "tune-cpu"="haswell" "unsafe-fp-math"="true" }
; CHECK:  attributes #[[AVX512]] = { mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake_avx512" "target-features"="+cmov,+mmx,+popcnt,+sse,+sse2,+sse3,+ssse3,+sse4.1,+sse4.2,+avx,+avx2,+fma,+avx512f,+bmi,+bmi2,+aes,+pclmul,+avx512vl,+avx512bw,+avx512dq,+avx512cd,+adx,+clflushopt,+clwb,+cx16,+cx8,+crc32,+f16c,+fsgsbase,+fxsr,+invpcid,+lzcnt,+movbe,+pku,+prfchw,+rdrnd,+rdseed,+sahf,+x87,+xsave,+xsavec,+xsaveopt,+xsaves,+evex512" "tune-cpu"="skylake_avx512" "unsafe-fp-math"="true" }
; CHECK:  attributes #[[DISP]] = { mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7" "target-features"="+crc32,+cx16,+cx8,+fxsr,+cmov,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="true" }
;
attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7" "target-features"="+crc32,+cx16,+cx8,+fxsr,+cmov,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="true" "vector-dispatch"="_ZGVxN4u__Z4funcPi:core_i7_sse4_2;_ZGVYN8u__Z4funcPi:haswell;_ZGVZN16u__Z4funcPi:skylake_avx512;_ZGVxN4l4__Z4funcPi:core_i7_sse4_2;_ZGVYN8l4__Z4funcPi:haswell;_ZGVZN16l4__Z4funcPi:skylake_avx512;_ZGVxN4v__Z4funcPi:core_i7_sse4_2;_ZGVYN8v__Z4funcPi:haswell;_ZGVZN16v__Z4funcPi:skylake_avx512,tigerlake" "vector-variants"="_ZGVxN4u__Z4funcPi,_ZGVYN8u__Z4funcPi,_ZGVZN16u__Z4funcPi,_ZGVxN4l4__Z4funcPi,_ZGVYN8l4__Z4funcPi,_ZGVZN16l4__Z4funcPi,_ZGVxN4v__Z4funcPi,_ZGVYN8v__Z4funcPi,_ZGVZN16v__Z4funcPi" }

!0 = !{!1, !1, i64 0}
!1 = !{!"int", !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C++ TBAA"}
; CHECK:  ![[MD1]] = !{!"skylake_avx512", !"tigerlake"}
