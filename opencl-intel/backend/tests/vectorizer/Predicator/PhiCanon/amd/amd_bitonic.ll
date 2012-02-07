; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'amd_bitonic.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"

@sgv = internal constant [6 x i8] c"20000\00"		; <[6 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]

; CHECK: @bitonicSort
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret

define void @bitonicSort(i32 addrspace(1)* %theArray, i32 %stage, i32 %passOfStage, i32 %width, i32 %direction, ...) nounwind {
entry:
  %call = call i32 @get_global_id(i32 0) nounwind
  %sub = sub i32 %stage, %passOfStage
  %and = and i32 %sub, 31
  %shl = shl i32 1, %and
  %0 = add i32 %shl, -1
  %rem = and i32 %call, %0
  %div = lshr i32 %call, %and
  %mul = shl i32 %shl, 1
  %mul15 = mul i32 %mul, %div
  %add = add i32 %mul15, %rem
  %add19 = add i32 %add, %shl
  %arrayidx = getelementptr i32 addrspace(1)* %theArray, i32 %add
  %tmp23 = load i32 addrspace(1)* %arrayidx, align 4
  %arrayidx27 = getelementptr i32 addrspace(1)* %theArray, i32 %add19
  %tmp28 = load i32 addrspace(1)* %arrayidx27, align 4
  %and31 = and i32 %stage, 31
  %1 = shl i32 1, %and31
  %rem381 = and i32 %call, %1
  %sub41 = sub i32 1, %direction
  %cmp39 = icmp eq i32 %rem381, 0
  %direction.sub41 = select i1 %cmp39, i32 %direction, i32 %sub41
  %cmp46 = icmp ugt i32 %tmp23, %tmp28
  %tmp673 = select i1 %cmp46, i32 %tmp23, i32 %tmp28
  %storemerge = select i1 %cmp46, i32 %tmp28, i32 %tmp23
  %tobool = icmp eq i32 %direction.sub41, 0
  %tmp673.storemerge = select i1 %tobool, i32 %tmp673, i32 %storemerge
  %storemerge.tmp673 = select i1 %tobool, i32 %storemerge, i32 %tmp673
  store i32 %tmp673.storemerge, i32 addrspace(1)* %arrayidx, align 4
  store i32 %storemerge.tmp673, i32 addrspace(1)* %arrayidx27, align 4
  ret void
}

declare i32 @get_global_id(i32)
