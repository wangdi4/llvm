; REQUIRES: asserts
; RUN: opt < %s -opaque-pointers -dope-vector-local-const-prop=false -disable-output -passes=dopevectorconstprop -debug-only=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; Check that dope vector constants get propagated for uplevels #0 and #1 in
; new_solver_IP_specific_, a contained subroutine for new_solver_.

; CHECK: DOPE VECTOR CONSTANT PROPAGATION: BEGIN
; CHECK: DV FOUND: ARG #0 new_solver_ 1 x <UNKNOWN ELEMENT TYPE>
; CHECK: VALID
; CHECK: LB[0] = 1
; CHECK: ST[0] = 4
; CHECK: EX[0] = 3
; CHECK: REPLACING 1 LOAD WITH 4
; CHECK: TESTING UPLEVEL #0 FOR new_solver_IP_specific_
; CHECK: REPLACING 1 LOAD WITH 3
; CHECK: REPLACING 1 LOAD WITH 4
; CHECK: DV FOUND: ARG #1 new_solver_ 2 x <UNKNOWN ELEMENT TYPE>
; CHECK: VALID
; CHECK: LB[0] = 1
; CHECK: ST[0] = 4
; CHECK: EX[0] = 3
; CHECK: LB[1] = 1
; CHECK: ST[1] = 12
; CHECK: EX[1] = 3
; CHECK: REPLACING 1 LOAD WITH 3
; CHECK: REPLACING 1 LOAD WITH 4
; CHECK: REPLACING 1 LOAD WITH 12
; CHECK: TESTING UPLEVEL #1 FOR new_solver_IP_specific_
; CHECK: REPLACING 1 LOAD WITH 3
; CHECK: REPLACING 1 LOAD WITH 3
; CHECK: REPLACING 1 LOAD WITH 4
; CHECK: REPLACING 1 LOAD WITH 12
; CHECK: DOPE VECTOR CONSTANT PROPAGATION: END

%uplevel_type = type { ptr, ptr }

@anon.9f612ed7d31cf3fc2b70611956e5ab37.0 = internal unnamed_addr constant i32 2
@"main_$PART" = internal global [3 x i32] zeroinitializer, align 16
@"main_$BLOCK" = internal global [3 x [3 x i32]] zeroinitializer, align 16

declare dso_local i32 @for_set_reentrancy(ptr) local_unnamed_addr

define internal void @new_solver_(ptr noalias %arg, ptr noalias %arg1) #0 {
bb:
  %i = alloca %uplevel_type, align 8
  %i2 = getelementptr inbounds %uplevel_type, ptr %i, i64 0, i32 0
  store ptr %arg, ptr %i2, align 8
  %i3 = getelementptr inbounds %uplevel_type, ptr %i, i64 0, i32 1
  store ptr %arg1, ptr %i3, align 8
  %i4 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %arg1, i64 0, i32 0
  %i5 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %arg1, i64 0, i32 6, i64 0
  %i6 = getelementptr inbounds { i64, i64, i64 }, ptr %i5, i64 0, i32 0
  %i7 = getelementptr inbounds { i64, i64, i64 }, ptr %i5, i64 0, i32 1
  %i8 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %arg, i64 0, i32 0
  %i9 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %arg, i64 0, i32 6, i64 0
  %i10 = getelementptr inbounds { i64, i64, i64 }, ptr %i9, i64 0, i32 1
  call void @new_solver_IP_specific_(ptr nonnull %i)
  %i11 = load ptr, ptr %i4, align 8
  %i12 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i7, i32 0)
  %i13 = load i64, ptr %i12, align 8
  %i14 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i7, i32 1)
  %i15 = load i64, ptr %i14, align 8
  %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i6, i32 1)
  %i17 = load i64, ptr %i16, align 8
  %i18 = load ptr, ptr %i8, align 8
  %i19 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i10, i32 0)
  %i20 = load i64, ptr %i19, align 8
  %i21 = icmp slt i64 %i17, 1
  br i1 %i21, label %bb30, label %bb22

bb22:                                             ; preds = %bb22, %bb
  %i23 = phi i64 [ %i28, %bb22 ], [ 1, %bb ]
  %i24 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i20, ptr elementtype(i32) %i18, i64 %i23)
  %i25 = load i32, ptr %i24, align 4
  %i26 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i15, ptr elementtype(i32) %i11, i64 %i23)
  %i27 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i13, ptr elementtype(i32) %i26, i64 1)
  store i32 %i25, ptr %i27, align 4
  %i28 = add nuw i64 %i23, 1
  %i29 = icmp eq i64 %i23, %i17
  br i1 %i29, label %bb30, label %bb22

bb30:                                             ; preds = %bb22, %bb
  ret void
}

define internal void @new_solver_IP_specific_(ptr nest noalias nocapture readonly %arg) #0 {
bb:
  %i = getelementptr inbounds %uplevel_type, ptr %arg, i64 0, i32 0
  %i1 = load ptr, ptr %i, align 8
  %i2 = getelementptr inbounds %uplevel_type, ptr %arg, i64 0, i32 1
  %i3 = load ptr, ptr %i2, align 8
  %i4 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %i1, i64 0, i32 0
  %i5 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %i1, i64 0, i32 6, i64 0
  %i6 = getelementptr inbounds { i64, i64, i64 }, ptr %i5, i64 0, i32 0
  %i7 = getelementptr inbounds { i64, i64, i64 }, ptr %i5, i64 0, i32 1
  %i8 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i3, i64 0, i32 0
  %i9 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i3, i64 0, i32 6, i64 0
  %i10 = getelementptr inbounds { i64, i64, i64 }, ptr %i9, i64 0, i32 0
  %i11 = getelementptr inbounds { i64, i64, i64 }, ptr %i9, i64 0, i32 1
  %i12 = load ptr, ptr %i4, align 8
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i7, i32 0)
  %i14 = load i64, ptr %i13, align 8
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i6, i32 0)
  %i16 = load i64, ptr %i15, align 8
  %i17 = icmp slt i64 %i16, 1
  br i1 %i17, label %bb34, label %bb18

bb18:                                             ; preds = %bb18, %bb
  %i19 = phi i64 [ %i21, %bb18 ], [ 1, %bb ]
  %i20 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i14, ptr elementtype(i32) %i12, i64 %i19)
  store i32 0, ptr %i20, align 4
  %i21 = add nuw i64 %i19, 1
  %i22 = icmp eq i64 %i19, %i16
  br i1 %i22, label %bb34, label %bb18

bb23:                                             ; preds = %bb31, %bb23
  %i24 = phi i64 [ 1, %bb31 ], [ %i26, %bb23 ]
  %i25 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i37, ptr elementtype(i32) %i33, i64 %i24)
  store i32 1, ptr %i25, align 4
  %i26 = add nuw i64 %i24, 1
  %i27 = icmp eq i64 %i24, %i39
  br i1 %i27, label %bb28, label %bb23

bb28:                                             ; preds = %bb23
  %i29 = add nuw i64 %i32, 1
  %i30 = icmp eq i64 %i32, %i43
  br i1 %i30, label %bb47, label %bb31

bb31:                                             ; preds = %bb34, %bb28
  %i32 = phi i64 [ %i29, %bb28 ], [ 1, %bb34 ]
  %i33 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i41, ptr elementtype(i32) %i35, i64 %i32)
  br label %bb23

bb34:                                             ; preds = %bb18, %bb
  %i35 = load ptr, ptr %i8, align 8
  %i36 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i11, i32 0)
  %i37 = load i64, ptr %i36, align 8
  %i38 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i10, i32 0)
  %i39 = load i64, ptr %i38, align 8
  %i40 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i11, i32 1)
  %i41 = load i64, ptr %i40, align 8
  %i42 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i10, i32 1)
  %i43 = load i64, ptr %i42, align 8
  %i44 = icmp slt i64 %i43, 1
  %i45 = icmp slt i64 %i39, 1
  %i46 = or i1 %i44, %i45
  br i1 %i46, label %bb47, label %bb31

bb47:                                             ; preds = %bb34, %bb28
  ret void
}

define dso_local void @MAIN__() #0 {
bb:
  %i = alloca { i32 }, align 8
  %i1 = alloca [4 x i8], align 1
  %i2 = alloca { i32 }, align 8
  %i3 = alloca [4 x i8], align 1
  %i4 = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %i5 = alloca { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, align 8
  %i6 = alloca [8 x i64], align 16
  %i7 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 0
  %i8 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 1
  %i9 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 2
  %i10 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 3
  %i11 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 4
  %i12 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %i5, i64 0, i32 6, i64 0
  %i13 = getelementptr inbounds { i64, i64, i64 }, ptr %i12, i64 0, i32 0
  %i14 = getelementptr inbounds { i64, i64, i64 }, ptr %i12, i64 0, i32 1
  %i15 = getelementptr inbounds { i64, i64, i64 }, ptr %i12, i64 0, i32 2
  %i16 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 0
  %i17 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 1
  %i18 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 2
  %i19 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 3
  %i20 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 4
  %i21 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %i4, i64 0, i32 6, i64 0
  %i22 = getelementptr inbounds { i64, i64, i64 }, ptr %i21, i64 0, i32 0
  %i23 = getelementptr inbounds { i64, i64, i64 }, ptr %i21, i64 0, i32 1
  %i24 = getelementptr inbounds { i64, i64, i64 }, ptr %i21, i64 0, i32 2
  %i25 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.9f612ed7d31cf3fc2b70611956e5ab37.0)
  store i64 4, ptr %i8, align 8
  store i64 1, ptr %i11, align 8
  store i64 0, ptr %i9, align 8
  %i26 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i14, i32 0)
  store i64 4, ptr %i26, align 8
  %i27 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i15, i32 0)
  store i64 1, ptr %i27, align 8
  %i28 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i13, i32 0)
  store i64 3, ptr %i28, align 8
  store ptr @"main_$PART", ptr %i7, align 8
  store i64 1, ptr %i10, align 8
  store i64 4, ptr %i17, align 8
  store i64 2, ptr %i20, align 8
  store i64 0, ptr %i18, align 8
  %i29 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i23, i32 0)
  store i64 4, ptr %i29, align 8
  %i30 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i24, i32 0)
  store i64 1, ptr %i30, align 8
  %i31 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i22, i32 0)
  store i64 3, ptr %i31, align 8
  %i32 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i23, i32 1)
  store i64 12, ptr %i32, align 8
  %i33 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i24, i32 1)
  store i64 1, ptr %i33, align 8
  %i34 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i22, i32 1)
  store i64 3, ptr %i34, align 8
  store ptr @"main_$BLOCK", ptr %i16, align 8
  store i64 1, ptr %i19, align 8
  call void @new_solver_(ptr nonnull %i5, ptr nonnull %i4)
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #1

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { "intel-lang"="fortran" }
attributes #1 = { nounwind readnone speculatable }
