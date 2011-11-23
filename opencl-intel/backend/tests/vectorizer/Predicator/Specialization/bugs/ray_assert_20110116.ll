; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; This is bug CSSD100005417

; CHECK-NOT: footer
; CHECK: ret

; ModuleID = 'ray.bc'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @kern(float %Shading, float %T, i32* nocapture %raystart, i32* nocapture %raylen, i128* nocapture %raytemplate, i32* nocapture %visible, float* nocapture %opacity, float* nocapture %color) nounwind {
; <label>:0
  %1 = tail call i32 @get_local_id(i32 0) nounwind
  %2 = getelementptr inbounds i32* %raystart, i32 %1
  %3 = load i32* %2
  %4 = getelementptr inbounds i32* %raylen, i32 %1
  %5 = load i32* %4
  br label %.outer

.outer:                                           ; preds = %18, %0
  %Color.0.ph = phi float [ 0.000000e+000, %0 ], [ %21, %18 ]
  %Opacity.0.ph = phi float [ 0.000000e+000, %0 ], [ %23, %18 ]
  %raypos.0.ph = phi i32 [ %3, %0 ], [ %tmp6, %18 ]
  %tmp = add i32 %raypos.0.ph, 1
  br label %6

; <label>:6                                       ; preds = %.backedge, %.outer
  %indvar = phi i32 [ %indvar.next, %.backedge ], [ 0, %.outer ]
  %tmp6 = add i32 %tmp, %indvar
  %raypos.0 = add i32 %raypos.0.ph, %indvar
  %7 = icmp slt i32 %raypos.0, %5
  br i1 %7, label %8, label %.loopexit

; <label>:8                                       ; preds = %6
  %scevgep = getelementptr i128* %raytemplate, i32 %raypos.0
  %9 = load i128* %scevgep
  %10 = trunc i128 %9 to i32
  %11 = getelementptr inbounds i32* %visible, i32 %10
  %12 = load i32* %11
  %13 = icmp eq i32 %12, 0
  br i1 %13, label %.backedge, label %14

.backedge:                                        ; preds = %14, %8
  %indvar.next = add i32 %indvar, 1
  br label %6

; <label>:14                                      ; preds = %8
  %15 = getelementptr inbounds float* %opacity, i32 %10
  %16 = load float* %15
  %17 = fcmp oeq float %16, 0.000000e+000
  br i1 %17, label %.backedge, label %18

; <label>:18                                      ; preds = %14
  %19 = fsub float 1.000000e+000, %Opacity.0.ph
  %20 = fmul float %19, %Shading
  %21 = fadd float %Color.0.ph, %20
  %22 = fmul float %16, %19
  %23 = fadd float %Opacity.0.ph, %22
  %24 = fcmp ogt float %23, %T
  br i1 %24, label %.loopexit, label %.outer

.loopexit:                                        ; preds = %18, %6
  %Color.1 = phi float [ %Color.0.ph, %6 ], [ %21, %18 ]
  %25 = getelementptr inbounds float* %color, i32 %1
  store float %Color.1, float* %25
  ret void
}

declare i32 @get_local_id(i32)
