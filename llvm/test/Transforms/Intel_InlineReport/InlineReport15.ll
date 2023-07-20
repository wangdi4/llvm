; Inline report
; RUN: opt -passes='cgscc(inline)' -inline-report=15 < %s -S 2>&1 | FileCheck %s
; Inline report via metadata
; RUN: opt -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -inline-report=142 < %s -S 2>&1 | FileCheck %s

; CHECK-NOT: <<Callee is always inline>>
; Test should not produce always inline message, as no function has been
; marked with "always inline" attribute.
;

declare i64 @strlen(ptr)

declare ptr @RelinquishMagickMemory(ptr)

define ptr @DestroyString(ptr %string) local_unnamed_addr {
entry:
  %call = tail call ptr @RelinquishMagickMemory(ptr %string)
  ret ptr %call
}

declare ptr @AcquireString(ptr)

declare void @_exit(i32) local_unnamed_addr

define i32 @ConcatenateString(ptr %destination, ptr %source) local_unnamed_addr {
entry:
  %cmp = icmp eq ptr %source, null
  br i1 %cmp, label %cleanup, label %if.end

if.end:                                           ; preds = %entry
  %i = load ptr, ptr %destination, align 8
  %cmp1 = icmp eq ptr %i, null
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.end
  %call = call ptr @AcquireString(ptr nonnull %source)
  store ptr %call, ptr %destination, align 8
  br label %cleanup

if.end3:                                          ; preds = %if.end
  %call4 = call i64 @strlen(ptr nonnull %i)
  %conv = trunc i64 %call4 to i32
  %call5 = call i64 @strlen(ptr nonnull %source)
  %conv6 = trunc i64 %call5 to i32
  %neg = xor i32 %conv, -1
  %cmp7 = icmp slt i32 %neg, %conv6
  br i1 %cmp7, label %if.then9, label %if.end11

if.then9:                                         ; preds = %if.end3
  %call10 = call ptr @DestroyString(ptr undef)
  call void @_exit(i32 -1)
  unreachable

if.end11:                                         ; preds = %if.end3
  %add = add nsw i32 %conv, %conv6
  %cmp13 = icmp sgt i32 %add, -4097
  br i1 %cmp13, label %if.then15, label %if.end18

if.then15:                                        ; preds = %if.end11
  %call17 = call ptr @DestroyString(ptr undef)
  call void @_exit(i32 -2)
  unreachable

if.end18:                                         ; preds = %if.end11
  %i1 = load ptr, ptr %destination, align 8
  %cmp19 = icmp eq ptr %i1, null
  br i1 %cmp19, label %if.then21, label %if.end24

if.then21:                                        ; preds = %if.end18
  %call23 = call ptr @DestroyString(ptr undef)
  call void @_exit(i32 -3)
  unreachable

if.end24:                                         ; preds = %if.end18
  %cmp25 = icmp eq i32 %conv6, 0
  br i1 %cmp25, label %if.end29, label %if.then27

if.then27:                                        ; preds = %if.end24
  %sext = shl i64 %call4, 32
  %idx.ext = ashr exact i64 %sext, 32
  %add.ptr = getelementptr inbounds i8, ptr %i1, i64 %idx.ext
  %sext50 = shl i64 %call5, 32
  %conv28 = ashr exact i64 %sext50, 32
  call void @llvm.memcpy.p0.p0.i64(ptr align 1 %add.ptr, ptr nonnull align 1 %source, i64 %conv28, i1 false)
  br label %if.end29

if.end29:                                         ; preds = %if.then27, %if.end24
  %i2 = load ptr, ptr %destination, align 8
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds i8, ptr %i2, i64 %idxprom
  store i8 0, ptr %arrayidx, align 1
  br label %cleanup

cleanup:                                          ; preds = %if.end29, %if.then2, %entry
  ret i32 1
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #0

attributes #0 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
