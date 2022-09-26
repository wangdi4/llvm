; Check that vectorizer can handle memrefs with loop variant stride.

; Incoming HIR
;     BEGIN REGION { }
;           %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
;           + DO i1 = 0, 1023, 1   <DO_LOOP>
;           |   %s = (%q)[0:0:8(i64*:0)];
;           |   (%p)[0:i1:%s(float*:0)] = 5.000000e+02;
;           + END LOOP
;
;           @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
;     END REGION

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -vplan-force-vf=2 -hir-details-dims -disable-output < %s 2>&1  | FileCheck %s --check-prefix=VPVAL-CG
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=2 -hir-details-dims -disable-output < %s 2>&1 | FileCheck %s --check-prefix=VPVAL-CG

; VPVAL-CG:           + DO i1 = 0, 1023, 2   <DO_LOOP> <auto-vectorized> <novectorize>
; VPVAL-CG-NEXT:      |   %.unifload = (%q)[0:0:8(i64*:0)];
; VPVAL-CG-NEXT:      |   (<2 x float>*)(%p)[0:i1 + <i64 0, i64 1>:%.unifload(float*:0)] = 5.000000e+02;
; VPVAL-CG-NEXT:      + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(float* noalias nocapture %p, i64* nocapture readnone noalias %q) {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %i = phi i64 [ 0, %entry ], [ %ip1, %for.body ]
  %s = load i64, i64* %q, align 4
  %idx = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 %s, float* elementtype(float) %p, i64 %i)
  store float 5.000000e+02, float* %idx, align 4
  %ip1 = add nuw nsw i64 %i, 1
  %exitcond = icmp eq i64 %ip1, 1024
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64)
