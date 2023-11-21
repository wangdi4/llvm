; RUN: opt -passes=vec-clone -mtriple=x86_64 -S < %s  | FileCheck %s

define zeroext i1 @_Z3fooj(i32 %i) #0 {
; CHECK: define zeroext <16 x i8> @_ZGVbN16v__Z3fooj(<16 x i32> %i) #[[SSE2:[0-9]+]] {
; CHECK: define zeroext <16 x i8> @_ZGVcN16v__Z3fooj(<16 x i32> %i) #[[AVX1:[0-9]+]] {
; CHECK: define zeroext <16 x i8> @_ZGVdN16v__Z3fooj(<16 x i32> %i) #[[AVX2:[0-9]+]] {
; CHECK: define zeroext <16 x i8> @_ZGVeN16v__Z3fooj(<16 x i32> %i) #[[AVX512:[0-9]+]] {
;
entry:
  %cmp = icmp eq i32 %i, 97
  ret i1 %cmp
}
attributes #0 = { noinline nounwind "vector-variants"="_ZGVbN16v__Z3fooj,_ZGVcN16v__Z3fooj,_ZGVdN16v__Z3fooj,_ZGVeN16v__Z3fooj" }

; CHECK: attributes #[[SSE2]] = { noinline nounwind memory(readwrite) "may-have-openmp-directive"="true" "target-cpu"="x86-64" "target-features"="+{{.*}}" }
; CHECK: attributes #[[AVX1]] = { noinline nounwind memory(readwrite) "may-have-openmp-directive"="true" "target-cpu"="corei7-avx" "target-features"="+{{.*}}" }
; CHECK: attributes #[[AVX2]] = { noinline nounwind memory(readwrite) "may-have-openmp-directive"="true" "target-cpu"="core-avx2" "target-features"="+{{.*}}" }
; CHECK: attributes #[[AVX512]] = { noinline nounwind memory(readwrite) "may-have-openmp-directive"="true" "target-cpu"="skylake-avx512" "target-features"="+{{.*}}" }

