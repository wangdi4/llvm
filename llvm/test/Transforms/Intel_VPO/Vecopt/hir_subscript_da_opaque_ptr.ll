; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" < %s

; Verify that we no longer crash in DA shape analysis in the presence
; of opaque pointers.  Test reduced from radialblur as reported in
; CMPLRLLVM-42386.  This is a crash test only.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define void @foo(float %centerx) {
entry:
  %uvy = alloca [8 x float], align 16
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.PRIVATE:TYPED"(ptr %uvy, [8 x float] zeroinitializer, i32 1) ]
  br label %simd.loop.header

simd.loop.header:
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.header ]
  %arrayidx37 = getelementptr inbounds [8 x float], ptr %uvy, i64 0, i64 0
  store float %centerx, ptr %arrayidx37, align 4
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop.header, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Variant with address-space casts.  We don't crash because an extra
; VPSubscriptInst is inserted by VPlan before the store.

define void @bar(float %centerx) {
entry:
  %uvy = alloca [8 x float], align 16
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.PRIVATE:TYPED"(ptr %uvy, [8 x float] zeroinitializer, i32 1) ]
  br label %simd.loop.header

simd.loop.header:
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.header ]
  %arrayidx37 = getelementptr inbounds [8 x float], ptr %uvy, i64 0, i64 0
  %castptr = addrspacecast ptr %arrayidx37 to ptr addrspace(1)
  %castbackptr = addrspacecast ptr addrspace(1) %castptr to ptr addrspace(0)
  store float %centerx, ptr %castbackptr, align 4
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop.header, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Variant with multiple stores having matching types.  Each of these also
; gets its own VPSubscriptInst.

define void @baz(float %centerx, float %another, i1 %flag) {
entry:
  %uvy = alloca [8 x float], align 16
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.PRIVATE:TYPED"(ptr %uvy, [8 x float] zeroinitializer, i32 1) ]
  br label %simd.loop.header

simd.loop.header:
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %latch ]
  %arrayidx37 = getelementptr inbounds [8 x float], ptr %uvy, i64 0, i64 0
  br i1 %flag, label %if.block, label %then.block

if.block:
  store float %centerx, ptr %arrayidx37, align 4
  br label %latch

then.block:
  store float %another, ptr %arrayidx37, align 4
  br label %latch

latch:
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop.header, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Variant with multiple stores having mismatched types.  Each of these also
; gets its own VPSubscriptInst.

define void @rebaz(float %centerx, i32 %another, i1 %flag) {
entry:
  %uvy = alloca [8 x float], align 16
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.PRIVATE:TYPED"(ptr %uvy, [8 x float] zeroinitializer, i32 1) ]
  br label %simd.loop.header

simd.loop.header:
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %latch ]
  %arrayidx37 = getelementptr inbounds [8 x float], ptr %uvy, i64 0, i64 0
  br i1 %flag, label %if.block, label %then.block

if.block:
  store float %centerx, ptr %arrayidx37, align 4
  br label %latch

then.block:
  store i32 %another, ptr %arrayidx37, align 4
  br label %latch

latch:
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop.header, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Variant with multiple stores and address-space casts.

define void @threebaz(float %centerx, float %another, i1 %flag) {
entry:
  %uvy = alloca [8 x float], align 16
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.PRIVATE:TYPED"(ptr %uvy, [8 x float] zeroinitializer, i32 1) ]
  br label %simd.loop.header

simd.loop.header:
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %latch ]
  %arrayidx37 = getelementptr inbounds [8 x float], ptr %uvy, i64 0, i64 0
  br i1 %flag, label %if.block, label %then.block

if.block:
  store float %centerx, ptr %arrayidx37, align 4
  br label %latch

then.block:
  %castptr = addrspacecast ptr %arrayidx37 to ptr addrspace(1)
  %castbackptr = addrspacecast ptr addrspace(1) %castptr to ptr addrspace(0)
  store float %another, ptr %castbackptr, align 4
  br label %latch

latch:
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop.header, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

