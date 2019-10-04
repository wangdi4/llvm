; RUN: %oclopt -runtimelib %p/../Full/runtime.bc -O3 -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -verify %s -S | FileCheck %s
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind
; CHECK-LABEL: @testKernel
define void @testKernel(i32 addrspace(1)* noalias %ballot_results) local_unnamed_addr {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #1
  %call1 = tail call i32 @_Z22get_sub_group_local_idv() #0
  %tobool = icmp ne i32 %call1, 0
  %call2 = tail call i32 @intel_sub_group_ballot(i1 zeroext %tobool) #0
; CHECK: call <4 x i32> @intel_sub_group_ballot_vf4(<4 x i1> {{%.*}}, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
  %cmp = icmp eq i32 %call2, 65534
  %cond = zext i1 %cmp to i32
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %ballot_results, i64 %call
  store i32 %cond, i32 addrspace(1)* %arrayidx, align 4
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32 %0) local_unnamed_addr #1

; Function Attrs: convergent
declare i32 @intel_sub_group_ballot(i1 zeroext %0) local_unnamed_addr #2

; Function Attrs: convergent
declare i32 @_Z22get_sub_group_local_idv() local_unnamed_addr #2

attributes #0 = { convergent nounwind }
attributes #1 = { convergent nounwind readnone }
attributes #2 = { convergent }
