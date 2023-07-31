;; Check that we classify pod alloca chains with non-pod operands to mem ops
;; geps, vpsubscript as unsafe.

; RUN: opt -S -passes=vplan-vec -vplan-enable-masked-variant=0 -vplan-enable-soa -vplan-dump-soa-info -disable-vplan-codegen %s 2>&1 | FileCheck %s

; RUN: opt -passes=hir-ssa-deconstruction,hir-vplan-vec -vplan-enable-masked-variant=0 -vplan-enable-soa-hir -vplan-dump-soa-info -vplan-enable-hir-private-arrays -disable-output  -disable-vplan-codegen %s 2>&1 | FileCheck %s

; REQUIRES:asserts

; CHECK: SOA profitability:
; CHECK: SOAUnsafe = [[VP_ARR_PRIV1:%.*]] (arr.priv1)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ClassA = type { i32, i32 }

define void @test_unsafe_non_pod_store(%struct.ClassA %nonpod) {
entry:
  %arr.priv1 = alloca [1024 x i64], align 4
  br label %preheader

preheader:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr %arr.priv1, i64 0, i32 1024) ]
  br label %header

header:
  %iv = phi i64 [ 0, %preheader ], [ %iv.next, %header ]
  store %struct.ClassA %nonpod, ptr %arr.priv1
  %iv.next = add nsw nuw i64 %iv, 1
  %exitcond = icmp ult i64 %iv.next, 1024
  br i1 %exitcond, label %header, label %exit

exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
