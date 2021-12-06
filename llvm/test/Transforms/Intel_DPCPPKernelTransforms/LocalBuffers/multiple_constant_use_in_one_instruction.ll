; RUN: opt -S -dpcpp-kernel-add-implicit-args -debugify -dpcpp-kernel-local-buffers -check-debugify %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -S -passes='dpcpp-kernel-add-implicit-args,debugify,dpcpp-kernel-local-buffers,check-debugify' %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -S -dpcpp-kernel-add-implicit-args -dpcpp-kernel-local-buffers %s | FileCheck %s
; RUN: opt -S -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-local-buffers' %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

@a.__local = internal addrspace(3) global [3 x i8] undef, align 1
; CHECK-NOT: @a.__local = internal addrspace(3) global [3 x i8]

define void @kernel(i64 addrspace(1)* %arrayidx) {
entry:
  store i64 sub (i64 ptrtoint (i8 addrspace(3)* getelementptr inbounds ([3 x i8], [3 x i8] addrspace(3)* @a.__local, i64 0, i64 1) to i64), i64 ptrtoint ([3 x i8] addrspace(3)* @a.__local to i64)), i64 addrspace(1)* %arrayidx, align 8
  ret void
}
; CHECK: [[LOCAL_MEM_PTR:%.*]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK: [[GEP_ASC:%.*]] = addrspacecast i8 addrspace(3)* [[LOCAL_MEM_PTR]] to [3 x i8] addrspace(3)**
; CHECK-NEXT: [[A_LOCAL_PTR:%.*]] = load [3 x i8] addrspace(3)*, [3 x i8] addrspace(3)** [[GEP_ASC]], align 8
; CHECK-NEXT: [[GEP1:%.*]] = getelementptr inbounds [3 x i8], [3 x i8] addrspace(3)* [[A_LOCAL_PTR]], i64 0, i64 1
; CHECK-NEXT: [[PTRTOINT1:%.*]] = ptrtoint i8 addrspace(3)* [[GEP1]] to i64
; CHECK: [[PTRTOINT:%.*]] = ptrtoint [3 x i8] addrspace(3)* [[A_LOCAL_PTR]] to i64
; CHECK-NEXT: [[SUB:%.*]] = sub i64 [[PTRTOINT1]], [[PTRTOINT]]
; CHECK-NEXT: store i64 [[SUB]], i64 addrspace(1)* %arrayidx, align 8

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
