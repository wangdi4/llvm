; Test to check gather-to-permute bails out on array of size 1.
;
; RUN: llc -o /dev/null -enable-intel-advanced-opts=true -O3 -print-after=x86-gather-to-load-permute %s 2>&1 | FileCheck %s

; CHECK: call <2 x i32> @llvm.masked.gather

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.s_descr = type { i32, %struct.s_descr*, [1 x i32] }

; Function Attrs: noinline nounwind uwtable
define dso_local void @sum(%struct.s_descr* nocapture noundef readonly %DescrList, <2 x i32>* noundef %num, i32 noundef %size) #0 {
entry:
  %broadcast.splatinsert = insertelement <2 x i32> poison, i32 %size, i64 0
  %broadcast.splat = shufflevector <2 x i32> %broadcast.splatinsert, <2 x i32> poison, <2 x i32> zeroinitializer
  %ndx = add <2 x i32> %broadcast.splat,  <i32 -1, i32 -1>
  %ndx64 = sext <2 x i32> %ndx to <2 x i64>
  %mm_vectorGEP = getelementptr inbounds %struct.s_descr, %struct.s_descr* %DescrList, i64 0, i32 2, <2 x i64> %ndx64
  %wide.masked.gather = call <2 x i32> @llvm.masked.gather.v2i32.v2p0i32(<2 x i32*> %mm_vectorGEP, i32 4, <2 x i1> <i1 true, i1 true>, <2 x i32> undef)

  store <2 x i32> %wide.masked.gather, <2 x i32>* %num, align 8
  ret void
}
attributes #0 = { noinline nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

; Function Attrs: nocallback nofree nosync nounwind readonly willreturn
declare <2 x i32> @llvm.masked.gather.v2i32.v2p0i32(<2 x i32*>, i32 immarg, <2 x i1>, <2 x i32>)

