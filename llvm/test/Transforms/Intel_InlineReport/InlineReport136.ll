; RUN: opt -passes='function(instcombine),print<inline-report>' -disable-output -inline-report=0xe807 < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='function(instcombine)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; Check that fwrite is converted to fputc.

; CHECK-LABEL: COMPILE FUNC: i_putchar
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, %struct._IO_codecvt*, %struct._IO_wide_data*, %struct._IO_FILE*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type opaque
%struct._IO_codecvt = type opaque
%struct._IO_wide_data = type opaque

@stdout = external dso_local local_unnamed_addr global %struct._IO_FILE*, align 8

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

declare dso_local noundef i64 @fwrite(i8* nocapture noundef, i64 noundef, i64 noundef, %struct._IO_FILE* nocapture noundef)

define internal i32 @i_putchar(i8 noundef signext %0) #15 {
  %2 = alloca i8, align 1
  %3 = alloca i8, align 1
  store i8 %0, i8* %2, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %3) #26
  store i8 13, i8* %3, align 1
  %4 = icmp eq i8 %0, 10
  br i1 %4, label %5, label %8

5:                                                ; preds = %1
  %6 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %7 = call i64 @fwrite(i8* noundef %3, i64 noundef 1, i64 noundef 1, %struct._IO_FILE* noundef %6)
  br label %8

8:                                                ; preds = %5, %1
  %9 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %10 = load i8, i8* %2, align 1
  %11 = call i64 @fwrite(i8* noundef nonnull %2, i64 noundef 1, i64 noundef 1, %struct._IO_FILE* noundef %9)
  %12 = load i8, i8* %2, align 1
  %13 = zext i8 %12 to i32
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %3) #26
  ret i32 %13
}

