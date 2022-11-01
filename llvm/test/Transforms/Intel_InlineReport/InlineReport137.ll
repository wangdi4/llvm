; RUN: opt -passes='function(mem2reg),print<inline-report>' -disable-output -inline-report=0xea07 < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup' -inline-report=0xea86 < %s -S | opt -passes='function(mem2reg)' -inline-report=0xea86 -S | opt -passes='inlinereportemitter' -inline-report=0xea86 -S 2>&1 | FileCheck %s

; Check that llvm.lifetime.start.p0i8 is deleted as dead code.

; CHECK-LABEL: COMPILE FUNC: i_putchar
; CHECK: DELETE: llvm.lifetime.start.p0i8 {{.*}}Dead code
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, %struct._IO_codecvt*, %struct._IO_wide_data*, %struct._IO_FILE*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type opaque
%struct._IO_codecvt = type opaque
%struct._IO_wide_data = type opaque

@stdout = external dso_local local_unnamed_addr global %struct._IO_FILE*, align 8

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

declare i32 @fputc(i32, %struct._IO_FILE*)

define internal i32 @i_putchar(i8 noundef signext %0) #15 {
  %2 = alloca i8, align 1
  %3 = alloca i8, align 1
  store i8 %0, i8* %2, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %3)
  store i8 13, i8* %3, align 1
  %4 = icmp eq i8 %0, 10
  br i1 %4, label %5, label %10

5:                                                ; preds = %1
  %6 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %7 = load i8, i8* %3, align 1
  %8 = sext i8 %7 to i32
  %9 = call i32 @fputc(i32 %8, %struct._IO_FILE* %6)
  br label %10

10:                                               ; preds = %5, %1
  %11 = load %struct._IO_FILE*, %struct._IO_FILE** @stdout, align 8
  %12 = load i8, i8* %2, align 1
  %13 = sext i8 %12 to i32
  %14 = call i32 @fputc(i32 %13, %struct._IO_FILE* %11)
  %15 = zext i8 %12 to i32
  ret i32 %15
}

