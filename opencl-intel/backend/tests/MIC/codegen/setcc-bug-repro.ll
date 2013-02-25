; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @traverse_loop1D_1() {
entry:
  %0 = load i32* undef, align 4
  br i1 undef, label %scalarIf.i, label %dim_1_vector_pre_head.i

dim_1_vector_pre_head.i:                          ; preds = %entry
  %sub170vector_func.i = add nsw i32 %0, -1
  %cmp17237vector_func.i = icmp slt i32 %sub170vector_func.i, 0
  %temp773vector_func.i = insertelement <16 x i1> undef, i1 %cmp17237vector_func.i, i32 0
  %vector772vector_func.i = shufflevector <16 x i1> %temp773vector_func.i, <16 x i1> undef, <16 x i32> zeroinitializer
  br label %entryvector_func.i

entryvector_func.i:                               ; preds = %__hmppcg_label_1vector_func.i, %dim_1_vector_pre_head.i
  %sub170vector_func.i1 = add nsw i32 %0, 1
  %cmp17237vector_func.i1 = icmp slt i32 %sub170vector_func.i1, 0
  %temp773vector_func.i1 = insertelement <16 x i1> undef, i1 %cmp17237vector_func.i1, i32 0
  %vector772vector_func.i1 = shufflevector <16 x i1> %temp773vector_func.i1, <16 x i1> undef, <16 x i32> zeroinitializer
  br i1 undef, label %for.bodyvector_func.preheader.i, label %__hmppcg_label_1vector_func.i

for.bodyvector_func.preheader.i:                  ; preds = %entryvector_func.i
  br i1 undef, label %if.elsevector_func.i, label %if.end169vector_func.i

if.elsevector_func.i:                             ; preds = %for.bodyvector_func.preheader.i
  unreachable

if.end169vector_func.i:                           ; preds = %for.bodyvector_func.preheader.i
  %if.end169_to_for.end774vector_func.i = and <16 x i1> undef, %vector772vector_func.i
  %if.end169_to_for.end774vector_func.i1 = and <16 x i1> undef, %vector772vector_func.i1
  unreachable

__hmppcg_label_1vector_func.i:                    ; preds = %entryvector_func.i
  br label %entryvector_func.i

scalarIf.i:                                       ; preds = %entry
  ret void
}
