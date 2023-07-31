;; Test bail out for the private aliased through select.

; RUN: opt -S -passes=vplan-vec -vplan-enable-masked-variant=0 -vplan-enable-soa -vplan-dump-soa-info -disable-vplan-codegen %s 2>&1 | FileCheck %s

;; TODO: Private aliases are not imported for HIR path, so skip it.
; RUNx: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-enable-masked-variant=0 -vplan-enable-soa-hir -vplan-dump-soa-info\
; RUNx: -disable-output  -disable-vplan-codegen %s 2>&1 | FileCheck %s

; REQUIRES:asserts

; CHECK: SOA profitability:
; CHECK: SOAUnsafe = [[VP_ARR_PRIV:%.*]] (arr.priv)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test_unsafe_addrspacecast(i1 zeroext %pred, ptr %ptr) {
entry:
  %arr.priv = alloca [1024 x i32], align 4
  br label %begin.region

begin.region:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr %arr.priv, i32 0, i32 1024), "QUAL.OMP.UNIFORM"(i1 %pred), "QUAL.OMP.UNIFORM:TYPED"(ptr %ptr, i32 0, i32 1) ]
  br label %preheader

preheader:
  %select = select i1 %pred, ptr %arr.priv, ptr %ptr
  br label %header

header:
  %iv = phi i64 [ 0, %preheader ], [ %iv.next, %header ]
  %gep = getelementptr inbounds i32, ptr %select, i64 %iv
  %ld = load i32, ptr %gep
  %iv.next = add nsw nuw i64 %iv, 1
  %exitcond = icmp ult i64 %iv.next, 1024
  br i1 %exitcond, label %exit, label %header

exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
