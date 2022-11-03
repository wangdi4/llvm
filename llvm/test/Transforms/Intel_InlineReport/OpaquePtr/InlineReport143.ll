; RUN: opt -passes='function(instcombine),print<inline-report>' -disable-output -inline-report=0xea07 < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup' -inline-report=0xea86 < %s -S | opt -passes='function(instcombine)' -inline-report=0xea86 -S | opt -passes='inlinereportemitter' -inline-report=0xea86 -S 2>&1 | FileCheck %s

; Check that calls to llvm.lifetime.start are deleted as dead code.

; CHECK-LABEL: COMPILE FUNC: i_send_buf_as_file
; CHECK: DELETE: llvm.lifetime.start.p0i8 {{.*}}Dead code
; CHECK: DELETE: llvm.lifetime.start.p0i8 {{.*}}Dead code

@uu_std = internal unnamed_addr constant [64 x i8] c"`!\22#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_", align 16

declare void @llvm.lifetime.start.p0i8(i64 immarg %0, i8* nocapture %1) #1
declare void @llvm.lifetime.end.p0i8(i64 immarg %0, i8* nocapture %1) #1

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, %struct._IO_codecvt*, %struct._IO_wide_data*, %struct._IO_FILE*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type opaque
%struct._IO_codecvt = type opaque
%struct._IO_wide_data = type opaque

; Function Attrs: nounwind uwtable
define internal i32 @i_send_buf_as_file(i8* noundef %0, i64 noundef %1, i8* noundef %2) #0 {
  %4 = alloca i8, align 1
  %5 = alloca i8, align 1
  %6 = alloca i8, align 1
  %7 = alloca i8, align 1
  %8 = alloca i8, align 1
  %9 = alloca i8, align 1
  %10 = alloca i8, align 1
  %11 = alloca i8, align 1
  %12 = alloca i8, align 1
  %13 = alloca [80 x i8], align 16
  %14 = trunc i64 %1 to i32
  %15 = getelementptr inbounds [80 x i8], [80 x i8]* %13, i64 0, i64 0
  call void @llvm.lifetime.start.p0i8(i64 80, i8* nonnull %15) #27
  %16 = icmp sgt i32 %14, 0
  %17 = icmp ne i8* %0, null
  %18 = and i1 %16, %17
  br i1 %18, label %20, label %19

19:                                               ; preds = %3
  br label %20

20:                                               ; preds = %3
  %21 = and i64 %1, 4294967295
  br label %22

22:
  %23 = alloca i1, align 1                        ; preds = %20
  br label %24

24:                                               ; preds = %28, %22
  %25 = phi i64 [ 0, %22 ], [ %32, %28 ]
  %26 = add nuw nsw i64 %25, 0
  %27 = icmp slt i64 %26, %21
  br i1 %27, label %28, label %34

28:                                               ; preds = %24
  %29 = getelementptr inbounds i8, i8* %0, i64 %26
  %30 = load i8, i8* %29, align 1
  %31 = getelementptr inbounds [80 x i8], [80 x i8]* %13, i64 0, i64 %25
  store i8 %30, i8* %31, align 1
  %32 = add nuw nsw i64 %25, 1
  %33 = icmp eq i64 %32, 45
  br i1 %33, label %37, label %24

34:                                               ; preds = %24
  %35 = trunc i64 %25 to i32
  %36 = icmp eq i32 %35, 0
  br label %37

37:                                               ; preds = %34, %28
  %38 = phi i32 [ %35, %34 ], [ 45, %28 ]
  %39 = add nuw i64 0, 45
  %40 = and i32 %38, 63
  %41 = zext i32 %40 to i64
  %42 = getelementptr inbounds [64 x i8], [64 x i8]* @uu_std, i64 0, i64 %41
  %43 = load i8, i8* %42, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %11)
  store i8 %43, i8* %11, align 1
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %12) #27
  ret i32 0
}
