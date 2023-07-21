; RUN: opt -passes='function(instcombine),print<inline-report>' -disable-output -inline-report=0xea07 < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup,function(instcombine),inlinereportemitter' -inline-report=0xea86 -S < %s 2>&1 | FileCheck %s

; Check that calls to llvm.lifetime.start are deleted as dead code.

; CHECK-LABEL: COMPILE FUNC: i_send_buf_as_file
; CHECK: DELETE: llvm.lifetime.start.p0 {{.*}}Dead code
; CHECK: DELETE: llvm.lifetime.start.p0 {{.*}}Dead code

@uu_std = internal unnamed_addr constant [64 x i8] c"`!\22#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_", align 16

define internal i32 @i_send_buf_as_file(ptr noundef %arg, i64 noundef %arg1, ptr noundef %arg2) {
bb:
  %i = alloca i8, align 1
  %i3 = alloca i8, align 1
  %i4 = alloca i8, align 1
  %i5 = alloca i8, align 1
  %i6 = alloca i8, align 1
  %i7 = alloca i8, align 1
  %i8 = alloca i8, align 1
  %i9 = alloca i8, align 1
  %i10 = alloca i8, align 1
  %i11 = alloca [80 x i8], align 16
  %i12 = trunc i64 %arg1 to i32
  %i13 = getelementptr inbounds [80 x i8], ptr %i11, i64 0, i64 0
  call void @llvm.lifetime.start.p0(i64 80, ptr nonnull %i13)
  %i14 = icmp sgt i32 %i12, 0
  %i15 = icmp ne ptr %arg, null
  %i16 = and i1 %i14, %i15
  br i1 %i16, label %bb18, label %bb17

bb17:                                             ; preds = %bb
  br label %bb18

bb18:                                             ; preds = %bb17, %bb
  %i19 = and i64 %arg1, 4294967295
  br label %bb20

bb20:                                             ; preds = %bb18
  %i21 = alloca i1, align 1
  br label %bb22

bb22:                                             ; preds = %bb26, %bb20
  %i23 = phi i64 [ 0, %bb20 ], [ %i30, %bb26 ]
  %i24 = add nuw nsw i64 %i23, 0
  %i25 = icmp slt i64 %i24, %i19
  br i1 %i25, label %bb26, label %bb32

bb26:                                             ; preds = %bb22
  %i27 = getelementptr inbounds i8, ptr %arg, i64 %i24
  %i28 = load i8, ptr %i27, align 1
  %i29 = getelementptr inbounds [80 x i8], ptr %i11, i64 0, i64 %i23
  store i8 %i28, ptr %i29, align 1
  %i30 = add nuw nsw i64 %i23, 1
  %i31 = icmp eq i64 %i30, 45
  br i1 %i31, label %bb35, label %bb22

bb32:                                             ; preds = %bb22
  %i33 = trunc i64 %i23 to i32
  %i34 = icmp eq i32 %i33, 0
  br label %bb35

bb35:                                             ; preds = %bb32, %bb26
  %i36 = phi i32 [ %i33, %bb32 ], [ 45, %bb26 ]
  %i37 = add nuw i64 0, 45
  %i38 = and i32 %i36, 63
  %i39 = zext i32 %i38 to i64
  %i40 = getelementptr inbounds [64 x i8], ptr @uu_std, i64 0, i64 %i39
  %i41 = load i8, ptr %i40, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i9)
  store i8 %i41, ptr %i9, align 1
  call void @llvm.lifetime.start.p0(i64 1, ptr nonnull %i10)
  ret i32 0
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

attributes #0 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
