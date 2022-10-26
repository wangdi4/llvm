; REQUIRES: asserts
; RUN: opt < %s -disable-output -passes=dtrans-transpose -dtrans-transpose-print-candidates 2>&1 | FileCheck %s

; Check that neither physpropmod_mp_physprop_ field 0 nor field 2 is marked
; as a transpose candidate, because there is a mismatch between the bounds
; of the dope vector arrays in field 0.

; CHECK-NOT: Transpose candidate: physpropmod_mp_physprop_
; CHECK-NOT: Nested Field Number : 0

; CHECK-NOT: Transpose candidate: physpropmod_mp_physprop_
; CHECK-NOT: Nested Field Number : 2

; Check that the array through which the indirect subscripting is occurring is
; a candidate for transposing but not profitable.

; CHECK: Transpose candidate: main_$MYK
; CHECK: IsValid{{ *}}: true
; CHECK: IsProfitable{{ *}}: false

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" = type { %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"PHYSPROPMOD$.btPHYSPROP_TYPE" = type { %"QNCA_a0$float*$rank2$", i32, %"QNCA_a0$float*$rank2$" }
%"QNCA_a0$float*$rank2$" = type { float*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@physpropmod_mp_physprop_ = internal global %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$" { %"PHYSPROPMOD$.btPHYSPROP_TYPE"* null, i64 0, i64 0, i64 0, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@"main_$MYK" = internal unnamed_addr global [1000 x [19 x i32]] zeroinitializer, align 16
@anon.834d714ac9ab6fb7516b01a96e7925b0.0 = internal unnamed_addr constant i32 2

; Function Attrs: nofree noinline nounwind uwtable
define internal void @initfield0_(%"PHYSPROPMOD$.btPHYSPROP_TYPE"* noalias nocapture dereferenceable(200) %0) #0 {
  %2 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %0, i64 0, i32 0
  %3 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 3
  %4 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 5
  store i64 0, i64* %4, align 1
  %5 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 1
  store i64 4, i64* %5, align 1
  %6 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 4
  store i64 2, i64* %6, align 1
  %7 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 2
  store i64 0, i64* %7, align 1
  %8 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 6, i64 0
  %9 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %8, i64 0, i32 2
  %10 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %9, i32 0)
  store i64 1, i64* %10, align 1
  %11 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %8, i64 0, i32 0
  %12 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %11, i32 0)
  store i64 19, i64* %12, align 1
  %13 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %9, i32 1)
  store i64 1, i64* %13, align 1
  %14 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %11, i32 1)
  store i64 1000, i64* %14, align 1
  %15 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %8, i64 0, i32 1
  %16 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %15, i32 0)
  store i64 4, i64* %16, align 1
  %17 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %15, i32 1)
  store i64 76, i64* %17, align 1
  store i64 1073741829, i64* %3, align 1
  %18 = bitcast %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %0 to i8**
  %19 = tail call i32 @for_allocate_handle(i64 76000, i8** nonnull %18, i32 262144, i8* null) #5
  %20 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %0, i64 0, i32 2
  %21 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 3
  %22 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 5
  store i64 0, i64* %22, align 1
  %23 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 1
  store i64 4, i64* %23, align 1
  %24 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 4
  store i64 2, i64* %24, align 1
  %25 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 2
  store i64 0, i64* %25, align 1
  %26 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 6, i64 0
  %27 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %26, i64 0, i32 2
  %28 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %27, i32 0)
  store i64 1, i64* %28, align 1
  %29 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %26, i64 0, i32 0
  %30 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %29, i32 0)
  store i64 1000, i64* %30, align 1
  %31 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %27, i32 1)
  store i64 1, i64* %31, align 1
  %32 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %29, i32 1)
  store i64 19, i64* %32, align 1
  %33 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %26, i64 0, i32 1
  %34 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %33, i32 0)
  store i64 4, i64* %34, align 1
  %35 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %33, i32 1)
  store i64 4000, i64* %35, align 1
  store i64 1073741829, i64* %21, align 1
  %36 = bitcast %"QNCA_a0$float*$rank2$"* %20 to i8**
  %37 = tail call i32 @for_allocate_handle(i64 76000, i8** nonnull %36, i32 262144, i8* null) #5
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 %0, i64 %1, i32 %2, i64* elementtype(i64) %3, i32 %4) #1

; Function Attrs: nofree
declare dso_local i32 @for_allocate_handle(i64 %0, i8** nocapture %1, i32 %2, i8* %3) local_unnamed_addr #2

; Function Attrs: nofree noinline nounwind uwtable
define internal void @initfield1_(%"PHYSPROPMOD$.btPHYSPROP_TYPE"* noalias nocapture dereferenceable(200) %0) #0 {
  %2 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %0, i64 0, i32 2
  %3 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 3
  %4 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 5
  store i64 0, i64* %4, align 1
  %5 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 1
  store i64 4, i64* %5, align 1
  %6 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 4
  store i64 2, i64* %6, align 1
  %7 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 2
  store i64 0, i64* %7, align 1
  %8 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %2, i64 0, i32 6, i64 0
  %9 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %8, i64 0, i32 2
  %10 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %9, i32 0)
  store i64 1, i64* %10, align 1
  %11 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %8, i64 0, i32 0
  %12 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %11, i32 0)
  store i64 19, i64* %12, align 1
  %13 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %9, i32 1)
  store i64 1, i64* %13, align 1
  %14 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %11, i32 1)
  store i64 1000, i64* %14, align 1
  %15 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %8, i64 0, i32 1
  %16 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %15, i32 0)
  store i64 4, i64* %16, align 1
  %17 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %15, i32 1)
  store i64 76, i64* %17, align 1
  store i64 1073741829, i64* %3, align 1
  %18 = bitcast %"QNCA_a0$float*$rank2$"* %2 to i8**
  %19 = tail call i32 @for_allocate_handle(i64 76000, i8** nonnull %18, i32 262144, i8* null) #5
  %20 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %0, i64 0, i32 0
  %21 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 3
  %22 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 5
  store i64 0, i64* %22, align 1
  %23 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 1
  store i64 4, i64* %23, align 1
  %24 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 4
  store i64 2, i64* %24, align 1
  %25 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 2
  store i64 0, i64* %25, align 1
  %26 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %20, i64 0, i32 6, i64 0
  %27 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %26, i64 0, i32 2
  %28 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %27, i32 0)
  store i64 1, i64* %28, align 1
  %29 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %26, i64 0, i32 0
  %30 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %29, i32 0)
  store i64 19, i64* %30, align 1
  %31 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %27, i32 1)
  store i64 1, i64* %31, align 1
  %32 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %29, i32 1)
  store i64 1000, i64* %32, align 1
  %33 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %26, i64 0, i32 1
  %34 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %33, i32 0)
  store i64 4, i64* %34, align 1
  %35 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %33, i32 1)
  store i64 76, i64* %35, align 1
  store i64 1073741829, i64* %21, align 1
  %36 = bitcast %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %0 to i8**
  %37 = tail call i32 @for_allocate_handle(i64 76000, i8** nonnull %36, i32 262144, i8* null) #5
  ret void
}

; Function Attrs: nofree noinline nounwind uwtable
define internal void @initfield2_(%"PHYSPROPMOD$.btPHYSPROP_TYPE"* noalias nocapture dereferenceable(200) %0) #0 {
  tail call void @initfield1_(%"PHYSPROPMOD$.btPHYSPROP_TYPE"* nonnull %0)
  ret void
}

; Function Attrs: nofree nounwind uwtable
define dso_local void @MAIN__() #3 {
  %1 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.834d714ac9ab6fb7516b01a96e7925b0.0) #5
  br label %2

2:                                                ; preds = %11, %0
  %3 = phi i64 [ %12, %11 ], [ 1, %0 ]
  %4 = trunc i64 %3 to i32
  br label %5

5:                                                ; preds = %5, %2
  %6 = phi i64 [ %9, %5 ], [ 1, %2 ]
  %7 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 76, i32* elementtype(i32) getelementptr inbounds ([1000 x [19 x i32]], [1000 x [19 x i32]]* @"main_$MYK", i64 0, i64 0, i64 0), i64 %6)
  %8 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %7, i64 %3)
  store i32 %4, i32* %8, align 1
  %9 = add nuw nsw i64 %6, 1
  %10 = icmp eq i64 %9, 1001
  br i1 %10, label %11, label %5

11:                                               ; preds = %5
  %12 = add nuw nsw i64 %3, 1
  %13 = icmp eq i64 %12, 20
  br i1 %13, label %14, label %2

14:                                               ; preds = %11
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 5), align 8
  store i64 200, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 1), align 8
  store i64 1, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 4), align 8
  store i64 0, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 2), align 8
  %15 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 2), i32 0)
  store i64 1, i64* %15, align 1
  %16 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 0), i32 0)
  store i64 100, i64* %16, align 1
  %17 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 6, i64 0, i32 1), i32 0)
  store i64 200, i64* %17, align 1
  store i64 1073741829, i64* getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 3), align 8
  %18 = tail call i32 @for_allocate_handle(i64 20000, i8** bitcast (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_ to i8**), i32 262144, i8* null) #5
  br label %19

19:                                               ; preds = %19, %14
  %20 = phi i64 [ %29, %19 ], [ 1, %14 ]
  %21 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %22 = load i64, i64* %17, align 1
  %23 = load i64, i64* %15, align 1
  %24 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %23, i64 %22, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %21, i64 %20)
  tail call void @initfield0_(%"PHYSPROPMOD$.btPHYSPROP_TYPE"* %24)
  %25 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %26 = load i64, i64* %17, align 1
  %27 = load i64, i64* %15, align 1
  %28 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %27, i64 %26, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %25, i64 %20)
  tail call void @initfield2_(%"PHYSPROPMOD$.btPHYSPROP_TYPE"* %28)
  %29 = add nuw nsw i64 %20, 1
  %30 = icmp eq i64 %29, 101
  br i1 %30, label %31, label %19

31:                                               ; preds = %94, %19
  %32 = phi i64 [ %95, %94 ], [ 1, %19 ]
  br label %33

33:                                               ; preds = %91, %31
  %34 = phi i64 [ %92, %91 ], [ 1, %31 ]
  %35 = sub nsw i64 %32, %34
  %36 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 76, i32* elementtype(i32) getelementptr inbounds ([1000 x [19 x i32]], [1000 x [19 x i32]]* @"main_$MYK", i64 0, i64 0, i64 0), i64 %34)
  br label %37

37:                                               ; preds = %37, %33
  %38 = phi i64 [ %89, %37 ], [ 2, %33 ]
  %39 = add nsw i64 %35, %38
  %40 = trunc i64 %39 to i32
  %41 = sitofp i32 %40 to float
  %42 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %43 = load i64, i64* %17, align 1
  %44 = load i64, i64* %15, align 1
  %45 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %44, i64 %43, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %42, i64 101)
  %46 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %45, i64 0, i32 2
  %47 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %46, i64 0, i32 0
  %48 = load float*, float** %47, align 1
  %49 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %46, i64 0, i32 6, i64 0
  %50 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %49, i64 0, i32 1
  %51 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %50, i32 0)
  %52 = load i64, i64* %51, align 1
  %53 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %49, i64 0, i32 2
  %54 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %53, i32 0)
  %55 = load i64, i64* %54, align 1
  %56 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %36, i64 %38)
  %57 = load i32, i32* %56, align 1
  %58 = add nsw i32 %57, 1
  %59 = sext i32 %58 to i64
  %60 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %50, i32 1)
  %61 = load i64, i64* %60, align 1
  %62 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %53, i32 1)
  %63 = load i64, i64* %62, align 1
  %64 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %63, i64 %61, float* elementtype(float) %48, i64 %34)
  %65 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %55, i64 %52, float* elementtype(float) %64, i64 %59)
  store float %41, float* %65, align 1
  %66 = load %"PHYSPROPMOD$.btPHYSPROP_TYPE"*, %"PHYSPROPMOD$.btPHYSPROP_TYPE"** getelementptr inbounds (%"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$", %"QNCA_a0$%\22PHYSPROPMOD$.btPHYSPROP_TYPE\22*$rank1$"* @physpropmod_mp_physprop_, i64 0, i32 0), align 8
  %67 = load i64, i64* %17, align 1
  %68 = load i64, i64* %15, align 1
  %69 = tail call %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 0, i64 %68, i64 %67, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %66, i64 101)
  %70 = getelementptr inbounds %"PHYSPROPMOD$.btPHYSPROP_TYPE", %"PHYSPROPMOD$.btPHYSPROP_TYPE"* %69, i64 0, i32 0
  %71 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %70, i64 0, i32 0
  %72 = load float*, float** %71, align 1
  %73 = getelementptr inbounds %"QNCA_a0$float*$rank2$", %"QNCA_a0$float*$rank2$"* %70, i64 0, i32 6, i64 0
  %74 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %73, i64 0, i32 1
  %75 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %74, i32 0)
  %76 = load i64, i64* %75, align 1
  %77 = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %73, i64 0, i32 2
  %78 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %77, i32 0)
  %79 = load i64, i64* %78, align 1
  %80 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %74, i32 1)
  %81 = load i64, i64* %80, align 1
  %82 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %77, i32 1)
  %83 = load i64, i64* %82, align 1
  %84 = load i32, i32* %56, align 1
  %85 = add nsw i32 %84, -1
  %86 = sext i32 %85 to i64
  %87 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %83, i64 %81, float* elementtype(float) %72, i64 %86)
  %88 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %79, i64 %76, float* elementtype(float) %87, i64 %34)
  store float %41, float* %88, align 1
  %89 = add nuw nsw i64 %38, 1
  %90 = icmp eq i64 %89, 19
  br i1 %90, label %91, label %37

91:                                               ; preds = %37
  %92 = add nuw nsw i64 %34, 1
  %93 = icmp eq i64 %92, 20
  br i1 %93, label %94, label %33

94:                                               ; preds = %91
  %95 = add nuw nsw i64 %32, 1
  %96 = icmp eq i64 %95, 101
  br i1 %96, label %97, label %31

97:                                               ; preds = %94
  ret void
}

; Function Attrs: nofree
declare dso_local i32 @for_set_reentrancy(i32* nocapture readonly %0) local_unnamed_addr #2

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 %0, i64 %1, i64 %2, i32* elementtype(i32) %3, i64 %4) #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare %"PHYSPROPMOD$.btPHYSPROP_TYPE"* @"llvm.intel.subscript.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64.i64.p0s_PHYSPROPMOD$.btPHYSPROP_TYPEs.i64"(i8 %0, i64 %1, i64 %2, %"PHYSPROPMOD$.btPHYSPROP_TYPE"* elementtype(%"PHYSPROPMOD$.btPHYSPROP_TYPE") %3, i64 %4) #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 %0, i64 %1, i64 %2, float* elementtype(float) %3, i64 %4) #1

; Function Attrs: nofree nosync nounwind readnone willreturn mustprogress
declare i64 @llvm.ssa.copy.i64(i64 returned %0) #4

attributes #0 = { nofree noinline nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nofree nosync nounwind readnone speculatable }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #4 = { nofree nosync nounwind readnone willreturn mustprogress }
attributes #5 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
