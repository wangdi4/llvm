; Inline report
; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 -disable-output < %s -S 2>&1 | FileCheck %s
; Inline report via metadata
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -disable-output 2>&1 | FileCheck %s

; This test tests various inlining report features for programs that
; contain varags intrinsics like llvm.va_arg_pack and llvm.va_arg_pack_len.
; The function myopener() should be inlined and the inlining report should
; show the call to myopenva() as nested within it.

; NOTE: The new pass manager does not dead code eliminate myopener and myopen
; as part of the inlining pass, hence the need to check for different results.

; Function Attrs: nounwind
declare i32 @llvm.va_arg_pack() #3

; Function Attrs: nounwind
declare i32 @llvm.va_arg_pack_len() #3

declare dso_local i32 @warn_open_too_many_arguments()

; Function Attrs: noreturn
declare dso_local void @abort() #1

%struct.__va_list_tag = type { i32, i32, i8*, i8* }
@expected_char = common dso_local global i8 0, align 1
@.str = private unnamed_addr constant [2 x i8] c"a\00", align 1
@l0 = common dso_local global i32 0, align 4
@myglobal = common dso_local global i32 0, align 4

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #2

; Function Attrs: nounwind
declare void @llvm.va_start(i8*) #3

; Function Attrs: nounwind
declare void @llvm.va_end(i8*) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #2

; Function Attrs: alwaysinline nounwind uwtable
define available_externally dso_local i32 @myopen(i8* %path, i32 %oflag, ...) local_unnamed_addr #5 {
entry:
  %0 = tail call i32 @llvm.va_arg_pack_len()
  %cmp = icmp sgt i32 %0, 1
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %call = tail call i32 @warn_open_too_many_arguments() #3
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %1 = tail call i32 @llvm.va_arg_pack_len()
  %cmp1 = icmp slt i32 %1, 1
  br i1 %cmp1, label %if.then2, label %if.end4

if.then2:                                         ; preds = %if.end
  %call3 = tail call i32 @myopen2(i8* %path, i32 %oflag)
  br label %return

if.end4:                                          ; preds = %if.end
  %2 = tail call i32 @llvm.va_arg_pack()
  %call5 = tail call i32 (i8*, i32, ...) @myopenva(i8* %path, i32 %oflag, i32 %2)
  br label %return

return:                                           ; preds = %if.end4, %if.then2
  %retval.0 = phi i32 [ %call3, %if.then2 ], [ 0, %if.end4 ]
  ret i32 %retval.0
}

; Function Attrs: alwaysinline nounwind uwtable
define available_externally dso_local i32 @myopener(i8* %path, i32 %oflag, ...) local_unnamed_addr #5 {
entry:
  %0 = tail call i32 @llvm.va_arg_pack()
  %call = tail call i32 (i8*, i32, ...) @myopenva(i8* %path, i32 %oflag, i32 %0)
  ret i32 0
}

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @myopen2(i8* nocapture readonly %path, i32 %oflag) local_unnamed_addr #0 {
entry:
  %0 = load i8, i8* @expected_char, align 1
  %inc = add i8 %0, 1
  store i8 %inc, i8* @expected_char, align 1
  %1 = load i8, i8* %path, align 1
  %conv1 = sext i8 %1 to i32
  %cmp = icmp eq i8 %0, %1
  br i1 %cmp, label %lor.lhs.false, label %if.then

lor.lhs.false:                                    ; preds = %entry
  %arrayidx3 = getelementptr inbounds i8, i8* %path, i64 1
  %2 = load i8, i8* %arrayidx3, align 1
  %cmp5 = icmp eq i8 %2, 0
  br i1 %cmp5, label %if.end, label %if.then

if.then:                                          ; preds = %lor.lhs.false, %entry
  tail call void @abort() #5
  unreachable

if.end:                                           ; preds = %lor.lhs.false
  switch i32 %conv1, label %sw.default [
    i32 102, label %sw.bb
    i32 103, label %sw.bb13
  ]

sw.bb:                                            ; preds = %if.end
  %cmp9 = icmp eq i32 %oflag, 2
  br i1 %cmp9, label %return, label %if.then11

if.then11:                                        ; preds = %sw.bb
  tail call void @abort() #5
  unreachable

sw.bb13:                                          ; preds = %if.end
  %cmp14 = icmp eq i32 %oflag, 67
  br i1 %cmp14, label %return, label %if.then16

if.then16:                                        ; preds = %sw.bb13
  tail call void @abort() #5
  unreachable

sw.default:                                       ; preds = %if.end
  tail call void @abort() #5
  unreachable

return:                                           ; preds = %sw.bb, %sw.bb13
  %retval.0 = phi i32 [ -6, %sw.bb13 ], [ 0, %sw.bb ]
  ret i32 %retval.0
}

define dso_local i32 @myopenva(i8* nocapture readonly %path, i32 %oflag, ...) local_unnamed_addr #0 {
entry:
  %ap = alloca [1 x %struct.__va_list_tag], align 16
  %0 = bitcast [1 x %struct.__va_list_tag]* %ap to i8*
  call void @llvm.lifetime.start.p0i8(i64 24, i8* nonnull %0) #3
  %and = and i32 %oflag, 64
  %cmp = icmp eq i32 %and, 0
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %1 = bitcast [1 x %struct.__va_list_tag]* %ap to i8*
  call void @llvm.va_start(i8* nonnull %1)
  %gp_offset_p56 = bitcast [1 x %struct.__va_list_tag]* %ap to i32*
  %gp_offset = load i32, i32* %gp_offset_p56, align 16
  %2 = sext i32 %gp_offset to i64
  %fits_in_gp = icmp ult i32 %gp_offset, 41
  br i1 %fits_in_gp, label %vaarg.in_reg, label %vaarg.in_mem

vaarg.in_reg:                                     ; preds = %if.then
  %3 = trunc i64 %2 to i32
  %4 = bitcast [1 x %struct.__va_list_tag]* %ap to i32*
  %5 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %ap, i64 0, i64 0, i32 3
  %reg_save_area = load i8*, i8** %5, align 16
  %6 = getelementptr i8, i8* %reg_save_area, i64 %2
  %7 = add i32 %3, 8
  store i32 %7, i32* %4, align 16
  br label %vaarg.end

vaarg.in_mem:                                     ; preds = %if.then
  %overflow_arg_area_p = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %ap, i64 0, i64 0, i32 2
  %overflow_arg_area = load i8*, i8** %overflow_arg_area_p, align 8
  %overflow_arg_area.next = getelementptr i8, i8* %overflow_arg_area, i64 8
  store i8* %overflow_arg_area.next, i8** %overflow_arg_area_p, align 8
  br label %vaarg.end

vaarg.end:                                        ; preds = %vaarg.in_mem, %vaarg.in_reg
  %vaarg.addr.in = phi i8* [ %6, %vaarg.in_reg ], [ %overflow_arg_area, %vaarg.in_mem ]
  %8 = bitcast [1 x %struct.__va_list_tag]* %ap to i8*
  %vaarg.addr = bitcast i8* %vaarg.addr.in to i32*
  %9 = load i32, i32* %vaarg.addr, align 4
  call void @llvm.va_end(i8* nonnull %8)
  br label %if.end

if.end:                                           ; preds = %entry, %vaarg.end
  %mode.0 = phi i32 [ %9, %vaarg.end ], [ 0, %entry ]
  %10 = load i8, i8* @expected_char, align 1
  %inc = add i8 %10, 1
  store i8 %inc, i8* @expected_char, align 1
  %11 = load i8, i8* %path, align 1
  %conv5 = sext i8 %11 to i32
  %cmp6 = icmp eq i8 %10, %11
  br i1 %cmp6, label %lor.lhs.false, label %if.then12

lor.lhs.false:                                    ; preds = %if.end
  %arrayidx8 = getelementptr inbounds i8, i8* %path, i64 1
  %12 = load i8, i8* %arrayidx8, align 1
  %cmp10 = icmp eq i8 %12, 0
  br i1 %cmp10, label %if.end13, label %if.then12

if.then12:                                        ; preds = %lor.lhs.false, %if.end
  call void @abort() #5
  unreachable

if.end13:                                         ; preds = %lor.lhs.false
  switch i32 %conv5, label %sw.default [
    i32 97, label %sw.bb
    i32 98, label %sw.bb23
    i32 99, label %sw.bb28
    i32 100, label %sw.bb33
    i32 101, label %sw.bb41
  ]

sw.bb:                                            ; preds = %if.end13
  %cmp16 = icmp ne i32 %oflag, 67
  %cmp19 = icmp ne i32 %mode.0, 420
  %or.cond = or i1 %cmp16, %cmp19
  br i1 %or.cond, label %if.then21, label %sw.epilog

if.then21:                                        ; preds = %sw.bb
  call void @abort() #5
  unreachable

sw.bb23:                                          ; preds = %if.end13
  %cmp24 = icmp eq i32 %oflag, 3
  br i1 %cmp24, label %sw.epilog, label %if.then26

if.then26:                                        ; preds = %sw.bb23
  call void @abort() #5
  unreachable

sw.bb28:                                          ; preds = %if.end13
  %cmp29 = icmp eq i32 %oflag, 2
  br i1 %cmp29, label %sw.epilog, label %if.then31

if.then31:                                        ; preds = %sw.bb28
  call void @abort() #5
  unreachable

sw.bb33:                                          ; preds = %if.end13
  %cmp34 = icmp ne i32 %oflag, 67
  %cmp37 = icmp ne i32 %mode.0, 384
  %or.cond46 = or i1 %cmp34, %cmp37
  br i1 %or.cond46, label %if.then39, label %sw.epilog

if.then39:                                        ; preds = %sw.bb33
  call void @abort() #5
  unreachable

sw.bb41:                                          ; preds = %if.end13
  %cmp42 = icmp eq i32 %oflag, 3
  br i1 %cmp42, label %sw.epilog, label %if.then44

if.then44:                                        ; preds = %sw.bb41
  call void @abort() #5
  unreachable

sw.default:                                       ; preds = %if.end13
  call void @abort() #5
  unreachable

sw.epilog:                                        ; preds = %sw.bb23, %sw.bb28, %sw.bb41, %sw.bb33, %sw.bb
  %13 = bitcast [1 x %struct.__va_list_tag]* %ap to i8*
  call void @llvm.lifetime.end.p0i8(i64 24, i8* nonnull %13) #3
  ret i32 0
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #4 {
entry:
  %call = call i32 (i8*, i32, ...) @myopener(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str, i64 0, i64 0), i32 35, i32 448)
  store i8 97, i8* @expected_char, align 1
  %call1 = call i32 (i8*, i32, ...) @myopen(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str, i64 0, i64 0), i32 67, i32 420)
  %tobool = icmp eq i32 %call1, 0
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @abort() #6
  unreachable

if.end:                                           ; preds = %entry
  ret i32 %call
}

attributes #0 = { noinline nounwind uwtable }
attributes #1 = { noreturn }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind }
attributes #4 = { nounwind uwtable }
attributes #5 = { alwaysinline nounwind uwtable }
attributes #6 = { noreturn }

; CHECK-LABEL: DEAD STATIC FUNC: myopen
; CHECK-LABEL: DEAD STATIC FUNC: myopener
; CHECK-LABEL: COMPILE FUNC: myopen2
; CHECK: EXTERN: abort
; CHECK: EXTERN: abort
; CHECK: EXTERN: abort
; CHECK: EXTERN: abort

; CHECK-LABEL: COMPILE FUNC: myopenva
; CHECK: llvm.va_start{{.*}}Callee is intrinsic
; CHECK: llvm.va_end{{.*}}Callee is intrinsic
; CHECK: EXTERN: abort
; CHECK: EXTERN: abort
; CHECK: EXTERN: abort
; CHECK: EXTERN: abort
; CHECK: EXTERN: abort
; CHECK: EXTERN: abort
; CHECK: EXTERN: abort

; CHECK-LABEL: COMPILE FUNC: main
; CHECK: INLINE: myopener{{.*}}Callee is always inline
; CHECK: DELETE: llvm.va_arg_pack
; CHECK: myopenva{{.*}}Callee has noinline attribute
; CHECK: INLINE: myopen{{.*}}Callee is always inline
; CHECK: DELETE: llvm.va_arg_pack_len
; CHECK: EXTERN: warn_open_too_many_arguments
; CHECK: myopen2{{.*}}Callee has noinline attribute
; CHECK: DELETE: llvm.va_arg_pack
; CHECK: myopenva{{.*}}Callee has noinline attribute
; CHECK: EXTERN: abort

