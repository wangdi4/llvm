; REQUIRES: asserts
; RUN: opt -opaque-pointers < %s -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s

; Check that it is profitable to transpose main_$MYA and main_$MYB, because their
; rank 0 index is indirectly subscripted.

; CHECK: Transpose candidate: main_$MYA
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: true

; CHECK: Transpose candidate: main_$MYB
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: true

; Check that the array through which the indirect subscripting is occurring is
; neither a candidate for transposing nor profitable.

; CHECK: Transpose candidate: main_$MYK
; CHECK: IsValid{{ *}}: false
; CHECK: IsProfitable{{ *}}: false

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@anon.027ecfc0ab928ef3b2e153cdd55f152e.0 = internal unnamed_addr constant i32 2
@"main_$MYA" = internal global [200 x [100 x float]] zeroinitializer, align 16
@"main_$MYB" = internal global [200 x [100 x float]] zeroinitializer, align 16
@"main_$MYK" = internal global [200 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 {
bb:
  %i = alloca %"QNCA_a0$float*$rank2$", align 8
  %i1 = alloca %"QNCA_a0$float*$rank2$", align 8
  %i2 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.027ecfc0ab928ef3b2e153cdd55f152e.0) #5
  %i3 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 3
  %i4 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 1
  store i64 4, ptr %i4, align 8
  %i5 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 4
  store i64 2, ptr %i5, align 8
  %i6 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 2
  store i64 0, ptr %i6, align 8
  %i7 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 6, i64 0
  %i8 = getelementptr inbounds { i64, i64, i64 }, ptr %i7, i64 0, i32 1
  %i9 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i8, i32 0)
  store i64 4, ptr %i9, align 1
  %i10 = getelementptr inbounds { i64, i64, i64 }, ptr %i7, i64 0, i32 2
  %i11 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i10, i32 0)
  store i64 1, ptr %i11, align 1
  %i12 = getelementptr inbounds { i64, i64, i64 }, ptr %i7, i64 0, i32 0
  %i13 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i12, i32 0)
  store i64 100, ptr %i13, align 1
  %i14 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i8, i32 1)
  store i64 400, ptr %i14, align 1
  %i15 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i10, i32 1)
  store i64 1, ptr %i15, align 1
  %i16 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i12, i32 1)
  store i64 200, ptr %i16, align 1
  %i17 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i, i64 0, i32 0
  store ptr @"main_$MYA", ptr %i17, align 8
  store i64 1, ptr %i3, align 8
  %i18 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i1, i64 0, i32 3
  %i19 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i1, i64 0, i32 1
  store i64 4, ptr %i19, align 8
  %i20 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i1, i64 0, i32 4
  store i64 2, ptr %i20, align 8
  %i21 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i1, i64 0, i32 2
  store i64 0, ptr %i21, align 8
  %i22 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i1, i64 0, i32 6, i64 0
  %i23 = getelementptr inbounds { i64, i64, i64 }, ptr %i22, i64 0, i32 1
  %i24 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i23, i32 0)
  store i64 4, ptr %i24, align 1
  %i25 = getelementptr inbounds { i64, i64, i64 }, ptr %i22, i64 0, i32 2
  %i26 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i25, i32 0)
  store i64 1, ptr %i26, align 1
  %i27 = getelementptr inbounds { i64, i64, i64 }, ptr %i22, i64 0, i32 0
  %i28 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i27, i32 0)
  store i64 100, ptr %i28, align 1
  %i29 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i23, i32 1)
  store i64 400, ptr %i29, align 1
  %i30 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i25, i32 1)
  store i64 1, ptr %i30, align 1
  %i31 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i27, i32 1)
  store i64 200, ptr %i31, align 1
  %i32 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i1, i64 0, i32 0
  store ptr @"main_$MYB", ptr %i32, align 8
  store i64 1, ptr %i18, align 8
  call void @foo_(ptr nonnull %i, ptr nonnull %i1, ptr @"main_$MYK")
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nofree noinline nosync nounwind uwtable
define internal void @foo_(ptr noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %arg, ptr noalias nocapture readonly dereferenceable(96) "assumed_shape" "ptrnoalias" %arg1, ptr noalias nocapture readonly dereferenceable(4) %arg2) #2 {
bb:
  %i = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %arg, i64 0, i32 6, i64 0
  %i3 = getelementptr inbounds { i64, i64, i64 }, ptr %i, i64 0, i32 0
  %i4 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i3, i32 1)
  %i5 = load i64, ptr %i4, align 1
  %i6 = icmp sgt i64 %i5, 0
  %i7 = select i1 %i6, i64 %i5, i64 0
  %i8 = trunc i64 %i7 to i32
  %i9 = icmp slt i32 %i8, 1
  br i1 %i9, label %bb57, label %bb10

bb10:                                             ; preds = %bb
  %i11 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i3, i32 0)
  %i12 = load i64, ptr %i11, align 1
  %i13 = icmp sgt i64 %i12, 0
  %i14 = select i1 %i13, i64 %i12, i64 0
  %i15 = trunc i64 %i14 to i32
  %i16 = icmp slt i32 %i15, 1
  %i17 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %arg1, i64 0, i32 0
  %i18 = load ptr, ptr %i17, align 1
  %i19 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %arg1, i64 0, i32 6, i64 0
  %i20 = getelementptr inbounds { i64, i64, i64 }, ptr %i19, i64 0, i32 1
  %i21 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i20, i32 0)
  %i22 = load i64, ptr %i21, align 1
  %i23 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i20, i32 1)
  %i24 = load i64, ptr %i23, align 1
  %i25 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %arg, i64 0, i32 0
  %i26 = load ptr, ptr %i25, align 1
  %i27 = getelementptr inbounds { i64, i64, i64 }, ptr %i, i64 0, i32 1
  %i28 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i27, i32 0)
  %i29 = load i64, ptr %i28, align 1
  %i30 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i27, i32 1)
  %i31 = load i64, ptr %i30, align 1
  %i32 = shl i64 %i7, 32
  %i33 = add i64 %i32, 4294967296
  %i34 = ashr exact i64 %i33, 32
  %i35 = shl i64 %i14, 32
  %i36 = add i64 %i35, 4294967296
  %i37 = ashr exact i64 %i36, 32
  br label %bb38

bb38:                                             ; preds = %bb54, %bb10
  %i39 = phi i64 [ 1, %bb10 ], [ %i55, %bb54 ]
  br i1 %i16, label %bb54, label %bb40

bb40:                                             ; preds = %bb38
  %i41 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 400, ptr nonnull elementtype(i32) @"main_$MYK", i64 %i39)
  %i42 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i24, ptr elementtype(float) %i18, i64 %i39)
  %i43 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i31, ptr elementtype(float) %i26, i64 %i39)
  br label %bb44

bb44:                                             ; preds = %bb44, %bb40
  %i45 = phi i64 [ 1, %bb40 ], [ %i52, %bb44 ]
  %i46 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i41, i64 %i45)
  %i47 = load i32, ptr %i46, align 1
  %i48 = sext i32 %i47 to i64
  %i49 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i22, ptr elementtype(float) %i42, i64 %i48)
  %i50 = load float, ptr %i49, align 1
  %i51 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %i29, ptr elementtype(float) %i43, i64 %i48)
  store float %i50, ptr %i51, align 1
  %i52 = add nuw nsw i64 %i45, 1
  %i53 = icmp eq i64 %i52, %i37
  br i1 %i53, label %bb54, label %bb44

bb54:                                             ; preds = %bb44, %bb38
  %i55 = add nuw nsw i64 %i39, 1
  %i56 = icmp eq i64 %i55, %i34
  br i1 %i56, label %bb57, label %bb38

bb57:                                             ; preds = %bb54, %bb
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare i64 @llvm.ssa.copy.i64(i64 returned) #3

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #4

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #4

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nofree noinline nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #3 = { nocallback nofree nosync nounwind readnone willreturn }
attributes #4 = { nounwind readnone speculatable }
attributes #5 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
