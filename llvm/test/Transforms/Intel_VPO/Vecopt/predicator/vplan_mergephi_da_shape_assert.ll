; RUN: opt -S -passes="vplan-vec" < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; This test checks to make sure we don't hit an assert when the predicator
; inserts merge-phis and updates their DA shapes during the blending
; transformation. In this case, it's possible that we have one merge-phi
; feeding another and since the pointers for these phis were added to a
; DenseMap, the ordering of when these phis were processed could be different
; depending on memory layout. This lead to flaky behavior during the DA shape
; processing for these phis because sometimes the merge-phi that contained a
; merge-phi operand was processed first and therefore it's shape was updated
; before the operand. This led to an assert because recomputeShapes() assumes
; that all operand shapes have previously been defined.

; CHECK-LABEL: vector.body

define void @_ZGVeN16uuuuuuuuuuu_ray_sphere() {
entry:
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %simd.loop.header

simd.loop.header:                                 ; preds = %for.cond.cleanup, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %for.cond.cleanup ]
  %conv.i = sitofp i32 %index to float
  br label %for.body

for.cond.cleanup:                                 ; preds = %if.then28, %if.else24, %if.then22
  %indvar = add i32 %index, 1
  br i1 false, label %simd.loop.header, label %simd.end.region

simd.end.region:                                  ; preds = %for.cond.cleanup
  call void @llvm.directive.region.exit(token %entry.region1) [ "DIR.OMP.END.SIMD"() ]
  ret void

for.body:                                         ; preds = %simd.loop.header
  br i1 false, label %if.then22, label %if.end18.thread

if.end18.thread:                                  ; preds = %for.body
  br i1 false, label %for.body.lr.ph.i, label %if.then28

for.body.lr.ph.i:                                 ; preds = %if.end18.thread
  %cmp54.i = fcmp olt float %conv.i, 0.000000e+00
  br i1 %cmp54.i, label %if.end78.i, label %if.then.i

if.then.i:                                        ; preds = %for.body.lr.ph.i
  br label %if.end78.i

if.end78.i:                                       ; preds = %if.then.i, %for.body.lr.ph.i
  %closest_sphere.3.i = phi i32 [ 1, %for.body.lr.ph.i ], [ 0, %if.then.i ]
  br i1 false, label %if.then22, label %if.else24

if.then22:                                        ; preds = %if.end78.i, %for.body
  br label %for.cond.cleanup

if.else24:                                        ; preds = %if.end78.i
  %cmp25.not = icmp eq i32 %closest_sphere.3.i, 0
  br i1 %cmp25.not, label %if.then28, label %for.cond.cleanup

if.then28:                                        ; preds = %if.else24, %if.end18.thread
  %cur_shadow_ray.sroa.4.2224 = phi <4 x float> [ zeroinitializer, %if.else24 ], [ zeroinitializer, %if.end18.thread ]
  br label %for.cond.cleanup
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

attributes #0 = { nounwind }
