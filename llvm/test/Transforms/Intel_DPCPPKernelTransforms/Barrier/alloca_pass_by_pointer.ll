; This test checks cross-barrier function argument is correctly handled when
; callee function has a barrier and is not inlined.
; Before this patch, only dst[0] has correct result in -g -cl-opt-disable mode.
;
; RUN: opt -dpcpp-kernel-data-per-value-analysis -dpcpp-kernel-analysis -dpcpp-kernel-barrier %s -S | FileCheck %s
;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @process(i32 addrspace(1)* noalias %dst, i32 addrspace(3)* noalias %x, i32* noalias %lid) #0 {
entry:
; CHECK-LABEL: entry:
; CHECK: %dst.addr.addr = alloca i32 addrspace(1)**
; CHECK: %x.addr.addr = alloca i32 addrspace(3)**
; CHECK: %lid.addr.addr = alloca i32**
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  %dst.addr = alloca i32 addrspace(1)*, align 8
  %x.addr = alloca i32 addrspace(3)*, align 8
  %lid.addr = alloca i32*, align 8
  store i32 addrspace(1)* %dst, i32 addrspace(1)** %dst.addr, align 8
  store i32 addrspace(3)* %x, i32 addrspace(3)** %x.addr, align 8
  store i32* %lid, i32** %lid.addr, align 8
  %0 = load i32 addrspace(3)*, i32 addrspace(3)** %x.addr, align 8
  %1 = load i32*, i32** %lid.addr, align 8
  %2 = load i32, i32* %1, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds i32, i32 addrspace(3)* %0, i64 %idxprom
  store i32 1, i32 addrspace(3)* %arrayidx, align 4
  br label %"Barrier BB1"

; CHECK-LABEL: SyncBB2:
; CHECK: [[SBIndex0:%SBIndex[0-9]+]] = load i64, i64* %pCurrSBIndex
; CHECK-NEXT: [[Offset0:%SB_LocalId_Offset[0-9]+]] = add nuw i64 [[SBIndex0]], {{[0-9]+}}
; CHECK-NEXT: [[GEP0:%[a-zA-Z0-9]+]] = getelementptr inbounds i8, i8* %pSB, i64 [[Offset0]]
; CHECK-NEXT: [[LocalId0:%pSB_LocalId[0-9]+]] = bitcast i8* [[GEP0]] to i32**
; CHECK-NEXT: store i32** [[LocalId0]], i32*** %lid.addr.addr
; CHECK-NEXT: [[L0:%[0-9]+]] = load i32**, i32*** %lid.addr.addr
; CHECK: store i32* %loadedValue{{[0-9]+}}, i32** [[L0]], align 8
; CHECK: %{{[0-9]+}} = load i32*, i32** [[L0]], align 8

"Barrier BB1":                                    ; preds = %entry
  call void @__builtin_dpcpp_kernel_barrier(i32 1) #4
  %3 = load i32 addrspace(3)*, i32 addrspace(3)** %x.addr, align 8
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(3)* %3, i64 0
  %4 = load i32, i32 addrspace(3)* %arrayidx1, align 4
  %5 = load i32 addrspace(1)*, i32 addrspace(1)** %dst.addr, align 8
  %6 = load i32*, i32** %lid.addr, align 8
  %7 = load i32, i32* %6, align 4
  %idxprom2 = sext i32 %7 to i64
  %arrayidx3 = getelementptr inbounds i32, i32 addrspace(1)* %5, i64 %idxprom2
  store i32 %4, i32 addrspace(1)* %arrayidx3, align 4
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %"Barrier BB1"
  call void @__builtin_dpcpp_kernel_barrier(i32 1)
  ret void
}

; Function Attrs: convergent
declare void @__builtin_dpcpp_kernel_barrier(i32) #1

define void @test(i32 addrspace(1)* noalias %dst, i32 addrspace(3)* noalias %x) #2 {
entry:
; CHECK-LABEL: entry:
; CHECK: %dst.addr.addr = alloca i32 addrspace(1)**
; CHECK: %x.addr.addr = alloca i32 addrspace(3)**
; CHECK: %lid.addr = alloca i32*
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  %dst.addr = alloca i32 addrspace(1)*, align 8
  %x.addr = alloca i32 addrspace(3)*, align 8
  %lid = alloca i32, align 4
  store i32 addrspace(1)* %dst, i32 addrspace(1)** %dst.addr, align 8
  store i32 addrspace(3)* %x, i32 addrspace(3)** %x.addr, align 8
  %call = call i64 @__builtin_get_local_id(i32 0) #5
  %conv = trunc i64 %call to i32
  store i32 %conv, i32* %lid, align 4
  %0 = load i32 addrspace(1)*, i32 addrspace(1)** %dst.addr, align 8
  %1 = load i32 addrspace(3)*, i32 addrspace(3)** %x.addr, align 8
  br label %"Barrier BB1"

; CHECK-LABEL: SyncBB3:
; CHECK: [[L1:%[0-9]+]] = load i32*, i32** %lid.addr
; CHECK: [[LocalId0:%LocalId_[0-9]+]] = load i64, i64* %pLocalId_0
; CHECK-NEXT: %conv = trunc i64 [[LocalId0]] to i32
; CHECK-NEXT: store i32 %conv, i32* [[L1]], align 4

"Barrier BB1":                                    ; preds = %entry
  call void @__builtin_dpcpp_kernel_barrier(i32 1)

; CHECK-LABEL: SyncBB1:
; CHECK: store i32* %pSB_LocalId{{[0-9]+}}, i32** %lid.addr
; CHECK-NEXT: [[L2:%[0-9]+]] = load i32*, i32** %lid.addr
; CHECK: call void @process(i32 addrspace(1)* %loadedValue{{[0-9]+}}, i32 addrspace(3)* %loadedValue{{[0-9]+}}, i32* [[L2]]) #5

  call void @process(i32 addrspace(1)* %0, i32 addrspace(3)* %1, i32* %lid) #6
  br label %"Barrier BB2"

"Barrier BB2":                                    ; preds = %"Barrier BB1"
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %"Barrier BB2"
  call void @__builtin_dpcpp_kernel_barrier(i32 1)
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @__builtin_get_local_id(i32) #3

declare void @__builtin_dpcpp_kernel_barrier_dummy()

attributes #0 = { convergent noinline "kernel-call-once" "kernel-convergent-call" }
attributes #1 = { convergent "kernel-call-once" "kernel-convergent-call" }
attributes #2 = { convergent noinline "sycl_kernel" "kernel-call-once" "kernel-convergent-call" }
attributes #3 = { convergent readnone }
attributes #4 = { convergent "kernel-call-once" "kernel-convergent-call" }
attributes #5 = { convergent nounwind readnone }
attributes #6 = { convergent }

