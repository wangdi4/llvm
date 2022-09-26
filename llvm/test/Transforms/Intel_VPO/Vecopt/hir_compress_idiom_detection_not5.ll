; RUN: opt %s -mattr=+avx512f,+avx512vl -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis 2>&1 | FileCheck %s

; <0>          BEGIN REGION { }
; <24>               + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
; <2>                |   %k.016 = %k.016  +  2;
; <5>                |   %0 = (%a)[i1];
; <7>                |   if (%0 != 0)
; <7>                |   {
; <13>               |      (%f)[%k.016] = %0;
; <14>               |      %k.016 = %k.016  +  1;
; <7>                |   }
; <24>               + END LOOP
; <0>          END REGION

; CHECK:      [Compress/Expand Idiom] Increment {sb:3}+1 detected: <14>         %k.016 = %k.016  +  1;
; CHECK-NEXT: [Compress/Expand Idiom] Inconsistent parent of dependency: <2>          %k.016 = %k.016  +  2;
; CHECK-NEXT: [Compress/Expand Idiom] Increment rejected: <14>         %k.016 = %k.016  +  1;
; CHECK-NEXT: Idiom List
; CHECK-NEXT:   No idioms detected.
  
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind uwtable
define dso_local noundef i32 @_Z3fooPiiS_(i32* nocapture noundef writeonly %f, i32 noundef %n, i32* nocapture noundef readonly %a) local_unnamed_addr #0 {
entry:
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %k.0.lcssa = phi i32 [ %k.0, %for.inc ]
  ret i32 %k.0.lcssa

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %k.016 = phi i32 [ 0, %entry ], [ %k.0, %for.inc ]
  %add = add nsw i32 %k.016, 2
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %tobool.not = icmp eq i32 %0, 0
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %idxprom3 = sext i32 %add to i64
  %arrayidx4 = getelementptr inbounds i32, i32* %f, i64 %idxprom3
  store i32 %0, i32* %arrayidx4, align 4
  %inc = add nsw i32 %add, 1
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %k.0 = phi i32 [ %inc, %if.then ], [ %add, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

attributes #0 = { argmemonly mustprogress nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

