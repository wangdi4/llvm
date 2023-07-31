; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4 < %s 2>&1 | FileCheck %s --check-prefix=CHECK-CTYPE-MASK
; RUN: opt -use-i1-mask-for-simd-funcs -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4 < %s 2>&1 | FileCheck %s --check-prefix=CHECK-I1-MASK

; CHECK-CTYPE-MASK:      %.vec = -1 * i1 + -1 * <i1 false, i1 true, i1 false, i1 true> != 0;
; CHECK-NEXT-CTYPE-MASK: %sext = sext.<4 x i1>.<4 x i64>(%.vec);
; CHECK-NEXT-CTYPE-MASK: @_ZGVdM4vlu__Z26ice_deposition_sublimationPdii(&((<4 x ptr>)(%t)[1000 * i1 + 1000 * <i64 0, i64 1, i64 2, i64 3>]),  i1,  3,  %sext)

; CHECK-I1-MASK:      %.vec = -1 * i1 + -1 * <i1 false, i1 true, i1 false, i1 true> != 0;
; CHECK-NEXT-I1-MASK: @_ZGVdM4vlu__Z26ice_deposition_sublimationPdii(&((<4 x ptr>)(%t)[1000 * i1 + 1000 * <i64 0, i64 1, i64 2, i64 3>]),  i1,  3,  %.vec)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare dso_local void @_Z26ice_deposition_sublimationPdii(ptr noundef, i32 noundef, i32 noundef) local_unnamed_addr #1

; Function Attrs: mustprogress uwtable
define dso_local void @_Z3subPd(ptr noundef %t) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %0 = trunc i64 %indvars.iv to i32
  %rem = and i32 %0, 1
  %tobool.not = icmp eq i32 %rem, 0
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %mul = mul nuw nsw i64 %indvars.iv, 1000
  %add.ptr = getelementptr inbounds double, ptr %t, i64 %mul
  tail call void @_Z26ice_deposition_sublimationPdii(ptr noundef %add.ptr, i32 noundef %0, i32 noundef 3)
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 300
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}

attributes #0 = { "target-features"="+avx512f" }
attributes #1 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "vector-variants"="_ZGVbN2vlu__Z26ice_deposition_sublimationPdii,_ZGVcN4vlu__Z26ice_deposition_sublimationPdii,_ZGVdN4vlu__Z26ice_deposition_sublimationPdii,_ZGVeN8vlu__Z26ice_deposition_sublimationPdii,_ZGVbM2vlu__Z26ice_deposition_sublimationPdii,_ZGVcM4vlu__Z26ice_deposition_sublimationPdii,_ZGVdM4vlu__Z26ice_deposition_sublimationPdii,_ZGVeM8vlu__Z26ice_deposition_sublimationPdii" }
