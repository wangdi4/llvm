;; Check that memory escape analysis correctly classifies this as Unsafe,
;; as the pointer alias is stored into itself. This way there are two ways to
;; reach this store, one way we classify store as safe, the other way
;; we say it is unsafe.
;; Test if a follow up to one of the points discussed in the code review
;; to demonstate that the memory escape analysis explores all paths until
;; the bad one is taken, and stops only then.

; RUN: opt -S -opaque-pointers -vplan-vec -vplan-enable-masked-variant=0 -vplan-enable-soa -vplan-dump-soa-info -disable-vplan-codegen %s 2>&1 | FileCheck %s

; RUN: opt -opaque-pointers -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-enable-masked-variant=0 -vplan-enable-soa-hir -vplan-dump-soa-info\
; RUN: -disable-output  -disable-vplan-codegen %s 2>&1 | FileCheck %s

; REQUIRES:asserts

; CHECK: SOA profitability:
; CHECK: SOAUnsafe = arr.priv1
; CHECK: SOA profitability:
; CHECK: SOAUnsafe = arr.priv2

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test_unsafe_store_variation_1() {
entry:
  %arr.priv1 = alloca [1024 x i64], align 4
  br label %preheader

preheader:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr %arr.priv1, i64 0, i32 1024) ]
  br label %header

header:
  %iv = phi i64 [ 0, %preheader ], [ %iv.next, %header ]
  %gep = getelementptr inbounds [1024 x i64], ptr %arr.priv1, i64 0
  store ptr %gep, ptr %arr.priv1
  %iv.next = add nsw nuw i64 %iv, 1
  %exitcond = icmp ult i64 %iv.next, 1024
  br i1 %exitcond, label %exit, label %header

exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

define void @test_unsafe_store_variation_2() {
entry:
  %arr.priv2 = alloca [1024 x i64], align 4
  br label %preheader

preheader:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr %arr.priv2, i64 0, i32 1024) ]
  br label %header

header:
  %iv = phi i64 [ 0, %preheader ], [ %iv.next, %header ]
  %gep = getelementptr inbounds [1024 x i64], ptr %arr.priv2, i64 0
  store ptr %arr.priv2, ptr %gep
  %iv.next = add nsw nuw i64 %iv, 1
  %exitcond = icmp ult i64 %iv.next, 1024
  br i1 %exitcond, label %exit, label %header

exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}
