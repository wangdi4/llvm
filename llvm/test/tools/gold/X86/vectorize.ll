; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; RUN: llvm-as %s -o %t.o
; INTEL - added loopopt in pipeline.
; RUN: %gold -m elf_x86_64 -plugin %llvmshlibdir/LLVMgold%shlibext \
<<<<<<< HEAD
; RUN:    --plugin-opt=save-temps \
; RUN:    -shared %t.o -o %t2.o
=======
; RUN:    --plugin-opt=save-temps -shared %t.o -o %t2.o
>>>>>>> 6ec8c6fc7b30bb770cf3e454a89ebd2be1bed88e
; RUN: llvm-dis %t2.o.0.4.opt.bc -o - | FileCheck %s

; test that the vectorizer is run.
; CHECK: fadd <4 x float>

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @f(float* nocapture %x, i64 %n) "loopopt-pipeline"="full" {
bb:
  br label %bb1

bb1:
  %i.0 = phi i64 [ 0, %bb ], [ %tmp4, %bb1 ]
  %tmp = getelementptr inbounds float, float* %x, i64 %i.0
  %tmp2 = load float, float* %tmp, align 4
  %tmp3 = fadd float %tmp2, 1.000000e+00
  store float %tmp3, float* %tmp, align 4
  %tmp4 = add nsw i64 %i.0, 1
  %tmp5 = icmp slt i64 %tmp4, %n
  br i1 %tmp5, label %bb1, label %bb6

bb6:
  ret void
}
; end INTEL_FEATURE_SW_ADVANCED

