; RUN: opt -passes=sycl-kernel-prepare-args -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-prepare-args -S %s | FileCheck %s

; Check that maximum alignment is deduced for %mem0. Alignments of %mem1, %mem2,
; %mem3 and %mem4 are deduced from their users.

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir64-unknown-unknown"

define spir_kernel void @test(ptr addrspace(1) nocapture %results, ptr addrspace(3) %mem0, ptr addrspace(3) %mem1, ptr addrspace(3) %mem2, ptr addrspace(3) %mem3, ptr addrspace(3) %mem4, ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) #0 !kernel_arg_base_type !3 {
entry:
; CHECK: [[GEP0:%[0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 8
; CHECK-NEXT: [[LOAD0:%[0-9]+]] = load i64, ptr [[GEP0]], align 8
; CHECK-NEXT: [[ALLOCA0:%[0-9]+]] = alloca i8, i64 [[LOAD0]], align 128
; CHECK-NEXT: %explicit_1 = addrspacecast ptr [[ALLOCA0]] to ptr addrspace(3)

; CHECK: [[GEP1:%[0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 16
; CHECK-NEXT: [[LOAD1:%[0-9]+]] = load i64, ptr [[GEP1]], align 8
; CHECK-NEXT: [[ALLOCA1:%[0-9]+]] = alloca i8, i64 [[LOAD1]], align 8
; CHECK-NEXT: %explicit_2 = addrspacecast ptr [[ALLOCA1]] to ptr addrspace(3)

; CHECK: [[GEP2:%[0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 24
; CHECK-NEXT: [[LOAD2:%[0-9]+]] = load i64, ptr [[GEP2]], align 8
; CHECK-NEXT: [[ALLOCA2:%[0-9]+]] = alloca i8, i64 [[LOAD2]], align 1
; CHECK-NEXT: %explicit_3 = addrspacecast ptr [[ALLOCA2]] to ptr addrspace(3)

; CHECK: [[GEP3:%[0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 32
; CHECK-NEXT: [[LOAD3:%[0-9]+]] = load i64, ptr [[GEP3]], align 8
; CHECK-NEXT: [[ALLOCA3:%[0-9]+]] = alloca i8, i64 [[LOAD3]], align 2
; CHECK-NEXT: %explicit_4 = addrspacecast ptr [[ALLOCA3]] to ptr addrspace(3)

; CHECK: [[GEP3:%[0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 40
; CHECK-NEXT: [[LOAD3:%[0-9]+]] = load i64, ptr [[GEP3]], align 8
; CHECK-NEXT: [[ALLOCA3:%[0-9]+]] = alloca i8, i64 [[LOAD3]], align 32
; CHECK-NEXT: %explicit_5 = addrspacecast ptr [[ALLOCA3]] to ptr addrspace(3)

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

; DEBUGIFY-COUNT-50: WARNING: Instruction with empty DebugLoc in function test
; DEBUGIFY-NOT: WARNING
