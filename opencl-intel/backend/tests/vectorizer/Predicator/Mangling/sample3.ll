; RUN: llvm-as %s -o %t.bc
; RUN: opt -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sample3.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
define void @func(i64 %n, i64* %A, i64* %B) nounwind {
; CHECK-NOT: @masked
; CHECK: ret
entry:
  %n.addr = alloca i64, align 8                   ; <i64*> [#uses=2]
  %A.addr = alloca i64*, align 8                  ; <i64**> [#uses=2]
  %B.addr = alloca i64*, align 8                  ; <i64**> [#uses=2]
  %sum = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=4]
  %A4 = alloca <4 x float>*, align 8              ; <<4 x float>**> [#uses=4]
  %B4 = alloca <4 x float>*, align 8              ; <<4 x float>**> [#uses=3]
  %i = alloca i64, align 8                        ; <i64*> [#uses=8]
  store i64 %n, i64* %n.addr
  store i64* %A, i64** %A.addr
  store i64* %B, i64** %B.addr
  store <4 x float> zeroinitializer, <4 x float>* %sum
  %tmp = load i64** %A.addr                       ; <i64*> [#uses=1]
  %0 = bitcast i64* %tmp to <4 x float>*          ; <<4 x float>*> [#uses=1]
  store <4 x float>* %0, <4 x float>** %A4
  %tmp2 = load i64** %B.addr                      ; <i64*> [#uses=1]
  %1 = bitcast i64* %tmp2 to <4 x float>*         ; <<4 x float>*> [#uses=1]
  store <4 x float>* %1, <4 x float>** %B4
  store i64 0, i64* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp4 = load i64* %i                            ; <i64> [#uses=1]
  %tmp5 = load i64* %n.addr                       ; <i64> [#uses=1]
  %div = sdiv i64 %tmp5, 4                        ; <i64> [#uses=1]
  %cmp = icmp slt i64 %tmp4, %div                 ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp6 = load i64* %i                            ; <i64> [#uses=1]
  %tmp7 = load <4 x float>** %B4                  ; <<4 x float>*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float>* %tmp7, i64 %tmp6 ; <<4 x float>*> [#uses=1]
  %tmp8 = load <4 x float>* %arrayidx             ; <<4 x float>> [#uses=1]
  %tmp9 = extractelement <4 x float> %tmp8, i32 0 ; <float> [#uses=1]
  %tmp10 = load <4 x float>** %A4                 ; <<4 x float>*> [#uses=1]
  %arrayidx11 = getelementptr inbounds <4 x float>* %tmp10, i64 0 ; <<4 x float>*> [#uses=1]
  %tmp12 = load <4 x float>* %arrayidx11          ; <<4 x float>> [#uses=1]
  %tmp13 = extractelement <4 x float> %tmp12, i32 0 ; <float> [#uses=1]
  %cmp14 = fcmp oeq float %tmp9, %tmp13           ; <i1> [#uses=1]
  br i1 %cmp14, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %tmp15 = load i64* %i                           ; <i64> [#uses=1]
  %tmp16 = load <4 x float>** %A4                 ; <<4 x float>*> [#uses=1]
  %arrayidx17 = getelementptr inbounds <4 x float>* %tmp16, i64 %tmp15 ; <<4 x float>*> [#uses=1]
  %tmp18 = load <4 x float>* %arrayidx17          ; <<4 x float>> [#uses=1]
  %tmp19 = load i64* %i                           ; <i64> [#uses=1]
  %tmp20 = load <4 x float>** %B4                 ; <<4 x float>*> [#uses=1]
  %arrayidx21 = getelementptr inbounds <4 x float>* %tmp20, i64 %tmp19 ; <<4 x float>*> [#uses=1]
  %tmp22 = load <4 x float>* %arrayidx21          ; <<4 x float>> [#uses=1]
  %add = fadd <4 x float> %tmp18, %tmp22          ; <<4 x float>> [#uses=1]
  %tmp23 = load <4 x float>* %sum                 ; <<4 x float>> [#uses=1]
  %add24 = fadd <4 x float> %tmp23, %add          ; <<4 x float>> [#uses=1]
  store <4 x float> %add24, <4 x float>* %sum
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %tmp25 = load <4 x float>* %sum                 ; <<4 x float>> [#uses=1]
  %tmp26 = load i64* %i                           ; <i64> [#uses=1]
  %tmp27 = load <4 x float>** %A4                 ; <<4 x float>*> [#uses=1]
  %arrayidx28 = getelementptr inbounds <4 x float>* %tmp27, i64 %tmp26 ; <<4 x float>*> [#uses=1]
  store <4 x float> %tmp25, <4 x float>* %arrayidx28
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %tmp29 = load i64* %i                           ; <i64> [#uses=1]
  %inc = add nsw i64 %tmp29, 1                    ; <i64> [#uses=1]
  store i64 %inc, i64* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}
