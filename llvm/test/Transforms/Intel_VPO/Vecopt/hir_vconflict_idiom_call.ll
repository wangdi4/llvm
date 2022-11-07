; Check if vconflict idiom recognition bails-out at the right places.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; REQUIRES: asserts
; RUN: opt -S -hir-ssa-deconstruction -hir-cost-model-throttling=0 -hir-vec-dir-insert -debug-only=parvec-analysis -tbaa < %s 2>&1 | FileCheck %s
declare void @foo(float * ) #1

;<0>          BEGIN REGION { }
;<16>               + DO i1 = 0, 1023, 1   <DO_LOOP>
;<3>                |   %0 = (%B)[i1];
;<6>                |   @foo(&((%A)[%0]));
;<7>                |   %conv = sitofp.i64.float(i1);
;<8>                |   %add = %conv  +  2.000000e+00;
;<9>                |   (%A)[%0] = %add;
;<16>               + END LOOP
;<0>          END REGION

; CHECK: [VConflict Idiom] Looking at store candidate:<[[NUM1:[0-9]+]]>          (%A)[%0] = %add;
; CHECK: [VConflict Idiom] Depends(WAR) on:<[[NUM2:[0-9]+]]>          @foo(&((%A)[%0]));
; CHECK: [VConflict Idiom] Skipped: Sink ref is fake ref.

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo15PiS_S_(float* noalias nocapture %A, i32* noalias nocapture readonly %B, i32* nocapture %C) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %0 = load i32, i32* %ptridx, align 4
  %idxprom1 = sext i32 %0 to i64
  %ptridx2 = getelementptr inbounds float, float* %A, i64 %idxprom1
  call void @foo(float* nonnull %ptridx2)
  %conv = sitofp i64 %indvars.iv to float
  %add = fadd float %conv, 2.
  store float %add, float* %ptridx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

attributes #0 = { nofree norecurse nosync nounwind uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { memory(read) }
