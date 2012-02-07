; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlPrefixSum.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"

@sgv = internal constant [5 x i8] c"1220\00"		; <[5 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@sgv1 = internal constant [4 x i8] c"210\00"		; <[4 x i8]*> [#uses=1]
@fgv2 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv3 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]

;CHECK: @prefixSumStep1 
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.cond23.preheader, %for.end57.loopexit
; CHECK: ret

define void @prefixSumStep1(i32 addrspace(1)* %puiInputArray, i32 addrspace(1)* %puiOutputArray, i32 addrspace(1)* %puiTmpArray, i32 %szElementsPerItem, ...) nounwind {
entry:
  %call = call i32 @get_global_id(i32 0) nounwind
  %mul = mul i32 %call, %szElementsPerItem
  %cmp16 = icmp eq i32 %szElementsPerItem, 0
  br i1 %cmp16, label %for.end57, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond23.preheader:                             ; preds = %for.body
  %storemerge11 = lshr i32 %szElementsPerItem, 1
  %cmp2512 = icmp eq i32 %storemerge11, 0
  br i1 %cmp2512, label %for.end57, label %for.body26.preheader

for.body26.preheader:                             ; preds = %for.cond23.preheader
  br label %for.body26

for.body:                                         ; preds = %for.body.preheader, %for.body
  %storemerge217 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %add.ptr8.sum7 = add i32 %storemerge217, %mul
  %arrayidx = getelementptr i32 addrspace(1)* %puiOutputArray, i32 %add.ptr8.sum7
  %arrayidx16 = getelementptr i32 addrspace(1)* %puiInputArray, i32 %add.ptr8.sum7
  %tmp17 = load i32 addrspace(1)* %arrayidx16, align 4
  store i32 %tmp17, i32 addrspace(1)* %arrayidx, align 4
  %inc = add i32 %storemerge217, 1
  %cmp = icmp ult i32 %inc, %szElementsPerItem
  br i1 %cmp, label %for.body, label %for.cond23.preheader

for.body26:                                       ; preds = %for.body26.preheader, %for.end51
  %storemerge14 = phi i32 [ %storemerge, %for.end51 ], [ %storemerge11, %for.body26.preheader ]
  %shl53313 = phi i32 [ %shl, %for.end51 ], [ 1, %for.body26.preheader ]
  %cmp338 = icmp eq i32 %storemerge14, 0
  br i1 %cmp338, label %for.end57.loopexit, label %for.body34.lr.ph

for.body34.lr.ph:                                 ; preds = %for.body26
  %sub = add i32 %shl53313, -1
  %add = add i32 %shl53313, %mul
  %shl = shl i32 %shl53313, 1
  br label %for.body34

for.body34:                                       ; preds = %for.body34.lr.ph, %for.body34
  %storemerge110 = phi i32 [ 0, %for.body34.lr.ph ], [ %inc50, %for.body34 ]
  %add4749 = phi i32 [ %sub, %for.body34.lr.ph ], [ %add47, %for.body34 ]
  %add.ptr8.sum = add i32 %add, %add4749
  %arrayidx38 = getelementptr i32 addrspace(1)* %puiOutputArray, i32 %add.ptr8.sum
  %tmp39 = load i32 addrspace(1)* %arrayidx38, align 4
  %add.ptr8.sum5 = add i32 %add4749, %mul
  %arrayidx42 = getelementptr i32 addrspace(1)* %puiOutputArray, i32 %add.ptr8.sum5
  %tmp43 = load i32 addrspace(1)* %arrayidx42, align 4
  %add44 = add i32 %tmp43, %tmp39
  store i32 %add44, i32 addrspace(1)* %arrayidx38, align 4
  %add47 = add i32 %add4749, %shl
  %inc50 = add i32 %storemerge110, 1
  %cmp33 = icmp ult i32 %inc50, %storemerge14
  br i1 %cmp33, label %for.body34, label %for.end51

for.end51:                                        ; preds = %for.body34
  %storemerge = lshr i32 %storemerge14, 1
  %cmp25 = icmp eq i32 %storemerge, 0
  br i1 %cmp25, label %for.end57.loopexit, label %for.body26

for.end57.loopexit:                               ; preds = %for.end51, %for.body26
  br label %for.end57

for.end57:                                        ; preds = %for.end57.loopexit, %entry, %for.cond23.preheader
  %arrayidx60 = getelementptr i32 addrspace(1)* %puiTmpArray, i32 %call
  %sub62 = add i32 %szElementsPerItem, -1
  %add.ptr8.sum6 = add i32 %sub62, %mul
  %arrayidx64 = getelementptr i32 addrspace(1)* %puiOutputArray, i32 %add.ptr8.sum6
  %tmp65 = load i32 addrspace(1)* %arrayidx64, align 4
  store i32 %tmp65, i32 addrspace(1)* %arrayidx60, align 4
  ret void
}

declare i32 @get_global_id(i32)

define void @prefixSumStep2(i32 addrspace(1)* %puiOutputArray, i32 addrspace(1)* %puiValueToAddArray, i32 %szElementsPerItem, ...) nounwind {
entry:
  %call = call i32 @get_global_id(i32 0) nounwind
  %mul = mul i32 %call, %szElementsPerItem
  %add.ptr8 = getelementptr i32 addrspace(1)* %puiValueToAddArray, i32 %call
  %shr = lshr i32 %szElementsPerItem, 1
  %sub = add i32 %szElementsPerItem, -1
  %add.ptr.sum = add i32 %sub, %mul
  %arrayidx = getelementptr i32 addrspace(1)* %puiOutputArray, i32 %add.ptr.sum
  store i32 0, i32 addrspace(1)* %arrayidx, align 4
  %cmp13 = icmp eq i32 %shr, 0
  br i1 %cmp13, label %for.cond57.preheader, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond57.preheader.loopexit:                    ; preds = %for.inc52
  br label %for.cond57.preheader

for.cond57.preheader:                             ; preds = %for.cond57.preheader.loopexit, %entry
  %cmp608 = icmp eq i32 %szElementsPerItem, 0
  br i1 %cmp608, label %for.end73, label %for.body61.preheader

for.body61.preheader:                             ; preds = %for.cond57.preheader
  br label %for.body61

for.body:                                         ; preds = %for.body.preheader, %for.inc52
  %storemerge115 = phi i32 [ %shl54, %for.inc52 ], [ 1, %for.body.preheader ]
  %tmp49314 = phi i32 [ %shr18, %for.inc52 ], [ %szElementsPerItem, %for.body.preheader ]
  %shr18 = lshr i32 %tmp49314, 1
  %cmp2610 = icmp eq i32 %storemerge115, 0
  br i1 %cmp2610, label %for.inc52, label %for.body27.lr.ph

for.body27.lr.ph:                                 ; preds = %for.body
  %sub21 = add i32 %shr18, -1
  %add = add i32 %shr18, %mul
  %shl = and i32 %tmp49314, -2
  br label %for.body27

for.body27:                                       ; preds = %for.body27.lr.ph, %for.body27
  %storemerge212 = phi i32 [ 0, %for.body27.lr.ph ], [ %inc, %for.body27 ]
  %add50411 = phi i32 [ %sub21, %for.body27.lr.ph ], [ %add50, %for.body27 ]
  %add.ptr.sum5 = add i32 %add, %add50411
  %arrayidx31 = getelementptr i32 addrspace(1)* %puiOutputArray, i32 %add.ptr.sum5
  %tmp32 = load i32 addrspace(1)* %arrayidx31, align 4
  %add.ptr.sum6 = add i32 %add50411, %mul
  %arrayidx41 = getelementptr i32 addrspace(1)* %puiOutputArray, i32 %add.ptr.sum6
  %tmp42 = load i32 addrspace(1)* %arrayidx41, align 4
  %add43 = add i32 %tmp42, %tmp32
  store i32 %add43, i32 addrspace(1)* %arrayidx31, align 4
  store i32 %tmp32, i32 addrspace(1)* %arrayidx41, align 4
  %add50 = add i32 %add50411, %shl
  %inc = add i32 %storemerge212, 1
  %cmp26 = icmp ult i32 %inc, %storemerge115
  br i1 %cmp26, label %for.body27, label %for.inc52.loopexit

for.inc52.loopexit:                               ; preds = %for.body27
  br label %for.inc52

for.inc52:                                        ; preds = %for.inc52.loopexit, %for.body
  %shl54 = shl i32 %storemerge115, 1
  %cmp = icmp ugt i32 %shl54, %shr
  br i1 %cmp, label %for.cond57.preheader.loopexit, label %for.body

for.body61:                                       ; preds = %for.body61.preheader, %for.body61
  %storemerge9 = phi i32 [ %inc72, %for.body61 ], [ 0, %for.body61.preheader ]
  %add.ptr.sum7 = add i32 %storemerge9, %mul
  %arrayidx64 = getelementptr i32 addrspace(1)* %puiOutputArray, i32 %add.ptr.sum7
  %tmp65 = load i32 addrspace(1)* %arrayidx64, align 4
  %tmp68 = load i32 addrspace(1)* %add.ptr8, align 4
  %add69 = add i32 %tmp68, %tmp65
  store i32 %add69, i32 addrspace(1)* %arrayidx64, align 4
  %inc72 = add i32 %storemerge9, 1
  %cmp60 = icmp ult i32 %inc72, %szElementsPerItem
  br i1 %cmp60, label %for.body61, label %for.end73.loopexit

for.end73.loopexit:                               ; preds = %for.body61
  br label %for.end73

for.end73:                                        ; preds = %for.end73.loopexit, %for.cond57.preheader
  ret void
}
