; RUN: %oclopt -sub-group-adaptation -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -sub-group-adaptation -verify -S < %s | FileCheck %s
; ModuleID = 'main'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: call {{.*}} @_Z20work_group_broadcastmmmm

; Function Attrs: nounwind
define void @test_bcast(i64 addrspace(1)* %in, <2 x i32> addrspace(1)* %xy, i64 addrspace(1)* %out) #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_type_qual !9 !kernel_arg_base_type !8 {
entry:
  %call = call i64 @_Z13get_global_idj(i32 0) #1
  %call1 = call i32 @_Z22get_sub_group_local_idv() #1
  %sext = shl i64 %call, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds <2 x i32>, <2 x i32> addrspace(1)* %xy, i64 %idxprom
  %0 = load <2 x i32>, <2 x i32> addrspace(1)* %arrayidx, align 8
  %1 = insertelement <2 x i32> %0, i32 %call1, i32 0
  store <2 x i32> %1, <2 x i32> addrspace(1)* %arrayidx, align 8
  %call2 = call i32 @_Z16get_sub_group_idv() #1
  %sext6 = shl i64 %call, 32
  %idxprom3 = ashr exact i64 %sext6, 32
  %arrayidx4 = getelementptr inbounds <2 x i32>, <2 x i32> addrspace(1)* %xy, i64 %idxprom3
  %2 = load <2 x i32>, <2 x i32> addrspace(1)* %arrayidx4, align 8
  %3 = insertelement <2 x i32> %2, i32 %call2, i32 1
  store <2 x i32> %3, <2 x i32> addrspace(1)* %arrayidx4, align 8
  %sext7 = shl i64 %call, 32
  %idxprom5 = ashr exact i64 %sext7, 32
  %arrayidx6 = getelementptr inbounds i64, i64 addrspace(1)* %in, i64 %idxprom5
  %4 = load i64, i64 addrspace(1)* %arrayidx6, align 8
  %conv7 = trunc i64 %4 to i32
  %rem = srem i32 %conv7, 100
  %call10 = call i64 @_Z19sub_group_broadcastmj(i64 %4, i32 %rem)
  %sext8 = shl i64 %call, 32
  %idxprom11 = ashr exact i64 %sext8, 32
  %arrayidx12 = getelementptr inbounds i64, i64 addrspace(1)* %out, i64 %idxprom11
  store i64 %call10, i64 addrspace(1)* %arrayidx12, align 8
  ret void
}

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) #1

; Function Attrs: nounwind readnone
declare i32 @_Z22get_sub_group_local_idv() #1

; Function Attrs: nounwind readnone
declare i32 @_Z16get_sub_group_idv() #1

; Function Attrs: nounwind
declare i64 @_Z19sub_group_broadcastmj(i64, i32) #0

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!spirv.Source = !{!0}
!opencl.spir.version = !{!1}
!opencl.ocl.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!3}
!spirv.Generator = !{!4}
!sycl.kernels = !{!5}

!0 = !{i32 3, i32 200000}
!1 = !{i32 2, i32 0}
!2 = !{!"cl_khr_subgroups"}
!3 = !{}
!4 = !{i16 11, i16 3}
!5 = !{void (i64 addrspace(1)*, <2 x i32> addrspace(1)*, i64 addrspace(1)*)* @test_bcast}
!6 = !{i32 1, i32 1, i32 1}
!7 = !{!"none", !"none", !"none"}
!8 = !{!"long*", !"int2*", !"long*"}
!9 = !{!"const", !"", !""}

; Instructions in these two functions are added to replace one originl instruction. No DebugLoc.
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _Z22get_sub_group_local_idv
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _Z19sub_group_broadcastmj
; DEBUGIFY: WARNING: Missing line 9
; DEBUGIFY-NOT: WARNING
