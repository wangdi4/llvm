; Test to check that users of local buffers in scatter intrinsics are llvm::Constant and not llvm::ConstantExpr.

; RUN: %oclopt -add-implicit-args -local-buffers -S < %s | FileCheck %s

; CHECK-LABEL: void @foo
; CHECK: [[GEP:%.*]] = getelementptr i8, i8 addrspace(3)* %pLocalMemBase, i32 0
; CHECK-NEXT: [[GEP_BC:%.*]] = bitcast i8 addrspace(3)* [[GEP]] to i32 addrspace(4)* addrspace(3)*
; CHECK-NEXT: [[INSERT_0:%.*]] = insertelement <2 x i32 addrspace(4)* addrspace(3)*> undef, i32 addrspace(4)* addrspace(3)* [[GEP_BC]], i32 0
; CHECK-NEXT: [[INSERT_1:%.*]] = insertelement <2 x i32 addrspace(4)* addrspace(3)*> [[INSERT_0]], i32 addrspace(4)* addrspace(3)* %1, i32 1
; CHECK: call void @llvm.masked.scatter.v2p4i32.v2p3p4i32(<2 x i32 addrspace(4)*> {{%.*}}, <2 x i32 addrspace(4)* addrspace(3)*> [[INSERT_1]], i32 8, <2 x i1> <i1 true, i1 true>)


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

@GlobalPtr = internal unnamed_addr addrspace(3) global i32 addrspace(4)* undef, align 8

define void @foo() {
entry:
  %.vec = alloca [2 x i32], align 8
  %privaddr = bitcast [2 x i32]* %.vec to i32*
  %0 = getelementptr i32, i32* %privaddr, <2 x i32> <i32 0, i32 1>
  %1 = addrspacecast <2 x i32*> %0 to <2 x i32 addrspace(4)*>
  call void @llvm.masked.scatter.v2p4i32.v2p3p4i32(<2 x i32 addrspace(4)*> %1, <2 x i32 addrspace(4)* addrspace(3)*> <i32 addrspace(4)* addrspace(3)* @GlobalPtr, i32 addrspace(4)* addrspace(3)* @GlobalPtr>, i32 8, <2 x i1> <i1 true, i1 true>)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.masked.scatter.v2p4i32.v2p3p4i32(<2 x i32 addrspace(4)*>, <2 x i32 addrspace(4)* addrspace(3)*>, i32 immarg, <2 x i1>)
