; RUN: opt -passes=sycl-kernel-prepare-args -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-prepare-args -S %s | FileCheck %s

; Check that maximum alignment is deduced for %mem0. Alignments of %mem1, %mem2,
; %mem3 and %mem4 are deduced from their users.

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir64-unknown-unknown"

define spir_kernel void @test(ptr addrspace(1) nocapture %results, ptr addrspace(3) %mem0, ptr addrspace(3) %mem1, ptr addrspace(3) %mem2, ptr addrspace(3) %mem3, ptr addrspace(3) %mem4, ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) #0 !kernel_arg_base_type !3 {
entry:
; CHECK: [[HEAP_MEM:%[0-9]+]] = getelementptr ptr, ptr %pWGId, i32 3
; CHECK-NEXT: %pHeapMem = load ptr, ptr [[HEAP_MEM]], align 1
; CHECK-NEXT: [[HEAP_MEM_NULL:%[0-9]+]] = icmp eq ptr %pHeapMem, null

; CHECK: [[GEP0:%[0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 8
; CHECK-NEXT: [[LOAD0:%[0-9]+]] = load i64, ptr [[GEP0]], align 8
; CHECK-NEXT: [[ALLOCA_SIZE0:%[0-9]+]] = select i1 [[HEAP_MEM_NULL]], i64 [[LOAD0]], i64 0
; CHECK-NEXT: [[ALLOCA0:%[0-9]+]] = alloca i8, i64 [[ALLOCA_SIZE0]], align 128
; CHECK-NEXT: [[HEAP_MEM_LOC0:%[0-9]+]] = getelementptr i8, ptr %pHeapMem, i64 0
; CHECK-NEXT: [[LOCAL_ARG_BUFFER0:%[0-9]+]] = select i1 [[HEAP_MEM_NULL]], ptr [[ALLOCA0]], ptr [[HEAP_MEM_LOC0]]
; CHECK-NEXT: %explicit_1 = addrspacecast ptr [[LOCAL_ARG_BUFFER0]] to ptr addrspace(3)
; CHECK-NEXT: [[HEAP_MEM_OFFSET0:%[0-9]+]] = add i64 0, [[LOAD0]]

; CHECK: [[GEP1:%[0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 16
; CHECK-NEXT: [[LOAD1:%[0-9]+]] = load i64, ptr [[GEP1]], align 8
; CHECK-NEXT: [[TMP1:%[0-9]+]] = add i64 [[HEAP_MEM_OFFSET0]], 7
; CHECK-NEXT: [[OFFSET_ALIGN0:%[0-9]+]] = and i64 [[TMP1]], -8
; CHECK-NEXT: [[ALLOCA_SIZE1:%[0-9]+]] = select i1 [[HEAP_MEM_NULL]], i64 [[LOAD1]], i64 0
; CHECK-NEXT: [[ALLOCA1:%[0-9]+]] = alloca i8, i64 [[ALLOCA_SIZE1]], align 8
; CHECK-NEXT: [[HEAP_MEM_LOC1:%[0-9]+]] = getelementptr i8, ptr %pHeapMem, i64 [[OFFSET_ALIGN0]]
; CHECK-NEXT: [[LOCAL_ARG_BUFFER1:%[0-9]+]] = select i1 [[HEAP_MEM_NULL]], ptr [[ALLOCA1]], ptr [[HEAP_MEM_LOC1]]
; CHECK-NEXT: %explicit_2 = addrspacecast ptr [[LOCAL_ARG_BUFFER1]] to ptr addrspace(3)
; CHECK-NEXT: [[HEAP_MEM_OFFSET1:%[0-9]+]] = add i64 [[OFFSET_ALIGN0]], [[LOAD1]]

; CHECK: [[GEP2:%[0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 24
; CHECK-NEXT: [[LOAD2:%[0-9]+]] = load i64, ptr [[GEP2]], align 8
; CHECK-NEXT: [[TMP3:%[0-9]+]] = add i64 [[HEAP_MEM_OFFSET1]], 0
; CHECK-NEXT: [[OFFSET_ALIGN1:%[0-9]+]] = and i64 [[TMP3]], -1
; CHECK-NEXT: [[ALLOCA_SIZE2:%[0-9]+]] = select i1 [[HEAP_MEM_NULL]], i64 [[LOAD2]], i64 0
; CHECK-NEXT: [[ALLOCA2:%[0-9]+]] = alloca i8, i64 [[ALLOCA_SIZE2]], align 1
; CHECK-NEXT: [[HEAP_MEM_LOC2:%[0-9]+]] = getelementptr i8, ptr %pHeapMem, i64 [[OFFSET_ALIGN1]]
; CHECK-NEXT: [[LOCAL_ARG_BUFFER2:%[0-9]+]] = select i1 [[HEAP_MEM_NULL]], ptr [[ALLOCA2]], ptr [[HEAP_MEM_LOC2]]
; CHECK-NEXT: %explicit_3 = addrspacecast ptr [[LOCAL_ARG_BUFFER2]] to ptr addrspace(3)
; CHECK-NEXT: [[HEAP_MEM_OFFSET2:%[0-9]+]] = add i64 [[OFFSET_ALIGN1]], [[LOAD2]]

; CHECK: [[GEP3:%[0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 32
; CHECK-NEXT: [[LOAD3:%[0-9]+]] = load i64, ptr [[GEP3]], align 8
; CHECK-NEXT: [[TMP5:%[0-9]+]] = add i64 [[HEAP_MEM_OFFSET2]], 1
; CHECK-NEXT: [[OFFSET_ALIGN2:%[0-9]+]] = and i64 [[TMP5]], -2
; CHECK-NEXT: [[ALLOCA_SIZE3:%[0-9]+]] = select i1 [[HEAP_MEM_NULL]], i64 [[LOAD3]], i64 0
; CHECK-NEXT: [[ALLOCA3:%[0-9]+]] = alloca i8, i64 [[ALLOCA_SIZE3]], align 2
; CHECK-NEXT: [[HEAP_MEM_LOC3:%[0-9]+]] = getelementptr i8, ptr %pHeapMem, i64 [[OFFSET_ALIGN2]]
; CHECK-NEXT: [[LOCAL_ARG_BUFFER3:%[0-9]+]] = select i1 [[HEAP_MEM_NULL]], ptr [[ALLOCA3]], ptr [[HEAP_MEM_LOC3]]
; CHECK-NEXT: %explicit_4 = addrspacecast ptr [[LOCAL_ARG_BUFFER3]] to ptr addrspace(3)
; CHECK-NEXT: [[HEAP_MEM_OFFSET3:%[0-9]+]] = add i64 [[OFFSET_ALIGN2]], [[LOAD3]]

; CHECK: [[GEP3:%[0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 40
; CHECK-NEXT: [[LOAD4:%[0-9]+]] = load i64, ptr [[GEP3]], align 8
; CHECK-NEXT: [[TMP7:%[0-9]+]] = add i64 [[HEAP_MEM_OFFSET3]], 31
; CHECK-NEXT: [[OFFSET_ALIGN3:%[0-9]+]] = and i64 [[TMP7]], -32
; CHECK-NEXT: [[ALLOCA_SIZE4:%[0-9]+]] = select i1 [[HEAP_MEM_NULL]], i64 [[LOAD4]], i64 0
; CHECK-NEXT: [[ALLOCA4:%[0-9]+]] = alloca i8, i64 [[ALLOCA_SIZE4]], align 32
; CHECK-NEXT: [[HEAP_MEM_LOC4:%[0-9]+]] = getelementptr i8, ptr %pHeapMem, i64 [[OFFSET_ALIGN3]]
; CHECK-NEXT: [[LOCAL_ARG_BUFFER4:%[0-9]+]] = select i1 [[HEAP_MEM_NULL]], ptr [[ALLOCA4]], ptr [[HEAP_MEM_LOC4]]
; CHECK-NEXT: %explicit_5 = addrspacecast ptr [[LOCAL_ARG_BUFFER4]] to ptr addrspace(3)
; CHECK-NEXT: [[HEAP_MEM_OFFSET4:%[0-9]+]] = add i64 [[OFFSET_ALIGN3]], [[LOAD4]]

  %array = addrspacecast ptr addrspace(1) %results to ptr addrspace(3)
  store ptr addrspace(3) %mem0, ptr addrspace(3) %array, align 8
  %0 = addrspacecast ptr addrspace(3) %mem1 to ptr addrspace(4)
  %1 = load <2 x float>, ptr addrspace(4) %0, align 8
  %2 = select i1 true, ptr addrspace(3) %mem2, ptr addrspace(3) %mem2
  store i8 1, ptr addrspace(3) %2, align 1
  br label %exit

exit:                                             ; preds = %entry
  %3 = phi ptr addrspace(3) [ %mem3, %entry ]
  %arrayidx5 = getelementptr inbounds i16, ptr addrspace(3) %3, i64 2
  %4 = load <16 x half>, ptr addrspace(3) %mem4, align 32
  ret void
}

attributes #0 = { nounwind }

!spirv.Source = !{!0}
!spirv.Generator = !{!1}
!sycl.kernels = !{!2}

!0 = !{i32 3, i32 300000}
!1 = !{i16 6, i16 14}
!2 = !{ptr @test}
!3 = !{!"long*", !"char*", !"float2*", !"char*", !"short*", !"half16*"}

; DEBUGIFY-COUNT-81: WARNING: Instruction with empty DebugLoc in function test
; DEBUGIFY-NOT: WARNING
