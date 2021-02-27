; Check if vocnflict idiom is successfully detected.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; REQUIRES: asserts
; RUN: opt -S -hir-ssa-deconstruction -hir-vec-dir-insert -enable-vconflict-idiom -debug-only=parvec-analysis < %s 2>&1 | FileCheck %s

; <15>               + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>                |   %0 = (%B)[i1];
; <6>                |   %1 = (%A)[%0];
; <7>                |   %add = %1  +  2.000000e+00;
; <8>                |   (%A)[%0] = %add;
; <15>               + END LOOP

; CHECK: [VConflict Idiom] Looking at store candidate:<[[NUM1:[0-9]+]]>          (%A)[%0] = %add;
; CHECK: [VConflict Idiom] Depends(WAR) on:<[[NUM2:[0-9]+]]>          %1 = (%A)[%0];
; CHECK: [VConflict Idiom] Detected!

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo1PfPi(float* noalias nocapture %A, i32* noalias nocapture readonly %B) local_unnamed_addr #0 {
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
  %1 = load float, float* %ptridx2, align 4
  %add = fadd fast float %1, 2.000000e+00
  store float %add, float* %ptridx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

; <19>               + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>                |   %0 = (%B)[i1];
; <6>                |   %1 = (%A)[%0];
; <7>                |   %add = %1  +  2.000000e+00;
; <8>                |   (%A)[%0] = %add;
; <10>               |   %2 = (%C)[%0];
; <12>               |   (%C)[%0] = %2 + 3;
; <19>               + END LOOP

; CHECK: [VConflict Idiom] Looking at store candidate:<[[NUM3:[0-9]+]]>          (%A)[%0] = %add;
; CHECK: [VConflict Idiom] Depends(WAR) on:<[[NUM4:[0-9]+]]>          %1 = (%A)[%0];
; CHECK: [VConflict Idiom] Detected!

; CHECK: [VConflict Idiom] Looking at store candidate:<[[NUM5:[0-9]+]]>         (%C)[%0] = %2 + 3;
; CHECK: [VConflict Idiom] Depends(WAR) on:<[[NUM6:[0-9]+]]>         %2 = (%C)[%0];
; CHECK: [VConflict Idiom] Detected!

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo2PfPiS0_(float* noalias nocapture %A, i32* noalias nocapture readonly %B, i32* noalias nocapture %C) local_unnamed_addr #0 {
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
  %1 = load float, float* %ptridx2, align 4
  %add = fadd fast float %1, 2.000000e+00
  store float %add, float* %ptridx2, align 4
  %ptridx6 = getelementptr inbounds i32, i32* %C, i64 %idxprom1
  %2 = load i32, i32* %ptridx6, align 4
  %add7 = add nsw i32 %2, 3
  store i32 %add7, i32* %ptridx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

; <16>               + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>                |   %0 = (%B)[i1];
; <7>                |   %1 = (%A)[%0 + 2];
; <8>                |   %add3 = %1  +  2.000000e+00;
; <9>                |   (%A)[%0 + 2] = %add3;
; <16>               + END LOOP

; CHECK: [VConflict Idiom] Looking at store candidate:<[[NUM7:[0-9]+]]>          (%A)[%0 + 2] = %add3;
; CHECK: [VConflict Idiom] Depends(WAR) on:<[[NUM8:[0-9]+]]>          %1 = (%A)[%0 + 2];
; CHECK: [VConflict Idiom] Detected!

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo3PfPi(float* noalias nocapture %A, i32* noalias nocapture readonly %B) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %0 = load i32, i32* %ptridx, align 4
  %add = add nsw i32 %0, 2
  %idxprom1 = sext i32 %add to i64
  %ptridx2 = getelementptr inbounds float, float* %A, i64 %idxprom1
  %1 = load float, float* %ptridx2, align 4
  %add3 = fadd fast float %1, 2.000000e+00
  store float %add3, float* %ptridx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

attributes #0 = { nofree norecurse nounwind uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
