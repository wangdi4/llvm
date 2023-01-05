; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts

; RUN: opt -opaque-pointers < %s -passes='module(ip-cloning)' -ip-specialization-cloning -debug-only=ipcloning -S  2>&1 | FileCheck %s

; Check that the arg sets appear as expected in the trace

; CHECK-LABEL: Cloning Analysis for:  convolutionalEncode
; CHECK: Selected Specialization cloning
; CHECK: Processing for Spe cloning  convolutionalEncode
; CHECK: Used in Loop
; CHECK: Used in Loop
; CHECK: Args sets collected
; CHECK: Inexact args sets found
; CHECK: Set_0
; CHECK-NEXT: position: 2 Value i16 2
; CHECK-NEXT: position: 3 Value i16 5
; CHECK-NEXT: position: 4 Value {{.*}} getelementptr
; CHECK-NEXT: Set_1
; CHECK-NEXT: position: 2 Value i16 2
; CHECK-NEXT: position: 3 Value i16 4
; CHECK-NEXT: position: 4 Value {{.*}} getelementptr
; CHECK-NEXT: Set_2
; CHECK-NEXT: position: 2 Value i16 2
; CHECK-NEXT: position: 3 Value i16 3
; CHECK-NEXT: position: 4 Value {{.*}} getelementptr

; Check that the clones were produced in the IR

; CHECK-LABEL: define internal i32 @t_run_test(
; CHECK: call void @convolutionalEncode.1(
; CHECK: call void @convolutionalEncode.2(
; CHECK: call void @convolutionalEncode.3(
; CHECK: call void @convolutionalEncode(
; CHECK: define internal void @convolutionalEncode(
; CHECK: define internal void @convolutionalEncode.1(
; CHECK: define internal void @convolutionalEncode.2(
; CHECK: define internal void @convolutionalEncode.3(

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.THTestResults = type { i64, i64, i16, i64, i64, i64, i64, ptr }

@t_run_test.info = internal global [64 x i8] zeroinitializer, align 16
@__const.t_run_test.CM_ONE = private unnamed_addr constant [3 x [2 x i8]] [[2 x i8] c"\01\01", [2 x i8] c"\01\00", [2 x i8] c"\01\01"], align 1
@__const.t_run_test.CM_THREE = private unnamed_addr constant [5 x [2 x i8]] [[2 x i8] c"\01\01", [2 x i8] c"\00\01", [2 x i8] c"\01\00", [2 x i8] c"\01\00", [2 x i8] c"\01\01"], align 1
@.str = private unnamed_addr constant [18 x i8] c"xk5r2diOutput.dat\00", align 1
@test_buf = internal unnamed_addr constant [1024 x i8] c"\00\00\01\01\00\01\01\00\00\01\00\01\01\01\00\00\00\01\00\00\01\00\00\00\00\01\01\01\00\01\01\01\01\01\01\00\00\00\01\00\01\00\01\01\00\01\01\01\00\00\01\01\01\00\00\00\01\00\00\01\01\00\00\00\00\00\01\00\01\01\00\01\00\01\00\01\01\01\00\00\00\01\00\00\01\00\00\00\00\01\01\01\00\01\01\01\01\01\01\00\00\00\01\00\01\00\01\01\00\01\01\01\00\00\01\01\01\00\00\00\01\00\00\01\01\00\00\00\00\00\01\00\01\01\00\01\00\01\00\01\01\01\00\00\00\01\00\00\01\00\00\00\00\01\01\01\00\01\01\01\01\01\01\00\00\00\01\00\01\00\01\01\00\01\01\01\00\00\01\01\01\00\00\00\01\00\00\01\01\00\00\00\00\00\01\00\01\01\00\01\00\01\00\01\01\01\00\00\00\01\00\00\01\00\00\00\00\01\01\01\00\01\01\01\01\01\01\00\00\00\01\00\01\00\01\01\00\01\01\01\00\00\01\01\01\00\00\00\01\00\00\01\01\00\00\00\00\00\01\00\01\01\00\01\00\01\00\01\01\01\00\00\00\01\00\00\01\00\00\00\00\01\01\01\00\01\01\01\01\01\01\00\00\00\01\00\01\00\01\01\00\01\01\01\00\00\01\01\01\00\00\00\01\00\00\01\01\00\00\00\00\00\01\00\01\01\00\01\00\01\00\01\01\01\00\00\00\01\00\00\01\00\00\00\00\01\01\01\00\01\01\01\01\01\01\00\00\00\01\00\01\00\01\01\00\01\01\01\00\00\01\01\01\00\00\00\01\00\00\01\01\00\00\00\00\00\01\00\01\01\00\01\00\01\00\01\01\01\00\00\00\01\00\00\01\00\00\00\00\01\01\01\00\01\01\01\01\01\01\00\00\00\01\00\01\00\01\01\00\01\01\01\00\00\01\01\01\00\00\00\01\00\00\01\01\00\00\00\00\00\01\00\01\01\00\01\00\01\00\01\01\01\00\00\00\01\00\00\01\00\00\00\00\01\01\01\00\01\01\01\01\01\01\00\00\00\01\00\01\00\01\01\00\01\01\01\00\00\01\01\01\00\00\00\01\00\00\01\01\00\00\00\00\00\01\00\01\01\00\01\00\01\00\01\01\01\00\00\00\01\00\00\01\00\00\00\00\01\01\01\00\01\01\01\01\01\01\00\00\00\01\00\01\00\01\01\00\01\01\01\00\00\01\01\01\00\00\00\01\00\00\01\01\00\00\00\00\00\01\00\01\01\00\01\00\01\00\01\01\01\00\00\00\01\00\00\01\00\00\00\00\01\01\01\00\01\01\01\01\01\01\00\00\00\01\00\01\00\01\01\00\01\01\01\00\00\01\01\01\00\00\00\01\00\00\01\01\00\00\00\00\00\01\00\01\01\00\01\00\01\00\01\01\01\00\00\00\01\00\00\01\00\00\00\00\01\01\01\00\01\01\01\01\01\01\00\00\00\01\00\01\00\01\01\00\01\01\01\00\00\01\01\01\00\00\00\01\00\00\01\01\00\00\00\00\00\01\00\01\01\00\01\00\01\00\01\01\01\00\00\00\01\00\00\01\00\00\00\00\01\01\01\00\01\01\01\01\01\01\00\00\00\01\00\01\00\01\01\00\01\01\01\00\00\01\01\01\00\00\00\01\00\00\01\01\00\00\00\00\00\01\00\01\01\00\01\00\01\00\01\01\01\00\00\00\01\00\00\01\00\00\00\00\01\01\01\00\01\01\01\01\01\01\00\00\00\01\00\01\00\01\01\00\01\01\01\00\00\01\01\01\00\00\00\01\00\00\01\01\00\00\00\00\00\01\00\01\01\00\01\00\01\00\01\01\01\00\00\00\01\00\00\01\00\00\00\00\01\01\01\00\01\01\01\01\01\01\00\00\00\01\00\01\00\01\01\00\01\01\01\00\00\01\01\01\00\00\00\01\00\00\01\01\00\00\00\00\00\01\00\01\01\00\01\00\01\00\01\01\01\00\00\00\01\00\00\01\00\00\00\00\01\01\01\00\01\01\01\01\01\01\00\00\00\01\00\01\00\01\01\00\01\01\01\00\00\01\01\01\00\00\00\01\00\00\01\01\00\00\00\00\00\01\00\01\01\00\01\00\01\00\01\01\01\00\00\00\01\00\00\01\00\00\00\00\01\01\01\00\01\01\01\01\01\01\00\00\00\01\00\01\00\01\01\00\01\01\01\00\00\01\01\01\00\00\00\01\00\00\01\01\00\00\00", align 16
@.str.1 = private unnamed_addr constant [17 x i8] c"conven00/bmark.c\00", align 1
@t_buf = internal unnamed_addr global ptr null, align 8
@.str.2 = private unnamed_addr constant [29 x i8] c"Cannot Allocate Memory %s:%d\00", align 1
@input_buf = internal global [512 x i8] c"\00\01\00\00\01\01\00\00\00\01\01\01\00\00\00\00\01\01\01\01\00\00\00\00\00\01\01\01\01\01\00\01\00\01\00\00\01\01\00\00\00\01\01\01\00\00\00\00\01\01\01\01\00\00\00\00\00\01\01\01\01\01\00\01\00\01\00\00\01\01\00\00\00\01\01\01\00\00\00\00\01\01\01\01\00\00\00\00\00\01\01\01\01\01\00\01\00\01\00\00\01\01\00\00\00\01\01\01\00\00\00\00\01\01\01\01\00\00\00\00\00\01\01\01\01\01\00\01\00\01\00\00\01\01\00\00\00\01\01\01\00\00\00\00\01\01\01\01\00\00\00\00\00\01\01\01\01\01\00\01\00\01\00\00\01\01\00\00\00\01\01\01\00\00\00\00\01\01\01\01\00\00\00\00\00\01\01\01\01\01\00\01\00\01\00\00\01\01\00\00\00\01\01\01\00\00\00\00\01\01\01\01\00\00\00\00\00\01\01\01\01\01\00\01\00\01\00\00\01\01\00\00\00\01\01\01\00\00\00\00\01\01\01\01\00\00\00\00\00\01\01\01\01\01\00\01\00\01\00\00\01\01\00\00\00\01\01\01\00\00\00\00\01\01\01\01\00\00\00\00\00\01\01\01\01\01\00\01\00\01\00\00\01\01\00\00\00\01\01\01\00\00\00\00\01\01\01\01\00\00\00\00\00\01\01\01\01\01\00\01\00\01\00\00\01\01\00\00\00\01\01\01\00\00\00\00\01\01\01\01\00\00\00\00\00\01\01\01\01\01\00\01\00\01\00\00\01\01\00\00\00\01\01\01\00\00\00\00\01\01\01\01\00\00\00\00\00\01\01\01\01\01\00\01\00\01\00\00\01\01\00\00\00\01\01\01\00\00\00\00\01\01\01\01\00\00\00\00\00\01\01\01\01\01\00\01\00\01\00\00\01\01\00\00\00\01\01\01\00\00\00\00\01\01\01\01\00\00\00\00\00\01\01\01\01\01\00\01\00\01\00\00\01\01\00\00\00\01\01\01\00\00\00\00\01\01\01\01\00\00\00\00\00\01\01\01\01\01\00\01\00\01\00\00\01\01\00\00\00\01\01\01\00\00\00\00\01\01\01\01\00\00\00\00\00\01\01\01\01\01\00\01", align 16
@.str.3 = private unnamed_addr constant [45 x i8] c"WARNING: Missing output filename  Using: %s\0A\00", align 1
@.str.4 = private unnamed_addr constant [49 x i8] c"WARNING: Cannot determine Code Index  Using: %d\0A\00", align 1
@.str.5 = private unnamed_addr constant [21 x i8] c"A note of basic info\00", align 1
@.str.6 = private unnamed_addr constant [48 x i8] c"%x: Failed at[%d] calculated(%d) != golden(%d)\0A\00", align 1

declare zeroext i16 @Calc_crc8(i8 noundef zeroext, i16 noundef zeroext)

declare void @th_exit(i32 noundef, ptr noundef, ...)

declare ptr @th_malloc_x(i64 noundef, ptr noundef, i32 noundef)

declare i32 @th_printf(ptr noundef, ...)

declare i32 @th_report_results(ptr noundef, i16 noundef zeroext)

declare i64 @th_signal_finished()

declare void @th_signal_start()

declare i32 @th_sprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ...)

define internal i32 @t_run_test(i64 noundef %arg, i32 noundef %arg1, ptr nocapture noundef readonly %arg2) {
bb:
  %i = alloca %struct.THTestResults, align 8
  %i3 = alloca [3 x [2 x i8]], align 1
  %i4 = alloca [4 x [2 x i8]], align 8
  %i5 = alloca [5 x [2 x i8]], align 1
  call void @llvm.lifetime.start.p0(i64 64, ptr nonnull %i)
  call void @llvm.lifetime.start.p0(i64 6, ptr nonnull %i3)
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 1 dereferenceable(6) %i3, ptr noundef nonnull align 1 dereferenceable(6) @__const.t_run_test.CM_ONE, i64 6, i1 false)
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i4)
  store i64 72340172821299457, ptr %i4, align 8
  call void @llvm.lifetime.start.p0(i64 10, ptr nonnull %i5)
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 1 dereferenceable(10) %i5, ptr noundef nonnull align 1 dereferenceable(10) @__const.t_run_test.CM_THREE, i64 10, i1 false)
  %i10 = tail call ptr @th_malloc_x(i64 noundef 1056, ptr noundef @.str.1, i32 noundef 240)
  store ptr %i10, ptr @t_buf, align 8, !tbaa !6
  %i11 = icmp eq ptr %i10, null
  br i1 %i11, label %bb12, label %bb14

bb12:                                             ; preds = %bb
  tail call void (i32, ptr, ...) @th_exit(i32 noundef 8, ptr noundef @.str.2, ptr noundef @.str.1, i32 noundef 243)
  %i13 = load ptr, ptr @t_buf, align 8, !tbaa !6
  br label %bb14

bb14:                                             ; preds = %bb12, %bb
  %i15 = phi ptr [ %i13, %bb12 ], [ %i10, %bb ]
  %i16 = getelementptr inbounds i8, ptr %i15, i64 512, !intel-tbaa !10
  %i18 = icmp slt i32 %arg1, 2
  br i1 %i18, label %bb19, label %bb22

bb19:                                             ; preds = %bb14
  %i20 = tail call i32 (ptr, ...) @th_printf(ptr noundef @.str.3, ptr noundef @.str)
  %i21 = tail call i32 (ptr, ...) @th_printf(ptr noundef @.str.4, i32 noundef 3)
  br label %bb40

bb22:                                             ; preds = %bb14
  %i23 = icmp eq i32 %arg1, 2
  br i1 %i23, label %bb30, label %bb24

bb24:                                             ; preds = %bb22
  %i25 = getelementptr inbounds ptr, ptr %arg2, i64 2
  %i26 = load ptr, ptr %i25, align 8, !tbaa !6
  %i27 = tail call i64 @strtol(ptr nocapture noundef nonnull %i26, ptr noundef null, i32 noundef 10)
  %i28 = trunc i64 %i27 to i16
  %i29 = icmp eq i16 %i28, 0
  br i1 %i29, label %bb30, label %bb32

bb30:                                             ; preds = %bb24, %bb22
  %i31 = tail call i32 (ptr, ...) @th_printf(ptr noundef @.str.4, i32 noundef 3)
  br label %bb40

bb32:                                             ; preds = %bb24
  %i33 = trunc i64 %i27 to i32
  %i34 = shl i32 %i33, 16
  %i35 = ashr exact i32 %i34, 16
  switch i32 %i35, label %bb42 [
    i32 1, label %bb36
    i32 2, label %bb38
    i32 3, label %bb40
  ]

bb36:                                             ; preds = %bb32
  %i37 = getelementptr inbounds [3 x [2 x i8]], ptr %i3, i64 0, i64 0
  br label %bb42

bb38:
  %i39 = getelementptr inbounds [4 x [2 x i8]], ptr %i4, i64 0, i64 0
  br label %bb42

bb40:                                             ; preds = %bb32, %bb30, %bb19
  %i41 = getelementptr inbounds [5 x [2 x i8]], ptr %i5, i64 0, i64 0
  br label %bb42

bb42:                                             ; preds = %bb40, %bb38, %bb36, %bb32
  %i43 = phi ptr [ %i16, %bb32 ], [ %i41, %bb40 ], [ %i39, %bb38 ], [ %i37, %bb36 ]
  %i44 = phi i1 [ true, %bb32 ], [ false, %bb40 ], [ false, %bb38 ], [ false, %bb36 ]
  %i45 = phi i16 [ 0, %bb32 ], [ 2, %bb40 ], [ 2, %bb38 ], [ 2, %bb36 ]
  %i46 = phi i16 [ 0, %bb32 ], [ 5, %bb40 ], [ 4, %bb38 ], [ 3, %bb36 ]
  tail call void @th_signal_start()
  %i47 = icmp eq i64 %arg, 0
  br i1 %i47, label %bb52, label %bb48

bb48:                                             ; preds = %bb48, %bb42
  %i49 = phi i64 [ %i50, %bb48 ], [ 0, %bb42 ]
  call void @convolutionalEncode(ptr noundef @input_buf, i16 noundef signext 512, i16 noundef signext %i45, i16 noundef signext %i46, ptr noundef %i43, ptr noundef %i15)
  %i50 = add nuw i64 %i49, 1
  %i51 = icmp eq i64 %i50, %arg
  br i1 %i51, label %bb52, label %bb48

bb52:                                             ; preds = %bb48, %bb42
  %i53 = call i64 @th_signal_finished()
  %i54 = getelementptr inbounds %struct.THTestResults, ptr %i, i64 0, i32 1, !intel-tbaa !11
  store i64 %i53, ptr %i54, align 8, !tbaa !11
  %i55 = getelementptr inbounds %struct.THTestResults, ptr %i, i64 0, i32 0, !intel-tbaa !15
  store i64 %arg, ptr %i55, align 8, !tbaa !15
  %i56 = getelementptr inbounds %struct.THTestResults, ptr %i, i64 0, i32 3, !intel-tbaa !16
  %i57 = getelementptr inbounds %struct.THTestResults, ptr %i, i64 0, i32 4, !intel-tbaa !17
  %i58 = getelementptr inbounds %struct.THTestResults, ptr %i, i64 0, i32 5, !intel-tbaa !18
  %i59 = getelementptr inbounds %struct.THTestResults, ptr %i, i64 0, i32 7, !intel-tbaa !19
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(32) %i56, i8 0, i64 32, i1 false)
  store ptr @t_run_test.info, ptr %i59, align 8, !tbaa !19
  %i61 = call i32 (ptr, ptr, ...) @th_sprintf(ptr noundef @t_run_test.info, ptr noundef @.str.5)
  %i62 = getelementptr inbounds %struct.THTestResults, ptr %i, i64 0, i32 2, !intel-tbaa !20
  store i16 0, ptr %i62, align 8, !tbaa !20
  br i1 %i44, label %bb91, label %bb63

bb63:                                             ; preds = %bb52
  %i64 = shl nuw nsw i16 %i45, 9
  %i65 = call i16 @llvm.umax.i16(i16 %i64, i16 1)
  %i66 = zext i16 %i65 to i64
  br label %bb67

bb67:                                             ; preds = %bb88, %bb63
  %i68 = phi i16 [ 0, %bb63 ], [ %i72, %bb88 ]
  %i69 = phi i64 [ 0, %bb63 ], [ %i89, %bb88 ]
  %i70 = getelementptr inbounds i8, ptr %i15, i64 %i69
  %i71 = load i8, ptr %i70, align 1, !tbaa !10
  %i72 = call zeroext i16 @Calc_crc8(i8 noundef zeroext %i71, i16 noundef zeroext %i68)
  store i16 %i72, ptr %i62, align 8, !tbaa !20
  %i73 = load i8, ptr %i70, align 1, !tbaa !10
  %i74 = getelementptr inbounds [1024 x i8], ptr @test_buf, i64 0, i64 %i69
  %i75 = load i8, ptr %i74, align 1, !tbaa !10
  %i76 = icmp eq i8 %i73, %i75
  br i1 %i76, label %bb88, label %bb77

bb77:                                             ; preds = %bb67
  %i78 = getelementptr inbounds i8, ptr %i15, i64 %i69
  %i79 = trunc i64 %i69 to i32
  %i80 = and i32 %i79, 65535
  %i81 = zext i8 %i75 to i32
  %i82 = zext i8 %i73 to i32
  %i83 = zext i16 %i72 to i32
  %i84 = call i32 (ptr, ...) @th_printf(ptr noundef @.str.6, i32 noundef %i83, i32 noundef %i80, i32 noundef %i82, i32 noundef %i81)
  store i64 %i69, ptr %i56, align 8, !tbaa !16
  %i85 = load i8, ptr %i78, align 1, !tbaa !10
  %i86 = zext i8 %i85 to i64
  store i64 %i86, ptr %i57, align 8, !tbaa !17
  %i87 = zext i8 %i75 to i64
  store i64 %i87, ptr %i58, align 8, !tbaa !18
  br label %bb91

bb88:                                             ; preds = %bb67
  %i89 = add nuw nsw i64 %i69, 1
  %i90 = icmp eq i64 %i89, %i66
  br i1 %i90, label %bb91, label %bb67

bb91:                                             ; preds = %bb88, %bb77, %bb52
  %i92 = call i32 @th_report_results(ptr noundef nonnull %i, i16 noundef zeroext -3568)
  call void @llvm.lifetime.end.p0(i64 10, ptr nonnull %i5)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i4)
  call void @llvm.lifetime.end.p0(i64 6, ptr nonnull %i3)
  call void @llvm.lifetime.end.p0(i64 64, ptr nonnull %i)
  ret i32 %i92
}

declare dso_local i64 @strtol(ptr noundef readonly, ptr nocapture noundef, i32 noundef) local_unnamed_addr

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i16 @llvm.umax.i16(i16, i16) #0

define internal void @convolutionalEncode(ptr nocapture noundef readonly %arg, i16 noundef signext %arg1, i16 noundef signext %arg2, i16 noundef signext %arg3, ptr nocapture noundef readonly %arg4, ptr nocapture noundef %arg5) {
bb:
  %i = alloca [8 x i8], align 1
  %i6 = getelementptr inbounds [8 x i8], ptr %i, i64 0, i64 0
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i6)
  %i7 = icmp sgt i16 %arg3, 0
  br i1 %i7, label %bb8, label %bb10

bb8:                                              ; preds = %bb
  %i9 = zext i16 %arg3 to i64
  call void @llvm.memset.p0.i64(ptr nonnull align 1 %i6, i8 0, i64 %i9, i1 false), !tbaa !21
  br label %bb10

bb10:                                             ; preds = %bb8, %bb
  %i11 = icmp sgt i16 %arg1, 0
  br i1 %i11, label %bb12, label %bb138

bb12:                                             ; preds = %bb10
  %i13 = add i16 %arg3, -1
  %i14 = icmp sgt i16 %i13, 0
  %i15 = icmp sgt i16 %arg2, 0
  br i1 %i15, label %bb16, label %bb116

bb16:                                             ; preds = %bb12
  br i1 %i7, label %bb17, label %bb66

bb17:                                             ; preds = %bb16
  %i18 = zext i16 %i13 to i64
  %i19 = zext i16 %arg1 to i64
  %i20 = sext i16 %arg2 to i64
  %i21 = sext i16 %arg3 to i64
  br label %bb22

bb22:                                             ; preds = %bb62, %bb17
  %i23 = phi i64 [ 0, %bb17 ], [ %i64, %bb62 ]
  %i24 = phi i16 [ 0, %bb17 ], [ %i63, %bb62 ]
  br i1 %i14, label %bb28, label %bb25

bb25:                                             ; preds = %bb28, %bb22
  %i26 = getelementptr inbounds i8, ptr %arg, i64 %i23
  %i27 = load i8, ptr %i26, align 1, !tbaa !10
  store i8 %i27, ptr %i6, align 1, !tbaa !21
  br label %bb39

bb28:                                             ; preds = %bb28, %bb22
  %i29 = phi i64 [ %i38, %bb28 ], [ %i18, %bb22 ]
  %i30 = phi i16 [ %i36, %bb28 ], [ %i13, %bb22 ]
  %i31 = add nuw nsw i64 %i29, 4294967295
  %i32 = and i64 %i31, 4294967295
  %i33 = getelementptr inbounds [8 x i8], ptr %i, i64 0, i64 %i32, !intel-tbaa !21
  %i34 = load i8, ptr %i33, align 1, !tbaa !21
  %i35 = getelementptr inbounds [8 x i8], ptr %i, i64 0, i64 %i29, !intel-tbaa !21
  store i8 %i34, ptr %i35, align 1, !tbaa !21
  %i36 = add nsw i16 %i30, -1
  %i37 = icmp eq i16 %i36, 0
  %i38 = add nsw i64 %i29, -1
  br i1 %i37, label %bb25, label %bb28

bb39:                                             ; preds = %bb58, %bb25
  %i40 = phi i64 [ %i56, %bb58 ], [ 0, %bb25 ]
  %i41 = phi i16 [ %i59, %bb58 ], [ %i24, %bb25 ]
  %i42 = sext i16 %i41 to i64
  %i43 = getelementptr inbounds i8, ptr %arg5, i64 %i42
  store i8 0, ptr %i43, align 1, !tbaa !10
  br label %bb44

bb44:                                             ; preds = %bb54, %bb39
  %i45 = phi i8 [ %i55, %bb54 ], [ 0, %bb39 ]
  %i46 = phi i64 [ %i56, %bb54 ], [ 0, %bb39 ]
  %i47 = getelementptr inbounds [2 x i8], ptr %arg4, i64 %i46, i64 %i40
  %i48 = load i8, ptr %i47, align 1, !tbaa !23
  %i49 = icmp eq i8 %i48, 0
  br i1 %i49, label %bb54, label %bb50

bb50:                                             ; preds = %bb44
  %i51 = getelementptr inbounds [8 x i8], ptr %i, i64 0, i64 %i46, !intel-tbaa !21
  %i52 = load i8, ptr %i51, align 1, !tbaa !21
  %i53 = xor i8 %i45, %i52
  store i8 %i53, ptr %i43, align 1, !tbaa !10
  br label %bb54

bb54:                                             ; preds = %bb50, %bb44
  %i55 = phi i8 [ %i53, %bb50 ], [ %i45, %bb44 ]
  %i56 = add nuw nsw i64 %i46, 1
  %i57 = icmp eq i64 %i56, %i21
  br i1 %i57, label %bb58, label %bb44

bb58:                                             ; preds = %bb54
  %i59 = add i16 %i41, 1
  %i60 = add nuw nsw i64 %i40, 1
  %i61 = icmp eq i64 %i60, %i20
  br i1 %i61, label %bb62, label %bb39

bb62:                                             ; preds = %bb58
  %i63 = add i16 %i24, %arg2
  %i64 = add nuw nsw i64 %i23, 1
  %i65 = icmp eq i64 %i64, %i19
  br i1 %i65, label %bb138, label %bb22

bb66:                                             ; preds = %bb16
  br i1 %i14, label %bb69, label %bb67

bb67:                                             ; preds = %bb66
  %i68 = zext i16 %arg1 to i64
  br label %bb101

bb69:                                             ; preds = %bb66
  %i70 = zext i16 %i13 to i64
  %i71 = zext i16 %arg1 to i64
  br label %bb72

bb72:                                             ; preds = %bb97, %bb69
  %i73 = phi i64 [ 0, %bb69 ], [ %i99, %bb97 ]
  %i74 = phi i16 [ 0, %bb69 ], [ %i98, %bb97 ]
  br label %bb83

bb75:                                             ; preds = %bb94, %bb75
  %i76 = phi i16 [ 0, %bb94 ], [ %i81, %bb75 ]
  %i77 = phi i16 [ %i74, %bb94 ], [ %i80, %bb75 ]
  %i78 = sext i16 %i77 to i64
  %i79 = getelementptr inbounds i8, ptr %arg5, i64 %i78
  store i8 0, ptr %i79, align 1, !tbaa !10
  %i80 = add i16 %i77, 1
  %i81 = add nuw nsw i16 %i76, 1
  %i82 = icmp eq i16 %i81, %arg2
  br i1 %i82, label %bb97, label %bb75

bb83:                                             ; preds = %bb83, %bb72
  %i84 = phi i64 [ %i70, %bb72 ], [ %i93, %bb83 ]
  %i85 = phi i16 [ %i13, %bb72 ], [ %i91, %bb83 ]
  %i86 = add nuw nsw i64 %i84, 4294967295
  %i87 = and i64 %i86, 4294967295
  %i88 = getelementptr inbounds [8 x i8], ptr %i, i64 0, i64 %i87, !intel-tbaa !21
  %i89 = load i8, ptr %i88, align 1, !tbaa !21
  %i90 = getelementptr inbounds [8 x i8], ptr %i, i64 0, i64 %i84, !intel-tbaa !21
  store i8 %i89, ptr %i90, align 1, !tbaa !21
  %i91 = add i16 %i85, -1
  %i92 = icmp sgt i16 %i91, 0
  %i93 = add nsw i64 %i84, -1
  br i1 %i92, label %bb83, label %bb94

bb94:                                             ; preds = %bb83
  %i95 = getelementptr inbounds i8, ptr %arg, i64 %i73
  %i96 = load i8, ptr %i95, align 1, !tbaa !10
  store i8 %i96, ptr %i6, align 1, !tbaa !21
  br label %bb75

bb97:                                             ; preds = %bb75
  %i98 = add i16 %i74, %arg2
  %i99 = add nuw nsw i64 %i73, 1
  %i100 = icmp eq i64 %i99, %i71
  br i1 %i100, label %bb138, label %bb72

bb101:                                            ; preds = %bb112, %bb67
  %i102 = phi i64 [ 0, %bb67 ], [ %i114, %bb112 ]
  %i103 = phi i16 [ 0, %bb67 ], [ %i113, %bb112 ]
  br label %bb104

bb104:                                            ; preds = %bb104, %bb101
  %i105 = phi i16 [ 0, %bb101 ], [ %i110, %bb104 ]
  %i106 = phi i16 [ %i103, %bb101 ], [ %i109, %bb104 ]
  %i107 = sext i16 %i106 to i64
  %i108 = getelementptr inbounds i8, ptr %arg5, i64 %i107
  store i8 0, ptr %i108, align 1, !tbaa !10
  %i109 = add i16 %i106, 1
  %i110 = add nuw nsw i16 %i105, 1
  %i111 = icmp eq i16 %i110, %arg2
  br i1 %i111, label %bb112, label %bb104

bb112:                                            ; preds = %bb104
  %i113 = add i16 %i103, %arg2
  %i114 = add nuw nsw i64 %i102, 1
  %i115 = icmp eq i64 %i114, %i68
  br i1 %i115, label %bb138, label %bb101

bb116:                                            ; preds = %bb12
  br i1 %i14, label %bb117, label %bb138

bb117:                                            ; preds = %bb116
  %i118 = zext i16 %i13 to i64
  %i119 = zext i16 %arg1 to i64
  br label %bb120

bb120:                                            ; preds = %bb133, %bb117
  %i121 = phi i64 [ 0, %bb117 ], [ %i136, %bb133 ]
  br label %bb122

bb122:                                            ; preds = %bb122, %bb120
  %i123 = phi i64 [ %i118, %bb120 ], [ %i132, %bb122 ]
  %i124 = phi i16 [ %i13, %bb120 ], [ %i130, %bb122 ]
  %i125 = add nuw nsw i64 %i123, 4294967295
  %i126 = and i64 %i125, 4294967295
  %i127 = getelementptr inbounds [8 x i8], ptr %i, i64 0, i64 %i126, !intel-tbaa !21
  %i128 = load i8, ptr %i127, align 1, !tbaa !21
  %i129 = getelementptr inbounds [8 x i8], ptr %i, i64 0, i64 %i123, !intel-tbaa !21
  store i8 %i128, ptr %i129, align 1, !tbaa !21
  %i130 = add i16 %i124, -1
  %i131 = icmp sgt i16 %i130, 0
  %i132 = add nsw i64 %i123, -1
  br i1 %i131, label %bb122, label %bb133

bb133:                                            ; preds = %bb122
  %i134 = getelementptr inbounds i8, ptr %arg, i64 %i121
  %i135 = load i8, ptr %i134, align 1, !tbaa !10
  store i8 %i135, ptr %i6, align 1, !tbaa !21
  %i136 = add nuw nsw i64 %i121, 1
  %i137 = icmp eq i64 %i136, %i119
  br i1 %i137, label %bb138, label %bb120

bb138:                                            ; preds = %bb133, %bb116, %bb112, %bb97, %bb62, %bb10
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i6)
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nocallback nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: argmemonly nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #3

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { argmemonly nocallback nofree nounwind willreturn }
attributes #3 = { argmemonly nocallback nofree nounwind willreturn writeonly }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{i32 1, !"LTOPostLink", i32 1}
!6 = !{!7, !7, i64 0}
!7 = !{!"pointer@_ZTSPc", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!8, !8, i64 0}
!11 = !{!12, !13, i64 8}
!12 = !{!"struct@THTestResults", !13, i64 0, !13, i64 8, !14, i64 16, !13, i64 24, !13, i64 32, !13, i64 40, !13, i64 48, !7, i64 56}
!13 = !{!"long", !8, i64 0}
!14 = !{!"short", !8, i64 0}
!15 = !{!12, !13, i64 0}
!16 = !{!12, !13, i64 24}
!17 = !{!12, !13, i64 32}
!18 = !{!12, !13, i64 40}
!19 = !{!12, !7, i64 56}
!20 = !{!12, !14, i64 16}
!21 = !{!22, !8, i64 0}
!22 = !{!"array@_ZTSA8_h", !8, i64 0}
!23 = !{!24, !8, i64 0}
!24 = !{!"array@_ZTSA2_h", !8, i64 0}

; end INTEL_FEATURE_SW_ADVANCED
