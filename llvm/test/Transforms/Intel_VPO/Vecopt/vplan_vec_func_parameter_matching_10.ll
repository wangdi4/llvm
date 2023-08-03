; RUN: opt %s -S -passes="vplan-vec" 2>&1 | FileCheck %s

; This test checks to make sure unreachable isn't executed for a call
; argument where the DA shape is undefined. Such as VPValue is still
; marked as divergent and is treated as vector during variant matching.

; CHECK: call <16 x float> @_Z21work_group_reduce_minDv16_f

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind
define void @foo(i64 %uni0, i64 %uni1, ptr addrspace(1) %add.ptr.i, i64 %load._arg_, i1 %load._arg_6, ptr addrspace(1) %load._arg_8, ptr addrspace(1) %ptridx.i52) {
simd.begin.region:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %simd.begin.region
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ %indvars.iv.next, %simd.loop.exit ], [ 0, %DIR.OMP.SIMD.2 ]
  %iv.add = add nuw nsw i64 %indvars.iv, %uni0
  %sub.i.i.i.i = add nuw nsw i64 %uni1, %indvars.iv
  %cmp.i = icmp ult i64 %sub.i.i.i.i, %load._arg_
  br i1 %cmp.i, label %cond.true.i, label %cond.end.i

cond.true.i:                                      ; preds = %simd.loop
  %ptridx.i.i = getelementptr inbounds float, ptr addrspace(1) %add.ptr.i, i64 %sub.i.i.i.i
  %ld.ptridx.i.i = load float, ptr addrspace(1) %ptridx.i.i, align 4
  br label %cond.end.i

cond.end.i:                                       ; preds = %cond.true.i, %simd.loop
  %cond.i = phi float [ %ld.ptridx.i.i, %cond.true.i ], [ 0x7FF0000000000000, %simd.loop ]
  %call.i.i.i = tail call float @_Z21work_group_reduce_minf(float %cond.i) #1
  %cmp9.i = icmp eq i64 %iv.add, 0
  br i1 %cmp9.i, label %if.then.i, label %simd.loop.exit

if.then.i:                                        ; preds = %cond.end.i
  br i1 %load._arg_6, label %if.then10.i, label %if.end.i

if.then10.i:                                      ; preds = %if.then.i
  %ld.load._arg_8 = load float, ptr addrspace(1) %load._arg_8, align 4
  %cmp.i.i.i = fcmp olt float %ld.load._arg_8, %call.i.i.i
  %cond.i.i = select i1 %cmp.i.i.i, float %ld.load._arg_8, float %call.i.i.i
  br label %if.end.i

if.end.i:                                         ; preds = %if.then10.i, %if.then.i
  %PSum.0.i = phi float [ %cond.i.i, %if.then10.i ], [ %call.i.i.i, %if.then.i ]
  store float %PSum.0.i, ptr addrspace(1) %ptridx.i52, align 4
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %cond.end.i, %if.end.i
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 16
  br i1 %exitcond.not, label %simd.end.region, label %simd.loop

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %simd.end.region
  ret void
}

; Function Attrs: convergent nounwind
declare float @_Z21work_group_reduce_minf(float) local_unnamed_addr

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

attributes #1 = { convergent nounwind "call-params-num"="1" "kernel-call-once" "kernel-convergent-call" "vector-variants"="_ZGVbM16v__Z21work_group_reduce_minf(_Z21work_group_reduce_minDv16_fDv16_j),_ZGVbN16v__Z21work_group_reduce_minf(_Z21work_group_reduce_minDv16_f)" }
