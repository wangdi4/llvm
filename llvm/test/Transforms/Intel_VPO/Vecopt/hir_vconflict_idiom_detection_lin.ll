; Check if vconflict idiom is correctly detected on linear memrefs.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; REQUIRES: asserts
; RUN: opt -S -mattr=+avx512vl,+avx512cd -passes=hir-ssa-deconstruction,hir-vec-dir-insert -debug-only=parvec-analysis < %s 2>&1 | FileCheck %s

;<22>               + DO i1 = 0, 1023, 1   <DO_LOOP>
;<4>                |   %ld = (%A)[3 * i1];
;<5>                |   %add3 = %ld  +  2.000000e+00;
;<6>                |   (%A)[3 * i1] = %add3;
;<8>                |   %ld2 = (%A)[3 * i1 + 5];
;<9>                |   %add4 = %ld2  +  2.000000e+00;
;<10>               |   (%A)[3 * i1 + 5] = %add4;
;<13>               |   %ld3 = (%B)[(i1)/u3];
;<14>               |   %add5 = %ld3  +  2.000000e+00;
;<15>               |   (%B)[(i1)/u3] = %add5;
;<22>               + END LOOP

; CHECK: [VConflict Idiom] Looking at store candidate:<[[N1:[0-9]+]]>          (%A)[3 * i1] = %add3;
; CHECK: [VConflict Idiom] Skipped: Store memory ref is linear
; CHECK: [VConflict Idiom] Looking at store candidate:<[[N3:[0-9]+]]>          (%A)[3 * i1 + 5] = %add4;
; CHECK: [VConflict Idiom] Skipped: Store memory ref is linear
; CHECK: [VConflict Idiom] Looking at store candidate:<[[N5:[0-9]+]]>          (%B)[(i1)/u3] = %add5;
; CHECK: [VConflict Idiom] Depends(WAR) on:<[[N6:[0-9]+]]>          %ld3 = (%B)[(i1)/u3];
; CHECK: [VConflict Idiom] Detected, legality pending further dependence checking!

; Function Attrs: nofree norecurse nounwind uwtable mustprogress
define dso_local void @_Z4foo3PfPi(ptr noalias nocapture %A, ptr noalias nocapture readonly %B) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %mul = mul nsw i64 %indvars.iv, 3
  %ptridx2 = getelementptr inbounds float, ptr %A, i64 %mul
  %ld = load float, ptr %ptridx2, align 4
  %add3 = fadd fast float %ld, 2.000000e+00
  store float %add3, ptr %ptridx2, align 4
  %ptridx3 = getelementptr inbounds float, ptr %ptridx2, i64 5
  %ld2 = load float, ptr %ptridx3, align 4
  %add4 = fadd fast float %ld2, 2.000000e+00
  store float %add4, ptr %ptridx3, align 4
  %div = udiv i64 %indvars.iv, 3
  %ptridx4 = getelementptr inbounds float, ptr %B, i64 %div
  %ld3 = load float, ptr %ptridx4, align 4
  %add5 = fadd fast float %ld3, 2.000000e+00
  store float %add5, ptr %ptridx4, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}



