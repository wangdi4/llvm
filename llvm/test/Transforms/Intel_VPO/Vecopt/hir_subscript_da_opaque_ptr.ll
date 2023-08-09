; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" \
; RUN:     -vplan-enable-hir-private-arrays -vplan-dump-plan-da --vplan-enable-masked-variant=0 -disable-output < %s 2>&1 | FileCheck %s

; Verify that we no longer crash in DA shape analysis in the presence
; of opaque pointers.  Test reduced from radialblur as reported in
; CMPLRLLVM-42386, with variants added.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

; CHECK: Printing Divergence info for foo:HIR.#1
; CHECK: Printing Divergence info for foo:HIR.#1
; CHECK: Basic Block: BB3
; CHECK: Divergent: [Shape: SOA Unit Stride, Stride: i64 4] ptr [[PTR1:%.*]] = subscript inbounds ptr [[PTR0:%.*]]
; CHECK: Divergent: [Shape: SOA Unit Stride, Stride: i64 4] store float %centerx ptr [[PTR1]]

define void @foo(float %centerx) {
entry:
  %uvy = alloca [8 x float], align 16
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.PRIVATE:TYPED"(ptr %uvy, [8 x float] zeroinitializer, i32 1) ]
  br label %simd.loop.header

simd.loop.header:
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.header ]
  store float %centerx, ptr %uvy, align 4
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop.header, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; This variant tests presence of address-space casts and subscripts
; having no dimensions.

; CHECK: Printing Divergence info for bar:HIR.#2
; CHECK: Printing Divergence info for bar:HIR.#2
; CHECK: Basic Block: BB9
; CHECK: Divergent: [Shape: SOA Unit Stride, Stride: i64 4] ptr [[PTR3:%.*]] = subscript inbounds ptr [[PTR2:%.*]]
; CHECK: Divergent: [Shape: SOA Unit Stride, Stride: i64 4] ptr addrspace(1) [[PTR4:%.*]] = addrspacecast ptr [[PTR3]]
; CHECK: Divergent: [Shape: SOA Unit Stride, Stride: i64 4] ptr addrspace(1) [[PTR5:%.*]] = subscript inbounds ptr addrspace(1) [[PTR4]]
; CHECK: Divergent: [Shape: SOA Unit Stride, Stride: i64 4] ptr [[PTR6:%.*]] = addrspacecast ptr addrspace(1) [[PTR5]]
; CHECK: Divergent: [Shape: SOA Unit Stride, Stride: i64 4] ptr [[PTR7:%.*]] = subscript inbounds ptr [[PTR6]]
; CHECK: Divergent: [Shape: SOA Unit Stride, Stride: i64 4] store float %centerx ptr [[PTR7]]

define void @bar(float %centerx) {
entry:
  %uvy = alloca [8 x float], align 16
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.PRIVATE:TYPED"(ptr %uvy, [8 x float] zeroinitializer, i32 1) ]
  br label %simd.loop.header

simd.loop.header:
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.header ]
  %castptr = addrspacecast ptr %uvy to ptr addrspace(1)
  %castbackptr = addrspacecast ptr addrspace(1) %castptr to ptr addrspace(0)
  store float %centerx, ptr %castbackptr, align 4
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop.header, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Variant with multiple stores having matching types.  Each of these gets
; its own VPSubscriptInst.

; CHECK: Printing Divergence info for baz:HIR.#3
; CHECK: Printing Divergence info for baz:HIR.#3
; CHECK: Basic Block: BB17
; CHECK: Divergent: [Shape: SOA Unit Stride, Stride: i64 4] ptr [[PTR9:%.*]] = subscript inbounds ptr [[PTR8:%.*]]
; CHECK: Divergent: [Shape: SOA Unit Stride, Stride: i64 4] store float %another ptr [[PTR9]]
; CHECK: Basic Block: BB16
; CHECK: Divergent: [Shape: SOA Unit Stride, Stride: i64 4] ptr [[PTR11:%.*]] = subscript inbounds ptr [[PTR10:%.*]]
; CHECK: Divergent: [Shape: SOA Unit Stride, Stride: i64 4] store float %centerx ptr [[PTR11]]

define void @baz(float %centerx, float %another, i1 %flag) {
entry:
  %uvy = alloca [8 x float], align 16
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.PRIVATE:TYPED"(ptr %uvy, [8 x float] zeroinitializer, i32 1) ]
  br label %simd.loop.header

simd.loop.header:
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %latch ]
  br i1 %flag, label %if.block, label %then.block

if.block:
  store float %centerx, ptr %uvy, align 4
  br label %latch

then.block:
  store float %another, ptr %uvy, align 4
  br label %latch

latch:
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop.header, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Variant with multiple stores having mismatched types of proper size.
; Each of these also gets its own VPSubscriptInst.

; CHECK: Printing Divergence info for rebaz:HIR.#4
; CHECK: Printing Divergence info for rebaz:HIR.#4
; CHECK: Basic Block: BB26
; CHECK: Divergent: [Shape: SOA Unit Stride, Stride: i64 4] ptr [[PTR13:%.*]] = subscript inbounds ptr [[PTR12:%.*]] 
; CHECK: Divergent: [Shape: SOA Unit Stride, Stride: i64 4] store i32 %another ptr [[PTR13]]
; CHECK: Basic Block: BB25
; CHECK: Divergent: [Shape: SOA Unit Stride, Stride: i64 4] ptr [[PTR15:%.*]] = subscript inbounds ptr [[PTR14:%.*]]
; CHECK: Divergent: [Shape: SOA Unit Stride, Stride: i64 4] store float %centerx ptr [[PTR15]]

define void @rebaz(float %centerx, i32 %another, i1 %flag) {
entry:
  %uvy = alloca [8 x float], align 16
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.PRIVATE:TYPED"(ptr %uvy, [8 x float] zeroinitializer, i32 1) ]
  br label %simd.loop.header

simd.loop.header:
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %latch ]
  br i1 %flag, label %if.block, label %then.block

if.block:
  store float %centerx, ptr %uvy, align 4
  br label %latch

then.block:
  store i32 %another, ptr %uvy, align 4
  br label %latch

latch:
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop.header, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Variant with multiple stores of different sizes and address-space casts.
; We punt SOA shape analysis in this case due to the size mismatch.

; CHECK: Printing Divergence info for threebaz:HIR.#5
; CHECK: Basic Block: BB35
; CHECK: Divergent: [Shape: Strided, Stride: i64 32] ptr [[PTR17:%.*]] = subscript inbounds ptr [[PTR16:%.*]]
; CHECK: Divergent: [Shape: Strided, Stride: i64 32] ptr addrspace(1) [[PTR18:%.*]] = addrspacecast ptr [[PTR17]]
; CHECK: Divergent: [Shape: Strided, Stride: i64 32] ptr addrspace(1) [[PTR19:%.*]] = subscript inbounds ptr addrspace(1) [[PTR18]]
; CHECK: Divergent: [Shape: Strided, Stride: i64 32] ptr [[PTR20:%.*]] = addrspacecast ptr addrspace(1) [[PTR19]]
; CHECK: Divergent: [Shape: Strided, Stride: i64 32] ptr [[PTR21:%.*]] = subscript inbounds ptr [[PTR20]]
; CHECK: Divergent: [Shape: Strided, Stride: i64 32] store i16 %another ptr [[PTR21]]
; CHECK: Basic Block: BB34
; CHECK: Divergent: [Shape: Strided, Stride: i64 32] ptr [[PTR22:%.*]] = subscript inbounds ptr [[PTR16]]
; CHECK: Divergent: [Shape: Strided, Stride: i64 32] store float %centerx ptr [[PTR22]]

define void @threebaz(float %centerx, i16 %another, i1 %flag) {
entry:
  %uvy = alloca [8 x float], align 16
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.PRIVATE:TYPED"(ptr %uvy, [8 x float] zeroinitializer, i32 1) ]
  br label %simd.loop.header

simd.loop.header:
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %latch ]
  br i1 %flag, label %if.block, label %then.block

if.block:
  store float %centerx, ptr %uvy, align 4
  br label %latch

then.block:
  %castptr = addrspacecast ptr %uvy to ptr addrspace(1)
  %castbackptr = addrspacecast ptr addrspace(1) %castptr to ptr addrspace(0)
  store i16 %another, ptr %castbackptr, align 4
  br label %latch

latch:
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop.header, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

