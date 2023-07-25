; RUN: opt -passes="instcombine" -S < %s 2>&1 | FileCheck %s
; This test check bailout for following pattern
;  %cond = phi i64 [ %sub, %cond.true ], [ %conv, %cond.false ]
;  %conv3 = trunc i64 %cond to i32
;  ...
;  %conv23 = zext i32 %conv3 to i64
;  ...
;  %conv66 = zext i32 %conv3 to i64
; Bailout is required becuase it is unclear whether converting PHI
; into 32-bits will be profitable if computation is not 100% 32-bits.

; CHECK:      cond.end:                                         ; preds = %cond.false, %cond.true
; CHECK-NEXT:   %cond = phi i64 [ %sub, %cond.true ], [ %conv, %cond.false ]
; CHECK-NEXT:   %conv3 = trunc i64 %cond to i32
; CHECK-NEXT:   %3 = load i32, ptr %len, align 4
; CHECK-NEXT:   %sub4 = sub i32 %3, %conv3
; CHECK-NEXT:   store i32 %sub4, ptr %len, align 4
; CHECK-NEXT:   %cmp5 = icmp ugt i32 %conv3, %distance
; CHECK-NEXT:   br i1 %cmp5, label %do.body, label %if.else

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.lzma_dict = type { ptr, i64, i64, i64, i64, i8 }

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg)

; Function Attrs: inlinehint nounwind uwtable
define internal fastcc zeroext i1 @dict_repeat(ptr noundef %dict, i32 noundef %distance, ptr noundef %len) {
entry:
  %limit = getelementptr inbounds %struct.lzma_dict, ptr %dict, i32 0, i32 3
  %0 = load i64, ptr %limit, align 8
  %pos = getelementptr inbounds %struct.lzma_dict, ptr %dict, i32 0, i32 1
  %1 = load i64, ptr %pos, align 8
  %sub = sub i64 %0, %1
  %2 = load i32, ptr %len, align 4
  %conv = zext i32 %2 to i64
  %cmp = icmp ult i64 %sub, %conv
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  br label %cond.end

cond.false:                                       ; preds = %entry
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i64 [ %sub, %cond.true ], [ %conv, %cond.false ]
  %conv3 = trunc i64 %cond to i32
  %3 = load i32, ptr %len, align 4
  %sub4 = sub i32 %3, %conv3
  store i32 %sub4, ptr %len, align 4
  %cmp5 = icmp ult i32 %distance, %conv3
  br i1 %cmp5, label %do.body, label %if.else

do.body:                                          ; preds = %cond.end, %do.body
  %left.0 = phi i32 [ %conv3, %cond.end ], [ %dec, %do.body ]
  %4 = load ptr, ptr %dict, align 8
  %5 = load i64, ptr %pos, align 8
  %arrayidx = getelementptr inbounds i8, ptr %4, i64 %5
  %6 = load i64, ptr %pos, align 8
  %inc = add i64 %6, 1
  store i64 %inc, ptr %pos, align 8
  %dec = add i32 %left.0, -1
  %cmp9 = icmp ugt i32 %dec, 0
  br i1 %cmp9, label %do.body, label %if.end71

if.else:                                          ; preds = %cond.end
  %conv11 = zext i32 %distance to i64
  %7 = load i64, ptr %pos, align 8
  %cmp13 = icmp ult i64 %conv11, %7
  br i1 %cmp13, label %if.then15, label %if.else26

if.then15:                                        ; preds = %if.else
  %8 = load ptr, ptr %dict, align 8
  %add.ptr = getelementptr inbounds i8, ptr %8, i64 %7
  %idx.neg = sub i64 0, %conv11
  %add.ptr21 = getelementptr inbounds i8, ptr %add.ptr, i64 %idx.neg
  %add.ptr22 = getelementptr inbounds i8, ptr %add.ptr21, i64 -1
  %conv23 = zext i32 %conv3 to i64
  call void @llvm.memcpy.p0.p0.i64(ptr align 1 %add.ptr, ptr align 1 %add.ptr22, i64 %conv23, i1 false)
  %9 = load i64, ptr %pos, align 8
  %add = add i64 %9, %conv23
  store i64 %add, ptr %pos, align 8
  br label %if.end71

if.else26:                                        ; preds = %if.else
  %sub29 = sub i64 %7, %conv11
  %sub30 = sub i64 %sub29, 1
  %size = getelementptr inbounds %struct.lzma_dict, ptr %dict, i32 0, i32 4
  %10 = load i64, ptr %size, align 8
  %add31 = add i64 %sub30, %10
  %conv32 = trunc i64 %add31 to i32
  %conv34 = zext i32 %conv32 to i64
  %sub35 = sub i64 %10, %conv34
  %conv36 = trunc i64 %sub35 to i32
  %cmp37 = icmp ult i32 %conv36, %conv3
  br i1 %cmp37, label %if.then39, label %if.else59

if.then39:                                        ; preds = %if.else26
  %11 = load ptr, ptr %dict, align 8
  %add.ptr42 = getelementptr inbounds i8, ptr %11, i64 %7
  %add.ptr45 = getelementptr inbounds i8, ptr %11, i64 %conv34
  %conv46 = zext i32 %conv36 to i64
  call void @llvm.memmove.p0.p0.i64(ptr align 1 %add.ptr42, ptr align 1 %add.ptr45, i64 %conv46, i1 false)
  %12 = load i64, ptr %pos, align 8
  %add49 = add i64 %12, %conv46
  store i64 %add49, ptr %pos, align 8
  %sub50 = sub i32 %conv3, %conv36
  %13 = load ptr, ptr %dict, align 8
  %add.ptr53 = getelementptr inbounds i8, ptr %13, i64 %add49
  %conv55 = zext i32 %sub50 to i64
  call void @llvm.memcpy.p0.p0.i64(ptr align 1 %add.ptr53, ptr align 1 %13, i64 %conv55, i1 false)
  %14 = load i64, ptr %pos, align 8
  %add58 = add i64 %14, %conv55
  store i64 %add58, ptr %pos, align 8
  br label %if.end

if.else59:                                        ; preds = %if.else26
  %15 = load ptr, ptr %dict, align 8
  %add.ptr62 = getelementptr inbounds i8, ptr %15, i64 %7
  %add.ptr65 = getelementptr inbounds i8, ptr %15, i64 %conv34
  %conv66 = zext i32 %conv3 to i64
  call void @llvm.memmove.p0.p0.i64(ptr align 1 %add.ptr62, ptr align 1 %add.ptr65, i64 %conv66, i1 false)
  %16 = load i64, ptr %pos, align 8
  %add69 = add i64 %16, %conv66
  store i64 %add69, ptr %pos, align 8
  br label %if.end

if.end:                                           ; preds = %if.else59, %if.then39
  br label %if.end71

if.end71:                                         ; preds = %if.then15, %if.end, %do.body
  %full = getelementptr inbounds %struct.lzma_dict, ptr %dict, i32 0, i32 2
  %17 = load i64, ptr %full, align 8
  %18 = load i64, ptr %pos, align 8
  %cmp73 = icmp ult i64 %17, %18
  br i1 %cmp73, label %if.then75, label %if.end78

if.then75:                                        ; preds = %if.end71
  store i64 %18, ptr %full, align 8
  br label %if.end78

if.end78:                                         ; preds = %if.then75, %if.end71
  %19 = load i32, ptr %len, align 4
  %cmp79 = icmp ne i32 %19, 0
  ret i1 %cmp79
}

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memmove.p0.p0.i64(ptr nocapture writeonly, ptr nocapture readonly, i64, i1 immarg)
