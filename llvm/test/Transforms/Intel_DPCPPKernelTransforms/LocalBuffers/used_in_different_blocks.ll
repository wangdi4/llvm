; RUN: opt -S -dpcpp-kernel-add-implicit-args -debugify -dpcpp-kernel-local-buffers -check-debugify %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -S -passes='dpcpp-kernel-add-implicit-args,debugify,dpcpp-kernel-local-buffers,check-debugify' %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -S -dpcpp-kernel-add-implicit-args -dpcpp-kernel-local-buffers %s | FileCheck %s
; RUN: opt -S -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-local-buffers' %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

@a.__local = internal addrspace(3) global [3 x i8] undef, align 1
; CHECK-NOT: @a.__local = internal addrspace(3) global [3 x i8]

; Check that a sequence of instructions are built in every basic block, right before the user instructions,
; so that the created instructions dominate all their uses.
define void @kernel(i64 addrspace(1)* %arrayidx, i1 %condition) {
; CHECK-LABEL: entry:
; CHECK: [[LOCAL_MEM_PTR:%.*]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK-NEXT: [[GEP_ASC:%.*]] = addrspacecast i8 addrspace(3)* [[LOCAL_MEM_PTR]] to [3 x i8] addrspace(3)**
; CHECK-NEXT: [[A_LOCAL_PTR:%.*]] = load [3 x i8] addrspace(3)*, [3 x i8] addrspace(3)** [[GEP_ASC]], align 8
entry:
  br i1 %condition, label %if, label %else

; CHECK-LABEL: if:
; CHECK: [[IF_GEP1:%.*]] = getelementptr inbounds [3 x i8], [3 x i8] addrspace(3)* [[A_LOCAL_PTR]], i64 0, i64 1
; CHECK-NEXT: [[IF_PTRTOINT1:%.*]] = ptrtoint i8 addrspace(3)* [[IF_GEP1]] to i64
; CHECK: [[IF_PTRTOINT:%.*]] = ptrtoint [3 x i8] addrspace(3)* [[A_LOCAL_PTR]] to i64
; CHECK-NEXT: [[IF_SUB:%.*]] = sub i64 [[IF_PTRTOINT1]], [[IF_PTRTOINT]]
; CHECK-NEXT: store i64 [[IF_SUB]], i64 addrspace(1)* %arrayidx, align 8
if:
  store i64 sub (i64 ptrtoint (i8 addrspace(3)* getelementptr inbounds ([3 x i8], [3 x i8] addrspace(3)* @a.__local, i64 0, i64 1) to i64), i64 ptrtoint ([3 x i8] addrspace(3)* @a.__local to i64)), i64 addrspace(1)* %arrayidx, align 8
  ret void

; CHECK-LABEL: else:
; CHECK: [[ELSE_GEP1:%.*]] = getelementptr inbounds [3 x i8], [3 x i8] addrspace(3)* [[A_LOCAL_PTR]], i64 0, i64 1
; CHECK-NEXT: [[ELSE_PTRTOINT1:%.*]] = ptrtoint i8 addrspace(3)* [[ELSE_GEP1]] to i64
; CHECK: [[ELSE_PTRTOINT:%.*]] = ptrtoint [3 x i8] addrspace(3)* [[A_LOCAL_PTR]] to i64
; CHECK-NEXT: [[ELSE_SUB:%.*]] = sub i64 [[ELSE_PTRTOINT1]], [[ELSE_PTRTOINT]]
; CHECK-NEXT: store i64 [[ELSE_SUB]], i64 addrspace(1)* %arrayidx, align 8
else:
  store i64 sub (i64 ptrtoint (i8 addrspace(3)* getelementptr inbounds ([3 x i8], [3 x i8] addrspace(3)* @a.__local, i64 0, i64 1) to i64), i64 ptrtoint ([3 x i8] addrspace(3)* @a.__local to i64)), i64 addrspace(1)* %arrayidx, align 8
  ret void
}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
