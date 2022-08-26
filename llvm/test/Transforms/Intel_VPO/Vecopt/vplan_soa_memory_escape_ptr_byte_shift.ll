;; In this test the pointer shifts by 2 bytes and i32 is loaded,
;; this the memory is accessed not at the array element boundary.
;; This is unsafe for SOA transformation.

; RUN: opt -S -vplan-vec -vplan-enable-masked-variant=0 -vplan-enable-soa -vplan-dump-soa-info -disable-vplan-codegen %s 2>&1 | FileCheck %s

; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-enable-masked-variant=0 -vplan-enable-soa-hir -vplan-dump-soa-info\
; RUN: -disable-output  -disable-vplan-codegen %s 2>&1 | FileCheck %s

; REQUIRES:asserts

; CHECK: SOA profitability:
; CHECK: SOAUnsafe = arr.priv

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test_unsafe_addrspacecast() {
entry:
  %arr.priv = alloca [1024 x i32], align 4
  br label %preheader

preheader:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"([1024 x i32]* %arr.priv, i32 0, i32 1024) ]
  br label %header

header:
  %iv = phi i64 [ 0, %preheader ], [ %iv.next, %header ]
  %down = bitcast [1024 x i32] *%arr.priv to i8*
  %gep = getelementptr inbounds i8, i8 *%down, i64 2
  %up = bitcast i8 *%gep to i32 *

  %ld = load i32, i32* %up

  %iv.next = add nsw nuw i64 %iv, 1
  %exitcond = icmp ult i64 %iv.next, 1024
  br i1 %exitcond, label %exit, label %header

exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
