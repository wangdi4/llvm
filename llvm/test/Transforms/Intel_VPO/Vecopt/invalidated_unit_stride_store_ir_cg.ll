; RUN: opt < %s -VPlanDriver -S -enable-vp-value-codegen=0 -vplan-force-vf=2 | FileCheck %s

; Test that invalidated store with unit-strideness of the pointer propagated
; from DA to Legal doesn't cause crashes due to getVectorValue requesting
; getScalarValue for lanes other than zero. Should be removed once we fully
; switch from LLVM-IR-based codegen to VPValue-based.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @foo(i32 %n, i64 *%p) local_unnamed_addr {
entry:
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %outer.preheader

outer.preheader:
  br label %outer.header

outer.header:
  %outer.iv = phi i32 [ 0, %outer.preheader ], [ %outer.iv.next, %outer.exit ]
  %tmp = sext i32 %outer.iv to i64
  %ld = load i64, i64* %p, align 8
  %x = add i64 %ld, %tmp
  br label %inner.header

inner.header:
  %inner.iv = phi i32 [ 0, %outer.header ], [ %inner.iv.next, %inner.latch ]
  %tmp9 = icmp slt i32 %inner.iv, %n
  br i1 %tmp9, label %inner.latch, label %inner.exit

inner.latch:
  %inner.iv.next = add i32 %inner.iv, 1
  %inner.exitcond = icmp eq i32 %inner.iv.next, %outer.iv
  br i1 %inner.exitcond, label %inner.exit, label %inner.header

inner.exit:
  %lcssa.phi = phi i32 [ undef, %inner.header ], [ 100, %inner.latch ]
  %gep = getelementptr inbounds i32, i32 addrspace(1)* undef, i64 %x
  store i32 %lcssa.phi, i32 addrspace(1)* %gep, align 4
; CHECK: call void @llvm.masked.scatter.{{.*}}(<2 x i32>
  br label %outer.exit

outer.exit:
  %outer.iv.next = add nuw i32 %outer.iv, 1
  br i1 false, label %outer.header, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
