; REQUIRES: asserts
; RUN: opt -opaque-pointers < %s -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s

; Check that the array through which the indirect subscripting is occurring is
; a candidate for transposing but not profitable.

; CHECK: Transpose candidate: main_$MYK
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: false

; Check that the arrays represented by dope vector arrays main_$MYA and
; main_$MYB can and should be transposed to ensure that the indirectly
; subscripted index is not the fastest varying subscript.

; CHECK: Transpose candidate: main_$MYB
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: true

; CHECK: Transpose candidate: main_$MYA
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: true

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$i32*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@"main_$MYK" = internal unnamed_addr constant [1000 x [19 x i32]] zeroinitializer, align 16
@"main_$MYB" = internal global %"QNCA_a0$i32*$rank2$" { ptr null, i64 0, i64 0, i64 1073741952, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }
@"main_$MYA" = internal global %"QNCA_a0$i32*$rank2$" { ptr null, i64 0, i64 0, i64 1073741952, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }
@anon.68ba48b9c6c80ce889c10c7426f57970.0 = internal unnamed_addr constant i32 2

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 {
bb:
  %i = tail call i32 @for_set_reentrancy(ptr nonnull @anon.68ba48b9c6c80ce889c10c7426f57970.0) #4
  %i1 = load i64, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYA", i64 0, i32 3), align 8
  %i2 = and i64 %i1, 1030792151296
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYA", i64 0, i32 5), align 8
  store i64 4, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYA", i64 0, i32 1), align 8
  store i64 2, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYA", i64 0, i32 4), align 16
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYA", i64 0, i32 2), align 16
  %i3 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYA", i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i3, align 1
  %i4 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYA", i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 19, ptr %i4, align 1
  %i5 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYA", i64 0, i32 6, i64 0, i32 2), i32 1)
  store i64 1, ptr %i5, align 1
  %i6 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYA", i64 0, i32 6, i64 0, i32 0), i32 1)
  store i64 1000, ptr %i6, align 1
  %i7 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYA", i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 4, ptr %i7, align 1
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYA", i64 0, i32 6, i64 0, i32 1), i32 1)
  store i64 76, ptr %i8, align 1
  %i9 = or i64 %i2, 1073741957
  store i64 %i9, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYA", i64 0, i32 3), align 8
  %i10 = lshr i64 %i2, 15
  %i11 = trunc i64 %i10 to i32
  %i12 = or i32 %i11, 262146
  %i13 = tail call i32 @for_alloc_allocatable_handle(i64 76000, ptr @"main_$MYA", i32 %i12, ptr null) #4
  %i14 = load i64, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYB", i64 0, i32 3), align 8
  %i15 = and i64 %i14, 1030792151296
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYB", i64 0, i32 5), align 8
  store i64 4, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYB", i64 0, i32 1), align 8
  store i64 2, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYB", i64 0, i32 4), align 16
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYB", i64 0, i32 2), align 16
  %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYB", i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i16, align 1
  %i17 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYB", i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 19, ptr %i17, align 1
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYB", i64 0, i32 6, i64 0, i32 2), i32 1)
  store i64 1, ptr %i18, align 1
  %i19 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYB", i64 0, i32 6, i64 0, i32 0), i32 1)
  store i64 1000, ptr %i19, align 1
  %i20 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYB", i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 4, ptr %i20, align 1
  %i21 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYB", i64 0, i32 6, i64 0, i32 1), i32 1)
  store i64 76, ptr %i21, align 1
  %i22 = or i64 %i15, 1073741957
  store i64 %i22, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @"main_$MYB", i64 0, i32 3), align 8
  %i23 = lshr i64 %i15, 15
  %i24 = trunc i64 %i23 to i32
  %i25 = or i32 %i24, 262146
  %i26 = tail call i32 @for_alloc_allocatable_handle(i64 76000, ptr @"main_$MYB", i32 %i25, ptr null) #4
  %i27 = load i64, ptr %i6, align 1
  %i28 = trunc i64 %i27 to i32
  %i29 = icmp slt i32 %i28, 1
  br i1 %i29, label %bb69, label %bb30

bb30:                                             ; preds = %bb
  %i31 = shl i64 %i27, 32
  %i32 = add i64 %i31, 4294967296
  %i33 = ashr exact i64 %i32, 32
  br label %bb34

bb34:                                             ; preds = %bb66, %bb30
  %i35 = phi i64 [ 1, %bb30 ], [ %i67, %bb66 ]
  %i36 = load i64, ptr %i4, align 1
  %i37 = trunc i64 %i36 to i32
  %i38 = icmp slt i32 %i37, 3
  br i1 %i38, label %bb66, label %bb39

bb39:                                             ; preds = %bb34
  %i40 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 76, ptr elementtype(i32) @"main_$MYK", i64 %i35)
  %i41 = shl i64 %i36, 32
  %i42 = ashr exact i64 %i41, 32
  br label %bb43

bb43:                                             ; preds = %bb43, %bb39
  %i44 = phi i64 [ 2, %bb39 ], [ %i64, %bb43 ]
  %i45 = load ptr, ptr @"main_$MYB", align 16
  %i46 = load i64, ptr %i16, align 1
  %i47 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i40, i64 %i44)
  %i48 = load i32, ptr %i47, align 1
  %i49 = add nsw i32 %i48, -1
  %i50 = sext i32 %i49 to i64
  %i51 = load i64, ptr %i21, align 1
  %i52 = load i64, ptr %i18, align 1
  %i53 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i52, i64 %i51, ptr elementtype(i32) %i45, i64 %i35)
  %i54 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i46, i64 4, ptr elementtype(i32) %i53, i64 %i50)
  %i55 = load i32, ptr %i54, align 1
  %i56 = load ptr, ptr @"main_$MYA", align 16
  %i57 = load i64, ptr %i3, align 1
  %i58 = add nsw i32 %i48, 1
  %i59 = sext i32 %i58 to i64
  %i60 = load i64, ptr %i8, align 1
  %i61 = load i64, ptr %i5, align 1
  %i62 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i61, i64 %i60, ptr elementtype(i32) %i56, i64 %i35)
  %i63 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i57, i64 4, ptr elementtype(i32) %i62, i64 %i59)
  store i32 %i55, ptr %i63, align 1
  %i64 = add nuw nsw i64 %i44, 1
  %i65 = icmp eq i64 %i64, %i42
  br i1 %i65, label %bb66, label %bb43

bb66:                                             ; preds = %bb43, %bb34
  %i67 = add nuw nsw i64 %i35, 1
  %i68 = icmp eq i64 %i67, %i33
  br i1 %i68, label %bb69, label %bb34

bb69:                                             ; preds = %bb66, %bb
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare i64 @llvm.ssa.copy.i64(i64 returned) #2

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #3

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #3

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nocallback nofree nosync nounwind readnone willreturn }
attributes #3 = { nounwind readnone speculatable }
attributes #4 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
