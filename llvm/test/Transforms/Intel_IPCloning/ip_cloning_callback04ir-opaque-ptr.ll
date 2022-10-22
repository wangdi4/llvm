; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -passes='module(post-inline-ip-cloning)' -ip-cloning-force-heuristics-off -ip-gen-cloning-force-on-callback-cloning -S 2>&1 | FileCheck %s

; Check that callback cloning is not attempted for @test because it is a
; varags function, even though -ip-gen-cloning-force-on-callback-cloning is
; specified.
; This is the same test as ip_cloning_callback04-opaque-ptr.ll, but checks for
; IR without requiring asserts.

; CHECK: define internal i32 @test(i32 %arg, ...)
; CHECK: define dso_local i32 @main()
; CHECK: call i32 (i32, ...) @test.2(i32 1, i64 8589934592, i64 25769803780)
; CHECK: call i32 (i32, ...) @test.1(i32 2, i64 4294967296, i64 12884901890, i64 4294967296, i64 12884901890)
; CHECK: call i32 (i32, ...) @test.1(i32 2, i64 4294967296, i64 12884901890, i64 4294967296, i64 12884901890)
; CHECK: define internal i32 @test.1(i32 %arg, ...)
; CHECK: define internal i32 @test.2(i32 %arg, ...)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct._IO_marker = type opaque
%struct._IO_codecvt = type opaque
%struct._IO_wide_data = type opaque
%union.M128 = type { <4 x float> }
%struct.__va_list_tag = type { i32, i32, ptr, ptr }

@stderr = external dso_local local_unnamed_addr global ptr, align 8
@.str = private unnamed_addr constant [32 x i8] c"test failed for: (%d,%d,%d,%d)\0A\00", align 1
@.str.1 = private unnamed_addr constant [46 x i8] c"test failed for: (%d,%d,%d,%d) (%d,%d,%d,%d)\0A\00", align 1
@str = private unnamed_addr constant [7 x i8] c"PASSED\00", align 1
@str.4 = private unnamed_addr constant [7 x i8] c"FAILED\00", align 1

; Function Attrs: nofree nosync nounwind uwtable
define internal i32 @test(i32 %arg, ...) #0 {
bb:
  %i = alloca %union.M128, align 16
  %i1 = alloca %union.M128, align 16
  %i2 = alloca [1 x %struct.__va_list_tag], align 16
  %i3 = alloca %union.M128, align 16
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %i) #8
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %i1) #8
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %i2) #8
  call void @llvm.va_start(ptr nonnull %i2)
  %i8 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %i2, i64 0, i64 0, i32 0
  %i9 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %i2, i64 0, i64 0, i32 2
  %i10 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %i2, i64 0, i64 0, i32 3
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 dereferenceable(16) %i1, i8 0, i64 16, i1 false)
  %i11 = load ptr, ptr %i10, align 16
  %i14 = icmp sgt i32 %arg, 0
  br i1 %i14, label %bb15, label %bb50

bb15:                                             ; preds = %bb
  %i16 = load i32, ptr %i8, align 16
  %i17 = icmp ult i32 %i16, 33
  br i1 %i17, label %bb20, label %bb18

bb18:                                             ; preds = %bb15
  %i19 = load ptr, ptr %i9, align 8
  br label %bb51

bb20:                                             ; preds = %bb38, %bb15
  %i21 = phi i32 [ %i37, %bb38 ], [ %i16, %bb15 ]
  %i22 = phi i32 [ %i39, %bb38 ], [ 0, %bb15 ]
  %i23 = icmp ult i32 %i21, 33
  br i1 %i23, label %bb31, label %bb24

bb24:                                             ; preds = %bb20
  %i25 = load ptr, ptr %i9, align 8
  %i26 = ptrtoint ptr %i25 to i64
  %i27 = add i64 %i26, 15
  %i28 = and i64 %i27, -16
  %i29 = inttoptr i64 %i28 to ptr
  %i30 = getelementptr i8, ptr %i29, i64 16
  store ptr %i30, ptr %i9, align 8
  br label %bb35

bb31:                                             ; preds = %bb20
  %i32 = zext i32 %i21 to i64
  %i33 = getelementptr i8, ptr %i11, i64 %i32
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 16 dereferenceable(16) %i3, ptr noundef nonnull align 8 dereferenceable(16) %i33, i64 16, i1 false)
  %i34 = add nuw nsw i32 %i21, 16
  store i32 %i34, ptr %i8, align 16
  br label %bb35

bb35:                                             ; preds = %bb31, %bb24
  %i36 = phi ptr [ %i3, %bb31 ], [ %i29, %bb24 ]
  %i37 = phi i32 [ %i34, %bb31 ], [ %i21, %bb24 ]
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 16 dereferenceable(16) %i, ptr noundef nonnull align 16 dereferenceable(16) %i36, i64 16, i1 false), !tbaa.struct !6
  br label %bb41

bb38:                                             ; preds = %bb41
  %i39 = add nuw nsw i32 %i22, 1
  %i40 = icmp eq i32 %i39, %arg
  br i1 %i40, label %bb50, label %bb20, !llvm.loop !13

bb41:                                             ; preds = %bb41, %bb35
  %i42 = phi i64 [ %i48, %bb41 ], [ 0, %bb35 ]
  %i43 = getelementptr inbounds [4 x i32], ptr %i, i64 0, i64 %i42
  %i44 = load i32, ptr %i43, align 4, !tbaa !12
  %i45 = getelementptr inbounds [4 x i32], ptr %i1, i64 0, i64 %i42
  %i46 = load i32, ptr %i45, align 4, !tbaa !12
  %i47 = add nsw i32 %i46, %i44
  store i32 %i47, ptr %i45, align 4, !tbaa !12
  %i48 = add nuw nsw i64 %i42, 1
  %i49 = icmp eq i64 %i48, 4
  br i1 %i49, label %bb38, label %bb41, !llvm.loop !16

bb50:                                             ; preds = %bb68, %bb38, %bb
  br label %bb71

bb51:                                             ; preds = %bb68, %bb18
  %i52 = phi ptr [ %i58, %bb68 ], [ %i19, %bb18 ]
  %i53 = phi i32 [ %i69, %bb68 ], [ 0, %bb18 ]
  %i54 = ptrtoint ptr %i52 to i64
  %i55 = add i64 %i54, 15
  %i56 = and i64 %i55, -16
  %i57 = inttoptr i64 %i56 to ptr
  %i58 = getelementptr i8, ptr %i57, i64 16
  store ptr %i58, ptr %i9, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 16 dereferenceable(16) %i, ptr noundef nonnull align 16 dereferenceable(16) %i57, i64 16, i1 false), !tbaa.struct !6
  br label %bb59

bb59:                                             ; preds = %bb59, %bb51
  %i60 = phi i64 [ 0, %bb51 ], [ %i66, %bb59 ]
  %i61 = getelementptr inbounds [4 x i32], ptr %i, i64 0, i64 %i60
  %i62 = load i32, ptr %i61, align 4, !tbaa !12
  %i63 = getelementptr inbounds [4 x i32], ptr %i1, i64 0, i64 %i60
  %i64 = load i32, ptr %i63, align 4, !tbaa !12
  %i65 = add nsw i32 %i64, %i62
  store i32 %i65, ptr %i63, align 4, !tbaa !12
  %i66 = add nuw nsw i64 %i60, 1
  %i67 = icmp eq i64 %i66, 4
  br i1 %i67, label %bb68, label %bb59, !llvm.loop !16

bb68:                                             ; preds = %bb59
  %i69 = add nuw nsw i32 %i53, 1
  %i70 = icmp eq i32 %i69, %arg
  br i1 %i70, label %bb50, label %bb51, !llvm.loop !17

bb71:                                             ; preds = %bb71, %bb50
  %i72 = phi i64 [ 0, %bb50 ], [ %i80, %bb71 ]
  %i73 = phi i32 [ 0, %bb50 ], [ %i79, %bb71 ]
  %i74 = getelementptr inbounds [4 x i32], ptr %i1, i64 0, i64 %i72
  %i75 = load i32, ptr %i74, align 4, !tbaa !12
  %i76 = shl nuw nsw i64 %i72, 1
  %i77 = zext i32 %i75 to i64
  %i78 = icmp eq i64 %i76, %i77
  %i79 = select i1 %i78, i32 %i73, i32 1
  %i80 = add nuw nsw i64 %i72, 1
  %i81 = icmp eq i64 %i80, 4
  br i1 %i81, label %bb82, label %bb71, !llvm.loop !18

bb82:                                             ; preds = %bb71
  call void @llvm.va_end(ptr nonnull %i2)
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %i2) #8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %i1) #8
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %i) #8
  ret i32 %i79
}

; Function Attrs: nofree nosync nounwind willreturn
declare void @llvm.va_start(ptr) #1

; Function Attrs: nofree nosync nounwind willreturn
declare void @llvm.va_end(ptr) #1

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @main() #2 {
bb:
  %i = tail call i32 (i32, ...) @test(i32 1, i64 8589934592, i64 25769803780)
  %i1 = icmp eq i32 %i, 0
  br i1 %i1, label %bb2, label %bb5

bb2:                                              ; preds = %bb
  %i3 = tail call i32 (i32, ...) @test(i32 2, i64 4294967296, i64 12884901890, i64 4294967296, i64 12884901890)
  %i4 = icmp eq i32 %i3, 0
  br i1 %i4, label %bb13, label %bb10

bb5:                                              ; preds = %bb
  %i6 = load ptr, ptr @stderr, align 8, !tbaa !19
  %i7 = tail call i32 (ptr, ptr, ...) @fprintf(ptr %i6, ptr getelementptr inbounds ([32 x i8], ptr @.str, i64 0, i64 0), i32 0, i32 2, i32 4, i32 6) #9
  %i8 = tail call i32 (i32, ...) @test(i32 2, i64 4294967296, i64 12884901890, i64 4294967296, i64 12884901890)
  %i9 = icmp eq i32 %i8, 0
  br i1 %i9, label %bb13, label %bb10

bb10:                                             ; preds = %bb5, %bb2
  %i11 = load ptr, ptr @stderr, align 8, !tbaa !19
  %i12 = tail call i32 (ptr, ptr, ...) @fprintf(ptr %i11, ptr getelementptr inbounds ([46 x i8], ptr @.str.1, i64 0, i64 0), i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3) #9
  br label %bb13

bb13:                                             ; preds = %bb10, %bb5, %bb2
  %i14 = phi ptr [ getelementptr inbounds ([7 x i8], ptr @str.4, i64 0, i64 0), %bb5 ], [ getelementptr inbounds ([7 x i8], ptr @str.4, i64 0, i64 0), %bb10 ], [ getelementptr inbounds ([7 x i8], ptr @str, i64 0, i64 0), %bb2 ]
  %i15 = phi i32 [ 1, %bb5 ], [ 1, %bb10 ], [ 0, %bb2 ]
  %i16 = tail call i32 @puts(ptr nonnull dereferenceable(1) %i14)
  ret i32 %i15
}

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @fprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ...) local_unnamed_addr #3

; Function Attrs: nofree nounwind
declare noundef i32 @puts(ptr nocapture noundef readonly) local_unnamed_addr #4

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #5

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #6

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #7

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #5

attributes #0 = { nofree nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { nofree nosync nounwind willreturn }
attributes #2 = { nofree nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #3 = { nofree nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #4 = { nofree nounwind }
attributes #5 = { argmemonly nofree nosync nounwind willreturn }
attributes #6 = { argmemonly nofree nounwind willreturn writeonly }
attributes #7 = { argmemonly nofree nounwind willreturn }
attributes #8 = { nounwind }
attributes #9 = { cold }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{i32 1, !"LTOPostLink", i32 1}
!6 = !{i64 0, i64 16, !7, i64 0, i64 16, !12}
!7 = !{!8, !8, i64 0}
!8 = !{!"array@_ZTSA4_i", !9, i64 0}
!9 = !{!"int", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
!12 = !{!10, !10, i64 0}
!13 = distinct !{!13, !14, !15}
!14 = !{!"llvm.loop.mustprogress"}
!15 = !{!"llvm.loop.unswitch.partial.disable"}
!16 = distinct !{!16, !14}
!17 = distinct !{!17, !14}
!18 = distinct !{!18, !14}
!19 = !{!20, !20, i64 0}
!20 = !{!"pointer@_ZTSP8_IO_FILE", !10, i64 0}
; end INTEL_FEATURE_SW_ADVANCED
