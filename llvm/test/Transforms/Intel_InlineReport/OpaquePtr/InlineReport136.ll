; RUN: opt -opaque-pointers -passes='function(instcombine),print<inline-report>' -disable-output -inline-report=0xe807 < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='function(instcombine)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; Check that fwrite is converted to fputc.

; CHECK-LABEL: COMPILE FUNC: i_putchar
; CHECK: EXTERN: fputc
; CHECK: EXTERN: fputc

@stdout = external dso_local local_unnamed_addr global ptr, align 8

declare dso_local noundef i64 @fwrite(ptr nocapture noundef, i64 noundef, i64 noundef, ptr nocapture noundef)

define internal i32 @i_putchar(i8 noundef signext %arg) {
bb:
  %i = alloca i8, align 1
  %i1 = alloca i8, align 1
  store i8 %arg, ptr %i, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i1)
  store i8 13, ptr %i1, align 1
  %i2 = icmp eq i8 %arg, 10
  br i1 %i2, label %bb3, label %bb6

bb3:                                              ; preds = %bb
  %i4 = load ptr, ptr @stdout, align 8
  %i5 = call i64 @fwrite(ptr noundef %i1, i64 noundef 1, i64 noundef 1, ptr noundef %i4)
  br label %bb6

bb6:                                              ; preds = %bb3, %bb
  %i7 = load ptr, ptr @stdout, align 8
  %i8 = load i8, ptr %i, align 1
  %i9 = call i64 @fwrite(ptr noundef nonnull %i, i64 noundef 1, i64 noundef 1, ptr noundef %i7)
  %i10 = load i8, ptr %i, align 1
  %i11 = zext i8 %i10 to i32
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i1)
  ret i32 %i11
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

attributes #0 = { argmemonly nocallback nofree nosync nounwind willreturn }
