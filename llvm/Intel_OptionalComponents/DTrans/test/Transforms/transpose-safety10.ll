; RUN: opt < %s -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s

; This test includes an uplevel nested type with name
;   %__DTRT_MYCONTAINED.uplevel_nested_type
; which contains a dope vector. It is currently not possible for an array
; represented by such a dope vector to be a transpose candidate, because it
; will be a local variable, and we now only handle dope vectors which are
; global. But I am including this test for completeness sake.

; The global dope vector smmod_mp_myglobal_, however, is a candidate but
; will not be profitable, so check for that.

; CHECK-LABEL: Transpose candidate: smmod_mp_myglobal_
; CHECK: IsValid      : true
; CHECK: IsProfitable : false


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$i8*$rank2$" = type { i8*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%"__DTRT_QNCA_a0$i32*$rank2$" = type { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%__DTRT_MYCONTAINED.uplevel_nested_type = type { %"__DTRT_QNCA_a0$i32*$rank2$" }
%vfe_uplevel_rec_type = type { i8*, i8* }

@anon.f1fda912cca3a8504be97bdb0beb62d9.0 = internal unnamed_addr constant i32 2
@"var$7" = internal unnamed_addr constant %"QNCA_a0$i8*$rank2$" { i8* null, i64 0, i64 0, i64 128, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }
@"hasvfe_IP_mycontained$blk_$format_pack" = internal unnamed_addr global [36 x i8] c"6\00\00\00$\02\00\00\01\00\00\00\00\00\00\00\00\00\00\00\00\00\0A\09\00\00\00\00\00\00\00\007\00\00\00", align 4
@smmod_mp_myglobal_ = internal global %"__DTRT_QNCA_a0$i32*$rank2$" { i32* null, i64 0, i64 0, i64 1073741952, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }

; Function Attrs: nounwind uwtable
define dso_local void @MAIN__() #0 {
  %1 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.f1fda912cca3a8504be97bdb0beb62d9.0) #6
  %2 = load i64, i64* getelementptr inbounds (%"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* @smmod_mp_myglobal_, i64 0, i32 3), align 8
  %3 = and i64 %2, 1030792151296
  store i64 0, i64* getelementptr inbounds (%"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* @smmod_mp_myglobal_, i64 0, i32 5), align 8
  store i64 4, i64* getelementptr inbounds (%"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* @smmod_mp_myglobal_, i64 0, i32 1), align 8
  store i64 2, i64* getelementptr inbounds (%"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* @smmod_mp_myglobal_, i64 0, i32 4), align 8
  store i64 0, i64* getelementptr inbounds (%"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* @smmod_mp_myglobal_, i64 0, i32 2), align 8
  %4 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* @smmod_mp_myglobal_, i64 0, i32 6, i64 0, i32 2), i32 0) #6
  store i64 1, i64* %4, align 1
  %5 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* @smmod_mp_myglobal_, i64 0, i32 6, i64 0, i32 0), i32 0) #6
  store i64 10, i64* %5, align 1
  %6 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* @smmod_mp_myglobal_, i64 0, i32 6, i64 0, i32 2), i32 1) #6
  store i64 1, i64* %6, align 1
  %7 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* @smmod_mp_myglobal_, i64 0, i32 6, i64 0, i32 0), i32 1) #6
  store i64 10, i64* %7, align 1
  %8 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* @smmod_mp_myglobal_, i64 0, i32 6, i64 0, i32 1), i32 0) #6
  store i64 4, i64* %8, align 1
  %9 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* @smmod_mp_myglobal_, i64 0, i32 6, i64 0, i32 1), i32 1) #6
  store i64 40, i64* %9, align 1
  %10 = or i64 %3, 1073741957
  store i64 %10, i64* getelementptr inbounds (%"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* @smmod_mp_myglobal_, i64 0, i32 3), align 8
  %11 = lshr i64 %3, 15
  %12 = trunc i64 %11 to i32
  %13 = or i32 %12, 262146
  %14 = tail call i32 @for_alloc_allocatable_handle(i64 400, i8** bitcast (%"__DTRT_QNCA_a0$i32*$rank2$"* @smmod_mp_myglobal_ to i8**), i32 %13, i8* null) #6
  tail call void @hasvfe_IP_mycontained_() #6
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly %0) local_unnamed_addr #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 %0, i64 %1, i32 %2, i64* elementtype(i64) %3, i32 %4) #2

; Function Attrs: nofree
declare dso_local i32 @for_alloc_allocatable_handle(i64 %0, i8** nocapture %1, i32 %2, i8* %3) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define internal void @hasvfe_IP_mycontained_() #0 {
  %1 = alloca [8 x i64], align 16
  %2 = alloca %__DTRT_MYCONTAINED.uplevel_nested_type, align 8
  %3 = alloca %vfe_uplevel_rec_type, align 8
  %4 = alloca [2 x i64], align 16
  %5 = alloca [48 x i8], align 16
  %6 = alloca [4 x i8], align 1
  %7 = getelementptr inbounds [48 x i8], [48 x i8]* %5, i64 0, i64 0
  %8 = getelementptr inbounds %vfe_uplevel_rec_type, %vfe_uplevel_rec_type* %3, i64 0, i32 0
  store i8* null, i8** %8, align 8
  %9 = getelementptr inbounds %vfe_uplevel_rec_type, %vfe_uplevel_rec_type* %3, i64 0, i32 1
  %10 = bitcast %__DTRT_MYCONTAINED.uplevel_nested_type* %2 to i8*
  store i8* %10, i8** %9, align 8
  %11 = getelementptr inbounds %__DTRT_MYCONTAINED.uplevel_nested_type, %__DTRT_MYCONTAINED.uplevel_nested_type* %2, i64 0, i32 0
  %12 = load %"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* bitcast (%"QNCA_a0$i8*$rank2$"* @"var$7" to %"__DTRT_QNCA_a0$i32*$rank2$"*), align 16
  %13 = extractvalue %"__DTRT_QNCA_a0$i32*$rank2$" %12, 0
  %14 = getelementptr inbounds %"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* %11, i64 0, i32 0
  store i32* %13, i32** %14, align 8
  %15 = getelementptr inbounds %"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* %11, i64 0, i32 1
  %16 = getelementptr inbounds %"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* %11, i64 0, i32 2
  %17 = extractvalue %"__DTRT_QNCA_a0$i32*$rank2$" %12, 3
  %18 = getelementptr inbounds %"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* %11, i64 0, i32 3
  %19 = getelementptr inbounds %"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* %11, i64 0, i32 4
  %20 = getelementptr inbounds %"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* %11, i64 0, i32 5
  %21 = getelementptr inbounds [8 x i64], [8 x i64]* %1, i64 0, i64 0
  %22 = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 0, i64 0, i64 8, i64* elementtype(i64) nonnull %21, i64 2)
  %23 = ptrtoint [2 x i64]* %4 to i64
  store i64 %23, i64* %22, align 1
  %24 = getelementptr inbounds [2 x i64], [2 x i64]* %4, i64 0, i64 0
  %25 = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 0, i64 0, i64 8, i64* elementtype(i64) nonnull %24, i64 0)
  store volatile i64 0, i64* %25, align 1
  %26 = call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 0, i64 0, i64 8, i64* elementtype(i64) nonnull %24, i64 1)
  %27 = bitcast %vfe_uplevel_rec_type* %3 to i8*
  call void @llvm.init.trampoline(i8* nonnull %7, i8* bitcast (i32 (%vfe_uplevel_rec_type*)* @"_vfe$0000mycontained$BLK" to i8*), i8* nonnull %27)
  %28 = call i8* @llvm.adjust.trampoline(i8* nonnull %7)
  %29 = ptrtoint i8* %28 to i64
  store volatile i64 %29, i64* %26, align 1
  %30 = and i64 %17, 1030792151296
  store i64 0, i64* %20, align 8
  store i64 4, i64* %15, align 8
  store i64 2, i64* %19, align 8
  store i64 0, i64* %16, align 8
  %31 = getelementptr inbounds %"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* %11, i64 0, i32 6, i64 0
  %32 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %31, i64 0, i32 2
  %33 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %32, i32 0)
  store i64 1, i64* %33, align 1
  %34 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %31, i64 0, i32 0
  %35 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %34, i32 0)
  store i64 10, i64* %35, align 1
  %36 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %32, i32 1)
  store i64 1, i64* %36, align 1
  %37 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %34, i32 1)
  store i64 10, i64* %37, align 1
  %38 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %31, i64 0, i32 1
  %39 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %38, i32 0)
  store i64 4, i64* %39, align 1
  %40 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %38, i32 1)
  store i64 40, i64* %40, align 1
  %41 = or i64 %30, 1073741957
  store i64 %41, i64* %18, align 8
  %42 = lshr i64 %30, 15
  %43 = trunc i64 %42 to i32
  %44 = or i32 %43, 262146
  %45 = bitcast %__DTRT_MYCONTAINED.uplevel_nested_type* %2 to i8**
  %46 = call i32 @for_alloc_allocatable_handle(i64 400, i8** nonnull %45, i32 %44, i8* null) #6
  %47 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* @smmod_mp_myglobal_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %48 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* @smmod_mp_myglobal_, i64 0, i32 6, i64 0, i32 1), i32 1)
  %49 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* @smmod_mp_myglobal_, i64 0, i32 6, i64 0, i32 2), i32 1)
  br label %50

50:                                               ; preds = %70, %0
  %51 = phi i64 [ %71, %70 ], [ 1, %0 ]
  br label %52

52:                                               ; preds = %52, %50
  %53 = phi i64 [ %68, %52 ], [ 1, %50 ]
  %54 = add nuw nsw i64 %53, %51
  %55 = load i32*, i32** %14, align 8
  %56 = load i64, i64* %33, align 1
  %57 = load i64, i64* %40, align 1, !range !3
  %58 = load i64, i64* %36, align 1
  %59 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 %58, i64 %57, i32* elementtype(i32) %55, i64 %53)
  %60 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 %56, i64 4, i32* elementtype(i32) %59, i64 %51)
  %61 = trunc i64 %54 to i32
  store i32 %61, i32* %60, align 1
  %62 = load i32*, i32** getelementptr inbounds (%"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* @smmod_mp_myglobal_, i64 0, i32 0), align 8
  %63 = load i64, i64* %47, align 1
  %64 = load i64, i64* %48, align 1, !range !3
  %65 = load i64, i64* %49, align 1
  %66 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 %65, i64 %64, i32* elementtype(i32) %62, i64 %53)
  %67 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 %63, i64 4, i32* elementtype(i32) %66, i64 %51)
  store i32 %61, i32* %67, align 1
  %68 = add nuw nsw i64 %53, 1
  %69 = icmp eq i64 %68, 11
  br i1 %69, label %70, label %52

70:                                               ; preds = %52
  %71 = add nuw nsw i64 %51, 1
  %72 = icmp eq i64 %71, 11
  br i1 %72, label %73, label %50

73:                                               ; preds = %70
  %74 = load i32*, i32** %14, align 8
  %75 = load i64, i64* %33, align 1
  %76 = load i64, i64* %40, align 1, !range !3
  %77 = load i64, i64* %36, align 1
  %78 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 %77, i64 %76, i32* elementtype(i32) %74, i64 5)
  %79 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 %75, i64 4, i32* elementtype(i32) %78, i64 5)
  %80 = load i32, i32* %79, align 1
  %81 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 0
  store i8 9, i8* %81, align 1
  %82 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 1
  store i8 1, i8* %82, align 1
  %83 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 2
  store i8 1, i8* %83, align 1
  %84 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 3
  store i8 0, i8* %84, align 1
  %85 = alloca { i32 }, align 8
  %86 = getelementptr inbounds { i32 }, { i32 }* %85, i64 0, i32 0
  store i32 %80, i32* %86, align 8
  %87 = bitcast [8 x i64]* %1 to i8*
  %88 = bitcast { i32 }* %85 to i8*
  %89 = call i32 (i8*, i32, i64, i8*, i8*, i8*, ...) @for_write_seq_fmt(i8* nonnull %87, i32 6, i64 1239157112576, i8* nonnull %81, i8* nonnull %88, i8* getelementptr inbounds ([36 x i8], [36 x i8]* @"hasvfe_IP_mycontained$blk_$format_pack", i64 0, i64 0)) #6
  %90 = load i64, i64* %18, align 8
  %91 = and i64 %90, 1
  %92 = icmp eq i64 %91, 0
  br i1 %92, label %116, label %93

93:                                               ; preds = %73
  %94 = load i8*, i8** %45, align 8
  %95 = trunc i64 %90 to i32
  %96 = shl i32 %95, 1
  %97 = and i32 %96, 4
  %98 = lshr i64 %90, 3
  %99 = trunc i64 %98 to i32
  %100 = and i32 %99, 256
  %101 = lshr i64 %90, 15
  %102 = trunc i64 %101 to i32
  %103 = and i32 %102, 31457280
  %104 = and i32 %102, 33554432
  %105 = or i32 %100, %97
  %106 = or i32 %105, %103
  %107 = or i32 %106, %104
  %108 = or i32 %107, 262146
  %109 = load i64, i64* %20, align 8
  %110 = inttoptr i64 %109 to i8*
  %111 = call i32 @for_dealloc_allocatable_handle(i8* %94, i32 %108, i8* %110) #6
  %112 = icmp eq i32 %111, 0
  br i1 %112, label %113, label %116

113:                                              ; preds = %93
  store i32* null, i32** %14, align 8
  %114 = load i64, i64* %18, align 8
  %115 = and i64 %114, -2050
  store i64 %115, i64* %18, align 8
  br label %116

116:                                              ; preds = %113, %93, %73
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 %0, i64 %1, i64 %2, i64* elementtype(i64) %3, i64 %4) #2

; Function Attrs: mustprogress nofree nounwind willreturn
define internal i32 @"_vfe$0000mycontained$BLK"(%vfe_uplevel_rec_type* nest nocapture readonly %0) #3 {
  %2 = getelementptr inbounds %vfe_uplevel_rec_type, %vfe_uplevel_rec_type* %0, i64 0, i32 1
  %3 = bitcast i8** %2 to %__DTRT_MYCONTAINED.uplevel_nested_type**
  %4 = load %__DTRT_MYCONTAINED.uplevel_nested_type*, %__DTRT_MYCONTAINED.uplevel_nested_type** %3, align 1
  %5 = getelementptr inbounds %__DTRT_MYCONTAINED.uplevel_nested_type, %__DTRT_MYCONTAINED.uplevel_nested_type* %4, i64 0, i32 0
  %6 = getelementptr inbounds %"__DTRT_QNCA_a0$i32*$rank2$", %"__DTRT_QNCA_a0$i32*$rank2$"* %5, i64 0, i32 6, i64 0
  %7 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %6, i64 0, i32 2
  %8 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %7, i32 0)
  %9 = load volatile i64, i64* %8, align 1
  %10 = load volatile i64, i64* %8, align 1
  %11 = load volatile i64, i64* %8, align 1
  %12 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %6, i64 0, i32 0
  %13 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %12, i32 0)
  %14 = load volatile i64, i64* %13, align 1
  %15 = load volatile i64, i64* %13, align 1
  %16 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %7, i32 1)
  %17 = load volatile i64, i64* %16, align 1
  %18 = load volatile i64, i64* %16, align 1
  %19 = load volatile i64, i64* %16, align 1
  %20 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %12, i32 1)
  %21 = load volatile i64, i64* %20, align 1
  %22 = load volatile i64, i64* %20, align 1
  %23 = mul nsw i64 %22, %15
  %24 = trunc i64 %23 to i32
  ret i32 %24
}

; Function Attrs: argmemonly nounwind
declare void @llvm.init.trampoline(i8* nocapture %0, i8* %1, i8* %2) #4

; Function Attrs: argmemonly nofree nounwind readonly
declare i8* @llvm.adjust.trampoline(i8* %0) #5

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 %0, i64 %1, i64 %2, i32* elementtype(i32) %3, i64 %4) #2

; Function Attrs: nofree
declare dso_local i32 @for_write_seq_fmt(i8* %0, i32 %1, i64 %2, i8* %3, i8* %4, i8* %5, ...) local_unnamed_addr #1

; Function Attrs: nofree
declare dso_local i32 @for_dealloc_allocatable_handle(i8* nocapture readonly %0, i32 %1, i8* %2) local_unnamed_addr #1

attributes #0 = { nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { nofree nosync nounwind readnone speculatable }
attributes #3 = { mustprogress nofree nounwind willreturn "pre_loopopt" }
attributes #4 = { argmemonly nounwind }
attributes #5 = { argmemonly nofree nounwind readonly }
attributes #6 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
!3 = !{i64 1, i64 -9223372036854775808}
