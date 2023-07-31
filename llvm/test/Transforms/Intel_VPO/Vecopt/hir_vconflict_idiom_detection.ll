; Check if vconflict idiom is successfully detected.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; REQUIRES: asserts
; RUN: opt -S -mattr=+avx512vl,+avx512cd -passes=hir-ssa-deconstruction,hir-vec-dir-insert -debug-only=parvec-analysis < %s 2>&1 | FileCheck %s --check-prefix=CHECK-VCONFLICT

; RUN: opt -S -mattr=+avx2 -passes=hir-ssa-deconstruction,hir-vec-dir-insert -debug-only=parvec-analysis < %s 2>&1 | FileCheck %s --check-prefix=CHECK-NO-VCONFLICT

; <15>               + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>                |   %0 = (%B)[i1];
; <6>                |   %1 = (%A)[%0];
; <7>                |   %add = %1  +  2.000000e+00;
; <8>                |   (%A)[%0] = %add;
; <15>               + END LOOP

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM1:[0-9]+]]>          (%A)[%0] = %add;
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM2:[0-9]+]]>          %1 = (%A)[%0];
; CHECK-VCONFLICT: [VConflict Idiom] Detected, legality pending further dependence checking!

; CHECK-NO-VCONFLICT: No idioms detected.

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo1PfPi(ptr noalias nocapture %A, ptr noalias nocapture readonly %B) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %0 = load i32, ptr %ptridx, align 4
  %idxprom1 = sext i32 %0 to i64
  %ptridx2 = getelementptr inbounds float, ptr %A, i64 %idxprom1
  %1 = load float, ptr %ptridx2, align 4
  %add = fadd fast float %1, 2.000000e+00
  store float %add, ptr %ptridx2, align 4
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

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM3:[0-9]+]]>          (%A)[%0] = %add;
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM4:[0-9]+]]>          %1 = (%A)[%0];
; CHECK-VCONFLICT: [VConflict Idiom] Detected, legality pending further dependence checking!

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM5:[0-9]+]]>         (%C)[%0] = %2 + 3;
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM6:[0-9]+]]>         %2 = (%C)[%0];
; CHECK-VCONFLICT: [VConflict Idiom] Detected, legality pending further dependence checking!

; CHECK-NO-VCONFLICT: No idioms detected.

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo2PfPiS0_(ptr noalias nocapture %A, ptr noalias nocapture readonly %B, ptr noalias nocapture %C) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %0 = load i32, ptr %ptridx, align 4
  %idxprom1 = sext i32 %0 to i64
  %ptridx2 = getelementptr inbounds float, ptr %A, i64 %idxprom1
  %1 = load float, ptr %ptridx2, align 4
  %add = fadd fast float %1, 2.000000e+00
  store float %add, ptr %ptridx2, align 4
  %ptridx6 = getelementptr inbounds i32, ptr %C, i64 %idxprom1
  %2 = load i32, ptr %ptridx6, align 4
  %add7 = add nsw i32 %2, 3
  store i32 %add7, ptr %ptridx6, align 4
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

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM7:[0-9]+]]>          (%A)[sext.i32.i64(%0) + 2] = %add3;
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM8:[0-9]+]]>          %1 = (%A)[sext.i32.i64(%0) + 2];
; CHECK-VCONFLICT: [VConflict Idiom] Detected, legality pending further dependence checking!

; CHECK-NO-VCONFLICT: No idioms detected.

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo3PfPi(ptr noalias nocapture %A, ptr noalias nocapture readonly %B) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %0 = load i32, ptr %ptridx, align 4
  %add = add nsw i32 %0, 2
  %idxprom1 = sext i32 %add to i64
  %ptridx2 = getelementptr inbounds float, ptr %A, i64 %idxprom1
  %1 = load float, ptr %ptridx2, align 4
  %add3 = fadd fast float %1, 2.000000e+00
  store float %add3, ptr %ptridx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

; <20>         + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295> <nounroll>
; <3>          |   %1 = (%B)[i1];
; <6>          |   %2 = (%A)[%1];
; <8>          |   (%E)[i1] = %0;
; <12>         |   %0 = (%D)[i1]  +  %0;
; <13>         |   (%A)[%1] = %2 + 1;
; <20>         + END LOOP

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM9:[0-9]+]]>          (%E)[i1] = %0;
; CHECK-VCONFLICT: [VConflict Idiom] Skipped: Store memory ref is linear
; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM10:[0-9]+]]>         (%A)[%1] = %2 + 1;
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM11:[0-9]+]]>          %2 = (%A)[%1];
; CHECK-VCONFLICT: [VConflict Idiom] Detected, legality pending further dependence checking!

; CHECK-NO-VCONFLICT: No idioms detected.

; Function Attrs: nofree norecurse nosync nounwind uwtable mustprogress
define dso_local void @_Z3foo4PiS_S_S_S_i(ptr noalias nocapture %A, ptr noalias nocapture readonly %B, ptr noalias nocapture %p, ptr noalias nocapture readonly %D, ptr noalias nocapture %E, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp25 = icmp sgt i32 %N, 0
  br i1 %cmp25, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %p.promoted = load i32, ptr %p, align 4
  %wide.trip.count27 = zext i32 %N to i64
  br label %for.body

for.cond.for.cond.cleanup_crit_edge:              ; preds = %for.body
  %add.lcssa = phi i32 [ %add, %for.body ]
  store i32 %add.lcssa, ptr %p, align 4
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.for.cond.cleanup_crit_edge, %entry
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %0 = phi i32 [ %p.promoted, %for.body.lr.ph ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 4
  %idxprom1 = sext i32 %1 to i64
  %arrayidx2 = getelementptr inbounds i32, ptr %A, i64 %idxprom1
  %2 = load i32, ptr %arrayidx2, align 4
  %arrayidx4 = getelementptr inbounds i32, ptr %E, i64 %indvars.iv
  store i32 %0, ptr %arrayidx4, align 4
  %inc = add nsw i32 %2, 1
  %arrayidx6 = getelementptr inbounds i32, ptr %D, i64 %indvars.iv
  %3 = load i32, ptr %arrayidx6, align 4
  %add = add nsw i32 %3, %0
  store i32 %inc, ptr %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count27
  br i1 %exitcond.not, label %for.cond.for.cond.cleanup_crit_edge, label %for.body
}

; <22>               + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>                |   %0 = (%B)[i1];
; <6>                |   %1 = (%A)[%0];
; <7>                |   %add = %1  +  2.000000e+00;
; <8>                |   (%A)[%0] = %add;
; <10>               |   %2 = (%C)[i1];
; <13>               |   %3 = (%A)[%2];
; <14>               |   %add9 = %3  +  3.000000e+00;
; <15>               |   (%A)[%2] = %add9;
; <22>               + END LOOP

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM9:[0-9]+]]>          (%A)[%0] = %add;
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM9:[0-9]+]]>          %1 = (%A)[%0];
; CHECK-VCONFLICT: [VConflict Idiom] Detected, legality pending further dependence checking!
; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM9:[0-9]+]]>         (%A)[%2] = %add9;
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM9:[0-9]+]]>         %3 = (%A)[%2];
; CHECK-VCONFLICT: [VConflict Idiom] Detected, legality pending further dependence checking!

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo7PfPiS0_(ptr noalias nocapture %A, ptr noalias nocapture readonly %B, ptr noalias nocapture readonly %C) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %0 = load i32, ptr %ptridx, align 4
  %idxprom1 = sext i32 %0 to i64
  %ptridx2 = getelementptr inbounds float, ptr %A, i64 %idxprom1
  %1 = load float, ptr %ptridx2, align 4
  %add = fadd fast float %1, 2.000000e+00
  store float %add, ptr %ptridx2, align 4
  %ptridx6 = getelementptr inbounds i32, ptr %C, i64 %indvars.iv
  %2 = load i32, ptr %ptridx6, align 4
  %idxprom7 = sext i32 %2 to i64
  %ptridx8 = getelementptr inbounds float, ptr %A, i64 %idxprom7
  %3 = load float, ptr %ptridx8, align 4
  %add9 = fadd fast float %3, 3.000000e+00
  store float %add9, ptr %ptridx8, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

attributes #0 = { nofree norecurse nounwind uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
