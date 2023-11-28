; RUN: opt -S -passes='sycl-kernel-add-implicit-args,debugify,sycl-kernel-local-buffers,check-debugify' %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -S -passes='sycl-kernel-add-implicit-args,sycl-kernel-local-buffers' %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

@a.__local = internal addrspace(3) global [3 x i8] undef, align 1
; CHECK-NOT: @a.__local = internal addrspace(3) global [3 x i8]

define void @kernel(ptr addrspace(1) %arrayidx, i1 %condition) !kernel_arg_base_type !1 !arg_type_null_val !2 {
; CHECK-LABEL: entry:
; CHECK: [[LOCAL_MEM_PTR:%.*]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; CHECK: [[ENTRY_GEP:%.*]] = getelementptr inbounds [3 x i8], ptr addrspace(3) [[LOCAL_MEM_PTR]], i64 0, i64 1
; CHECK: [[ENTRY_PTRTOINT_GEP:%.*]] = ptrtoint ptr addrspace(3) [[ENTRY_GEP]] to i64
; CHECK: [[ENTRY_PTRTOINT:%.*]] = ptrtoint ptr addrspace(3) [[LOCAL_MEM_PTR]] to i64
entry:
  br i1 %condition, label %A, label %B

; CHECK-LABEL: A:
; CHECK-DAG: [[A_PTRTOINT:%.*]] = ptrtoint ptr addrspace(3) [[LOCAL_MEM_PTR]] to i64
; CHECK-DAG: [[A_PTRTOINT1:%.*]] = ptrtoint ptr addrspace(3) [[LOCAL_MEM_PTR]] to i64
A:
  br label %B

; CHECK-LABEL: B:
; CHECK-NEXT: %sub_op0 = phi i64 [ [[ENTRY_PTRTOINT_GEP]], %entry ], [ [[A_PTRTOINT]], %A ]
; CHECK-NEXT: %sub_op1 = phi i64 [ [[ENTRY_PTRTOINT]], %entry ], [ [[A_PTRTOINT1]], %A ]
B:
  %sub_op0 = phi i64 [ ptrtoint (ptr addrspace(3) getelementptr inbounds ([3 x i8], ptr addrspace(3) @a.__local, i64 0, i64 1) to i64), %entry ], [ ptrtoint (ptr addrspace(3) @a.__local to i64), %A ]
  %sub_op1 = phi i64 [ ptrtoint (ptr addrspace(3) @a.__local to i64), %entry ], [ ptrtoint (ptr addrspace(3) @a.__local to i64), %A ]
  %sub = sub i64 %sub_op0, %sub_op1
  store i64 %sub, ptr addrspace(1) %arrayidx, align 8
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @kernel}
!1 = !{!"long*", !"bool"}
!2 = !{ptr addrspace(1) null, i1 0}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
