; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlATIPrefixSum.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"

@sgv = internal constant [5 x i8] c"2290\00"		; <[5 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]

; CHECK: @prefixSum
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret

define void @prefixSum(float addrspace(1)* %output, float addrspace(1)* %input, float addrspace(3)* %block, i32 %length, ...) nounwind {
entry:
  %call = call i32 @get_local_id(i32 0) nounwind
  %mul = shl i32 %call, 1
  %arrayidx = getelementptr float addrspace(3)* %block, i32 %mul
  %arrayidx5 = getelementptr float addrspace(1)* %input, i32 %mul
  %tmp6 = load float addrspace(1)* %arrayidx5, align 4
  store float %tmp6, float addrspace(3)* %arrayidx, align 4
  %add1 = or i32 %mul, 1
  %arrayidx10 = getelementptr float addrspace(3)* %block, i32 %add1
  %arrayidx15 = getelementptr float addrspace(1)* %input, i32 %add1
  %tmp16 = load float addrspace(1)* %arrayidx15, align 4
  store float %tmp16, float addrspace(3)* %arrayidx10, align 4
  %shr = lshr i32 %length, 1
  %cmp13 = icmp eq i32 %shr, 0
  br i1 %cmp13, label %for.end, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %add33 = add i32 %mul, 2
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %if.end
  %storemerge615 = phi i32 [ %shr, %for.body.lr.ph ], [ %shr48, %if.end ]
  %tmp80914 = phi i32 [ 1, %for.body.lr.ph ], [ %mul46, %if.end ]
  call void @barrier(i32 1) nounwind
  %cmp22 = icmp slt i32 %call, %storemerge615
  br i1 %cmp22, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %mul28 = mul i32 %tmp80914, %add1
  %sub = add i32 %mul28, -1
  %mul34 = mul i32 %tmp80914, %add33
  %sub35 = add i32 %mul34, -1
  %arrayidx38 = getelementptr float addrspace(3)* %block, i32 %sub35
  %tmp39 = load float addrspace(3)* %arrayidx38, align 4
  %arrayidx42 = getelementptr float addrspace(3)* %block, i32 %sub
  %tmp43 = load float addrspace(3)* %arrayidx42, align 4
  %add44 = fadd float %tmp39, %tmp43
  store float %add44, float addrspace(3)* %arrayidx38, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %mul46 = shl i32 %tmp80914, 1
  %shr48 = ashr i32 %storemerge615, 1
  %cmp = icmp sgt i32 %shr48, 0
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %if.end
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %tmp809.lcssa = phi i32 [ 1, %entry ], [ %mul46, %for.end.loopexit ]
  %cmp50 = icmp eq i32 %call, 0
  br i1 %cmp50, label %if.then51, label %for.cond59.preheader

if.then51:                                        ; preds = %for.end
  %sub53 = add i32 %length, -1
  %arrayidx55 = getelementptr float addrspace(3)* %block, i32 %sub53
  store float 0.000000e+00, float addrspace(3)* %arrayidx55, align 4
  br label %for.cond59.preheader

for.cond59.preheader:                             ; preds = %if.then51, %for.end
  %cmp6210 = icmp ugt i32 %length, 1
  br i1 %cmp6210, label %for.body63.lr.ph, label %for.end108

for.body63.lr.ph:                                 ; preds = %for.cond59.preheader
  %add83 = add i32 %mul, 2
  br label %for.body63

for.body63:                                       ; preds = %for.body63.lr.ph, %for.inc105
  %storemerge12 = phi i32 [ 1, %for.body63.lr.ph ], [ %mul107, %for.inc105 ]
  %tmp80811 = phi i32 [ %tmp809.lcssa, %for.body63.lr.ph ], [ %shr65, %for.inc105 ]
  %shr65 = ashr i32 %tmp80811, 1
  call void @barrier(i32 1) nounwind
  %cmp68 = icmp slt i32 %call, %storemerge12
  br i1 %cmp68, label %if.then69, label %for.inc105

if.then69:                                        ; preds = %for.body63
  %mul76 = mul i32 %shr65, %add1
  %sub77 = add i32 %mul76, -1
  %mul84 = mul i32 %shr65, %add83
  %sub85 = add i32 %mul84, -1
  %arrayidx89 = getelementptr float addrspace(3)* %block, i32 %sub77
  %tmp90 = load float addrspace(3)* %arrayidx89, align 4
  %arrayidx96 = getelementptr float addrspace(3)* %block, i32 %sub85
  %tmp97 = load float addrspace(3)* %arrayidx96, align 4
  store float %tmp97, float addrspace(3)* %arrayidx89, align 4
  %tmp101 = load float addrspace(3)* %arrayidx96, align 4
  %add103 = fadd float %tmp101, %tmp90
  store float %add103, float addrspace(3)* %arrayidx96, align 4
  br label %for.inc105

for.inc105:                                       ; preds = %for.body63, %if.then69
  %mul107 = shl i32 %storemerge12, 1
  %cmp62 = icmp ult i32 %mul107, %length
  br i1 %cmp62, label %for.body63, label %for.end108.loopexit

for.end108.loopexit:                              ; preds = %for.inc105
  br label %for.end108

for.end108:                                       ; preds = %for.end108.loopexit, %for.cond59.preheader
  call void @barrier(i32 1) nounwind
  %arrayidx112 = getelementptr float addrspace(1)* %output, i32 %mul
  %tmp117 = load float addrspace(3)* %arrayidx, align 4
  store float %tmp117, float addrspace(1)* %arrayidx112, align 4
  %arrayidx122 = getelementptr float addrspace(1)* %output, i32 %add1
  %tmp128 = load float addrspace(3)* %arrayidx10, align 4
  store float %tmp128, float addrspace(1)* %arrayidx122, align 4
  ret void
}

declare i32 @get_local_id(i32)

declare void @barrier(i32)
