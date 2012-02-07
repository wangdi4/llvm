; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'amd_convolution.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"

@sgv = internal constant [6 x i8] c"22200\00"		; <[6 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]

; CHECK: simpleConvolution
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.end119.loopexit, %for.end119.loopexit13
; CHECK: ret

define void @simpleConvolution(float addrspace(1)* %output, i32 addrspace(1)* %input, float addrspace(1)* %mask, <2 x i32> %inputDimensions, <2 x i32> %maskDimensions, ...) nounwind {
entry:
  %call = call i32 @get_global_id(i32 0) nounwind
  %tmp1 = extractelement <2 x i32> %inputDimensions, i32 0
  %tmp4 = extractelement <2 x i32> %inputDimensions, i32 1
  %cmp = icmp ne i32 %tmp1, 0
  %nonzero = select i1 %cmp, i32 %tmp1, i32 1
  %rem = urem i32 %call, %nonzero
  %div = udiv i32 %call, %nonzero
  %tmp15 = extractelement <2 x i32> %maskDimensions, i32 0
  %tmp18 = extractelement <2 x i32> %maskDimensions, i32 1
  %sub = add i32 %tmp15, -1
  %div21 = lshr i32 %sub, 1
  %sub24 = add i32 %tmp18, -1
  %div25 = lshr i32 %sub24, 1
  %sub32 = sub i32 %rem, %div21
  %cmp29 = icmp ult i32 %rem, %div21
  %.sub32 = select i1 %cmp29, i32 0, i32 %sub32
  %add = add i32 %rem, %div21
  %cmp37 = icmp ult i32 %add, %tmp1
  %sub40 = add i32 %tmp1, -1
  %cond46 = select i1 %cmp37, i32 %add, i32 %sub40
  %sub55 = sub i32 %div, %div25
  %cmp50 = icmp ult i32 %div, %div25
  %.sub55 = select i1 %cmp50, i32 0, i32 %sub55
  %add61 = add i32 %div, %div25
  %cmp63 = icmp ult i32 %add61, %tmp4
  %sub66 = add i32 %tmp4, -1
  %cond72 = select i1 %cmp63, i32 %add61, i32 %sub66
  %cmp789 = icmp ugt i32 %.sub32, %cond46
  br i1 %cmp789, label %for.end119, label %for.cond81.preheader.lr.ph

for.cond81.preheader.lr.ph:                       ; preds = %entry
  %cmp846 = icmp ugt i32 %.sub55, %cond72
  %sub901 = sub i32 %div25, %div
  %sub962 = sub i32 %div21, %rem
  br i1 %cmp846, label %for.inc116.us.preheader, label %for.body85.lr.ph.preheader

for.inc116.us.preheader:                          ; preds = %for.cond81.preheader.lr.ph
  br label %for.inc116.us

for.body85.lr.ph.preheader:                       ; preds = %for.cond81.preheader.lr.ph
  br label %for.body85.lr.ph

for.inc116.us:                                    ; preds = %for.inc116.us.preheader, %for.inc116.us
  %storemerge11.us = phi i32 [ %inc118.us, %for.inc116.us ], [ %.sub32, %for.inc116.us.preheader ]
  %inc118.us = add i32 %storemerge11.us, 1
  %cmp78.us = icmp ugt i32 %inc118.us, %cond46
  br i1 %cmp78.us, label %for.end119.loopexit13, label %for.inc116.us

for.body85.lr.ph:                                 ; preds = %for.body85.lr.ph.preheader, %for.inc116
  %storemerge11 = phi i32 [ %inc118, %for.inc116 ], [ %.sub32, %for.body85.lr.ph.preheader ]
  %add121510 = phi float [ %add114, %for.inc116 ], [ 0.000000e+00, %for.body85.lr.ph.preheader ]
  %sub97 = add i32 %sub962, %storemerge11
  br label %for.body85

for.body85:                                       ; preds = %for.body85.lr.ph, %for.body85
  %storemerge38 = phi i32 [ %.sub55, %for.body85.lr.ph ], [ %inc, %for.body85 ]
  %add12147 = phi float [ %add121510, %for.body85.lr.ph ], [ %add114, %for.body85 ]
  %sub91 = add i32 %sub901, %storemerge38
  %mul = mul i32 %sub91, %tmp15
  %add98 = add i32 %sub97, %mul
  %mul102 = mul i32 %storemerge38, %tmp1
  %add104 = add i32 %mul102, %storemerge11
  %arrayidx = getelementptr i32 addrspace(1)* %input, i32 %add104
  %tmp108 = load i32 addrspace(1)* %arrayidx, align 4
  %conv = uitofp i32 %tmp108 to float
  %arrayidx111 = getelementptr float addrspace(1)* %mask, i32 %add98
  %tmp112 = load float addrspace(1)* %arrayidx111, align 4
  %mul113 = fmul float %conv, %tmp112
  %add114 = fadd float %add12147, %mul113
  %inc = add i32 %storemerge38, 1
  %cmp84 = icmp ugt i32 %inc, %cond72
  br i1 %cmp84, label %for.inc116, label %for.body85

for.inc116:                                       ; preds = %for.body85
  %inc118 = add i32 %storemerge11, 1
  %cmp78 = icmp ugt i32 %inc118, %cond46
  br i1 %cmp78, label %for.end119.loopexit, label %for.body85.lr.ph

for.end119.loopexit:                              ; preds = %for.inc116
  br label %for.end119

for.end119.loopexit13:                            ; preds = %for.inc116.us
  br label %for.end119

for.end119:                                       ; preds = %for.end119.loopexit13, %for.end119.loopexit, %entry
  %add1215.lcssa = phi float [ 0.000000e+00, %entry ], [ %add114, %for.end119.loopexit ], [ 0.000000e+00, %for.end119.loopexit13 ]
  %add121 = fadd float %add1215.lcssa, 5.000000e-01
  %arrayidx124 = getelementptr float addrspace(1)* %output, i32 %call
  store float %add121, float addrspace(1)* %arrayidx124, align 4
  ret void
}

declare i32 @get_global_id(i32)
