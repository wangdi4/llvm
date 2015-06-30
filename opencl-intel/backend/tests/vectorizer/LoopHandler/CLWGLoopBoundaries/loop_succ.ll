; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../../Full/runtime.bc -kernel-analysis -cl-loop-bound -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @sample_test
; CHECK: ret
; CHECK: @WG.boundaries.sample_test
; CHECK: ret


; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @sample_test(i32 addrspace(1)* nocapture %dst, i32 %count) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %cmp1 = icmp sgt i32 %count, 0
  br i1 %cmp1, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %i.02 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  %add = add nsw i32 %i.02, %call
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %dst, i32 %i.02
  store i32 %add, i32 addrspace(1)* %arrayidx, align 4
  %inc = add nsw i32 %i.02, 1
  %exitcond = icmp eq i32 %inc, %count
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

declare i32 @_Z13get_global_idj(i32) nounwind readnone

!opencl.kernels = !{!0}
!opencl.compiler.options = !{!2}

!0 = !{void (i32 addrspace(1)*, i32)* @sample_test, !1}
!1 = !{!"image_access_qualifier", i32 3, i32 3}
!2 = !{}
