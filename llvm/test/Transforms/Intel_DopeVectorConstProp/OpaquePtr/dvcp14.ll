; RUN: opt < %s -opaque-pointers -S -dope-vector-local-const-prop=false -passes=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; Check that dope vector constants get propagated for uplevels #0 and #1 in
; new_solver_IP_specific_, a contained subroutine for new_solver_.

; This is the same test as dvcp04.ll, but checks the IR rather than the traces.

%uplevel_type = type { ptr, ptr }

@anon.9f612ed7d31cf3fc2b70611956e5ab37.0 = internal unnamed_addr constant i32 2
@"main_$PART" = internal global [3 x i32] zeroinitializer, align 16
@"main_$BLOCK" = internal global [3 x [3 x i32]] zeroinitializer, align 16

declare dso_local i32 @for_set_reentrancy(ptr) local_unnamed_addr

define internal void @new_solver_(ptr noalias %0, ptr noalias %1) #0 {
  %3 = alloca %uplevel_type, align 8
  %4 = getelementptr inbounds %uplevel_type, ptr %3, i64 0, i32 0
  store ptr %0, ptr %4, align 8
  %5 = getelementptr inbounds %uplevel_type, ptr %3, i64 0, i32 1
  store ptr %1, ptr %5, align 8
  %6 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %1, i64 0, i32 0
  %7 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %1, i64 0, i32 6, i64 0
  %8 = getelementptr inbounds { i64, i64, i64 }, ptr %7, i64 0, i32 0
  %9 = getelementptr inbounds { i64, i64, i64 }, ptr %7, i64 0, i32 1
  %10 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %0, i64 0, i32 0
  %11 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %0, i64 0, i32 6, i64 0
  %12 = getelementptr inbounds { i64, i64, i64 }, ptr %11, i64 0, i32 1
  call void @new_solver_IP_specific_(ptr nonnull %3)
  %13 = load ptr, ptr %6, align 8
  %14 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %9, i32 0)
  %15 = load i64, ptr %14, align 8
  %16 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %9, i32 1)
  %17 = load i64, ptr %16, align 8
  %18 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %8, i32 1)
  %19 = load i64, ptr %18, align 8
  %20 = load ptr, ptr %10, align 8
  %21 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %12, i32 0)
  %22 = load i64, ptr %21, align 8
  %23 = icmp slt i64 %19, 1  
; NOTE: Replace %19 with 3
; CHECK: %23 = icmp slt i64 3, 1
  br i1 %23, label %32, label %24

24:                                               ; preds = %24, %2
  %25 = phi i64 [ %30, %24 ], [ 1, %2 ]
  %26 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %22, ptr elementtype(i32) %20, i64 %25)
; NOTE: Replace %22 with 4
; CHECK: %26 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %20, i64 %25)
  %27 = load i32, ptr %26, align 4
  %28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %17, ptr elementtype(i32) %13, i64 %25)
; NOTE: Replace %17 with 12
; CHECK: %28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 12, ptr elementtype(i32) %13, i64 %25)
  %29 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %15, ptr elementtype(i32) %28, i64 1)
; NOTE: Replace %15 with 4
; CHECK: %29 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %28, i64 1)
  store i32 %27, ptr %29, align 4
  %30 = add nuw i64 %25, 1
  %31 = icmp eq i64 %25, %19
; NOTE: Replace %19 with 3
; CHECK: %31 = icmp eq i64 %25, 3
  br i1 %31, label %32, label %24

32:                                               ; preds = %24, %2
  ret void
}

define internal void @new_solver_IP_specific_(ptr nest noalias nocapture readonly %0) #0 {
  %2 = getelementptr inbounds %uplevel_type, ptr %0, i64 0, i32 0
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds %uplevel_type, ptr %0, i64 0, i32 1
  %5 = load ptr, ptr %4, align 8
  %6 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %3, i64 0, i32 0
  %7 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %3, i64 0, i32 6, i64 0
  %8 = getelementptr inbounds { i64, i64, i64 }, ptr %7, i64 0, i32 0
  %9 = getelementptr inbounds { i64, i64, i64 }, ptr %7, i64 0, i32 1
  %10 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 0
  %11 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 6, i64 0
  %12 = getelementptr inbounds { i64, i64, i64 }, ptr %11, i64 0, i32 0
  %13 = getelementptr inbounds { i64, i64, i64 }, ptr %11, i64 0, i32 1
  %14 = load ptr, ptr %6, align 8
  %15 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %9, i32 0)
  %16 = load i64, ptr %15, align 8
  %17 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %8, i32 0)
  %18 = load i64, ptr %17, align 8
  %19 = icmp slt i64 %18, 1
; NOTE: Replace %18 with 3
; CHECK: %19 = icmp slt i64 3, 1
  br i1 %19, label %36, label %20

20:                                               ; preds = %20, %1
  %21 = phi i64 [ %23, %20 ], [ 1, %1 ]
  %22 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %16, ptr elementtype(i32) %14, i64 %21)
; NOTE: Replace %16 with 4
; CHECK: %22 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %14, i64 %21)
  store i32 0, ptr %22, align 4
  %23 = add nuw i64 %21, 1
  %24 = icmp eq i64 %21, %18
; NOTE: Replace %18 with 3
; CHECK: %24 = icmp eq i64 %21, 3
  br i1 %24, label %36, label %20

25:                                               ; preds = %33, %25
  %26 = phi i64 [ 1, %33 ], [ %28, %25 ]
  %27 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %39, ptr elementtype(i32) %35, i64 %26)
; NOTE: Replace %39 with 4
; CHECK: %27 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %35, i64 %26)
  store i32 1, ptr %27, align 4
  %28 = add nuw i64 %26, 1
  %29 = icmp eq i64 %26, %41
; NOTE: Replace %41 with 3
; CHECK: %29 = icmp eq i64 %26, 3
  br i1 %29, label %30, label %25

30:                                               ; preds = %25
  %31 = add nuw i64 %34, 1
  %32 = icmp eq i64 %34, %45
; NOTE: Replace %45 with 3
; CHECK: %32 = icmp eq i64 %34, 3
  br i1 %32, label %49, label %33

33:                                               ; preds = %36, %30
  %34 = phi i64 [ %31, %30 ], [ 1, %36 ]
  %35 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %43, ptr elementtype(i32) %37, i64 %34)
; NOTE: Replace %43 with 12
; CHECK: %35 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 12, ptr elementtype(i32) %37, i64 %34)
  br label %25

36:                                               ; preds = %20, %1
  %37 = load ptr, ptr %10, align 8
  %38 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %13, i32 0)
  %39 = load i64, ptr %38, align 8
  %40 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %12, i32 0)
  %41 = load i64, ptr %40, align 8
  %42 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %13, i32 1)
  %43 = load i64, ptr %42, align 8
  %44 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %12, i32 1)
  %45 = load i64, ptr %44, align 8
  %46 = icmp slt i64 %45, 1
; NOTE: Replace %45 with 3
; CHECK: %46 = icmp slt i64 3, 1
   %47 = icmp slt i64 %41, 1
; NOTE: Replace %41 with 3
; CHECK: %47 = icmp slt i64 3, 1
   %48 = or i1 %46, %47
  br i1 %48, label %49, label %33

49:                                               ; preds = %36, %30
  ret void
}

define dso_local void @MAIN__() #0 {
  %1 = alloca { i32 }, align 8
  %2 = alloca [4 x i8], align 1
  %3 = alloca { i32 }, align 8
  %4 = alloca [4 x i8], align 1
  %5 = alloca { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, align 8
  %6 = alloca { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, align 8
  %7 = alloca [8 x i64], align 16
  %8 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 0
  %9 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 1
  %10 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 2
  %11 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 3
  %12 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 4
  %13 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %6, i64 0, i32 6, i64 0
  %14 = getelementptr inbounds { i64, i64, i64 }, ptr %13, i64 0, i32 0
  %15 = getelementptr inbounds { i64, i64, i64 }, ptr %13, i64 0, i32 1
  %16 = getelementptr inbounds { i64, i64, i64 }, ptr %13, i64 0, i32 2
  %17 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 0
  %18 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 1
  %19 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 2
  %20 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 3
  %21 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 4
  %22 = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, ptr %5, i64 0, i32 6, i64 0
  %23 = getelementptr inbounds { i64, i64, i64 }, ptr %22, i64 0, i32 0
  %24 = getelementptr inbounds { i64, i64, i64 }, ptr %22, i64 0, i32 1
  %25 = getelementptr inbounds { i64, i64, i64 }, ptr %22, i64 0, i32 2
  %26 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.9f612ed7d31cf3fc2b70611956e5ab37.0)
  store i64 4, ptr %9, align 8
  store i64 1, ptr %12, align 8
  store i64 0, ptr %10, align 8
  %27 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %15, i32 0)
  store i64 4, ptr %27, align 8
  %28 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %16, i32 0)
  store i64 1, ptr %28, align 8
  %29 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %14, i32 0)
  store i64 3, ptr %29, align 8
  store ptr @"main_$PART", ptr %8, align 8
  store i64 1, ptr %11, align 8
  store i64 4, ptr %18, align 8
  store i64 2, ptr %21, align 8
  store i64 0, ptr %19, align 8
  %30 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %24, i32 0)
  store i64 4, ptr %30, align 8
  %31 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %25, i32 0)
  store i64 1, ptr %31, align 8
  %32 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %23, i32 0)
  store i64 3, ptr %32, align 8
  %33 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %24, i32 1)
  store i64 12, ptr %33, align 8
  %34 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %25, i32 1)
  store i64 1, ptr %34, align 8
  %35 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %23, i32 1)
  store i64 3, ptr %35, align 8
  store ptr @"main_$BLOCK", ptr %17, align 8
  store i64 1, ptr %20, align 8
  call void @new_solver_(ptr nonnull %6, ptr nonnull %5)
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #1

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { "intel-lang"="fortran" }
attributes #1 = { nounwind readnone speculatable }
