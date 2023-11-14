; Test to check that users of local buffers in scatter intrinsics are llvm::Constant and not llvm::ConstantExpr.

; RUN: opt -passes='sycl-kernel-add-implicit-args,debugify,sycl-kernel-local-buffers,check-debugify' -S < %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-local-buffers' -S < %s | FileCheck %s

; CHECK-LABEL: void @foo
; CHECK: [[GEP:%.*]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; CHECK: [[INSERT_0:%.*]] = insertelement <2 x ptr addrspace(3)> poison, ptr addrspace(3) [[GEP]], i32 0
; CHECK-NEXT: [[INSERT_1:%.*]] = insertelement <2 x ptr addrspace(3)> [[INSERT_0]], ptr addrspace(3) [[GEP]], i32 1
; CHECK-NEXT: call void @llvm.masked.scatter.v2p4.v2p3(<2 x ptr addrspace(4)> {{%.*}}, <2 x ptr addrspace(3)> [[INSERT_1]], i32 8, <2 x i1> <i1 true, i1 true>)


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

@GlobalPtr = internal unnamed_addr addrspace(3) global ptr addrspace(4) undef, align 8

define void @foo() {
entry:
  %.vec = alloca [2 x i32], align 8
  %0 = getelementptr i32, ptr %.vec, <2 x i32> <i32 0, i32 1>
  %1 = addrspacecast <2 x ptr> %0 to <2 x ptr addrspace(4)>
  call void @llvm.masked.scatter.v2p4.v2p3(<2 x ptr addrspace(4)> %1, <2 x ptr addrspace(3)> <ptr addrspace(3) @GlobalPtr, ptr addrspace(3) @GlobalPtr>, i32 8, <2 x i1> <i1 true, i1 true>)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.masked.scatter.v2p4.v2p3(<2 x ptr addrspace(4)>, <2 x ptr addrspace(3)>, i32 immarg, <2 x i1>)

!sycl.kernels = !{!0}

!0 = !{ptr @foo}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
