; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'PhiCanonSwitchCase1'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; This module was already processed by -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify passes

; CHECK: @PhiCanonSwitchCase1
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: BB1:                                              ; preds = %entry
; CHECK: BB2:                                              ; preds = %BB1
; CHECK: phi-split-bb:                                     ; preds = %BB2, %BB2
; CHECK: phi-split-bb1:                                    ; preds = %BB1, %phi-split-bb
; CHECK:   %new_phi = phi float [ %arg2, %phi-split-bb ], [ %arg1, %BB1 ]
; CHECK: BB3:                                              ; preds = %phi-split-bb1, %BB1
; CHECK:   %res = phi float [ %arg1, %BB1 ], [ %new_phi, %phi-split-bb1 ]
; CHECK: phi-split-bb[[tag1:[0-9]*]]:                                    ; preds = %BB2, %BB3
; CHECK: END:                                              ; preds = %phi-split-bb[[tag1]], %BB1
; CHECK: ret

define void @PhiCanonSwitchCase1(float %arg1, float %arg2, float addrspace(1)* nocapture %a) nounwind {
entry:
  %gid = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %lid = tail call i32 @_Z12get_local_idj(i32 0) nounwind readnone
  br label %BB1

BB1:                                       ; preds = %entry
  switch i32 %gid, label %END [
    i32 2, label %BB2
    i32 4, label %BB3
    i32 0, label %BB3
  ]

BB2:                                       ; preds = %BB1
  switch i32 %lid, label %END [
    i32 4, label %BB3
    i32 0, label %BB3
  ]

BB3:                                       ; preds = %BB1, %BB1, %BB2, %BB2
  %res = phi float [ %arg1, %BB1 ], [ %arg1, %BB1 ], [ %arg2, %BB2 ], [ %arg2, %BB2 ]
  store float %res, float addrspace(1)* %a, align 4
  br label %END

END:
  ret void
}

declare i32 @_Z13get_global_idj(i32) readnone
declare i32 @_Z12get_local_idj(i32) readnone
