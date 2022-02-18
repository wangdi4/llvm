; Check if vconflict idiom recognition bails-out at the right places.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -S -mattr=+avx512vl,+avx512cd -hir-ssa-deconstruction -analyze -hir-parvec-analysis -debug-only=parvec-analysis < %s 2>&1 | FileCheck %s --check-prefix=CHECK-VCONFLICT
; RUN: opt -enable-new-pm=0 -S -mattr=+avx2 -hir-ssa-deconstruction -analyze -hir-parvec-analysis -debug-only=parvec-analysis < %s 2>&1 | FileCheck %s --check-prefix=CHECK-NO-VCONFLICT

; RUN: opt  -S -mattr=+avx512vl,+avx512cd -passes='hir-ssa-deconstruction,print<hir-parvec-analysis>'  -debug-only=parvec-analysis < %s 2>&1 | FileCheck %s --check-prefix=CHECK-VCONFLICT
; RUN: opt  -S -mattr=+avx2 -passes='hir-ssa-deconstruction,print<hir-parvec-analysis>' -debug-only=parvec-analysis < %s 2>&1 | FileCheck %s --check-prefix=CHECK-NO-VCONFLICT

; CHECK-VCONFLICT: Idiom List
; CHECK-VCONFLICT: No idioms detected.

; CHECK-NO-VCONFLICT: No idioms detected.

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z3foo1PfPi(float* nocapture %A, i32* nocapture readonly %B) local_unnamed_addr #0 {
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
  %add = fadd fast float %1, 1.000000e+00
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; <19>               + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>                |   %0 = (%B)[i1];
; <6>                |   %1 = (%A)[%0];
; <8>                |   (%A)[%0] = %1 + 2;
; <10>               |   %2 = (%C)[%0];
; <12>               |   (%C)[%0] = %2 + 3;
; <19>               + END LOOP

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM1:[0-9]+]]>          (%A)[%0] = %1 + 2;
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM2:[0-9]+]]>          %0 = (%B)[i1];
; CHECK-VCONFLICT: [VConflict Idiom] Skipped: Wrong memory dependency.

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM3:[0-9]+]]>         (%C)[%0] = %2 + 3;
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM4:[0-9]+]]>          %0 = (%B)[i1];
; CHECK-VCONFLICT: [VConflict Idiom] Skipped: Wrong memory dependency.

; CHECK-NO-VCONFLICT: No idioms detected.

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo2PiS_S_(i32* nocapture %A, i32* nocapture readonly %B, i32* nocapture %C) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %0 = load i32, i32* %ptridx, align 4
  %idxprom1 = sext i32 %0 to i64
  %ptridx2 = getelementptr inbounds i32, i32* %A, i64 %idxprom1
  %1 = load i32, i32* %ptridx2, align 4
  %add = add nsw i32 %1, 2
  store i32 %add, i32* %ptridx2, align 4
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

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM5:[0-9]+]]>          (%A)[sext.i32.i64(%0) + 2] = %add3;
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM6:[0-9]+]]>          %0 = (%B)[i1];
; CHECK-VCONFLICT: [VConflict Idiom] Skipped: Wrong memory dependency.

; CHECK-NO-VCONFLICT: No idioms detected.

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo3PfPi(float* nocapture %A, i32* nocapture readonly %B) local_unnamed_addr #0 {
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

; <26>               + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>                |   %0 = (%B)[i1];
; <27>               |
; <27>               |   + DO i2 = 0, 1023, 1   <DO_LOOP>
; <10>               |   |   %1 = (%A)[i2];
; <11>               |   |   %add = %1  +  2.000000e+00;
; <12>               |   |   (%A)[%0] = %add;
; <27>               |   + END LOOP
; <26>               + END LOOP

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM7:[0-9]+]]>         (%A)[%0] = %add;
; CHECK-VCONFLICT: [VConflict Idiom] Skipped: Invariant index is not supported.

; CHECK-NO-VCONFLICT: No idioms detected.

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo4PfPi(float* noalias nocapture %A, i32* noalias nocapture readonly %B) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.body:                                         ; preds = %entry, %for.cond.cleanup3
  %indvars.iv22 = phi i64 [ 0, %entry ], [ %indvars.iv.next23, %for.cond.cleanup3 ]
  %ptridx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv22
  %0 = load i32, i32* %ptridx, align 4
  %idxprom7 = sext i32 %0 to i64
  %ptridx8 = getelementptr inbounds float, float* %A, i64 %idxprom7
  br label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond24.not = icmp eq i64 %indvars.iv.next23, 1024
  br i1 %exitcond24.not, label %for.cond.cleanup, label %for.body

for.body4:                                        ; preds = %for.body, %for.body4
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body4 ]
  %ptridx6 = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %1 = load float, float* %ptridx6, align 4
  %add = fadd fast float %1, 2.000000e+00
  store float %add, float* %ptridx8, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup3, label %for.body4
}

; <19>               + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>                |   %0 = (%B)[i1];
; <6>                |   %1 = (%A)[%0];
; <7>                |   %add = %1  +  2.000000e+00;
; <8>                |   (%A)[%0] = %add;
; <10>               |   %2 = (%A)[i1];
; <11>               |   %add7 = %2  +  3.000000e+00;
; <12>               |   (%A)[i1] = %add7;
; <19>               + END LOOP

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM9:[0-9]+]]>          (%A)[%0] = %add;
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM10:[0-9]+]]>          %1 = (%A)[%0];
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM11:[0-9]+]]>         %2 = (%A)[i1];
; CHECK-VCONFLICT: [VConflict Idiom] Skipped: Too many dependencies.

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM12:[0-9]+]]>         (%A)[i1] = %add7;
; CHECK-VCONFLICT: [VConflict Idiom] Skipped: Store memory ref is linear

; CHECK-NO-VCONFLICT: No idioms detected.

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo5PfPi(float* noalias nocapture %A, i32* noalias nocapture readonly %B) local_unnamed_addr #0 {
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
  %ptridx6 = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %2 = load float, float* %ptridx6, align 4
  %add7 = fadd fast float %2, 3.000000e+00
  store float %add7, float* %ptridx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

; <15>               + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>                |   %0 = (@L)[0][i1];
; <6>                |   %1 = (@K)[0][%0];
; <7>                |   %add = %1  +  2.000000e+00;
; <8>                |   (@K)[0][%0] = %add;
; <15>               + END LOOP

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM14:[0-9]+]]>          (@K)[0][%0] = %add;
; CHECK-VCONFLICT: [VConflict Idiom] Skipped: Multidimensional arrays are not supported.

; CHECK-NO-VCONFLICT: No idioms detected.

@K = dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16
@L = dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo6v() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @L, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %idxprom1 = sext i32 %0 to i64
  %arrayidx2 = getelementptr inbounds [1024 x float], [1024 x float]* @K, i64 0, i64 %idxprom1
  %1 = load float, float* %arrayidx2, align 4
  %add = fadd fast float %1, 2.000000e+00
  store float %add, float* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
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

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM16:[0-9]+]]>          (%A)[%0] = %add;
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM17:[0-9]+]]>          %1 = (%A)[%0];
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM18:[0-9]+]]>         %3 = (%A)[%2];
; CHECK-VCONFLICT: [VConflict Idiom] Skipped: Too many dependencies.

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM19:[0-9]+]]>         (%A)[%2] = %add9;
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM20:[0-9]+]]>          %1 = (%A)[%0];
; CHECK-VCONFLICT: [VConflict Idiom] Skipped:  Wrong memory dependency.

; CHECK-NO-VCONFLICT: No idioms detected.

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo7PfPiS0_(float* noalias nocapture %A, i32* noalias nocapture readonly %B, i32* noalias nocapture readonly %C) local_unnamed_addr #0 {
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
  %ptridx6 = getelementptr inbounds i32, i32* %C, i64 %indvars.iv
  %2 = load i32, i32* %ptridx6, align 4
  %idxprom7 = sext i32 %2 to i64
  %ptridx8 = getelementptr inbounds float, float* %A, i64 %idxprom7
  %3 = load float, float* %ptridx8, align 4
  %add9 = fadd fast float %3, 3.000000e+00
  store float %add9, float* %ptridx8, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

; <23>               + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>                |   %0 = (%B)[i1];
; <6>                |   %1 = (%A)[%0];
; <7>                |   %conv = fptosi.float.i32(%1);
; <9>                |   %2 = (%A)[i1];
; <10>               |   %conv5 = fptosi.float.i32(%2);
; <12>               |   %conv6 = sitofp.i32.float(%conv + 2);
; <13>               |   (%A)[%0] = %conv6;
; <15>               |   %conv10 = sitofp.i32.float(%conv5 + 3);
; <16>               |   (%A)[i1] = %conv10;
; <23>               + END LOOP

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM21:[0-9]+]]>         (%A)[%0] = %conv6;
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM22:[0-9]+]]>          %1 = (%A)[%0];
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM23:[0-9]+]]>          %2 = (%A)[i1];
; CHECK-VCONFLICT: [VConflict Idiom] Skipped: Too many dependencies.

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM24:[0-9]+]]>         (%A)[i1] = %conv10;
; CHECK-VCONFLICT: [VConflict Idiom] Skipped: Store memory ref is linear

; CHECK-NO-VCONFLICT: No idioms detected.

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo8PfPi(float* noalias nocapture %A, i32* noalias nocapture readonly %B) local_unnamed_addr #0 {
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
  %conv = fptosi float %1 to i32
  %ptridx4 = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %2 = load float, float* %ptridx4, align 4
  %conv5 = fptosi float %2 to i32
  %add = add nsw i32 %conv, 2
  %conv6 = sitofp i32 %add to float
  store float %conv6, float* %ptridx2, align 4
  %add9 = add nsw i32 %conv5, 3
  %conv10 = sitofp i32 %add9 to float
  store float %conv10, float* %ptridx4, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

; <18>               + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>                |   %0 = (%B)[i1];
; <6>                |   %1 = (%A)[%0];
; <7>                |   %add = %1  +  2.000000e+00;
; <8>                |   (%A)[%0] = %add;
; <9>                |   %add7 = %1  +  5.000000e+00;
; <11>               |   (%A)[i1] = %add7;
; <18>               + END LOOP

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM26:[0-9]+]]>          (%A)[%0] = %add;
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM27:[0-9]+]]>          %1 = (%A)[%0];
; CHECK-VCONFLICT: [VConflict Idiom] Skipped: The output dependency is expected to be self-dependency.

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM28:[0-9]+]]>         (%A)[i1] = %add7;
; CHECK-VCONFLICT: [VConflict Idiom] Skipped: Store memory ref is linear

; CHECK-NO-VCONFLICT: No idioms detected.

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo9PfPi(float* noalias nocapture %A, i32* noalias nocapture readonly %B) local_unnamed_addr #0 {
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
  %add7 = fadd fast float %1, 5.000000e+00
  %ptridx9 = getelementptr inbounds float, float* %A, i64 %indvars.iv
  store float %add7, float* %ptridx9, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

; <15>               + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>                |   %0 = (%A)[i1];
; <6>                |   %1 = (%A)[%0];
; <8>                |   (%A)[%0] = %1 + 2;
; <15>               + END LOOP

; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM30:[0-9]+]]>          (%A)[%0] = %1 + 2;
; CHECK-VCONFLICT: [VConflict Idiom] Depends(WAR) on:<[[NUM31:[0-9]+]]>          %0 = (%A)[i1];
; CHECK-VCONFLICT: [VConflict Idiom] Skipped: Wrong memory dependency.

; CHECK-NO-VCONFLICT: No idioms detected.

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z5foo10Pi(i32* nocapture %A) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = load i32, i32* %ptridx, align 4
  %idxprom1 = sext i32 %0 to i64
  %ptridx2 = getelementptr inbounds i32, i32* %A, i64 %idxprom1
  %1 = load i32, i32* %ptridx2, align 4
  %add = add nsw i32 %1, 2
  store i32 %add, i32* %ptridx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

; <27>               + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295> <nounroll>
; <2>                |   %index.0.lcssa = 0;
; <3>                |   %temp2.0.lcssa = 0;
; <4>                |   if (%L > 0)
; <4>                |   {
; <8>                |      %1 = (%B)[%L + -1];
; <11>               |      %2 = (%A)[%1];
; <14>               |      %index.0.lcssa = %1;
; <15>               |      %temp2.0.lcssa = %1 + %2 + 2;
; <4>                |   }
; <20>               |   (%A)[%index.0.lcssa] = %temp2.0.lcssa;
; <27>               + END LOOP

; Note: %index.0.lcssa is not currently supported as vconflict index for one of two
; reasons:
;   1) num incoming edges to the store <20> > 1
;   2) definition is coming from two different parents
; Either is ok at the moment for preventing the idiom from working.
; There really shouldn't be a reason we can't support this case in the future.
; CHECK-VCONFLICT: [VConflict Idiom] Looking at store candidate:<[[NUM32:[0-9]+]]>         (%A)[%index.0.lcssa] = %temp2.0.lcssa;
; CHECK-VCONFLICT: [VConflict Idiom] Skipped: Conflict index not supported

; CHECK-NO-VCONFLICT: No idioms detected.

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z5foo11PiS_ii(i32* noalias nocapture %A, i32* noalias nocapture readonly %B, i32 %N, i32 %L) local_unnamed_addr #0 {
entry:
  %cmp30 = icmp sgt i32 %N, 0
  br i1 %cmp30, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp227 = icmp sgt i32 %L, 0
  %0 = add i32 %L, -1
  %idxprom.le = zext i32 %0 to i64
  %ptridx.le = getelementptr inbounds i32, i32* %B, i64 %idxprom.le
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.cond.cleanup3
  %j.031 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %inc13, %for.cond.cleanup3 ]
  br i1 %cmp227, label %for.body4.preheader, label %for.cond.cleanup3

for.body4.preheader:                              ; preds = %for.cond1.preheader
  %1 = load i32, i32* %ptridx.le, align 4
  %idxprom5.le = sext i32 %1 to i64
  %ptridx6.le = getelementptr inbounds i32, i32* %A, i64 %idxprom5.le
  %2 = load i32, i32* %ptridx6.le, align 4
  %add.le = add i32 %1, 2
  %add9.le = add i32 %add.le, %2
  br label %for.cond.cleanup3

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.cond.cleanup3:                                ; preds = %for.cond1.preheader, %for.body4.preheader
  %index.0.lcssa = phi i32 [ %1, %for.body4.preheader ], [ 0, %for.cond1.preheader ]
  %temp2.0.lcssa = phi i32 [ %add9.le, %for.body4.preheader ], [ 0, %for.cond1.preheader ]
  %idxprom10 = sext i32 %index.0.lcssa to i64
  %ptridx11 = getelementptr inbounds i32, i32* %A, i64 %idxprom10
  store i32 %temp2.0.lcssa, i32* %ptridx11, align 4
  %inc13 = add nuw nsw i32 %j.031, 1
  %exitcond.not = icmp eq i32 %inc13, %N
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.cond1.preheader
}

attributes #0 = { nofree norecurse nounwind uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }

;<0>          BEGIN REGION { }
;<29>               + DO i1 = 0, 1023, 1   <DO_LOOP>
;<3>                |   %0 = (%B)[i1];
;<4>                |   %conv = sitofp.i32.float(%0);
;<6>                |   %1 = (%C)[i1];
;<8>                |   if (%1 < %conv)
;<8>                |   {
;<14>               |      %2 = (%A)[%0];
;<15>               |      (%C)[i1] = %2;
;<8>                |   }
;<8>                |   else
;<8>                |   {
;<20>               |      (%A)[%0] = %1;
;<8>                |   }
;<29>               + END LOOP
;<0>          END REGION

; CHECK-VCONFLICT:  [VConflict Idiom] Looking at store candidate:<[[NUM34:[0-9]+]]>         (%C)[i1] = %2;
; CHECK-VCONFLICT: [VConflict Idiom] Skipped: Store memory ref is linear
; CHECK-VCONFLICT:  [VConflict Idiom] Looking at store candidate:<[[NUM35:[0-9]+]]>         (%A)[%0] = %1;
; CHECK-VCONFLICT:  [VConflict Idiom] Depends(WAR) on:<[[NUM36:[0-9]+]]>         %2 = (%A)[%0];
; CHECK-VCONFLICT:  [VConflict Idiom] Skipped: Sink node is in a different IF-branch.

; CHECK-NO-VCONFLICT: No idioms detected.

; Function Attrs: nofree norecurse nosync nounwind uwtable mustprogress
define dso_local void @_Z3foo12PfPiS_(float* noalias nocapture %A, i32* nocapture readonly %B, float* noalias nocapture %C) local_unnamed_addr #0 {
;
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %if.end
  ret void

for.body:                                         ; preds = %entry, %if.end
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %if.end ]
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %conv = sitofp i32 %0 to float
  %arrayidx2 = getelementptr inbounds float, float* %C, i64 %indvars.iv
  %1 = load float, float* %arrayidx2, align 4
  %cmp3 = fcmp fast olt float %1, %conv
  br i1 %cmp3, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %idxprom4 = sext i32 %0 to i64
  %arrayidx5 = getelementptr inbounds float, float* %A, i64 %idxprom4
  %2 = load float, float* %arrayidx5, align 4
  store float %2, float* %arrayidx2, align 4
  br label %if.end

if.else:                                          ; preds = %for.body
  %idxprom10 = sext i32 %0 to i64
  %arrayidx11 = getelementptr inbounds float, float* %A, i64 %idxprom10
  store float %1, float* %arrayidx11, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

attributes #0 = { nofree norecurse nosync nounwind uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

;<0>          BEGIN REGION { }
;<29>               + DO i1 = 0, 1023, 1   <DO_LOOP>
;<3>                |   %0 = (%B)[i1];
;<4>                |   %conv = sitofp.i32.float(%0);
;<6>                |   %1 = (%C)[i1];
;<8>                |   if (%1 < %conv)
;<8>                |   {
;<14>               |      (%A)[%0] = %1;
;<8>                |   }
;<8>                |   else
;<8>                |   {
;<19>               |      %2 = (%A)[%0];
;<20>               |      (%C)[i1] = %2;
;<8>                |   }
;<29>               + END LOOP
;<0>          END REGION


; CHECK-VCONFLICT:  [VConflict Idiom] Looking at store candidate:<[[NUM37:[0-9]+]]>         (%A)[%0] = %1;
; CHECK-VCONFLICT:  [VConflict Idiom] Depends(WAR) on:<[[NUM38:[0-9]+]]>         %2 = (%A)[%0];
; CHECK-VCONFLICT:  [VConflict Idiom] Skipped: Sink node is in a different IF-branch.
; CHECK-VCONFLICT:  [VConflict Idiom] Looking at store candidate:<[[NUM39:[0-9]+]]>         (%C)[i1] = %2;
; CHECK-VCONFLICT: [VConflict Idiom] Skipped: Store memory ref is linear

; CHECK-NO-VCONFLICT: No idioms detected.

; Function Attrs: nofree norecurse nosync nounwind uwtable mustprogress
define dso_local void @_Z3foo13PfPiS_(float* noalias nocapture %A, i32* nocapture readonly %B, float* noalias nocapture %C) local_unnamed_addr #0 {
;
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %if.end
  ret void

for.body:                                         ; preds = %entry, %if.end
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %if.end ]
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %conv = sitofp i32 %0 to float
  %arrayidx2 = getelementptr inbounds float, float* %C, i64 %indvars.iv
  %1 = load float, float* %arrayidx2, align 4
  %cmp3 = fcmp fast olt float %1, %conv
  br i1 %cmp3, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %idxprom6 = sext i32 %0 to i64
  %arrayidx7 = getelementptr inbounds float, float* %A, i64 %idxprom6
  store float %1, float* %arrayidx7, align 4
  br label %if.end

if.else:                                          ; preds = %for.body
  %idxprom8 = sext i32 %0 to i64
  %arrayidx9 = getelementptr inbounds float, float* %A, i64 %idxprom8
  %2 = load float, float* %arrayidx9, align 4
  store float %2, float* %arrayidx2, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

;<0>          BEGIN REGION { }
;<18>               + DO i1 = 0, 1023, 1   <DO_LOOP>
;<3>                |   %0 = (%B)[i1];
;<6>                |   %conv = sitofp.i32.float(%0);
;<7>                |   %add = %conv  +  2.000000e+00;
;<8>                |   (%A)[%0] = %add;
;<9>                |   %1 = (%A)[%0];
;<10>               |   %add2 = %1  +  2.000000e+00;
;<11>               |   (%A)[%0] = %add2;
;<18>               + END LOOP
;<0>          END REGION
;
; CHECK-VCONFLICT:  [VConflict Idiom] Looking at store candidate:<[[NUM38:[0-9]+]]>          (%A)[%0] = %add;
; CHECK-VCONFLICT:  [VConflict Idiom] Depends(WAR) on:<[[NUM39:[0-9]+]]>          %1 = (%A)[%0];
; CHECK-VCONFLICT:  [VConflict Idiom] Skipped: Nodes are not in the right order.
; CHECK-VCONFLICT:  [VConflict Idiom] Looking at store candidate:<[[NUM40:[0-9]+]]>         (%A)[%0] = %add2;
; CHECK-VCONFLICT:  [VConflict Idiom] Skipped: The output dependency is expected to be self-dependency.

; CHECK-NO-VCONFLICT: No idioms detected.

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo14PiS_S_(float* noalias nocapture %A, i32* noalias nocapture readonly %B, i32* nocapture %C) local_unnamed_addr #0 {
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
  %conv = sitofp i32 %0 to float
  %add = fadd float %conv, 2.
  store float %add, float* %ptridx2, align 4
  %1 = load float, float* %ptridx2, align 4
  %add2 = fadd float %1, 2.
  store float %add2, float* %ptridx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

attributes #0 = { nofree norecurse nosync nounwind uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

