; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'wlATIRadixSort.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32"
target triple = "i686-pc-win32"

@sgv = internal constant [5 x i8] c"1209\00"		; <[5 x i8]*> [#uses=1]
@fgv = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]
@sgv1 = internal constant [6 x i8] c"11092\00"		; <[6 x i8]*> [#uses=1]
@fgv2 = internal constant [0 x i8] zeroinitializer		; <[0 x i8]*> [#uses=1]
@lvgv3 = internal constant [0 x i8*] zeroinitializer		; <[0 x i8*]*> [#uses=1]

; CHECK: @histogram
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret
; CHECK: @permute
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret

define void @histogram(i32 addrspace(1)* %unsortedData, i32 addrspace(1)* %buckets, i32 %shiftCount, i16 addrspace(3)* %sharedArray, ...) nounwind {
entry:
  %call = call i32 @get_local_id(i32 0) nounwind
  %call1 = call i32 @get_global_id(i32 0) nounwind
  %call2 = call i32 @get_group_id(i32 0) nounwind
  %0 = shl i32 %call, 8
  %scevgep = getelementptr i16 addrspace(3)* %sharedArray, i32 %0
  %scevgep6 = bitcast i16 addrspace(3)* %scevgep to i8 addrspace(3)*
  call void @llvm.memset.p3i8.i32(i8 addrspace(3)* %scevgep6, i8 0, i32 512, i32 2, i1 false)
  call void @barrier(i32 1) nounwind
  %mul15 = shl i32 %call1, 8
  %and = and i32 %shiftCount, 31
  %mul25 = shl i32 %call, 8
  br label %for.body12

for.body12:                                       ; preds = %entry, %for.body12
  %storemerge14 = phi i32 [ 0, %entry ], [ %inc34, %for.body12 ]
  %add17 = add i32 %storemerge14, %mul15
  %arrayidx19 = getelementptr i32 addrspace(1)* %unsortedData, i32 %add17
  %tmp20 = load i32 addrspace(1)* %arrayidx19, align 4
  %shr = lshr i32 %tmp20, %and
  %and23 = and i32 %shr, 255
  %add27 = or i32 %and23, %mul25
  %arrayidx29 = getelementptr i16 addrspace(3)* %sharedArray, i32 %add27
  %tmp30 = load i16 addrspace(3)* %arrayidx29, align 2
  %inc31 = add i16 %tmp30, 1
  store i16 %inc31, i16 addrspace(3)* %arrayidx29, align 2
  %inc34 = add i32 %storemerge14, 1
  %cmp11 = icmp slt i32 %inc34, 256
  br i1 %cmp11, label %for.body12, label %for.end35

for.end35:                                        ; preds = %for.body12
  call void @barrier(i32 1) nounwind
  %mul45 = shl i32 %call2, 12
  %add48 = add i32 %mul45, %mul25
  br label %for.body41

for.body41:                                       ; preds = %for.end35, %for.body41
  %storemerge3 = phi i32 [ 0, %for.end35 ], [ %inc63, %for.body41 ]
  %add50 = add i32 %add48, %storemerge3
  %arrayidx53 = getelementptr i32 addrspace(1)* %buckets, i32 %add50
  %add57 = add i32 %storemerge3, %mul25
  %arrayidx59 = getelementptr i16 addrspace(3)* %sharedArray, i32 %add57
  %tmp60 = load i16 addrspace(3)* %arrayidx59, align 2
  %conv = zext i16 %tmp60 to i32
  store i32 %conv, i32 addrspace(1)* %arrayidx53, align 4
  %inc63 = add i32 %storemerge3, 1
  %cmp40 = icmp slt i32 %inc63, 256
  br i1 %cmp40, label %for.body41, label %for.end64

for.end64:                                        ; preds = %for.body41
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_global_id(i32)

declare i32 @get_group_id(i32)

declare void @barrier(i32)

define void @permute(i32 addrspace(1)* %unsortedData, i32 addrspace(1)* %prescanedBuckets, i32 %shiftCount, i16 addrspace(3)* %sharedBuckets, i32 addrspace(1)* %sortedData, ...) nounwind {
entry:
  %call = call i32 @get_group_id(i32 0) nounwind
  %call1 = call i32 @get_local_id(i32 0) nounwind
  %call2 = call i32 @get_global_id(i32 0) nounwind
  %mul5 = shl i32 %call, 12
  %mul7 = shl i32 %call1, 8
  %add = add i32 %mul7, %mul5
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %storemerge13 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %add9 = add i32 %add, %storemerge13
  %add13 = add i32 %storemerge13, %mul7
  %arrayidx = getelementptr i16 addrspace(3)* %sharedBuckets, i32 %add13
  %arrayidx17 = getelementptr i32 addrspace(1)* %prescanedBuckets, i32 %add9
  %tmp18 = load i32 addrspace(1)* %arrayidx17, align 4
  %conv = trunc i32 %tmp18 to i16
  store i16 %conv, i16 addrspace(3)* %arrayidx, align 2
  %inc = add i32 %storemerge13, 1
  %cmp = icmp slt i32 %inc, 256
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @barrier(i32 1) nounwind
  %mul29 = shl i32 %call2, 8
  %and = and i32 %shiftCount, 31
  br label %for.body26

for.body26:                                       ; preds = %for.end, %for.body26
  %storemerge2 = phi i32 [ 0, %for.end ], [ %inc68, %for.body26 ]
  %add31 = add i32 %storemerge2, %mul29
  %arrayidx33 = getelementptr i32 addrspace(1)* %unsortedData, i32 %add31
  %tmp34 = load i32 addrspace(1)* %arrayidx33, align 4
  %shr = lshr i32 %tmp34, %and
  %and37 = and i32 %shr, 255
  %add42 = or i32 %and37, %mul7
  %arrayidx44 = getelementptr i16 addrspace(3)* %sharedBuckets, i32 %add42
  %tmp45 = load i16 addrspace(3)* %arrayidx44, align 2
  %conv46 = zext i16 %tmp45 to i32
  %arrayidx49 = getelementptr i32 addrspace(1)* %sortedData, i32 %conv46
  store i32 %tmp34, i32 addrspace(1)* %arrayidx49, align 4
  %add64 = add i16 %tmp45, 1
  store i16 %add64, i16 addrspace(3)* %arrayidx44, align 2
  %inc68 = add i32 %storemerge2, 1
  %cmp24 = icmp slt i32 %inc68, 256
  br i1 %cmp24, label %for.body26, label %for.end69

for.end69:                                        ; preds = %for.body26
  ret void
}

declare void @llvm.memset.p3i8.i32(i8 addrspace(3)* nocapture, i8, i32, i32, i1) nounwind
