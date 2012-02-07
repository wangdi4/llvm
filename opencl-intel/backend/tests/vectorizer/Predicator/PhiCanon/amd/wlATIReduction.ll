; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlATIReduction.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"

@sgv = internal constant [4 x i8] c"229\00"		; <[4 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]

; CHECK: @reduce
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret

define void @reduce(<4 x i32> addrspace(1)* %input, <4 x i32> addrspace(1)* %output, <4 x i32> addrspace(3)* %sdata, ...) nounwind {
entry:
  %call = call i32 @get_local_id(i32 0) nounwind
  %call1 = call i32 @get_group_id(i32 0) nounwind
  %call2 = call i32 @get_global_id(i32 0) nounwind
  %call3 = call i32 @get_local_size(i32 0) nounwind
  %arrayidx = getelementptr <4 x i32> addrspace(3)* %sdata, i32 %call
  %arrayidx7 = getelementptr <4 x i32> addrspace(1)* %input, i32 %call2
  %tmp8 = load <4 x i32> addrspace(1)* %arrayidx7, align 16
  store <4 x i32> %tmp8, <4 x i32> addrspace(3)* %arrayidx, align 16
  call void @barrier(i32 1) nounwind
  %storemerge1 = lshr i32 %call3, 1
  %cmp2 = icmp eq i32 %storemerge1, 0
  br i1 %cmp2, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %if.end
  %storemerge3 = phi i32 [ %storemerge, %if.end ], [ %storemerge1, %for.body.preheader ]
  %cmp14 = icmp ult i32 %call, %storemerge3
  br i1 %cmp14, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %tmp18 = load <4 x i32> addrspace(3)* %arrayidx, align 16
  %add = add i32 %storemerge3, %call
  %arrayidx22 = getelementptr <4 x i32> addrspace(3)* %sdata, i32 %add
  %tmp23 = load <4 x i32> addrspace(3)* %arrayidx22, align 16
  %add24 = add <4 x i32> %tmp18, %tmp23
  store <4 x i32> %add24, <4 x i32> addrspace(3)* %arrayidx, align 16
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  call void @barrier(i32 1) nounwind
  %storemerge = lshr i32 %storemerge3, 1
  %cmp = icmp eq i32 %storemerge, 0
  br i1 %cmp, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %if.end
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %cmp27 = icmp eq i32 %call, 0
  br i1 %cmp27, label %if.then28, label %if.end35

if.then28:                                        ; preds = %for.end
  %arrayidx31 = getelementptr <4 x i32> addrspace(1)* %output, i32 %call1
  %tmp34 = load <4 x i32> addrspace(3)* %sdata, align 16
  store <4 x i32> %tmp34, <4 x i32> addrspace(1)* %arrayidx31, align 16
  br label %if.end35

if.end35:                                         ; preds = %if.then28, %for.end
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_group_id(i32)

declare i32 @get_global_id(i32)

declare i32 @get_local_size(i32)

declare void @barrier(i32)
