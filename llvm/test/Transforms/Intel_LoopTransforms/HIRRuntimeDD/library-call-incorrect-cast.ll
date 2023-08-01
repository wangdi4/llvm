; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that we are able to successfully multiversion the loop using library call.
; It was compfailing during type normalization because element type was not set correctly.

; + DO i1 = 0, smax(1, %n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; |   (%AC)[i1 + 1] = 0.000000e+00;
; |   (%K)[i1 + 1] = 1.000000e+00;
; |   (%AD)[i1 + 1] = 0.000000e+00;
; |   %ld1 = (%ptr2)[i1 + 1];
; |   %ld2 = (%cptr)[i1 + 1].0;
; |   if (%ld1 > %ld2)
; |   {
; |      (%var2)[0] = %ld2;
; |   }
; |   %ld3 = (%ptr1)[0];
; |   (%var1)[0] = %ld3;
; |   (%M)[0] = i1 + 2;
; + END LOOP


; CHECK: __intel_rtdd_indep


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%complex_64bit = type { float, float }

define void @foo(ptr %AC, ptr %K, ptr %AD, i32 %n, ptr %ptr1, ptr %ptr2, ptr %cptr, ptr %M, ptr %var1, ptr %var2) #0 {
entry:
  br label %loop

loop:                                             ; preds = %entry, %latch
  %iv = phi i32 [ 1, %entry ], [ %add.11, %latch ]
  %int_sext73 = zext i32 %iv to i64
  %sub1 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %AC, i64 %int_sext73)
  store float 0.000000e+00, ptr %sub1, align 1
  %sub2 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %K, i64 %int_sext73)
  store float 1.000000e+00, ptr %sub2, align 1
  %sub3 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %AD, i64 %int_sext73)
  store float 0.000000e+00, ptr %sub3, align 1
  %sub4 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %ptr2, i64 %int_sext73)
  %ld1 = load float, ptr %sub4, align 1
  %sub5 = call ptr @llvm.intel.subscript.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(%complex_64bit) %cptr, i64 %int_sext73)
  %ld2 = load float, ptr %sub5, align 1
  %rel.20 = fcmp reassoc ninf nsz arcp contract afn ogt float %ld1, %ld2
  br i1 %rel.20, label %bb_new29_then, label %latch

bb_new29_then:                                    ; preds = %loop
  store float %ld2, ptr %var2, align 1
  br label %latch

latch:                                       ; preds = %bb_new29_then, %loop
  %ld3 = load float, ptr %ptr1, align 1
  store float %ld3, ptr %var1, align 1
  %add.11 = add nuw nsw i32 %iv, 1
  store i32 %add.11, ptr %M, align 8
  %rel.21.not.not = icmp slt i32 %iv, %n
  br i1 %rel.21.not.not, label %loop, label %bb27_endif.loopexit

bb27_endif.loopexit:
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { noreturn nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nounwind readnone speculatable }

