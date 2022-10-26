; REQUIRES: asserts
; RUN: opt -opaque-pointers < %s -disable-output -passes=dtrans-transpose -debug-only=dtrans-transpose-transform 2>&1 | FileCheck %s

; Check "nested dope vector" candidates (i.e. candidates which are fields in
; a structure, which is in an array of structures).

; Check that the array through which the indirect subscripting is occurring is
; should not be transposed.

; CHECK-LABEL: Transform candidate: main_$MYK
; CHECK-NOT: Before
; CHECK-NOT: After

; Check that the array represented by the 0th field of main_$PHYSPROP
; is transposed to ensure that the indirectly subscripted index is not the
; fastest varying subscript. Also check that the strides are replaced by
; literal constants.

; CHECK-LABEL: Transform candidate: main_$PHYSPROP[0]
; CHECK-NEXT: Before: MAIN__:  %i[[N0:[A-Za-z0-9]+]] = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i[[I0:[A-Za-z0-9]+]],
; CHECK-NEXT: After : MAIN__:  %i[[N0]] = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4
; CHECK-NEXT: Before: MAIN__:  %i[[N1:[A-Za-z0-9]+]] = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i[[I1:[A-Za-z0-9]+]],
; CHECK-NEXT: After : MAIN__:  %i[[N1]] = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 4000,


; Check that the array represented by the 1st field of main_$PHYSPROP
; is not transposed because the indirectly subscripted index is not the
; fastest varying subscript.

; CHECK-LABEL: Transform candidate: main_$PHYSPROP[1]
; CHECK-NOT: Before
; CHECK-NOT: After

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"MAIN$.btPHYSPROP_TYPE" = type { %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$" }
%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@"main_$MYK" = internal unnamed_addr constant [1000 x [19 x i32]] zeroinitializer, align 16
@"main_$PHYSPROP" = internal global %"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$" { ptr null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@anon.35d4f0c186b9bde99ef526d487a05dff.0 = internal unnamed_addr constant i32 2

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #0 {
bb:
  %i = tail call i32 @for_set_reentrancy(ptr nonnull @anon.35d4f0c186b9bde99ef526d487a05dff.0) #4
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$", ptr @"main_$PHYSPROP", i64 0, i32 5), align 8
  store i64 192, ptr getelementptr inbounds (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$", ptr @"main_$PHYSPROP", i64 0, i32 1), align 8
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$", ptr @"main_$PHYSPROP", i64 0, i32 4), align 16
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$", ptr @"main_$PHYSPROP", i64 0, i32 2), align 16
  %i1 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$", ptr @"main_$PHYSPROP", i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, ptr %i1, align 1
  %i2 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$", ptr @"main_$PHYSPROP", i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 100, ptr %i2, align 1
  %i3 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$", ptr @"main_$PHYSPROP", i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 192, ptr %i3, align 1
  store i64 1073741829, ptr getelementptr inbounds (%"QNCA_a0$%\22MAIN$.btPHYSPROP_TYPE\22*$rank1$", ptr @"main_$PHYSPROP", i64 0, i32 3), align 8
  %i4 = tail call i32 @for_allocate_handle(i64 19200, ptr @"main_$PHYSPROP", i32 262144, ptr null) #4
  br label %bb5

bb5:                                              ; preds = %bb5, %bb
  %i6 = phi i64 [ %i58, %bb5 ], [ 1, %bb ]
  %i7 = load ptr, ptr @"main_$PHYSPROP", align 16
  %i8 = load i64, ptr %i3, align 1
  %i9 = load i64, ptr %i1, align 1
  %i10 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 192, ptr elementtype(%"MAIN$.btPHYSPROP_TYPE") %i7, i64 %i6)
  %i12 = getelementptr inbounds %"MAIN$.btPHYSPROP_TYPE", ptr %i10, i64 0, i32 0
  %i13 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i12, i64 0, i32 3
  store i64 5, ptr %i13, align 1, !alias.scope !3
  %i14 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i12, i64 0, i32 5
  store i64 0, ptr %i14, align 1, !alias.scope !3
  %i15 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i12, i64 0, i32 1
  store i64 4, ptr %i15, align 1, !alias.scope !3
  %i16 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i12, i64 0, i32 4
  store i64 2, ptr %i16, align 1, !alias.scope !3
  %i17 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i12, i64 0, i32 2
  store i64 0, ptr %i17, align 1, !alias.scope !3
  %i18 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i12, i64 0, i32 6, i64 0
  %i19 = getelementptr inbounds { i64, i64, i64 }, ptr %i18, i64 0, i32 2
  %i20 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i19, i32 0) #4
  store i64 1, ptr %i20, align 1, !alias.scope !3
  %i21 = getelementptr inbounds { i64, i64, i64 }, ptr %i18, i64 0, i32 0
  %i22 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i21, i32 0) #4
  store i64 19, ptr %i22, align 1, !alias.scope !3
  %i23 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i19, i32 1) #4
  store i64 1, ptr %i23, align 1, !alias.scope !3
  %i24 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i21, i32 1) #4
  store i64 1000, ptr %i24, align 1, !alias.scope !3
  %i25 = getelementptr inbounds { i64, i64, i64 }, ptr %i18, i64 0, i32 1
  %i26 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i25, i32 0) #4
  store i64 4, ptr %i26, align 1, !alias.scope !3
  %i27 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i25, i32 1) #4
  store i64 76, ptr %i27, align 1, !alias.scope !3
  %i28 = load i64, ptr %i13, align 1, !alias.scope !3
  %i29 = and i64 %i28, -68451041281
  %i30 = or i64 %i29, 1073741824
  store i64 %i30, ptr %i13, align 1, !alias.scope !3
  %i31 = load i64, ptr %i14, align 1, !alias.scope !3
  %i32 = inttoptr i64 %i31 to ptr
  %i33 = bitcast ptr %i10 to ptr
  %i34 = tail call i32 @for_allocate_handle(i64 76000, ptr nonnull %i33, i32 262144, ptr %i32) #4
  %i35 = getelementptr inbounds %"MAIN$.btPHYSPROP_TYPE", ptr %i10, i64 0, i32 1
  %i36 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i35, i64 0, i32 3
  store i64 5, ptr %i36, align 1, !alias.scope !3
  %i37 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i35, i64 0, i32 5
  store i64 0, ptr %i37, align 1, !alias.scope !3
  %i38 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i35, i64 0, i32 1
  store i64 4, ptr %i38, align 1, !alias.scope !3
  %i39 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i35, i64 0, i32 4
  store i64 2, ptr %i39, align 1, !alias.scope !3
  %i40 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i35, i64 0, i32 2
  store i64 0, ptr %i40, align 1, !alias.scope !3
  %i41 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i35, i64 0, i32 6, i64 0
  %i42 = getelementptr inbounds { i64, i64, i64 }, ptr %i41, i64 0, i32 2
  %i43 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i42, i32 0) #4
  store i64 1, ptr %i43, align 1, !alias.scope !3
  %i44 = getelementptr inbounds { i64, i64, i64 }, ptr %i41, i64 0, i32 0
  %i45 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 0) #4
  store i64 19, ptr %i45, align 1, !alias.scope !3
  %i46 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i42, i32 1) #4
  store i64 1, ptr %i46, align 1, !alias.scope !3
  %i47 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i44, i32 1) #4
  store i64 1000, ptr %i47, align 1, !alias.scope !3
  %i48 = getelementptr inbounds { i64, i64, i64 }, ptr %i41, i64 0, i32 1
  %i49 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i48, i32 0) #4
  store i64 4, ptr %i49, align 1, !alias.scope !3
  %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i48, i32 1) #4
  store i64 76, ptr %i50, align 1, !alias.scope !3
  %i51 = load i64, ptr %i36, align 1, !alias.scope !3
  %i52 = and i64 %i51, -68451041281
  %i53 = or i64 %i52, 1073741824
  store i64 %i53, ptr %i36, align 1, !alias.scope !3
  %i54 = load i64, ptr %i37, align 1, !alias.scope !3
  %i55 = inttoptr i64 %i54 to ptr
  %i56 = bitcast ptr %i35 to ptr
  %i57 = tail call i32 @for_allocate_handle(i64 76000, ptr nonnull %i56, i32 262144, ptr %i55) #4
  %i58 = add nuw nsw i64 %i6, 1
  %i59 = icmp eq i64 %i58, 101
  br i1 %i59, label %bb60, label %bb5

bb60:                                             ; preds = %bb115, %bb5
  %i61 = phi i64 [ %i116, %bb115 ], [ 1, %bb5 ]
  br label %bb62

bb62:                                             ; preds = %bb112, %bb60
  %i63 = phi i64 [ %i113, %bb112 ], [ 1, %bb60 ]
  %i64 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 76, ptr elementtype(i32) @"main_$MYK", i64 %i63)
  br label %bb65

bb65:                                             ; preds = %bb65, %bb62
  %i66 = phi i64 [ %i110, %bb65 ], [ 2, %bb62 ]
  %i67 = load ptr, ptr @"main_$PHYSPROP", align 16
  %i68 = load i64, ptr %i3, align 1
  %i69 = load i64, ptr %i1, align 1
  %i70 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 192, ptr elementtype(%"MAIN$.btPHYSPROP_TYPE") %i67, i64 %i61)
  %i71 = getelementptr inbounds %"MAIN$.btPHYSPROP_TYPE", ptr %i70, i64 0, i32 1
  %i72 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i71, i64 0, i32 0
  %i73 = load ptr, ptr %i72, align 1
  %i74 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i71, i64 0, i32 6, i64 0
  %i75 = getelementptr inbounds { i64, i64, i64 }, ptr %i74, i64 0, i32 1
  %i76 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i75, i32 0)
  %i77 = load i64, ptr %i76, align 1
  %i78 = getelementptr inbounds { i64, i64, i64 }, ptr %i74, i64 0, i32 2
  %i79 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i78, i32 0)
  %i80 = load i64, ptr %i79, align 1
  %i81 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i75, i32 1)
  %i82 = load i64, ptr %i81, align 1
  %i83 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i78, i32 1)
  %i84 = load i64, ptr %i83, align 1
  %i85 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %i64, i64 %i66)
  %i86 = load i32, ptr %i85, align 1
  %i87 = add nsw i32 %i86, -1
  %i88 = sext i32 %i87 to i64
  %i89 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i84, i64 %i82, ptr elementtype(float) %i73, i64 %i88)
  %i90 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i80, i64 %i77, ptr elementtype(float) %i89, i64 %i63)
  %i91 = load float, ptr %i90, align 1
  %i92 = getelementptr inbounds %"MAIN$.btPHYSPROP_TYPE", ptr %i70, i64 0, i32 0
  %i93 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i92, i64 0, i32 0
  %i94 = load ptr, ptr %i93, align 1
  %i95 = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %i92, i64 0, i32 6, i64 0
  %i96 = getelementptr inbounds { i64, i64, i64 }, ptr %i95, i64 0, i32 1
  %i97 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i96, i32 0)
  %i98 = load i64, ptr %i97, align 1
  %i99 = getelementptr inbounds { i64, i64, i64 }, ptr %i95, i64 0, i32 2
  %i100 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i99, i32 0)
  %i101 = load i64, ptr %i100, align 1
  %i102 = add nsw i32 %i86, 1
  %i103 = sext i32 %i102 to i64
  %i104 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i96, i32 1)
  %i105 = load i64, ptr %i104, align 1
  %i106 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %i99, i32 1)
  %i107 = load i64, ptr %i106, align 1
  %i108 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %i107, i64 %i105, ptr elementtype(float) %i94, i64 %i63)
  %i109 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i101, i64 %i98, ptr elementtype(float) %i108, i64 %i103)
  store float %i91, ptr %i109, align 1
  %i110 = add nuw nsw i64 %i66, 1
  %i111 = icmp eq i64 %i110, 19
  br i1 %i111, label %bb112, label %bb65

bb112:                                            ; preds = %bb65
  %i113 = add nuw nsw i64 %i63, 1
  %i114 = icmp eq i64 %i113, 1001
  br i1 %i114, label %bb115, label %bb62

bb115:                                            ; preds = %bb112
  %i116 = add nuw nsw i64 %i61, 1
  %i117 = icmp eq i64 %i116, 101
  br i1 %i117, label %bb118, label %bb60

bb118:                                            ; preds = %bb115
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

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
!3 = !{!4}
!4 = distinct !{!4, !5, !"initfield_: %initfield_$PHYSPROPFIELD"}
!5 = distinct !{!5, !"initfield_"}
