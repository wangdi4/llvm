;
; Test to check processing of masked variant with unconditional lastprivates.
; Bailout for now due to unsupported in CG.
;
; RUN: opt -vplan-vec -disable-output -vplan-print-after-vpentity-instrs -vplan-print-after-create-masked-vplan -vplan-enable-masked-variant < %s 2>&1 | FileCheck %s
; RUN: opt -passes="vplan-vec" -disable-output -vplan-print-after-vpentity-instrs -vplan-print-after-create-masked-vplan -vplan-enable-masked-variant < %s 2>&1 | FileCheck %s

; TODO: Enable test for HIR when vectors are supported by loopopt
; R_UN: opt -disable-output -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -print-after=hir-vplan-vec -vplan-vec-scenario="n1;v16;m16" -vplan-print-after-create-masked-vplan -vplan-enable-new-cfg-merge-hir -vplan-enable-masked-variant-hir < %s 2>&1 | FileCheck %s --check-prefix=HIR
; R_UN: opt -disable-output -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-vec-scenario="n1;v16;m16" -vplan-print-after-create-masked-vplan -vplan-enable-new-cfg-merge-hir -vplan-enable-masked-variant-hir < %s 2>&1 | FileCheck %s --check-prefix=HIR

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-NOT: VPlan after emitting masked variant

define <2 x i32> @foo() {
entry:
  %xp = alloca <2 x i32>, align 4
  br label %preheader

preheader:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:TYPED"(<2 x i32>* %xp, <2 x i32> zeroinitializer, i32 1)]
  br label %header
header:
  %iv = phi i32 [ 0, %preheader ], [ %iv.next, %latch ]
  %iv.next = add nsw i32 %iv, 1
  br label %latch

latch:
 %broadcast.insert = insertelement <2 x i32> undef, i32 %iv, i32 0
 %broadcast.splat = shufflevector <2 x i32> %broadcast.insert, <2 x i32> undef, <2 x i32> zeroinitializer
  %x = add nsw <2 x i32> %broadcast.splat, <i32 1, i32 1>
  %bottom_test = icmp eq i32 %iv.next, 128
  br i1 %bottom_test, label %loopexit, label %header

loopexit:
  %x.lcssa = phi <2 x i32> [%x, %latch]
  store <2 x i32> %x.lcssa, <2 x i32>* %xp
  br label %endloop

endloop:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:
  ret <2 x i32> %x.lcssa
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
