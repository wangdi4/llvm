; RUN: opt -opaque-pointers -passes='function(sroa),print<inline-report>' -disable-output -inline-report=0xea07 < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xea86 < %s -S | opt -passes='function(sroa)' -inline-report=0xea86 -S | opt -passes='inlinereportemitter' -inline-report=0xea86 -S 2>&1 | FileCheck %s

; Check that calls to llvm.lifetime.start and llvm.lifetime.end are deleted
; as dead code.

; CHECK-LABEL: COMPILE FUNC: i_putchar
; CHECK-DAG: DELETE: llvm.lifetime.start.p0 {{.*}}Dead code
; CHECK-DAG: DELETE: llvm.lifetime.start.p0 {{.*}}Dead code
; CHECK-DAG: DELETE: llvm.lifetime.start.p0 {{.*}}Dead code
; CHECK-DAG: DELETE: llvm.lifetime.start.p0 {{.*}}Dead code
; CHECK-DAG: DELETE: llvm.lifetime.end.p0 {{.*}}Dead code
; CHECK-DAG: DELETE: llvm.lifetime.end.p0 {{.*}}Dead code
; CHECK-DAG: DELETE: llvm.lifetime.end.p0 {{.*}}Dead code
; CHECK-DAG: DELETE: llvm.lifetime.end.p0 {{.*}}Dead code
; CHECK-DAG: EXTERN: fputc
; CHECK-DAG: EXTERN: fputc

@stdout = external dso_local local_unnamed_addr global ptr, align 8

declare i32 @fputc(i32, ptr)

define internal i32 @i_putchar(i8 noundef signext %arg) {
bb:
  %i = alloca i8, align 1
  %i1 = alloca i8, align 1
  store i8 %arg, ptr %i, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr %i1)
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i1)
  store i8 13, ptr %i1, align 1
  store i8 13, ptr %i1, align 1
  %i2 = icmp eq i8 %arg, 10
  br i1 %i2, label %bb3, label %bb8

bb3:                                              ; preds = %bb
  %i4 = load ptr, ptr @stdout, align 8
  %i5 = load i8, ptr %i1, align 1
  %i6 = sext i8 %i5 to i32
  %i7 = call i32 @fputc(i32 %i6, ptr %i4)
  br label %bb8

bb8:                                              ; preds = %bb3, %bb
  %i9 = load ptr, ptr @stdout, align 8
  %i10 = load i8, ptr %i, align 1
  %i11 = sext i8 %i10 to i32
  %i12 = call i32 @fputc(i32 %i11, ptr %i9)
  %i13 = zext i8 %i10 to i32
  call void @llvm.lifetime.end.p0(i64 1, ptr %i1)
  call void @llvm.lifetime.end.p0(i64 1, ptr nonnull %i1)
  ret i32 %i13
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

attributes #0 = { argmemonly nocallback nofree nosync nounwind willreturn }
