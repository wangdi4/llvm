; RUN: opt -inline -inline-report=15 < %s -S 2>&1 | FileCheck %s
; CHECK-NOT: <<Callee is always inline>>
; Test should not produce always inline message, as no function has been
; marked with "always inline" attribute.
;
declare i64 @strlen(i8*) #2

declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #1

declare i8* @RelinquishMagickMemory(i8*) #2

; Function Attrs: nounwind uwtable
define i8* @DestroyString(i8* %string) local_unnamed_addr #3 {
entry:
  %call = tail call i8* @RelinquishMagickMemory(i8* %string) #6
  ret i8* %call
}

declare i8* @AcquireString(i8*) #2

; Function Attrs: noreturn
declare void @_exit(i32) local_unnamed_addr #4

; Function Attrs: nounwind uwtable
define i32 @ConcatenateString(i8** %destination, i8* %source) local_unnamed_addr #0 {
entry:
  %cmp = icmp eq i8* %source, null
  br i1 %cmp, label %cleanup, label %if.end

if.end:                                           ; preds = %entry
  %0 = load i8*, i8** %destination, align 8
  %cmp1 = icmp eq i8* %0, null
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.end
  %call = call i8* @AcquireString(i8* nonnull %source) #6
  store i8* %call, i8** %destination, align 8
  br label %cleanup

if.end3:                                          ; preds = %if.end
  %call4 = call i64 @strlen(i8* nonnull %0)
  %conv = trunc i64 %call4 to i32
  %call5 = call i64 @strlen(i8* nonnull %source)
  %conv6 = trunc i64 %call5 to i32
  %neg = xor i32 %conv, -1
  %cmp7 = icmp slt i32 %neg, %conv6
  br i1 %cmp7, label %if.then9, label %if.end11

if.then9:                                         ; preds = %if.end3
  %call10 = call i8* @DestroyString(i8* undef)
  call void @_exit(i32 -1) #7
  unreachable

if.end11:                                         ; preds = %if.end3
  %add = add nsw i32 %conv, %conv6
  %cmp13 = icmp sgt i32 %add, -4097
  br i1 %cmp13, label %if.then15, label %if.end18

if.then15:                                        ; preds = %if.end11
  %call17 = call i8* @DestroyString(i8* undef)
  call void @_exit(i32 -2) #7
  unreachable

if.end18:                                         ; preds = %if.end11
  %1 = load i8*, i8** %destination, align 8
  %cmp19 = icmp eq i8* %1, null
  br i1 %cmp19, label %if.then21, label %if.end24

if.then21:                                        ; preds = %if.end18
  %call23 = call i8* @DestroyString(i8* undef)
  call void @_exit(i32 -3) #7
  unreachable

if.end24:                                         ; preds = %if.end18
  %cmp25 = icmp eq i32 %conv6, 0
  br i1 %cmp25, label %if.end29, label %if.then27

if.then27:                                        ; preds = %if.end24
  %sext = shl i64 %call4, 32
  %idx.ext = ashr exact i64 %sext, 32
  %add.ptr = getelementptr inbounds i8, i8* %1, i64 %idx.ext
  %sext50 = shl i64 %call5, 32
  %conv28 = ashr exact i64 %sext50, 32
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %add.ptr, i8* nonnull %source, i64 %conv28, i32 1, i1 false)
  br label %if.end29

if.end29:                                         ; preds = %if.end24, %if.then27
  %2 = load i8*, i8** %destination, align 8
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds i8, i8* %2, i64 %idxprom
  store i8 0, i8* %arrayidx, align 1
  br label %cleanup

cleanup:                                          ; preds = %entry, %if.end29, %if.then2
  ret i32 1
}
