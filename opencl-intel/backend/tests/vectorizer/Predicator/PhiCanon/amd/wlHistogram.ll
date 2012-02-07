; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlHistogram.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"

@sgv = internal constant [4 x i8] c"120\00"		; <[4 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@sgv1 = internal constant [7 x i8] c"122000\00"		; <[7 x i8]*> [#uses=1]
@fgv2 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv3 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@sgv4 = internal constant [5 x i8] c"1200\00"		; <[5 x i8]*> [#uses=1]
@fgv5 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv6 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@sgv7 = internal constant [6 x i8] c"22000\00"		; <[6 x i8]*> [#uses=1]
@fgv8 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv9 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@sgv10 = internal constant [6 x i8] c"22000\00"		; <[6 x i8]*> [#uses=1]
@fgv11 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv12 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@sgv13 = internal constant [6 x i8] c"22000\00"		; <[6 x i8]*> [#uses=1]
@fgv14 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv15 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@sgv16 = internal constant [6 x i8] c"22000\00"		; <[6 x i8]*> [#uses=1]
@fgv17 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv18 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@sgv19 = internal constant [6 x i8] c"22000\00"		; <[6 x i8]*> [#uses=1]
@fgv20 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv21 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]

; CHECK: @histogramGrouped
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.body51.lr.ph, %for.end83.loopexit
; CHECK: ret
; CHECK: @histogramStep2int
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.body.lr.ph, %for.end31.loopexit
; CHECK: ret
; CHECK: @histogramStep2int2
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.body.lr.ph, %for.end31.loopexit
; CHECK: ret
; CHECK: @histogramStep2int4
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.body.lr.ph, %for.end31.loopexit
; CHECK: ret
; CHECK: @histogramStep2int8
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.body.lr.ph, %for.end31.loopexit
; CHECK: ret
; CHECK: @histogramStep2int16
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.body.lr.ph, %for.end31.loopexit
; CHECK: ret


define void @histogramScalar(i32 addrspace(1)* %puiInputMatrix, i32 addrspace(1)* %puiOutputArray, i32 %szMatrix, ...) nounwind {
entry:
  %call = call i32 @get_global_id(i32 0) nounwind
  %cmp = icmp ne i32 %call, 0
  %cmp21 = icmp eq i32 %szMatrix, 0
  %or.cond = or i1 %cmp, %cmp21
  br i1 %or.cond, label %return, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %storemerge2 = phi i32 [ %inc10, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr i32 addrspace(1)* %puiInputMatrix, i32 %storemerge2
  %tmp5 = load i32 addrspace(1)* %arrayidx, align 4
  %arrayidx7 = getelementptr i32 addrspace(1)* %puiOutputArray, i32 %tmp5
  %tmp8 = load i32 addrspace(1)* %arrayidx7, align 4
  %inc = add i32 %tmp8, 1
  store i32 %inc, i32 addrspace(1)* %arrayidx7, align 4
  %inc10 = add i32 %storemerge2, 1
  %cmp2 = icmp ult i32 %inc10, %szMatrix
  br i1 %cmp2, label %for.body, label %return.loopexit

return.loopexit:                                  ; preds = %for.body
  br label %return

return:                                           ; preds = %return.loopexit, %entry
  ret void
}

declare i32 @get_global_id(i32)

define void @histogramGrouped(i32 addrspace(1)* %puiInputMatrix, i32 addrspace(1)* %puiTmpArray, i32 addrspace(1)* %puiGroupOutputArray, i32 %szBin, i32 %szBinsPerItem, i32 %szElemenetsPerItem, ...) nounwind {
entry:
  %call = call i32 @get_global_id(i32 0) nounwind
  %call1 = call i32 @get_local_id(i32 0) nounwind
  %mul = mul i32 %call, %szBin
  %add = add i32 %call, 1
  %mul7 = mul i32 %szBinsPerItem, %szBin
  %mul9 = mul i32 %mul7, %add
  %mul15 = mul i32 %mul, %szBinsPerItem
  %cmp8 = icmp ult i32 %mul15, %mul9
  br i1 %cmp8, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %storemerge29 = phi i32 [ %inc25, %for.body ], [ %mul15, %for.body.preheader ]
  %arrayidx = getelementptr i32 addrspace(1)* %puiInputMatrix, i32 %storemerge29
  %tmp20 = load i32 addrspace(1)* %arrayidx, align 4
  %add.ptr.sum = add i32 %tmp20, %mul
  %arrayidx22 = getelementptr i32 addrspace(1)* %puiTmpArray, i32 %add.ptr.sum
  %tmp23 = load i32 addrspace(1)* %arrayidx22, align 4
  %inc = add i32 %tmp23, 1
  store i32 %inc, i32 addrspace(1)* %arrayidx22, align 4
  %inc25 = add i32 %storemerge29, 1
  %cmp = icmp ult i32 %inc25, %mul9
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %call26 = call i32 @get_local_size(i32 0) nounwind
  %add29 = add i32 %call1, 1
  %mul31 = mul i32 %add29, %szElemenetsPerItem
  %call33 = call i32 @get_group_id(i32 0) nounwind
  %call34 = call i32 @get_local_size(i32 0) nounwind
  %mul35 = mul i32 %call34, %call33
  %call42 = call i32 @get_group_id(i32 0) nounwind
  %mul44 = mul i32 %call42, %szBin
  %cmp506 = icmp eq i32 %call26, 0
  br i1 %cmp506, label %for.end83, label %for.body51.lr.ph

for.body51.lr.ph:                                 ; preds = %for.end
  %mul61 = mul i32 %call1, %szElemenetsPerItem
  %cmp654 = icmp ult i32 %mul61, %mul31
  br i1 %cmp654, label %for.body66.lr.ph.us.preheader, label %for.end83

for.body66.lr.ph.us.preheader:                    ; preds = %for.body51.lr.ph
  br label %for.body66.lr.ph.us

for.inc80.us:                                     ; preds = %for.body66.us
  %inc82.us = add i32 %storemerge7.us, 1
  %cmp50.us = icmp ult i32 %inc82.us, %call26
  br i1 %cmp50.us, label %for.body66.lr.ph.us, label %for.end83.loopexit

for.body66.us:                                    ; preds = %for.body66.lr.ph.us, %for.body66.us
  %storemerge15.us = phi i32 [ %mul61, %for.body66.lr.ph.us ], [ %inc78.us, %for.body66.us ]
  %add.ptr45.sum.us = add i32 %storemerge15.us, %mul44
  %arrayidx69.us = getelementptr i32 addrspace(1)* %puiGroupOutputArray, i32 %add.ptr45.sum.us
  %tmp70.us = load i32 addrspace(1)* %arrayidx69.us, align 4
  %add.ptr56.sum.us = add i32 %storemerge15.us, %add.ptr38.sum.us
  %arrayidx73.us = getelementptr i32 addrspace(1)* %puiTmpArray, i32 %add.ptr56.sum.us
  %tmp74.us = load i32 addrspace(1)* %arrayidx73.us, align 4
  %add75.us = add i32 %tmp74.us, %tmp70.us
  store i32 %add75.us, i32 addrspace(1)* %arrayidx69.us, align 4
  %inc78.us = add i32 %storemerge15.us, 1
  %cmp65.us = icmp ult i32 %inc78.us, %mul31
  br i1 %cmp65.us, label %for.body66.us, label %for.inc80.us

for.body66.lr.ph.us:                              ; preds = %for.body66.lr.ph.us.preheader, %for.inc80.us
  %storemerge7.us = phi i32 [ %inc82.us, %for.inc80.us ], [ 0, %for.body66.lr.ph.us.preheader ]
  %mul373.us = add i32 %storemerge7.us, %mul35
  %add.ptr38.sum.us = mul i32 %mul373.us, %szBin
  br label %for.body66.us

for.end83.loopexit:                               ; preds = %for.inc80.us
  br label %for.end83

for.end83:                                        ; preds = %for.end83.loopexit, %for.body51.lr.ph, %for.end
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_local_size(i32)

declare i32 @get_group_id(i32)

define void @histogramStep1(i32 addrspace(1)* %puiInputMatrix, i32 addrspace(1)* %puiTmpArray, i32 %szBin, i32 %szBinsPerItem, ...) nounwind {
entry:
  %call = call i32 @get_global_id(i32 0) nounwind
  %mul = mul i32 %call, %szBin
  %add = add i32 %call, 1
  %mul6 = mul i32 %szBinsPerItem, %szBin
  %mul8 = mul i32 %mul6, %add
  %mul14 = mul i32 %mul, %szBinsPerItem
  %cmp1 = icmp ult i32 %mul14, %mul8
  br i1 %cmp1, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %storemerge2 = phi i32 [ %inc24, %for.body ], [ %mul14, %for.body.preheader ]
  %arrayidx = getelementptr i32 addrspace(1)* %puiInputMatrix, i32 %storemerge2
  %tmp19 = load i32 addrspace(1)* %arrayidx, align 4
  %add.ptr.sum = add i32 %tmp19, %mul
  %arrayidx21 = getelementptr i32 addrspace(1)* %puiTmpArray, i32 %add.ptr.sum
  %tmp22 = load i32 addrspace(1)* %arrayidx21, align 4
  %inc = add i32 %tmp22, 1
  store i32 %inc, i32 addrspace(1)* %arrayidx21, align 4
  %inc24 = add i32 %storemerge2, 1
  %cmp = icmp ult i32 %inc24, %mul8
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

define void @histogramStep2int(i32 addrspace(1)* %puiTmpArray, i32 addrspace(1)* %puiOutputArray, i32 %szBin, i32 %szBinsInTmp, i32 %szElemenetsPerItem, ...) nounwind {
entry:
  %call = call i32 @get_global_id(i32 0) nounwind
  %add = add i32 %call, 1
  %mul = mul i32 %add, %szElemenetsPerItem
  %cmp4 = icmp eq i32 %szBinsInTmp, 0
  br i1 %cmp4, label %for.end31, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %mul13 = mul i32 %call, %szElemenetsPerItem
  %cmp172 = icmp ult i32 %mul13, %mul
  br i1 %cmp172, label %for.body18.lr.ph.us.preheader, label %for.end31

for.body18.lr.ph.us.preheader:                    ; preds = %for.body.lr.ph
  br label %for.body18.lr.ph.us

for.inc28.us:                                     ; preds = %for.body18.us
  %inc30.us = add i32 %storemerge5.us, 1
  %cmp.us = icmp ult i32 %inc30.us, %szBinsInTmp
  br i1 %cmp.us, label %for.body18.lr.ph.us, label %for.end31.loopexit

for.body18.us:                                    ; preds = %for.body18.lr.ph.us, %for.body18.us
  %storemerge13.us = phi i32 [ %mul13, %for.body18.lr.ph.us ], [ %inc.us, %for.body18.us ]
  %arrayidx.us = getelementptr i32 addrspace(1)* %puiOutputArray, i32 %storemerge13.us
  %tmp21.us = load i32 addrspace(1)* %arrayidx.us, align 4
  %add.ptr.sum.us = add i32 %storemerge13.us, %mul9.us
  %arrayidx24.us = getelementptr i32 addrspace(1)* %puiTmpArray, i32 %add.ptr.sum.us
  %tmp25.us = load i32 addrspace(1)* %arrayidx24.us, align 4
  %add26.us = add i32 %tmp25.us, %tmp21.us
  store i32 %add26.us, i32 addrspace(1)* %arrayidx.us, align 4
  %inc.us = add i32 %storemerge13.us, 1
  %cmp17.us = icmp ult i32 %inc.us, %mul
  br i1 %cmp17.us, label %for.body18.us, label %for.inc28.us

for.body18.lr.ph.us:                              ; preds = %for.body18.lr.ph.us.preheader, %for.inc28.us
  %storemerge5.us = phi i32 [ %inc30.us, %for.inc28.us ], [ 0, %for.body18.lr.ph.us.preheader ]
  %mul9.us = mul i32 %storemerge5.us, %szBin
  br label %for.body18.us

for.end31.loopexit:                               ; preds = %for.inc28.us
  br label %for.end31

for.end31:                                        ; preds = %for.end31.loopexit, %for.body.lr.ph, %entry
  ret void
}

define void @histogramStep2int2(<2 x i32> addrspace(1)* %puiTmpArray, <2 x i32> addrspace(1)* %puiOutputArray, i32 %szBin, i32 %szBinsInTmp, i32 %szElemenetsPerItem, ...) nounwind {
entry:
  %call = call i32 @get_global_id(i32 0) nounwind
  %add = add i32 %call, 1
  %mul = mul i32 %add, %szElemenetsPerItem
  %cmp4 = icmp eq i32 %szBinsInTmp, 0
  br i1 %cmp4, label %for.end31, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %mul13 = mul i32 %call, %szElemenetsPerItem
  %cmp172 = icmp ult i32 %mul13, %mul
  br i1 %cmp172, label %for.body18.lr.ph.us.preheader, label %for.end31

for.body18.lr.ph.us.preheader:                    ; preds = %for.body.lr.ph
  br label %for.body18.lr.ph.us

for.inc28.us:                                     ; preds = %for.body18.us
  %inc30.us = add i32 %storemerge5.us, 1
  %cmp.us = icmp ult i32 %inc30.us, %szBinsInTmp
  br i1 %cmp.us, label %for.body18.lr.ph.us, label %for.end31.loopexit

for.body18.us:                                    ; preds = %for.body18.lr.ph.us, %for.body18.us
  %storemerge13.us = phi i32 [ %mul13, %for.body18.lr.ph.us ], [ %inc.us, %for.body18.us ]
  %arrayidx.us = getelementptr <2 x i32> addrspace(1)* %puiOutputArray, i32 %storemerge13.us
  %tmp21.us = load <2 x i32> addrspace(1)* %arrayidx.us, align 8
  %add.ptr.sum.us = add i32 %storemerge13.us, %mul9.us
  %arrayidx24.us = getelementptr <2 x i32> addrspace(1)* %puiTmpArray, i32 %add.ptr.sum.us
  %tmp25.us = load <2 x i32> addrspace(1)* %arrayidx24.us, align 8
  %add26.us = add <2 x i32> %tmp21.us, %tmp25.us
  store <2 x i32> %add26.us, <2 x i32> addrspace(1)* %arrayidx.us, align 8
  %inc.us = add i32 %storemerge13.us, 1
  %cmp17.us = icmp ult i32 %inc.us, %mul
  br i1 %cmp17.us, label %for.body18.us, label %for.inc28.us

for.body18.lr.ph.us:                              ; preds = %for.body18.lr.ph.us.preheader, %for.inc28.us
  %storemerge5.us = phi i32 [ %inc30.us, %for.inc28.us ], [ 0, %for.body18.lr.ph.us.preheader ]
  %mul9.us = mul i32 %storemerge5.us, %szBin
  br label %for.body18.us

for.end31.loopexit:                               ; preds = %for.inc28.us
  br label %for.end31

for.end31:                                        ; preds = %for.end31.loopexit, %for.body.lr.ph, %entry
  ret void
}

define void @histogramStep2int4(<4 x i32> addrspace(1)* %puiTmpArray, <4 x i32> addrspace(1)* %puiOutputArray, i32 %szBin, i32 %szBinsInTmp, i32 %szElemenetsPerItem, ...) nounwind {
entry:
  %call = call i32 @get_global_id(i32 0) nounwind
  %add = add i32 %call, 1
  %mul = mul i32 %add, %szElemenetsPerItem
  %cmp4 = icmp eq i32 %szBinsInTmp, 0
  br i1 %cmp4, label %for.end31, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %mul13 = mul i32 %call, %szElemenetsPerItem
  %cmp172 = icmp ult i32 %mul13, %mul
  br i1 %cmp172, label %for.body18.lr.ph.us.preheader, label %for.end31

for.body18.lr.ph.us.preheader:                    ; preds = %for.body.lr.ph
  br label %for.body18.lr.ph.us

for.inc28.us:                                     ; preds = %for.body18.us
  %inc30.us = add i32 %storemerge5.us, 1
  %cmp.us = icmp ult i32 %inc30.us, %szBinsInTmp
  br i1 %cmp.us, label %for.body18.lr.ph.us, label %for.end31.loopexit

for.body18.us:                                    ; preds = %for.body18.lr.ph.us, %for.body18.us
  %storemerge13.us = phi i32 [ %mul13, %for.body18.lr.ph.us ], [ %inc.us, %for.body18.us ]
  %arrayidx.us = getelementptr <4 x i32> addrspace(1)* %puiOutputArray, i32 %storemerge13.us
  %tmp21.us = load <4 x i32> addrspace(1)* %arrayidx.us, align 16
  %add.ptr.sum.us = add i32 %storemerge13.us, %mul9.us
  %arrayidx24.us = getelementptr <4 x i32> addrspace(1)* %puiTmpArray, i32 %add.ptr.sum.us
  %tmp25.us = load <4 x i32> addrspace(1)* %arrayidx24.us, align 16
  %add26.us = add <4 x i32> %tmp21.us, %tmp25.us
  store <4 x i32> %add26.us, <4 x i32> addrspace(1)* %arrayidx.us, align 16
  %inc.us = add i32 %storemerge13.us, 1
  %cmp17.us = icmp ult i32 %inc.us, %mul
  br i1 %cmp17.us, label %for.body18.us, label %for.inc28.us

for.body18.lr.ph.us:                              ; preds = %for.body18.lr.ph.us.preheader, %for.inc28.us
  %storemerge5.us = phi i32 [ %inc30.us, %for.inc28.us ], [ 0, %for.body18.lr.ph.us.preheader ]
  %mul9.us = mul i32 %storemerge5.us, %szBin
  br label %for.body18.us

for.end31.loopexit:                               ; preds = %for.inc28.us
  br label %for.end31

for.end31:                                        ; preds = %for.end31.loopexit, %for.body.lr.ph, %entry
  ret void
}

define void @histogramStep2int8(<8 x i32> addrspace(1)* %puiTmpArray, <8 x i32> addrspace(1)* %puiOutputArray, i32 %szBin, i32 %szBinsInTmp, i32 %szElemenetsPerItem, ...) nounwind {
entry:
  %call = call i32 @get_global_id(i32 0) nounwind
  %add = add i32 %call, 1
  %mul = mul i32 %add, %szElemenetsPerItem
  %cmp4 = icmp eq i32 %szBinsInTmp, 0
  br i1 %cmp4, label %for.end31, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %mul13 = mul i32 %call, %szElemenetsPerItem
  %cmp172 = icmp ult i32 %mul13, %mul
  br i1 %cmp172, label %for.body18.lr.ph.us.preheader, label %for.end31

for.body18.lr.ph.us.preheader:                    ; preds = %for.body.lr.ph
  br label %for.body18.lr.ph.us

for.inc28.us:                                     ; preds = %for.body18.us
  %inc30.us = add i32 %storemerge5.us, 1
  %cmp.us = icmp ult i32 %inc30.us, %szBinsInTmp
  br i1 %cmp.us, label %for.body18.lr.ph.us, label %for.end31.loopexit

for.body18.us:                                    ; preds = %for.body18.lr.ph.us, %for.body18.us
  %storemerge13.us = phi i32 [ %mul13, %for.body18.lr.ph.us ], [ %inc.us, %for.body18.us ]
  %arrayidx.us = getelementptr <8 x i32> addrspace(1)* %puiOutputArray, i32 %storemerge13.us
  %tmp21.us = load <8 x i32> addrspace(1)* %arrayidx.us, align 32
  %add.ptr.sum.us = add i32 %storemerge13.us, %mul9.us
  %arrayidx24.us = getelementptr <8 x i32> addrspace(1)* %puiTmpArray, i32 %add.ptr.sum.us
  %tmp25.us = load <8 x i32> addrspace(1)* %arrayidx24.us, align 32
  %add26.us = add <8 x i32> %tmp21.us, %tmp25.us
  store <8 x i32> %add26.us, <8 x i32> addrspace(1)* %arrayidx.us, align 32
  %inc.us = add i32 %storemerge13.us, 1
  %cmp17.us = icmp ult i32 %inc.us, %mul
  br i1 %cmp17.us, label %for.body18.us, label %for.inc28.us

for.body18.lr.ph.us:                              ; preds = %for.body18.lr.ph.us.preheader, %for.inc28.us
  %storemerge5.us = phi i32 [ %inc30.us, %for.inc28.us ], [ 0, %for.body18.lr.ph.us.preheader ]
  %mul9.us = mul i32 %storemerge5.us, %szBin
  br label %for.body18.us

for.end31.loopexit:                               ; preds = %for.inc28.us
  br label %for.end31

for.end31:                                        ; preds = %for.end31.loopexit, %for.body.lr.ph, %entry
  ret void
}

define void @histogramStep2int16(<16 x i32> addrspace(1)* %puiTmpArray, <16 x i32> addrspace(1)* %puiOutputArray, i32 %szBin, i32 %szBinsInTmp, i32 %szElemenetsPerItem, ...) nounwind {
entry:
  %call = call i32 @get_global_id(i32 0) nounwind
  %add = add i32 %call, 1
  %mul = mul i32 %add, %szElemenetsPerItem
  %cmp4 = icmp eq i32 %szBinsInTmp, 0
  br i1 %cmp4, label %for.end31, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %mul13 = mul i32 %call, %szElemenetsPerItem
  %cmp172 = icmp ult i32 %mul13, %mul
  br i1 %cmp172, label %for.body18.lr.ph.us.preheader, label %for.end31

for.body18.lr.ph.us.preheader:                    ; preds = %for.body.lr.ph
  br label %for.body18.lr.ph.us

for.inc28.us:                                     ; preds = %for.body18.us
  %inc30.us = add i32 %storemerge5.us, 1
  %cmp.us = icmp ult i32 %inc30.us, %szBinsInTmp
  br i1 %cmp.us, label %for.body18.lr.ph.us, label %for.end31.loopexit

for.body18.us:                                    ; preds = %for.body18.lr.ph.us, %for.body18.us
  %storemerge13.us = phi i32 [ %mul13, %for.body18.lr.ph.us ], [ %inc.us, %for.body18.us ]
  %arrayidx.us = getelementptr <16 x i32> addrspace(1)* %puiOutputArray, i32 %storemerge13.us
  %tmp21.us = load <16 x i32> addrspace(1)* %arrayidx.us, align 64
  %add.ptr.sum.us = add i32 %storemerge13.us, %mul9.us
  %arrayidx24.us = getelementptr <16 x i32> addrspace(1)* %puiTmpArray, i32 %add.ptr.sum.us
  %tmp25.us = load <16 x i32> addrspace(1)* %arrayidx24.us, align 64
  %add26.us = add <16 x i32> %tmp21.us, %tmp25.us
  store <16 x i32> %add26.us, <16 x i32> addrspace(1)* %arrayidx.us, align 64
  %inc.us = add i32 %storemerge13.us, 1
  %cmp17.us = icmp ult i32 %inc.us, %mul
  br i1 %cmp17.us, label %for.body18.us, label %for.inc28.us

for.body18.lr.ph.us:                              ; preds = %for.body18.lr.ph.us.preheader, %for.inc28.us
  %storemerge5.us = phi i32 [ %inc30.us, %for.inc28.us ], [ 0, %for.body18.lr.ph.us.preheader ]
  %mul9.us = mul i32 %storemerge5.us, %szBin
  br label %for.body18.us

for.end31.loopexit:                               ; preds = %for.inc28.us
  br label %for.end31

for.end31:                                        ; preds = %for.end31.loopexit, %for.body.lr.ph, %entry
  ret void
}
