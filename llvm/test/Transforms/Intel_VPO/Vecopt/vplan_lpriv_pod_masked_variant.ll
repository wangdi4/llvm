; RUN: opt -S -vplan-vec -vplan-force-vf=2 -vplan-print-after-create-masked-vplan -vplan-enable-masked-variant  -vplan-print-after-vpentity-instrs < %s -disable-output | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: VPlan after insertion of VPEntities instructions
; CHECK-NOT:  VPlan after emitting masked variant:

define i32 @main() {
entry:
  br label %preheader

preheader:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header
; +--->header
; |      |
; |      v
; |    latch
; |      |
; +------+
;        v
;     loopexit
;        |
;        v
;      exit
header:
  %iv = phi i32 [ 0, %preheader ], [ %iv.next, %latch ]
  %iv.next = add nsw i32 %iv, 1
  br label %latch

latch:
  %x = add nsw i32 %iv, 1
  %bottom_test = icmp eq i32 %iv.next, 128
  br i1 %bottom_test, label %loopexit, label %header

loopexit:
  %x.lcssa = phi i32 [%x, %latch]
  br label %endloop

endloop:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:
  ret i32 %x.lcssa
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
