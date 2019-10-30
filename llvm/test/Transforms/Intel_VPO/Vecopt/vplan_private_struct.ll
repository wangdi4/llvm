; Check that private structs are correctly handled in VPlan CodeGen.

; TODO: Address the non-determinism in the placement of allocas (CMPLRLLVM-10636) and make necessary
; changes for VPValue CG.

;RUN: opt -S -VPlanDriver -vplan-force-vf=2 -enable-vp-value-codegen=false %s | FileCheck %s --check-prefixes=CHECK
;RUN: opt -S -VPlanDriver -vplan-force-vf=2 -enable-vp-value-codegen %s  | FileCheck %s --check-prefixes=CHECK-VPVALUE


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECKS for IR-based codegen.
; CHECK-LABEL: @_ZGVdN8u___block_fn_block_invoke_kernel

; CHECK: [[A:%.*]] = alloca [2 x %struct.ndrange_t.6], align 8
; CHECK-NEXT: [[B:%.*]] = alloca [2 x %struct.ndrange_t.6], align 8
; CHECK-NEXT: [[C:%.*]] = alloca [2 x <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>], align 8

; CHECK: [[BC3:%.*]] = bitcast [2 x <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>]* [[C]] to <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>*
; CHECK-NEXT: [[PRIV_BC_ADDRS3:%.*]] =  getelementptr <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* [[BC3]], <2 x i32> <i32 0, i32 1>

; CHECK-DAG: [[LT1:%.*]] = bitcast <2 x <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>*> [[PRIV_BC_ADDRS3]] to <2 x i8*>
; CHECK-NEXT: [[E1:%.*]] = extractelement <2 x i8*> [[LT1]], i32 1
; CHECK-NEXT: [[E2:%.*]] = extractelement <2 x i8*> [[LT1]], i32 0
; CHECK-NEXT: call void @llvm.lifetime.start.p0i8(i64 28, i8* nonnull [[E2]])
; CHECK-NEXT: call void @llvm.lifetime.start.p0i8(i64 28, i8* nonnull [[E1]])

; CHECK: [[BC2:%.*]] = bitcast [2 x %struct.ndrange_t.6]* [[B]] to %struct.ndrange_t.6*
; CHECK-NEXT: [[PRIV_BC_ADDRS2:%.*]] = getelementptr %struct.ndrange_t.6, %struct.ndrange_t.6* [[BC2]], <2 x i32> <i32 0, i32 1>

; CHECK: [[BC1:%.*]] = bitcast [2 x %struct.ndrange_t.6]* [[A]] to %struct.ndrange_t.6*
; CHECK-NEXT: [[PRIV_BC_ADDRS1:%.*]] = getelementptr %struct.ndrange_t.6, %struct.ndrange_t.6* [[BC1]], <2 x i32> <i32 0, i32 1>


; CHECKS for vpvalue-based codegen.

; CHECK-VPVALUE-DAG: [[A:%.*]] = alloca [2 x %struct.ndrange_t.6], align 8
; CHECK-VPVALUE-DAG: [[BC1:%.*]] = bitcast [2 x %struct.ndrange_t.6]* [[A]] to %struct.ndrange_t.6*
; CHECK-VPVALUE-DAG: [[PRIV_BC_ADDRS1:%.*]] = getelementptr %struct.ndrange_t.6, %struct.ndrange_t.6* [[BC1]], <2 x i32> <i32 0, i32 1>

; CHECK-VPVALUE-DAG: [[C:%.*]] = alloca [2 x <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>], align 8
; CHECK-VPVALUE-DAG: [[BC3:%.*]] = bitcast [2 x <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>]* [[C]] to <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>*
; CHECK-VPVALUE-DAG: [[PRIV_BC_ADDRS3:%.*]] =  getelementptr <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* [[BC3]], <2 x i32> <i32 0, i32 1>

; CHECK-VPVALUE-DAG: [[B:%.*]] = alloca [2 x %struct.ndrange_t.6], align 8
; CHECK-VPVALUE-DAG: [[BC2:%.*]] = bitcast [2 x %struct.ndrange_t.6]* [[B]] to %struct.ndrange_t.6*
; CHECK-VPVALUE-DAG: [[PRIV_BC_ADDRS2:%.*]] = getelementptr %struct.ndrange_t.6, %struct.ndrange_t.6* [[BC2]], <2 x i32> <i32 0, i32 1>

; CHECK-VPVALUE: [[LT1:%.*]] = bitcast <2 x <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>*> [[PRIV_BC_ADDRS3]] to <2 x i8*>
; CHECK-VPVALUE: [[E1:%.*]] = extractelement <2 x i8*> [[LT1]], i32 1
; CHECK-VPVALUE: [[E2:%.*]] = extractelement <2 x i8*> [[LT1]], i32 0
; CHECK-VPVALUE: call void @llvm.lifetime.start.p0i8(i64 28, i8* [[E2]])
; CHECK-VPVALUE: call void @llvm.lifetime.start.p0i8(i64 28, i8* [[E1]])


%struct.ndrange_t.6 = type { i32, [3 x i64], [3 x i64], [3 x i64] }


; Function Attrs: nounwind
define dso_local void @_ZGVdN8u___block_fn_block_invoke_kernel(i8 addrspace(4)*) {
entry:
  %ndrange.i.i = alloca %struct.ndrange_t.6, align 8
  %block = alloca <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>, align 8
  %tmp = alloca %struct.ndrange_t.6, align 8
  %call.i.i = tail call i64 @_Z13get_global_idj(i32 0)
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(i8 addrspace(4)* %0), "QUAL.OMP.PRIVATE"(%struct.ndrange_t.6* %ndrange.i.i, <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block, %struct.ndrange_t.6* %tmp), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %simd.loop

simd.loop:
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %1 = sext i32 %index to i64
  %add = add nuw i64 %1, %call.i.i
  %block.capture.addr.i = getelementptr inbounds i8, i8 addrspace(4)* %0, i64 16
  %2 = bitcast i8 addrspace(4)* %block.capture.addr.i to i32 addrspace(1)* addrspace(4)*
  %3 = load i32 addrspace(1)*, i32 addrspace(1)* addrspace(4)* %2, align 8
  %block.capture.addr1.i = getelementptr inbounds i8, i8 addrspace(4)* %0, i64 24
  %4 = bitcast i8 addrspace(4)* %block.capture.addr1.i to i32 addrspace(4)*
  %5 = load i32, i32 addrspace(4)* %4, align 8
  %6 = bitcast <{ i32, i32, i8 addrspace(4)*, i32 addrspace(1)*, i32 }>* %block to i8*
  call void @llvm.lifetime.start.p0i8(i64 28, i8* nonnull %6)
  %7 = bitcast %struct.ndrange_t.6* %tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %7)
  %8 = bitcast %struct.ndrange_t.6* %ndrange.i.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %8)
  %cmp.i.i = icmp slt i32 %5, 1
  br i1 %cmp.i.i, label %__block_fn_block_invoke.exit, label %simd.loop.exit

__block_fn_block_invoke.exit:
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %8)
  call void @llvm.lifetime.end.p0i8(i64 28, i8* nonnull %6)
  call void @llvm.lifetime.end.p0i8(i64 80, i8* nonnull %7)
  br label %simd.loop.exit

simd.loop.exit:
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture)

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

