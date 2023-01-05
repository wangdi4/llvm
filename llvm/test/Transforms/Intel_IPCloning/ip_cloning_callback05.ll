; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
; RUN: opt < %s -passes='module(post-inline-ip-cloning)' -ip-cloning-force-heuristics-off -S -debug-only=ipcloning 2>&1 | FileCheck %s

; Check that callback cloning is not attempted for @test because it is a
; varags function with no special force on or off flags.

; CHECK: Not attempting callback cloning for test
; CHECK: Cloned call:     %9 = tail call i32 (i32, ...) @test.1(i32 2, i64 4294967296, i64 12884901890, i64 4294967296, i64 12884901890)
; CHECK: Cloned call:     %4 = tail call i32 (i32, ...) @test.1(i32 2, i64 4294967296, i64 12884901890, i64 4294967296, i64 12884901890)
; CHECK: Cloned call:     %1 = tail call i32 (i32, ...) @test.2(i32 1, i64 8589934592, i64 25769803780)

; CHECK: define internal i32 @test(i32 %0, ...)
; CHECK: define dso_local i32 @main()
; CHECK: call i32 (i32, ...) @test.2(i32 1, i64 8589934592, i64 25769803780)
; CHECK: call i32 (i32, ...) @test.1(i32 2, i64 4294967296, i64 12884901890, i64 4294967296, i64 12884901890)
; CHECK: call i32 (i32, ...) @test.1(i32 2, i64 4294967296, i64 12884901890, i64 4294967296, i64 12884901890)
; CHECK: define internal i32 @test.1(i32 %0, ...)
; CHECK: define internal i32 @test.2(i32 %0, ...)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, %struct._IO_codecvt*, %struct._IO_wide_data*, %struct._IO_FILE*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type opaque
%struct._IO_codecvt = type opaque
%struct._IO_wide_data = type opaque
%union.M128 = type { <4 x float> }
%struct.__va_list_tag = type { i32, i32, i8*, i8* }

@stderr = external dso_local local_unnamed_addr global %struct._IO_FILE*, align 8
@.str = private unnamed_addr constant [32 x i8] c"test failed for: (%d,%d,%d,%d)\0A\00", align 1
@.str.1 = private unnamed_addr constant [46 x i8] c"test failed for: (%d,%d,%d,%d) (%d,%d,%d,%d)\0A\00", align 1
@str = private unnamed_addr constant [7 x i8] c"PASSED\00", align 1
@str.4 = private unnamed_addr constant [7 x i8] c"FAILED\00", align 1

; Function Attrs: nofree nosync nounwind uwtable
define internal i32 @test(i32 %0, ...) #0 {
  %2 = alloca %union.M128, align 16
  %3 = alloca %union.M128, align 16
  %4 = alloca [1 x %struct.__va_list_tag], align 16
  %5 = alloca %union.M128, align 16
  %6 = bitcast %union.M128* %2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %6) #7
  %7 = bitcast %union.M128* %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %7) #7
  %8 = bitcast [1 x %struct.__va_list_tag]* %4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 24, i8* nonnull %8) #7
  call void @llvm.va_start(i8* nonnull %8)
  %9 = bitcast %union.M128* %3 to [4 x i32]*
  %10 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %4, i64 0, i64 0, i32 0
  %11 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %4, i64 0, i64 0, i32 2
  %12 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %4, i64 0, i64 0, i32 3
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 16 dereferenceable(16) %7, i8 0, i64 16, i1 false)
  %13 = load i8*, i8** %12, align 16
  %14 = bitcast %union.M128* %5 to i8*
  %15 = bitcast %union.M128* %2 to [4 x i32]*
  %16 = icmp sgt i32 %0, 0
  br i1 %16, label %17, label %52

17:                                               ; preds = %1
  %18 = load i32, i32* %10, align 16
  %19 = icmp ult i32 %18, 33
  br i1 %19, label %22, label %20

20:                                               ; preds = %17
  %21 = load i8*, i8** %11, align 8
  br label %53

22:                                               ; preds = %40, %17
  %23 = phi i32 [ %39, %40 ], [ %18, %17 ]
  %24 = phi i32 [ %41, %40 ], [ 0, %17 ]
  %25 = icmp ult i32 %23, 33
  br i1 %25, label %33, label %26

26:                                               ; preds = %22
  %27 = load i8*, i8** %11, align 8
  %28 = ptrtoint i8* %27 to i64
  %29 = add i64 %28, 15
  %30 = and i64 %29, -16
  %31 = inttoptr i64 %30 to i8*
  %32 = getelementptr i8, i8* %31, i64 16
  store i8* %32, i8** %11, align 8
  br label %37

33:                                               ; preds = %22
  %34 = zext i32 %23 to i64
  %35 = getelementptr i8, i8* %13, i64 %34
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* noundef nonnull align 16 dereferenceable(16) %14, i8* noundef nonnull align 8 dereferenceable(16) %35, i64 16, i1 false)
  %36 = add nuw nsw i32 %23, 16
  store i32 %36, i32* %10, align 16
  br label %37

37:                                               ; preds = %33, %26
  %38 = phi i8* [ %14, %33 ], [ %31, %26 ]
  %39 = phi i32 [ %36, %33 ], [ %23, %26 ]
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* noundef nonnull align 16 dereferenceable(16) %6, i8* noundef nonnull align 16 dereferenceable(16) %38, i64 16, i1 false), !tbaa.struct !6
  br label %43

40:                                               ; preds = %43
  %41 = add nuw nsw i32 %24, 1
  %42 = icmp eq i32 %41, %0
  br i1 %42, label %52, label %22, !llvm.loop !13

43:                                               ; preds = %43, %37
  %44 = phi i64 [ %50, %43 ], [ 0, %37 ]
  %45 = getelementptr inbounds [4 x i32], [4 x i32]* %15, i64 0, i64 %44
  %46 = load i32, i32* %45, align 4, !tbaa !12
  %47 = getelementptr inbounds [4 x i32], [4 x i32]* %9, i64 0, i64 %44
  %48 = load i32, i32* %47, align 4, !tbaa !12
  %49 = add nsw i32 %48, %46
  store i32 %49, i32* %47, align 4, !tbaa !12
  %50 = add nuw nsw i64 %44, 1
  %51 = icmp eq i64 %50, 4
  br i1 %51, label %40, label %43, !llvm.loop !16

52:                                               ; preds = %70, %40, %1
  br label %73

53:                                               ; preds = %70, %20
  %54 = phi i8* [ %60, %70 ], [ %21, %20 ]
  %55 = phi i32 [ %71, %70 ], [ 0, %20 ]
  %56 = ptrtoint i8* %54 to i64
  %57 = add i64 %56, 15
  %58 = and i64 %57, -16
  %59 = inttoptr i64 %58 to i8*
  %60 = getelementptr i8, i8* %59, i64 16
  store i8* %60, i8** %11, align 8
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* noundef nonnull align 16 dereferenceable(16) %6, i8* noundef nonnull align 16 dereferenceable(16) %59, i64 16, i1 false), !tbaa.struct !6
  br label %61

61:                                               ; preds = %61, %53
  %62 = phi i64 [ 0, %53 ], [ %68, %61 ]
  %63 = getelementptr inbounds [4 x i32], [4 x i32]* %15, i64 0, i64 %62
  %64 = load i32, i32* %63, align 4, !tbaa !12
  %65 = getelementptr inbounds [4 x i32], [4 x i32]* %9, i64 0, i64 %62
  %66 = load i32, i32* %65, align 4, !tbaa !12
  %67 = add nsw i32 %66, %64
  store i32 %67, i32* %65, align 4, !tbaa !12
  %68 = add nuw nsw i64 %62, 1
  %69 = icmp eq i64 %68, 4
  br i1 %69, label %70, label %61, !llvm.loop !16

70:                                               ; preds = %61
  %71 = add nuw nsw i32 %55, 1
  %72 = icmp eq i32 %71, %0
  br i1 %72, label %52, label %53, !llvm.loop !17

73:                                               ; preds = %73, %52
  %74 = phi i64 [ 0, %52 ], [ %82, %73 ]
  %75 = phi i32 [ 0, %52 ], [ %81, %73 ]
  %76 = getelementptr inbounds [4 x i32], [4 x i32]* %9, i64 0, i64 %74
  %77 = load i32, i32* %76, align 4, !tbaa !12
  %78 = shl nuw nsw i64 %74, 1
  %79 = zext i32 %77 to i64
  %80 = icmp eq i64 %78, %79
  %81 = select i1 %80, i32 %75, i32 1
  %82 = add nuw nsw i64 %74, 1
  %83 = icmp eq i64 %82, 4
  br i1 %83, label %84, label %73, !llvm.loop !18

84:                                               ; preds = %73
  call void @llvm.va_end(i8* nonnull %8)
  call void @llvm.lifetime.end.p0i8(i64 24, i8* nonnull %8) #7
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %7) #7
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %6) #7
  ret i32 %81
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg %0, i8* nocapture %1) #1

; Function Attrs: nofree nosync nounwind willreturn
declare void @llvm.va_start(i8* %0) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly %0, i8 %1, i64 %2, i1 immarg %3) #3

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly %0, i8* noalias nocapture readonly %1, i64 %2, i1 immarg %3) #1

; Function Attrs: nofree nosync nounwind willreturn
declare void @llvm.va_end(i8* %0) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg %0, i8* nocapture %1) #1

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @main() #4 {
  %1 = tail call i32 (i32, ...) @test(i32 1, i64 8589934592, i64 25769803780)
  %2 = icmp eq i32 %1, 0
  br i1 %2, label %3, label %6

3:                                                ; preds = %0
  %4 = tail call i32 (i32, ...) @test(i32 2, i64 4294967296, i64 12884901890, i64 4294967296, i64 12884901890)
  %5 = icmp eq i32 %4, 0
  br i1 %5, label %14, label %11

6:                                                ; preds = %0
  %7 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !19
  %8 = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %7, i8* getelementptr inbounds ([32 x i8], [32 x i8]* @.str, i64 0, i64 0), i32 0, i32 2, i32 4, i32 6) #8
  %9 = tail call i32 (i32, ...) @test(i32 2, i64 4294967296, i64 12884901890, i64 4294967296, i64 12884901890)
  %10 = icmp eq i32 %9, 0
  br i1 %10, label %14, label %11

11:                                               ; preds = %6, %3
  %12 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !19
  %13 = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %12, i8* getelementptr inbounds ([46 x i8], [46 x i8]* @.str.1, i64 0, i64 0), i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3) #8
  br label %14

14:                                               ; preds = %11, %6, %3
  %15 = phi i8* [ getelementptr inbounds ([7 x i8], [7 x i8]* @str.4, i64 0, i64 0), %6 ], [ getelementptr inbounds ([7 x i8], [7 x i8]* @str.4, i64 0, i64 0), %11 ], [ getelementptr inbounds ([7 x i8], [7 x i8]* @str, i64 0, i64 0), %3 ]
  %16 = phi i32 [ 1, %6 ], [ 1, %11 ], [ 0, %3 ]
  %17 = tail call i32 @puts(i8* nonnull dereferenceable(1) %15)
  ret i32 %16
}

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @fprintf(%struct._IO_FILE* nocapture noundef %0, i8* nocapture noundef readonly %1, ...) local_unnamed_addr #5

; Function Attrs: nofree nounwind
declare noundef i32 @puts(i8* nocapture noundef readonly %0) local_unnamed_addr #6

attributes #0 = { nofree nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nofree nosync nounwind willreturn }
attributes #3 = { argmemonly nofree nosync nounwind willreturn writeonly }
attributes #4 = { nofree nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #5 = { nofree nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #6 = { nofree nounwind }
attributes #7 = { nounwind }
attributes #8 = { cold }

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
