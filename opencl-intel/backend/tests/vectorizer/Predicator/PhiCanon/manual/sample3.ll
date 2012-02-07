; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sample3.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret

define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %0 = bitcast i64* %A to <4 x float>*
  %1 = bitcast i64* %B to <4 x float>*
  %div = sdiv i64 %n, 4
  %cmp3 = icmp sgt i64 %n, 3
  br i1 %cmp3, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %if.end
  %storemerge5 = phi i64 [ %inc, %if.end ], [ 0, %for.body.preheader ]
  %tmp2514 = phi <4 x float> [ %tmp252, %if.end ], [ zeroinitializer, %for.body.preheader ]
  %arrayidx = getelementptr inbounds <4 x float>* %1, i64 %storemerge5
  %tmp8 = load <4 x float>* %arrayidx, align 16
  %tmp9 = extractelement <4 x float> %tmp8, i32 0
  %tmp12 = load <4 x float>* %0, align 16
  %tmp13 = extractelement <4 x float> %tmp12, i32 0
  %cmp14 = fcmp oeq float %tmp9, %tmp13
  %arrayidx17 = getelementptr inbounds <4 x float>* %0, i64 %storemerge5
  br i1 %cmp14, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %tmp18 = load <4 x float>* %arrayidx17, align 16
  %add = fadd <4 x float> %tmp18, %tmp8
  %add24 = fadd <4 x float> %tmp2514, %add
  br label %if.end

if.end:                                           ; preds = %for.body, %if.then
  %tmp252 = phi <4 x float> [ %add24, %if.then ], [ %tmp2514, %for.body ]
  store <4 x float> %tmp252, <4 x float>* %arrayidx17, align 16
  %inc = add nsw i64 %storemerge5, 1
  %cmp = icmp slt i64 %inc, %div
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %if.end
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
