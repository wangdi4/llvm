;
; Test to check processing of masked variant with unconditional lastprivates.
; Bailout for now due to unsupported in CG.
;
; RUN: opt -vplan-vec  -disable-output -vplan-print-after-vpentity-instrs -vplan-print-after-create-masked-vplan -vplan-enable-masked-variant < %s 2>&1 | FileCheck %s
; RUN: opt -passes="vplan-vec" -disable-output -vplan-print-after-vpentity-instrs -vplan-print-after-create-masked-vplan -vplan-enable-masked-variant < %s 2>&1 | FileCheck %s

; TODO: Enable test for HIR when vectors are supported by loopopt
; R_UN: opt -disable-output -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -print-after=hir-vplan-vec -vplan-vec-scenario="n1;v16;m16" -vplan-print-after-create-masked-vplan -vplan-enable-masked-variant-hir < %s 2>&1 | FileCheck %s --check-prefix=HIR
; R_UN: opt -disable-output -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-vec-scenario="n1;v16;m16" -vplan-print-after-create-masked-vplan -vplan-enable-masked-variant-hir < %s 2>&1 | FileCheck %s --check-prefix=HIR

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-NOT: VPlan after emitting masked variant

define <2 x i32> @foo() {
entry:
  %x = alloca <2 x i32>, align 4
  br label %preheader

preheader:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE"(<2 x i32>* %x) ]
  br label %header
header:
  %iv = phi i32 [ 0, %preheader ], [ %iv.next, %latch ]
  %iv.next = add nsw i32 %iv, 1
  br label %latch

latch:
 %broadcast.insert = insertelement <2 x i32> undef, i32 %iv, i32 0
 %broadcast.splat = shufflevector <2 x i32> %broadcast.insert, <2 x i32> undef, <2 x i32> zeroinitializer
  %xv = add nsw <2 x i32> %broadcast.splat, <i32 1, i32 1>
  store <2 x i32> %xv, <2 x i32>* %x, align 4
  %bottom_test = icmp eq i32 %iv.next, 128
  br i1 %bottom_test, label %loopexit, label %header

loopexit:
  br label %endloop

endloop:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:
  %r = load <2 x i32>, <2 x i32>* %x, align 4
  ret <2 x i32> %r
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
