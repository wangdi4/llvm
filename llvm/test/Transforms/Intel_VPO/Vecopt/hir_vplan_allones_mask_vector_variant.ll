; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4 < %s 2>&1 | FileCheck %s

; CHECK:      %sext = sext.<4 x i1>.<4 x i32>(1);
; CHECK-NEXT: %_ZGVbM4vvv__Z3bariii = @_ZGVbM4vvv__Z3bariii(%a,  i1 + <i64 0, i64 1, i64 2, i64 3>,  3,  %sext);

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare dso_local noundef i32 @_Z3bariii(i32 noundef, i32 noundef, i32 noundef) #1
declare dso_local noundef <4 x i32> @_ZGVbM4vvv__Z3bariii(<4 x i32> noundef, <4 x i32> noundef, <4 x i32> noundef, <4 x i32>) local_unnamed_addr

; Function Attrs: mustprogress noinline uwtable
define dso_local void @_Z3fooiPi(i32 noundef %a, ptr nocapture noundef writeonly %r) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  %call = tail call noundef i32 @_Z3bariii(i32 noundef %a, i32 noundef %0, i32 noundef 3)
  %arrayidx = getelementptr inbounds i32, ptr %r, i64 %indvars.iv
  store i32 %call, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 300
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { "target-features"="+avx512f" }
attributes #1 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "vector-variants"="_ZGVbM4vvv__Z3bariii,_ZGVcM8vvv__Z3bariii,_ZGVdM8vvv__Z3bariii,_ZGVeM16vvv__Z3bariii" }
