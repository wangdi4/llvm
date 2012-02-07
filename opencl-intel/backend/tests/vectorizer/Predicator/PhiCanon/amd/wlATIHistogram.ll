; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlATIHistogram.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"

@sgv = internal constant [5 x i8] c"1920\00"		; <[5 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]

; CHECK: @histogram
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.end88.loopexit, %for.end88.loopexit14
; CHECK: ret

define void @histogram(i32 addrspace(1)* %data, i8 addrspace(3)* %sharedArray, i32 addrspace(1)* %binResult, i32 %BIN_SIZE, ...) nounwind {
entry:
  %call = call i32 @get_local_id(i32 0) nounwind
  %call1 = call i32 @get_global_id(i32 0) nounwind
  %call2 = call i32 @get_group_id(i32 0) nounwind
  %call3 = call i32 @get_local_size(i32 0) nounwind
  %cmp12 = icmp eq i32 %BIN_SIZE, 0
  br i1 %cmp12, label %for.end.thread, label %for.body16.lr.ph

for.end.thread:                                   ; preds = %entry
  call void @barrier(i32 1) nounwind
  br label %for.end38

for.body16.lr.ph:                                 ; preds = %entry
  %mul = mul i32 %call, %BIN_SIZE
  %scevgep = getelementptr i8 addrspace(3)* %sharedArray, i32 %mul
  call void @llvm.memset.p3i8.i32(i8 addrspace(3)* %scevgep, i8 0, i32 %BIN_SIZE, i32 1, i1 false)
  call void @barrier(i32 1) nounwind
  %mul20 = mul i32 %call1, %BIN_SIZE
  %mul28 = mul i32 %call, %BIN_SIZE
  br label %for.body16

for.body16:                                       ; preds = %for.body16.lr.ph, %for.body16
  %storemerge211 = phi i32 [ 0, %for.body16.lr.ph ], [ %inc37, %for.body16 ]
  %add22 = add i32 %storemerge211, %mul20
  %arrayidx24 = getelementptr i32 addrspace(1)* %data, i32 %add22
  %tmp25 = load i32 addrspace(1)* %arrayidx24, align 4
  %add30 = add i32 %tmp25, %mul28
  %arrayidx32 = getelementptr i8 addrspace(3)* %sharedArray, i32 %add30
  %tmp33 = load i8 addrspace(3)* %arrayidx32, align 1
  %inc34 = add i8 %tmp33, 1
  store i8 %inc34, i8 addrspace(3)* %arrayidx32, align 1
  %inc37 = add i32 %storemerge211, 1
  %cmp15 = icmp ult i32 %inc37, %BIN_SIZE
  br i1 %cmp15, label %for.body16, label %for.end38.loopexit

for.end38.loopexit:                               ; preds = %for.body16
  br label %for.end38

for.end38:                                        ; preds = %for.end38.loopexit, %for.end.thread
  call void @barrier(i32 1) nounwind
  %cmp45 = icmp ne i32 %call3, 0
  %nonzero = select i1 %cmp45, i32 %call3, i32 1
  %div = udiv i32 %BIN_SIZE, %nonzero
  %cmp468 = icmp eq i32 %div, 0
  br i1 %cmp468, label %for.end88, label %for.cond50.preheader.lr.ph

for.cond50.preheader.lr.ph:                       ; preds = %for.end38
  %cmp535 = icmp eq i32 %call3, 0
  %mul75 = mul i32 %call2, %BIN_SIZE
  %add79 = add i32 %mul75, %call
  br i1 %cmp535, label %for.end72.preheader, label %for.body54.lr.ph.us.preheader

for.end72.preheader:                              ; preds = %for.cond50.preheader.lr.ph
  br label %for.end72

for.body54.lr.ph.us.preheader:                    ; preds = %for.cond50.preheader.lr.ph
  br label %for.body54.lr.ph.us

for.end72.us:                                     ; preds = %for.body54.us
  %add81.us = add i32 %add79, %mul61.us
  %arrayidx83.us = getelementptr i32 addrspace(1)* %binResult, i32 %add81.us
  store i32 %add68.us, i32 addrspace(1)* %arrayidx83.us, align 4
  %inc87.us = add i32 %storemerge9.us, 1
  %cmp46.us = icmp ult i32 %inc87.us, %div
  br i1 %cmp46.us, label %for.body54.lr.ph.us, label %for.end88.loopexit

for.body54.us:                                    ; preds = %for.body54.lr.ph.us, %for.body54.us
  %storemerge17.us = phi i32 [ 0, %for.body54.lr.ph.us ], [ %inc71.us, %for.body54.us ]
  %tmp8446.us = phi i32 [ 0, %for.body54.lr.ph.us ], [ %add68.us, %for.body54.us ]
  %mul58.us = mul i32 %storemerge17.us, %BIN_SIZE
  %add64.us = add i32 %add62.us, %mul58.us
  %arrayidx66.us = getelementptr i8 addrspace(3)* %sharedArray, i32 %add64.us
  %tmp67.us = load i8 addrspace(3)* %arrayidx66.us, align 1
  %conv.us = zext i8 %tmp67.us to i32
  %add68.us = add i32 %conv.us, %tmp8446.us
  %inc71.us = add i32 %storemerge17.us, 1
  %cmp53.us = icmp ult i32 %inc71.us, %call3
  br i1 %cmp53.us, label %for.body54.us, label %for.end72.us

for.body54.lr.ph.us:                              ; preds = %for.body54.lr.ph.us.preheader, %for.end72.us
  %storemerge9.us = phi i32 [ %inc87.us, %for.end72.us ], [ 0, %for.body54.lr.ph.us.preheader ]
  %mul61.us = mul i32 %storemerge9.us, %call3
  %add62.us = add i32 %mul61.us, %call
  br label %for.body54.us

for.end72:                                        ; preds = %for.end72.preheader, %for.end72
  %storemerge9 = phi i32 [ %inc87, %for.end72 ], [ 0, %for.end72.preheader ]
  %arrayidx83 = getelementptr i32 addrspace(1)* %binResult, i32 %add79
  store i32 0, i32 addrspace(1)* %arrayidx83, align 4
  %inc87 = add i32 %storemerge9, 1
  %cmp46 = icmp ult i32 %inc87, %div
  br i1 %cmp46, label %for.end72, label %for.end88.loopexit14

for.end88.loopexit:                               ; preds = %for.end72.us
  br label %for.end88

for.end88.loopexit14:                             ; preds = %for.end72
  br label %for.end88

for.end88:                                        ; preds = %for.end88.loopexit14, %for.end88.loopexit, %for.end38
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_global_id(i32)

declare i32 @get_group_id(i32)

declare i32 @get_local_size(i32)

declare void @barrier(i32)

declare void @llvm.memset.p3i8.i32(i8 addrspace(3)* nocapture, i8, i32, i32, i1) nounwind
