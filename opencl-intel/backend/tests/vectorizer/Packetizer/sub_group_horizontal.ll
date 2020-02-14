; RUN: %oclopt -runtimelib %p/../Full/runtime.bc -O3 -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -verify %s -S | FileCheck %s

; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32 %0) local_unnamed_addr #1

; Function Attrs: convergent
declare i32 @_Z16get_sub_group_idv() local_unnamed_addr #2

; Function Attrs: convergent
declare i32 @_Z13sub_group_alli(i32 %0) local_unnamed_addr #2

; Function Attrs: convergent
declare void @_Z17sub_group_barrierj(i32 %0) local_unnamed_addr #2

declare i64 @_Z14get_local_sizej(i32 %0)

declare i64 @get_base_global_id.(i32 %0)

; Function Attrs: convergent nounwind
; CHECK-LABEL: @testKernel
define void @testKernel(i32 addrspace(1)* %results, i32 addrspace(1)* %sub_groups_ids) local_unnamed_addr {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %call1 = tail call i32 @_Z16get_sub_group_idv() #4
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %sub_groups_ids, i64 %call
  store i32 %call1, i32 addrspace(1)* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, i32 addrspace(1)* %results, i64 %call
  store i32 0, i32 addrspace(1)* %arrayidx2, align 4
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %0 = phi i32 [ %.pre, %while.body ], [ 0, %entry ]
  %cmp = icmp eq i32 %0, %call1
  %conv = zext i1 %cmp to i32
; CHECK: call <4 x i32> @_Z13sub_group_allDv4_iDv4_j(<4 x i32> {{%.*}}, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
  %call4 = tail call i32 @_Z13sub_group_alli(i32 %conv) #4
  %tobool = icmp eq i32 %call4, 0
  br i1 %tobool, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %1 = load i32, i32 addrspace(1)* %arrayidx2, align 4
  %inc = add i32 %1, 1
  store i32 %inc, i32 addrspace(1)* %arrayidx2, align 4
  tail call void @_Z17sub_group_barrierj(i32 2) #4
  %.pre = load i32, i32 addrspace(1)* %arrayidx2, align 4
  br label %while.cond

while.end:                                        ; preds = %while.cond
  ret void
}

attributes #3 = { convergent nounwind readnone }
attributes #4 = { convergent nounwind }

