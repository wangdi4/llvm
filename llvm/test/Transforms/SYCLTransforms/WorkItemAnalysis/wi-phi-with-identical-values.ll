; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

@.str = private unnamed_addr addrspace(2) constant [13 x i8] c"do something\00", align 1

;CHECK: WorkItemAnalysis for function __Vectorized_.PhiCanonIdenticalValues:
; CHECK-NEXT: SEQ   %call = tail call i32 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: RND   %cmp = icmp slt i32 %call, 12
; CHECK-NEXT: RND   br i1 %cmp, label %if.then, label %phi-split-bb
; CHECK-NEXT: RND   %call1 = tail call i32 (ptr addrspace(2), ...) @printf(ptr addrspace(2) @.str)
; CHECK-NEXT: UNI   br label %phi-split-bb
; CHECK-NEXT: UNI   %new_phi = phi i32 [ 0, %if.then ], [ 0, %entry ]
; CHECK-NEXT: UNI   br label %for.body
; CHECK-NEXT: UNI   %i.01 = phi i32 [ %inc3, %for.body ], [ %new_phi, %phi-split-bb ]
; CHECK-NEXT: UNI   %arrayidx = getelementptr inbounds float, ptr addrspace(1) %a, i32 %i.01
; CHECK-NEXT: UNI   %load = load float, ptr addrspace(1) %arrayidx, align 4
; CHECK-NEXT: UNI   %inc = fadd float %load, 1.000000e+00
; CHECK-NEXT: UNI   store float %inc, ptr addrspace(1) %arrayidx, align 4
; CHECK-NEXT: UNI   %inc3 = add nsw i32 %i.01, 1
; CHECK-NEXT: UNI   %exitcond = icmp eq i32 %inc3, 6400
; CHECK-NEXT: UNI   br i1 %exitcond, label %for.end, label %for.body
; CHECK-NEXT: UNI   ret void

define void @__Vectorized_.PhiCanonIdenticalValues(ptr addrspace(1) nocapture %a, ptr addrspace(1) nocapture %b) #0 {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) #3
  %cmp = icmp slt i32 %call, 12
  br i1 %cmp, label %if.then, label %phi-split-bb

if.then:                                          ; preds = %entry
  %call1 = tail call i32 (ptr addrspace(2), ...) @printf(ptr addrspace(2) @.str) #4
  br label %phi-split-bb

phi-split-bb:                                     ; preds = %entry, %if.then
  %new_phi = phi i32 [ 0, %if.then ], [ 0, %entry ]
  br label %for.body

for.body:                                         ; preds = %phi-split-bb, %for.body
  %i.01 = phi i32 [ %inc3, %for.body ], [ %new_phi, %phi-split-bb ]
  %arrayidx = getelementptr inbounds float, ptr addrspace(1) %a, i32 %i.01
  %load = load float, ptr addrspace(1) %arrayidx, align 4
  %inc = fadd float %load, 1.000000e+00
  store float %inc, ptr addrspace(1) %arrayidx, align 4
  %inc3 = add nsw i32 %i.01, 1
  %exitcond = icmp eq i32 %inc3, 6400
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

declare i32 @_Z13get_global_idj(i32) #1

declare i32 @printf(ptr addrspace(2), ...) #2
