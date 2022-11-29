; RUN: opt -S -passes='dpcpp-kernel-add-implicit-args,debugify,dpcpp-kernel-local-buffers,check-debugify' %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers -S -passes='dpcpp-kernel-add-implicit-args,debugify,dpcpp-kernel-local-buffers,check-debugify' %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -S -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-local-buffers' %s | FileCheck %s -check-prefixes=CHECK,NONOPAQUE
; RUN: opt -opaque-pointers -S -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-local-buffers' %s | FileCheck %s -check-prefixes=CHECK,OPAQUE

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

@a.__local = internal addrspace(3) global [3 x i8] undef, align 1
; CHECK-NOT: @a.__local = internal addrspace(3) global [3 x i8]

define void @kernel(i64 addrspace(1)* %arrayidx, i1 %condition) {
; CHECK-LABEL: entry:
; NONOPAQUE: [[LOCAL_MEM_PTR:%.*]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; NONOPAQUE: [[BC:%.*]] = bitcast i8 addrspace(3)* %0 to [3 x i8] addrspace(3)*
; NONOPAQUE: [[ENTRY_GEP:%.*]] = getelementptr inbounds [3 x i8], [3 x i8] addrspace(3)* [[BC]], i64 0, i64 1
; NONOPAQUE: [[ENTRY_PTRTOINT_GEP:%.*]] = ptrtoint i8 addrspace(3)* [[ENTRY_GEP]] to i64
; NONOPAQUE: [[ENTRY_PTRTOINT:%.*]] = ptrtoint [3 x i8] addrspace(3)* [[BC]] to i64
; OPAQUE: [[LOCAL_MEM_PTR:%.*]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; OPAQUE: [[ENTRY_GEP:%.*]] = getelementptr inbounds [3 x i8], ptr addrspace(3) [[LOCAL_MEM_PTR]], i64 0, i64 1
; OPAQUE: [[ENTRY_PTRTOINT_GEP:%.*]] = ptrtoint ptr addrspace(3) [[ENTRY_GEP]] to i64
; OPAQUE: [[ENTRY_PTRTOINT:%.*]] = ptrtoint ptr addrspace(3) [[LOCAL_MEM_PTR]] to i64
entry:
  br i1 %condition, label %A, label %B

; CHECK-LABEL: A:
; NONOPAQUE-DAG: [[A_PTRTOINT:%.*]] = ptrtoint [3 x i8] addrspace(3)* [[BC]] to i64
; NONOPAQUE-DAG: [[A_PTRTOINT1:%.*]] = ptrtoint [3 x i8] addrspace(3)* [[BC]] to i64
; OPAQUE-DAG: [[A_PTRTOINT:%.*]] = ptrtoint ptr addrspace(3) [[LOCAL_MEM_PTR]] to i64
; OPAQUE-DAG: [[A_PTRTOINT1:%.*]] = ptrtoint ptr addrspace(3) [[LOCAL_MEM_PTR]] to i64
A:
  br label %B

; CHECK-LABEL: B:
; CHECK-NEXT: %sub_op0 = phi i64 [ [[ENTRY_PTRTOINT_GEP]], %entry ], [ [[A_PTRTOINT]], %A ]
; CHECK-NEXT: %sub_op1 = phi i64 [ [[ENTRY_PTRTOINT]], %entry ], [ [[A_PTRTOINT1]], %A ]
B:
  %sub_op0 = phi i64 [ ptrtoint (i8 addrspace(3)* getelementptr inbounds ([3 x i8], [3 x i8] addrspace(3)* @a.__local, i64 0, i64 1) to i64), %entry ], [ ptrtoint ([3 x i8] addrspace(3)* @a.__local to i64), %A ]
  %sub_op1 = phi i64 [ ptrtoint ([3 x i8] addrspace(3)* @a.__local to i64), %entry ], [ ptrtoint ([3 x i8] addrspace(3)* @a.__local to i64), %A ]
  %sub = sub i64 %sub_op0, %sub_op1
  store i64 %sub, i64 addrspace(1)* %arrayidx, align 8
  ret void
}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
