; RUN: opt -dpcpp-kernel-add-implicit-args -dpcpp-kernel-prepare-args -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-add-implicit-args -dpcpp-kernel-prepare-args -S %s | FileCheck %s --check-prefixes CHECK,CHECK-ARG
; RUN: opt -dpcpp-kernel-enable-tls-globals -dpcpp-kernel-add-tls-globals -dpcpp-kernel-prepare-args -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY-TLS %s
; RUN: opt -dpcpp-kernel-enable-tls-globals -dpcpp-kernel-add-tls-globals -dpcpp-kernel-prepare-args -S %s | FileCheck %s --check-prefixes CHECK,CHECK-TLS
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s | FileCheck %s --check-prefixes CHECK,CHECK-ARG
; RUN: opt -dpcpp-kernel-enable-tls-globals -passes='dpcpp-kernel-add-tls-globals,dpcpp-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY-TLS %s
; RUN: opt -dpcpp-kernel-enable-tls-globals -passes='dpcpp-kernel-add-tls-globals,dpcpp-kernel-prepare-args' -S %s | FileCheck %s --check-prefixes CHECK,CHECK-TLS

; This test checks that kernel with optnone are properly inlined into wrapper
; kernel.

; CHECK: define dso_local i32 @foo(
; CHECK: define dso_local void @test(

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local i32 @foo(i32 addrspace(1)* noalias noundef %dst, i32 noundef %v, i64 noundef %gid) #0 {
entry:
  %retval = alloca i32, align 4
  %dst.addr = alloca i32 addrspace(1)*, align 8
  %v.addr = alloca i32, align 4
  %gid.addr = alloca i64, align 8
  store i32 addrspace(1)* %dst, i32 addrspace(1)** %dst.addr, align 8
  store i32 %v, i32* %v.addr, align 4
  store i64 %gid, i64* %gid.addr, align 8
  %0 = load i32, i32* %v.addr, align 4
  %1 = load i32 addrspace(1)*, i32 addrspace(1)** %dst.addr, align 8
  %2 = load i64, i64* %gid.addr, align 8
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %1, i64 %2
  store i32 %0, i32 addrspace(1)* %arrayidx, align 4
  %3 = load i32, i32* %retval, align 4
  ret i32 %3
}

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @test(i32 addrspace(1)* noalias noundef align 4 %dst, i32 noundef %v) #1 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_name !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !11 !kernel_arg_buffer_location !11 !no_barrier_path !12 !kernel_has_sub_groups !13 !barrier_buffer_size !14 !kernel_execution_length !15 !kernel_has_global_sync !13 !recommended_vector_length !16 !private_memory_size !17 {
  %dst.addr = alloca i32 addrspace(1)*, align 8
  %v.addr = alloca i32, align 4
  %gid = alloca i64, align 8
  %base.gid.dim0 = call i64 @get_base_global_id.(i32 0)
  %local.size.dim0 = call i64 @_Z14get_local_sizej(i32 0)
  %max.gid.dim0 = add i64 %base.gid.dim0, %local.size.dim0
  %base.gid.dim1 = call i64 @get_base_global_id.(i32 1)
  %local.size.dim1 = call i64 @_Z14get_local_sizej(i32 1)
  %max.gid.dim1 = add i64 %base.gid.dim1, %local.size.dim1
  %base.gid.dim2 = call i64 @get_base_global_id.(i32 2)
  %local.size.dim2 = call i64 @_Z14get_local_sizej(i32 2)
  %max.gid.dim2 = add i64 %base.gid.dim2, %local.size.dim2
  br label %dim_2_pre_head

dim_2_pre_head:                                   ; preds = %0
  br label %dim_1_pre_head

dim_1_pre_head:                                   ; preds = %dim_1_exit, %dim_2_pre_head
  %dim_2_ind_var = phi i64 [ %base.gid.dim2, %dim_2_pre_head ], [ %dim_2_inc_ind_var, %dim_1_exit ]
  br label %dim_0_pre_head

dim_0_pre_head:                                   ; preds = %dim_0_exit, %dim_1_pre_head
  %dim_1_ind_var = phi i64 [ %base.gid.dim1, %dim_1_pre_head ], [ %dim_1_inc_ind_var, %dim_0_exit ]
  br label %scalar_kernel_entry

scalar_kernel_entry:                              ; preds = %scalar_kernel_entry, %dim_0_pre_head
; CHECK-LABEL: scalar_kernel_entry
; CHECK-ARG: call i32 @foo(i32 addrspace(1)* noalias noundef {{.*}}, i32 noundef {{.*}}, i64 noundef {{.*}}, i8 addrspace(3)* noalias %LocalMem_foo, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* noalias %pWorkDim, i64* noalias %pWGId, [4 x i64] %BaseGlbId, i8* noalias %pSpecialBuf, {}* noalias %RuntimeHandle)
; CHECK-TLS: call i32 @foo(i32 addrspace(1)* noundef {{.*}}, i32 noundef {{.*}}, i64 noundef {{.*}})

  %dim_0_ind_var = phi i64 [ %base.gid.dim0, %dim_0_pre_head ], [ %dim_0_inc_ind_var, %scalar_kernel_entry ]
  store i32 addrspace(1)* %dst, i32 addrspace(1)** %dst.addr, align 8
  store i32 %v, i32* %v.addr, align 4
  store i64 %dim_0_ind_var, i64* %gid, align 8
  %1 = load i32 addrspace(1)*, i32 addrspace(1)** %dst.addr, align 8
  %2 = load i32, i32* %v.addr, align 4
  %3 = load i64, i64* %gid, align 8
  %call1 = call i32 @foo(i32 addrspace(1)* noundef %1, i32 noundef %2, i64 noundef %3) #3
  %dim_0_inc_ind_var = add nuw nsw i64 %dim_0_ind_var, 1
  %dim_0_cmp.to.max = icmp eq i64 %dim_0_inc_ind_var, %max.gid.dim0
  br i1 %dim_0_cmp.to.max, label %dim_0_exit, label %scalar_kernel_entry

dim_0_exit:                                       ; preds = %scalar_kernel_entry
  %dim_1_inc_ind_var = add nuw nsw i64 %dim_1_ind_var, 1
  %dim_1_cmp.to.max = icmp eq i64 %dim_1_inc_ind_var, %max.gid.dim1
  br i1 %dim_1_cmp.to.max, label %dim_1_exit, label %dim_0_pre_head

dim_1_exit:                                       ; preds = %dim_0_exit
  %dim_2_inc_ind_var = add nuw nsw i64 %dim_2_ind_var, 1
  %dim_2_cmp.to.max = icmp eq i64 %dim_2_inc_ind_var, %max.gid.dim2
  br i1 %dim_2_cmp.to.max, label %dim_2_exit, label %dim_1_pre_head

dim_2_exit:                                       ; preds = %dim_1_exit
  br label %exit

exit:                                             ; preds = %dim_2_exit
  ret void
}

; Function Attrs: convergent nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) #2

declare i64 @get_base_global_id.(i32)

declare i64 @_Z14get_local_sizej(i32)

attributes #0 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #1 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #2 = { convergent nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #3 = { convergent }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}
!sycl.kernels = !{!3}

!0 = !{i32 2, i32 0}
!1 = !{!"-cl-std=CL2.0", !"-cl-opt-disable"}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{void (i32 addrspace(1)*, i32)* @test}
!4 = !{i32 1, i32 0}
!5 = !{!"none", !"none"}
!6 = !{!"int*", !"int"}
!7 = !{!"restrict", !""}
!8 = !{!"dst", !"v"}
!9 = !{i1 false, i1 false}
!10 = !{i32 0, i32 0}
!11 = !{!"", !""}
!12 = !{i1 true}
!13 = !{i1 false}
!14 = !{i32 0}
!15 = !{i32 12}
!16 = !{i32 1}
!17 = !{i32 56}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} getelementptr i8, i8* %UniformArgs, i32 0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} bitcast i8*
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} load i32
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} getelementptr i8, i8* %UniformArgs, i32 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} bitcast i8*
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} load i32
; DEBUGIFY-TLS: WARNING: Instruction with empty DebugLoc in function test {{.*}} store i8 addrspace(3)* null, i8 addrspace(3)** @pLocalMemBase, align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} getelementptr i8, i8* %UniformArgs, i32 16
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} bitcast i8*
; DEBUGIFY-TLS: WARNING: Instruction with empty DebugLoc in function test {{.*}} store { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* {{.*}}, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }** @pWorkDim, align 8
; DEBUGIFY-TLS: WARNING: Instruction with empty DebugLoc in function test {{.*}} store i64* %pWGId, i64** @pWGId, align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} load i64
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} load i64
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} load i64
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} load i64
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} load i64
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} load i64
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} getelementptr i64, i64* %pWGId, i32 0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} load i64
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} getelementptr i64, i64* %pWGId, i32 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} load i64
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} getelementptr i64, i64* %pWGId, i32 2
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} load i64
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} mul i64 %InternalLocalSize_0, %GroupID_0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} add i64 %14, %GlobalOffset_0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} mul i64 %InternalLocalSize_1, %GroupID_1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} add i64 %16, %GlobalOffset_1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} mul i64 %InternalLocalSize_2, %GroupID_2
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} add i64 %18, %GlobalOffset_2
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} insertvalue [4 x i64] undef
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} insertvalue [4 x i64]
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} insertvalue [4 x i64]
; DEBUGIFY-TLS: WARNING: Instruction with empty DebugLoc in function test {{.*}} store [4 x i64] {{.*}}, [4 x i64]* @BaseGlbId, align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} mul nuw nsw i64 %InternalLocalSize_0, %InternalLocalSize_1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} mul nuw nsw i64
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} mul nuw nsw i64 0, %LocalSizeProd
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} alloca i8, i64 %BarrierBufferSize, align 128
; DEBUGIFY-TLS: WARNING: Instruction with empty DebugLoc in function test {{.*}} store i8* {{.*}}, i8** @pSpecialBuf, align 8
; DEBUGIFY-TLS: WARNING: Instruction with empty DebugLoc in function test {{.*}} store {}* %RuntimeHandle, {}** @RuntimeHandle, align 8

; DEBUGIFY-NOT: WARNING
