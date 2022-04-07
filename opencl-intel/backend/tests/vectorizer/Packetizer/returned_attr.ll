; RUN: %oclopt  -runtimelib %p/../Full/runtime.bc -packetize -packet-size=4 %s -S | FileCheck %s
; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare i64 @_Z12get_local_idj(i32) nounwind readnone

; Function Attrs: convergent
declare i32 @_Z14work_group_alli(i32 %0) local_unnamed_addr

; Function Attrs: convergent norecurse nounwind
define void @__Vectorized_.test(i32 addrspace(1)* %a, i32 addrspace(1)* %b) local_unnamed_addr #0 {
entry:
  %call = tail call i64 @_Z12get_local_idj(i32 0) #3
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %call
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4
  %call1 = tail call i32 @_Z14work_group_alli(i32 %0) #4
;CHECK: call <4 x i32> @_Z14work_group_allDv4_i(<4 x i32> {{%.*}})
  store i32 %call1, i32 addrspace(1)* %b, align 4
  ret void
}

;CHECK: declare <4 x i32> @_Z14work_group_allDv4_i(<4 x i32>)
