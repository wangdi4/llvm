; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" -hir-verify-cf-def-level < %s 2>&1 | FileCheck %s

; Check stability after self-blob upperbound is extended.

; CHECK:Function: RE_zbuf_accumulate_vecblur

; CHECK:       BEGIN REGION { }
; CHECK:             + DO i1 = 0, %xsize, 1   <DO_LOOP>
; CHECK:             |   (undef)[4 * i1 + 1] = 0.000000e+00;
; CHECK:             |   (undef)[4 * i1 + 1] = 0.000000e+00;
; CHECK:             |   (undef)[4 * i1 + 3] = 0.000000e+00;
; CHECK:             |   (undef)[4 * i1 + 3] = 0.000000e+00;
; CHECK:             + END LOOP
; CHECK:       END REGION

; CHECK:Function: RE_zbuf_accumulate_vecblur

; CHECK:       BEGIN REGION { }
; CHECK:             + DO i1 = 0, 2 * sext.i32.i64(%xsize) + 1, 1   <DO_LOOP>
; CHECK:             |   (undef)[2 * i1 + 1] = 0.000000e+00;
; CHECK:             |   (undef)[2 * i1 + 1] = 0.000000e+00;
; CHECK:             + END LOOP
; CHECK:       END REGION

; ModuleID = 'bugpoint-reduced-simplified.bc'
source_filename = "blender/source/blender/render/intern/source/zbuf.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @RE_zbuf_accumulate_vecblur(i32 %xsize) local_unnamed_addr #0 {
entry:
  %add98 = add i32 %xsize, 1
  br label %for.body408

for.body408:                                      ; preds = %for.body408, %entry
  %dz1.01357 = phi ptr [ %add.ptr415, %for.body408 ], [ undef, %entry ]
  %dz2.01356 = phi ptr [ %add.ptr416, %for.body408 ], [ undef, %entry ]
  %x.31355 = phi i32 [ %inc414, %for.body408 ], [ 0, %entry ]
  %arrayidx409 = getelementptr inbounds float, ptr %dz1.01357, i64 1
  store float 0.000000e+00, ptr %arrayidx409, align 4
  %arrayidx410 = getelementptr inbounds float, ptr %dz2.01356, i64 1
  store float 0.000000e+00, ptr %arrayidx410, align 4
  %arrayidx411 = getelementptr inbounds float, ptr %dz1.01357, i64 3
  store float 0.000000e+00, ptr %arrayidx411, align 4
  %arrayidx412 = getelementptr inbounds float, ptr %dz2.01356, i64 3
  store float 0.000000e+00, ptr %arrayidx412, align 4
  %inc414 = add nuw nsw i32 %x.31355, 1
  %add.ptr415 = getelementptr inbounds float, ptr %dz1.01357, i64 4
  %add.ptr416 = getelementptr inbounds float, ptr %dz2.01356, i64 4
  %exitcond1398 = icmp eq i32 %inc414, %add98
  br i1 %exitcond1398, label %for.end417.loopexit, label %for.body408

for.end417.loopexit:                              ; preds = %for.body408
  unreachable
}



