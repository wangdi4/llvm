; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -runtime=ocl -print-wia-check -WIAnalysis %t.bc -S -o %t1.ll  | FileCheck %s

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

@.str = private unnamed_addr addrspace(2) constant [13 x i8] c"do something\00", align 1

;CHECK-NOT: WI-RunOnFunction 4   %new_phi

define void @__Vectorized_.PhiCanonIdenticalValues(float addrspace(1)* nocapture %a, float addrspace(1)* nocapture %b) #0 {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) #3
  %cmp = icmp slt i32 %call, 12
  br i1 %cmp, label %if.then, label %phi-split-bb

if.then:                                          ; preds = %entry
  %call1 = tail call i32 (i8 addrspace(2)*, ...)* @printf(i8 addrspace(2)* getelementptr inbounds ([13 x i8] addrspace(2)* @.str, i32 0, i32 0)) #4
  br label %phi-split-bb

phi-split-bb:                                     ; preds = %entry, %if.then
  %new_phi = phi i32 [ 0, %if.then ], [ 0, %entry ]
  br label %for.body

for.body:                                         ; preds = %phi-split-bb, %for.body
  %i.01 = phi i32 [ %inc3, %for.body ], [ %new_phi, %phi-split-bb ]
  %arrayidx = getelementptr inbounds float addrspace(1)* %a, i32 %i.01
  %0 = load float addrspace(1)* %arrayidx, align 4
  %inc = fadd float %0, 1.000000e+00
  store float %inc, float addrspace(1)* %arrayidx, align 4
  %inc3 = add nsw i32 %i.01, 1
  %exitcond = icmp eq i32 %inc3, 6400
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

declare i32 @_Z13get_global_idj(i32) #1

declare i32 @printf(i8 addrspace(2)*, ...) #2
