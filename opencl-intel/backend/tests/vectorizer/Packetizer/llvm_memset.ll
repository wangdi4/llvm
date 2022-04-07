; RUN: %oclopt -S -runtimelib %p/../Full/runtime.bc -packet-size=4 -packetize -dce %s | FileCheck %s

; The test checks that unsupported llvm.memset are correctly serialized, and
; llvm.memset on address who is only referenced by safe llvm intrinsics are
; vectorized.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test() local_unnamed_addr {
; CHECK-LABEL: define void @test
; CHECK-LABEL: entry:
; CHECK-NEXT: alloca [1024 x i8]
; CHECK-NEXT: alloca [1024 x i8]
; CHECK-NEXT: alloca [1024 x i8]
; CHECK-NEXT: alloca [1024 x i8]
; CHECK-NEXT: bitcast [1024 x i8]* {{.*}} to i8*
; CHECK-NEXT: bitcast [1024 x i8]* {{.*}} to i8*
; CHECK-NEXT: bitcast [1024 x i8]* {{.*}} to i8*
; CHECK-NEXT: bitcast [1024 x i8]* {{.*}} to i8*
; CHECK-NEXT: call void @foo
; CHECK-NEXT: call void @foo
; CHECK-NEXT: call void @foo
; CHECK-NEXT: call void @foo
; CHECK-NEXT: call void @llvm.memset.p0i8.i64
; CHECK-NEXT: call void @llvm.memset.p0i8.i64
; CHECK-NEXT: call void @llvm.memset.p0i8.i64
; CHECK-NEXT: call void @llvm.memset.p0i8.i64
; CHECK-NEXT: ret void
entry:
  %local_ptr = alloca [1024 x i8], align 4
  %local_ptr.bc = bitcast [1024 x i8]* %local_ptr to i8*
  call void @foo(i8* %local_ptr.bc)
  call void @llvm.memset.p0i8.i64(i8* nonnull align 8 dereferenceable(1024) %local_ptr.bc, i8 0, i64 1024, i1 false)
  ret void
}

define void @test2() local_unnamed_addr {
; CHECK-LABEL: define void @test2
; CHECK-LABEL: entry:
; CHECK: [[PACKED_ALLOCA:%.*]] = alloca [1024 x <4 x i8>], align 16
; CHECK: [[ADDR:%.*]] = bitcast [1024 x <4 x i8>]* [[PACKED_ALLOCA]] to i8*
; CHECK: call void @llvm.lifetime.start.p0i8(i64 1024, i8* nonnull [[ADDR]])
; CHECK: call void @llvm.memset.p0i8.i64(i8* nonnull align 8 dereferenceable(1024) [[ADDR]], i8 0, i64 4096, i1 false)
; CHECK: call void @llvm.lifetime.end.p0i8(i64 1024, i8* nonnull [[ADDR]])
; CHECK: ret void
entry:
  %local_ptr = alloca [1024 x i8], align 4
  %local_ptr.bc = bitcast [1024 x i8]* %local_ptr to i8*
  call void @llvm.lifetime.start.p0i8(i64 1024, i8* nonnull %local_ptr.bc)
  call void @llvm.memset.p0i8.i64(i8* nonnull align 8 dereferenceable(1024) %local_ptr.bc, i8 0, i64 1024, i1 false)
  call void @llvm.lifetime.end.p0i8(i64 1024, i8* nonnull %local_ptr.bc)
  ret void
}

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)
declare void @foo(i8*)

attributes #0 = { nounwind readnone speculatable willreturn }

!sycl.kernels = !{!0}

!0 = !{void()* @test, void()* @test2}
