; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe807 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-BEFORE
; Inline report via metadata
; RUN: opt -inlinereportsetup -inline-report=0xe886 < %s -S | opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe886 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-AFTER

; Check that no instances of @sw_IP_ddx_ and @sw_IP_ddy_ are inlined
; because the calls to each of @sw_IP_ddx_ and @sw_IP_ddy_ have no matching
; actual parameters.

%uplevel_type = type { i32, i32 }
%"QNCA_a0$double*$rank2$.3" = type { double*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

; CHECK-BEFORE: call{{.*}}sw_IP_ddx_
; CHECK-BEFORE: call{{.*}}sw_IP_ddy_
; CHECK-NOT: INLINE: sw_IP_ddx_{{.*}}Has inline budget for small application
; CHECK-NOT: INLINE: sw_IP_ddy_{{.*}}Has inline budget for small application
; CHECK-NOT: INLINE: sw_IP_ddx_{{.*}}Callee has single callsite and local linkage
; CHECK-NOT: INLINE: sw_IP_ddy_{{.*}}Callee has single callsite and local linkage
; CHECK-AFTER: call{{.*}}sw_IP_ddx_
; CHECK-AFTER: call{{.*}}sw_IP_ddy_

; Function Attrs: nofree nounwind uwtable
define internal fastcc void @sw_IP_ddx_(%uplevel_type* noalias nocapture %0, %"QNCA_a0$double*$rank2$.3"* noalias nocapture readonly dereferenceable(96) %1, %"QNCA_a0$double*$rank2$.3"* noalias nocapture readonly dereferenceable(96) "ptrnoalias" %2) unnamed_addr #0 {
  %4 = getelementptr inbounds %uplevel_type, %uplevel_type* %0, i64 0, i32 0
  %5 = getelementptr inbounds %uplevel_type, %uplevel_type* %0, i64 0, i32 1
  %6 = getelementptr inbounds %"QNCA_a0$double*$rank2$.3", %"QNCA_a0$double*$rank2$.3"* %2, i64 0, i32 0
  %7 = getelementptr inbounds %"QNCA_a0$double*$rank2$.3", %"QNCA_a0$double*$rank2$.3"* %2, i64 0, i32 6, i64 0, i32 0
  %8 = getelementptr inbounds %"QNCA_a0$double*$rank2$.3", %"QNCA_a0$double*$rank2$.3"* %2, i64 0, i32 6, i64 0, i32 1
  %9 = getelementptr inbounds %"QNCA_a0$double*$rank2$.3", %"QNCA_a0$double*$rank2$.3"* %1, i64 0, i32 0
  %10 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %7, i32 0)
  %11 = load i64, i64* %10, align 1
  %12 = icmp sgt i64 %11, 0
  %13 = select i1 %12, i64 %11, i64 0
  %14 = trunc i64 %13 to i32
  %15 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %7, i32 1)
  store i32 %14, i32* %4, align 1
  %16 = load i64, i64* %15, align 1
  %17 = icmp sgt i64 %16, 0
  %18 = select i1 %17, i64 %16, i64 0
  %19 = trunc i64 %18 to i32
  store i32 %19, i32* %5, align 1
  %20 = load double*, double** %9, align 1
  %21 = shl i64 %13, 32
  %22 = ashr exact i64 %21, 32
  %23 = ashr exact i64 %21, 29
  %24 = load double*, double** %6, align 1
  %25 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %8, i32 0)
  %26 = load i64, i64* %25, align 1
  %27 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %8, i32 1)
  %28 = load i64, i64* %27, align 1
  %29 = icmp slt i32 %19, 1
  br i1 %29, label %91, label %30

30:                                               ; preds = %3
  %31 = icmp slt i64 %22, 3
  br i1 %31, label %71, label %32

32:                                               ; preds = %30
  %33 = add nuw nsw i64 %22, 1
  %34 = shl i64 %18, 32
  %35 = ashr exact i64 %34, 32
  %36 = add nsw i64 %35, 1
  br label %54

37:                                               ; preds = %54, %37
  %38 = phi i64 [ 3, %54 ], [ %47, %37 ]
  %39 = phi i64 [ 2, %54 ], [ %48, %37 ]
  %40 = phi i64 [ 1, %54 ], [ %49, %37 ]
  %41 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %26, double* elementtype(double) %56, i64 %38)
  %42 = load double, double* %41, align 1
  %43 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %26, double* elementtype(double) %56, i64 %40)
  %44 = load double, double* %43, align 1
  %45 = fsub fast double %42, %44
  %46 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %57, i64 %39)
  store double %45, double* %46, align 1
  %47 = add nuw nsw i64 %38, 1
  %48 = add nuw nsw i64 %39, 1
  %49 = add nuw nsw i64 %40, 1
  %50 = icmp eq i64 %47, %33
  br i1 %50, label %51, label %37

51:                                               ; preds = %37
  %52 = add nuw nsw i64 %55, 1
  %53 = icmp eq i64 %52, %36
  br i1 %53, label %71, label %54

54:                                               ; preds = %51, %32
  %55 = phi i64 [ %52, %51 ], [ 1, %32 ]
  %56 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %28, double* elementtype(double) %24, i64 %55)
  %57 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %23, double* elementtype(double) %20, i64 %55)
  br label %37

58:                                               ; preds = %71, %58
  %59 = phi i64 [ %69, %58 ], [ 1, %71 ]
  %60 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %28, double* elementtype(double) %24, i64 %59)
  %61 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %26, double* elementtype(double) %60, i64 2)
  %62 = load double, double* %61, align 1
  %63 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %26, double* elementtype(double) %60, i64 1)
  %64 = load double, double* %63, align 1
  %65 = fsub fast double %62, %64
  %66 = fmul fast double %65, 2.000000e+00
  %67 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %23, double* elementtype(double) %20, i64 %59)
  %68 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %67, i64 1)
  store double %66, double* %68, align 1
  %69 = add nuw nsw i64 %59, 1
  %70 = icmp eq i64 %69, %74
  br i1 %70, label %88, label %58

71:                                               ; preds = %51, %30
  %72 = shl i64 %18, 32
  %73 = ashr exact i64 %72, 32
  %74 = add nsw i64 %73, 1
  br label %58

75:                                               ; preds = %88, %75
  %76 = phi i64 [ %86, %75 ], [ 1, %88 ]
  %77 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %28, double* elementtype(double) nonnull %24, i64 %76)
  %78 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %26, double* elementtype(double) %77, i64 %22)
  %79 = load double, double* %78, align 1
  %80 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %26, double* elementtype(double) %77, i64 %90)
  %81 = load double, double* %80, align 1
  %82 = fsub fast double %79, %81
  %83 = fmul fast double %82, 2.000000e+00
  %84 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %23, double* elementtype(double) nonnull %20, i64 %76)
  %85 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %84, i64 %22)
  store double %83, double* %85, align 1
  %86 = add nuw nsw i64 %76, 1
  %87 = icmp eq i64 %86, %74
  br i1 %87, label %91, label %75

88:                                               ; preds = %58
  %89 = add i64 %21, -4294967296
  %90 = ashr exact i64 %89, 32
  br label %75

91:                                               ; preds = %75, %3
  ret void
}

; Function Attrs: nofree nounwind uwtable
define internal fastcc void @sw_IP_ddy_(%uplevel_type* noalias nocapture %0, %"QNCA_a0$double*$rank2$.3"* noalias nocapture readonly dereferenceable(96) %1, %"QNCA_a0$double*$rank2$.3"* noalias nocapture readonly dereferenceable(96) "ptrnoalias" %2) unnamed_addr #0 {
  %4 = getelementptr inbounds %uplevel_type, %uplevel_type* %0, i64 0, i32 0
  %5 = getelementptr inbounds %uplevel_type, %uplevel_type* %0, i64 0, i32 1
  %6 = getelementptr inbounds %"QNCA_a0$double*$rank2$.3", %"QNCA_a0$double*$rank2$.3"* %2, i64 0, i32 0
  %7 = getelementptr inbounds %"QNCA_a0$double*$rank2$.3", %"QNCA_a0$double*$rank2$.3"* %2, i64 0, i32 6, i64 0, i32 0
  %8 = getelementptr inbounds %"QNCA_a0$double*$rank2$.3", %"QNCA_a0$double*$rank2$.3"* %2, i64 0, i32 6, i64 0, i32 1
  %9 = getelementptr inbounds %"QNCA_a0$double*$rank2$.3", %"QNCA_a0$double*$rank2$.3"* %1, i64 0, i32 0
  %10 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %7, i32 0)
  %11 = load i64, i64* %10, align 1
  %12 = icmp sgt i64 %11, 0
  %13 = select i1 %12, i64 %11, i64 0
  %14 = trunc i64 %13 to i32
  %15 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %7, i32 1)
  store i32 %14, i32* %4, align 1
  %16 = load i64, i64* %15, align 1
  %17 = icmp sgt i64 %16, 0
  %18 = select i1 %17, i64 %16, i64 0
  %19 = trunc i64 %18 to i32
  store i32 %19, i32* %5, align 1
  %20 = load double*, double** %9, align 1
  %21 = shl i64 %13, 32
  %22 = ashr exact i64 %21, 32
  %23 = ashr exact i64 %21, 29
  %24 = shl i64 %18, 32
  %25 = add i64 %24, -4294967296
  %26 = ashr exact i64 %25, 32
  %27 = load double*, double** %6, align 1
  %28 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %8, i32 0)
  %29 = load i64, i64* %28, align 1
  %30 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %8, i32 1)
  %31 = load i64, i64* %30, align 1
  %32 = icmp sgt i64 %26, 1
  br i1 %32, label %33, label %60

33:                                               ; preds = %3
  %34 = icmp slt i64 %22, 1
  br i1 %34, label %94, label %35

35:                                               ; preds = %33
  %36 = add nsw i64 %22, 1
  %37 = add nuw nsw i64 %26, 2
  br label %53

38:                                               ; preds = %53, %38
  %39 = phi i64 [ 1, %53 ], [ %46, %38 ]
  %40 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %29, double* elementtype(double) %57, i64 %39)
  %41 = load double, double* %40, align 1
  %42 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %29, double* elementtype(double) %58, i64 %39)
  %43 = load double, double* %42, align 1
  %44 = fsub fast double %41, %43
  %45 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %59, i64 %39)
  store double %44, double* %45, align 1
  %46 = add nuw nsw i64 %39, 1
  %47 = icmp eq i64 %46, %36
  br i1 %47, label %48, label %38

48:                                               ; preds = %38
  %49 = add nuw nsw i64 %54, 1
  %50 = add nuw nsw i64 %55, 1
  %51 = add nuw nsw i64 %56, 1
  %52 = icmp eq i64 %49, %37
  br i1 %52, label %60, label %53

53:                                               ; preds = %48, %35
  %54 = phi i64 [ %49, %48 ], [ 3, %35 ]
  %55 = phi i64 [ %50, %48 ], [ 2, %35 ]
  %56 = phi i64 [ %51, %48 ], [ 1, %35 ]
  %57 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %31, double* elementtype(double) %27, i64 %54)
  %58 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %31, double* elementtype(double) %27, i64 %56)
  %59 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %23, double* elementtype(double) %20, i64 %55)
  br label %38

60:                                               ; preds = %48, %3
  %61 = icmp slt i64 %22, 1
  br i1 %61, label %94, label %62

62:                                               ; preds = %60
  %63 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %31, double* elementtype(double) %27, i64 2)
  %64 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %31, double* elementtype(double) %27, i64 1)
  %65 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %23, double* elementtype(double) %20, i64 1)
  %66 = add nsw i64 %22, 1
  br label %67

67:                                               ; preds = %67, %62
  %68 = phi i64 [ 1, %62 ], [ %76, %67 ]
  %69 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %29, double* elementtype(double) %63, i64 %68)
  %70 = load double, double* %69, align 1
  %71 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %29, double* elementtype(double) %64, i64 %68)
  %72 = load double, double* %71, align 1
  %73 = fsub fast double %70, %72
  %74 = fmul fast double %73, 2.000000e+00
  %75 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %65, i64 %68)
  store double %74, double* %75, align 1
  %76 = add nuw nsw i64 %68, 1
  %77 = icmp eq i64 %76, %66
  br i1 %77, label %89, label %67

78:                                               ; preds = %89, %78
  %79 = phi i64 [ 1, %89 ], [ %87, %78 ]
  %80 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %29, double* elementtype(double) %91, i64 %79)
  %81 = load double, double* %80, align 1
  %82 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %29, double* elementtype(double) %92, i64 %79)
  %83 = load double, double* %82, align 1
  %84 = fsub fast double %81, %83
  %85 = fmul fast double %84, 2.000000e+00
  %86 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %93, i64 %79)
  store double %85, double* %86, align 1
  %87 = add nuw nsw i64 %79, 1
  %88 = icmp eq i64 %87, %66
  br i1 %88, label %94, label %78

89:                                               ; preds = %67
  %90 = ashr exact i64 %24, 32
  %91 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %31, double* elementtype(double) nonnull %27, i64 %90)
  %92 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %31, double* elementtype(double) nonnull %27, i64 %26)
  %93 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %23, double* elementtype(double) nonnull %20, i64 %90)
  br label %78

94:                                               ; preds = %78, %60, %33
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @MAIN__() local_unnamed_addr #0 {
entry:
  %t2 = alloca %uplevel_type, align 8
  %t3 = alloca %uplevel_type, align 8
  %t176 = alloca %"QNCA_a0$double*$rank2$.3", align 8
  %t177 = alloca %"QNCA_a0$double*$rank2$.3", align 8
  %t178 = alloca %"QNCA_a0$double*$rank2$.3", align 8
  %t179 = alloca %"QNCA_a0$double*$rank2$.3", align 8
  %t180 = alloca %"QNCA_a0$double*$rank2$.3", align 8
  %t181 = alloca %"QNCA_a0$double*$rank2$.3", align 8
  %t182 = alloca %"QNCA_a0$double*$rank2$.3", align 8
  %t183 = alloca %"QNCA_a0$double*$rank2$.3", align 8
  br label %L1161

L1161:                                             ; preds = %1461, %1104
  %t1164 = phi i32 [ 1, %entry ], [ %t1462, %L1161 ]
  call fastcc void @sw_IP_ddx_(%uplevel_type* nonnull %t2, %"QNCA_a0$double*$rank2$.3"* nonnull %t177, %"QNCA_a0$double*$rank2$.3"* nonnull %t176)
  call fastcc void @sw_IP_ddy_(%uplevel_type* nonnull %t2, %"QNCA_a0$double*$rank2$.3"* nonnull %t179, %"QNCA_a0$double*$rank2$.3"* nonnull %t178)
  call fastcc void @sw_IP_ddx_(%uplevel_type* nonnull %t3, %"QNCA_a0$double*$rank2$.3"* nonnull %t181, %"QNCA_a0$double*$rank2$.3"* nonnull %t180)
  call fastcc void @sw_IP_ddy_(%uplevel_type* nonnull %t3, %"QNCA_a0$double*$rank2$.3"* nonnull %t183, %"QNCA_a0$double*$rank2$.3"* nonnull %t182)
  %t1462 = add nuw nsw i32 %t1164, 1
  %t1463 = icmp eq i32 %t1462, 6481
  br i1 %t1463, label %L1464, label %L1161

L1464:
  ret void
}

declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 %0, i64 %1, i64 %2, double* %3, i64 %4)

declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 %0, i64 %1, i32 %2, i64* %3, i32 %4)

attributes #0 = { "intel-lang"="fortran" }

; end INTEL_FEATURE_SW_ADVANCED
