; This test case checks def level for %TempArray and its blobs when the innermost loop's loop level is larger than 3
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-create-function-level-region -hir-store-result-into-temp-array -hir-details -print-after=hir-store-result-into-temp-array < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-store-result-into-temp-array,print<hir>" -hir-create-function-level-region -hir-details 2>&1 < %s | FileCheck %s
;
;*** IR Dump After HIR Store Result Into Temp Array ***
;Function: jacobian_
;
; CHECK:    BEGIN REGION { modified }
; CHECK:           + DO i32 i1 = 0, %21 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:           |   %call = @llvm.stacksave();
; CHECK:           |   %array_size = sext.i32.i64(%10) + 1  *  sext.i32.i64(%9) + 1;
; CHECK:           |   %array_size5 = sext.i32.i64(%15) + 1  *  %array_size;
; CHECK:           |   %TempArray = alloca %array_size5;
; CHECK:           |   <LVAL-REG> NON-LINEAR double* %TempArray {sb:99}
; CHECK:           |   <RVAL-REG> NON-LINEAR i64 %array_size5 {sb:98}
;                  |
; CHECK:           |   + DO i64 i2 = 0, sext.i32.i64(%15), 1   <DO_LOOP>
; CHECK:           |   |   + DO i64 i3 = 0, sext.i32.i64(%10), 1   <DO_LOOP>
; CHECK:           |   |   |   + DO i64 i4 = 0, sext.i32.i64(%9), 1   <DO_LOOP>
; CHECK:           |   |   |   |   %61 = (%0)[i2 + 1][i3 + 2][i4 + 2][0];
; CHECK:           |   |   |   |   %64 = (%0)[i2 + 1][i3 + 2][i4 + 2][1]  /  %61;
; CHECK:           |   |   |   |   %67 = (%0)[i2 + 1][i3 + 2][i4 + 2][2]  /  %61;
; CHECK:           |   |   |   |   %68 = %64  *  %64;
; CHECK:           |   |   |   |   %69 = %67  *  %67;
; CHECK:           |   |   |   |   %70 = %68  +  %69;
; CHECK:           |   |   |   |   %71 = %70  *  2.000000e+00;
; CHECK:           |   |   |   |   %72 = %71  /  %61;
; CHECK:           |   |   |   |   (%TempArray)[i2][i3][i4] = @llvm.pow.f64(%72,  2.250000e+00);
; CHECK:           |   |   |   |   <LVAL-REG> (LINEAR double* %TempArray{def@1})[LINEAR i64 i2][LINEAR i64 i3][LINEAR i64 i4] inbounds  {sb:100}
; CHECK:           |   |   |   |      <BLOB> LINEAR double* %TempArray{def@1} {sb:99}
; CHECK:           |   |   |   |      <BLOB> LINEAR i32 %10 {sb:25}
; CHECK:           |   |   |   |      <BLOB> LINEAR i32 %9 {sb:34}
; CHECK:           |   |   |   |   <RVAL-REG> NON-LINEAR double %72 {sb:53}
; CHECK:           |   |   |   + END LOOP
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
;                  |
; CHECK:           |   + DO i64 i2 = 0, sext.i32.i64(%15) + -1, 1   <DO_LOOP>
; CHECK:           |   |   + DO i64 i3 = 0, sext.i32.i64(%10) + -1, 1   <DO_LOOP>
; CHECK:           |   |   |   %47 = i3 + 3  %  %10;
; CHECK:           |   |   |   + DO i64 i4 = 0, sext.i32.i64(%9) + -1, 1   <DO_LOOP>
; CHECK:           |   |   |   |   %57 = i4 + 4  %  %9;
; CHECK:           |   |   |   |   %73 = (%TempArray)[i2][i3][i4];
; CHECK:           |   |   |   |   <LVAL-REG> NON-LINEAR double %73 {sb:54}
; CHECK:           |   |   |   |   <RVAL-REG> (LINEAR double* %TempArray{def@1})[LINEAR i64 i2][LINEAR i64 i3][LINEAR i64 i4] inbounds  {sb:101}
; CHECK:           |   |   |   |      <BLOB> LINEAR double* %TempArray{def@1} {sb:99}
; CHECK:           |   |   |   |      <BLOB> LINEAR i32 %10 {sb:25}
; CHECK:           |   |   |   |      <BLOB> LINEAR i32 %9 {sb:34}
;                  |   |   |   |
; CHECK:           |   |   |   |   %89 = (%TempArray)[i2 + 1][zext.i32.i64(%47) + -2][zext.i32.i64(%57) + -2];
; CHECK:           |   |   |   |   <LVAL-REG> NON-LINEAR double %89 {sb:70}
; CHECK:           |   |   |   |   <RVAL-REG> (LINEAR double* %TempArray{def@1})[LINEAR i64 i2 + 1][LINEAR i64 zext.i32.i64(%47) + -2{def@3}][NON-LINEAR i64 zext.i32.i64(%57) + -2] inbounds  {sb:101}
; CHECK:           |   |   |   |      <BLOB> LINEAR double* %TempArray{def@1} {sb:99}
; CHECK:           |   |   |   |      <BLOB> LINEAR i32 %10 {sb:25}
; CHECK:           |   |   |   |      <BLOB> LINEAR i32 %9 {sb:34}
; CHECK:           |   |   |   |      <BLOB> NON-LINEAR i32 %57 {sb:37}
; CHECK:           |   |   |   |      <BLOB> LINEAR i32 %47{def@3} {sb:28}
;                  |   |   |   |
; CHECK:           |   |   |   |   %90 = %34  +  %73;
; CHECK:           |   |   |   |   %34 = %90  +  %89;
; CHECK:           |   |   |   + END LOOP
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
;                  |
; CHECK:           |   @llvm.stackrestore(&((%call)[0]));
;                  |
; CHECK:           + END LOOP
;
; CHECK:           ret ;
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 'ld-temp.o'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @jacobian_(double* noalias nocapture readonly dereferenceable(8) %0, i32* noalias nocapture readonly dereferenceable(4) %1, i32* noalias nocapture readonly dereferenceable(4) %2, i32* noalias nocapture readonly dereferenceable(4) %3, i32* noalias nocapture readonly dereferenceable(4) %4) local_unnamed_addr #0 {
  %6 = alloca [8 x i64], align 16
  %7 = alloca [4 x i8], align 1
  %8 = alloca { double }, align 8
  %9 = load i32, i32* %1, align 1
  %10 = load i32, i32* %2, align 1
  %11 = sext i32 %9 to i64
  %12 = mul nsw i64 %11, 40
  %13 = sext i32 %10 to i64
  %14 = mul nsw i64 %12, %13
  %15 = load i32, i32* %3, align 1
  %16 = icmp slt i32 %15, 1
  %17 = icmp slt i32 %10, 1
  %18 = or i1 %16, %17
  %19 = icmp slt i32 %9, 1
  %20 = or i1 %18, %19
  %21 = load i32, i32* %4, align 1
  %22 = icmp slt i32 %21, 1
  %23 = or i1 %20, %22
  br i1 %23, label %113, label %24

24:                                               ; preds = %5
  %25 = add nuw nsw i32 %9, 1
  %26 = add nuw nsw i32 %10, 1
  %27 = add nuw nsw i32 %15, 1
  %28 = add nuw nsw i32 %21, 1
  %29 = sext i32 %27 to i64
  %30 = sext i32 %26 to i64
  %31 = sext i32 %25 to i64
  br label %32

32:                                               ; preds = %99, %24
  %33 = phi i32 [ %101, %99 ], [ 1, %24 ]
  %34 = phi double [ %100, %99 ], [ 0.000000e+00, %24 ]
  br label %35

35:                                               ; preds = %96, %32
  %36 = phi i64 [ %38, %96 ], [ 1, %32 ]
  %37 = phi double [ %97, %96 ], [ %34, %32 ]
  %38 = add nuw nsw i64 %36, 1
  %39 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %14, double* elementtype(double) nonnull %0, i64 %38)
  %40 = add nuw nsw i64 %36, 2
  %41 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %14, double* elementtype(double) nonnull %0, i64 %40)
  br label %42

42:                                               ; preds = %93, %35
  %43 = phi i64 [ %48, %93 ], [ 1, %35 ]
  %44 = phi double [ %94, %93 ], [ %37, %35 ]
  %45 = trunc i64 %43 to i32
  %46 = add i32 %45, 2
  %47 = srem i32 %46, %10
  %48 = add nuw nsw i64 %43, 1
  %49 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %12, double* elementtype(double) nonnull %39, i64 %48)
  %50 = zext i32 %47 to i64
  %51 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %12, double* elementtype(double) nonnull %41, i64 %50)
  br label %52

52:                                               ; preds = %52, %42
  %53 = phi i64 [ 1, %42 ], [ %58, %52 ]
  %54 = phi double [ %44, %42 ], [ %91, %52 ]
  %55 = trunc i64 %53 to i32
  %56 = add i32 %55, 3
  %57 = srem i32 %56, %9
  %58 = add nuw nsw i64 %53, 1
  %59 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 40, double* elementtype(double) nonnull %49, i64 %58)
  %60 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %59, i64 1)
  %61 = load double, double* %60, align 1
  %62 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %59, i64 2)
  %63 = load double, double* %62, align 1
  %64 = fdiv reassoc ninf nsz arcp contract afn double %63, %61
  %65 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %59, i64 3)
  %66 = load double, double* %65, align 1
  %67 = fdiv reassoc ninf nsz arcp contract afn double %66, %61
  %68 = fmul reassoc ninf nsz arcp contract afn double %64, %64
  %69 = fmul reassoc ninf nsz arcp contract afn double %67, %67
  %70 = fadd reassoc ninf nsz arcp contract afn double %68, %69
  %71 = fmul reassoc ninf nsz arcp contract afn double %70, 2.000000e+00
  %72 = fdiv reassoc ninf nsz arcp contract afn double %71, %61
  %73 = tail call reassoc ninf nsz arcp contract afn double @llvm.pow.f64(double %72, double 2.250000e+00)
  %74 = zext i32 %57 to i64
  %75 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 40, double* elementtype(double) nonnull %51, i64 %74)
  %76 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %75, i64 1)
  %77 = load double, double* %76, align 1
  %78 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %75, i64 2)
  %79 = load double, double* %78, align 1
  %80 = fdiv reassoc ninf nsz arcp contract afn double %79, %77
  %81 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %75, i64 3)
  %82 = load double, double* %81, align 1
  %83 = fdiv reassoc ninf nsz arcp contract afn double %82, %77
  %84 = fmul reassoc ninf nsz arcp contract afn double %80, %80
  %85 = fmul reassoc ninf nsz arcp contract afn double %83, %83
  %86 = fadd reassoc ninf nsz arcp contract afn double %84, %85
  %87 = fmul reassoc ninf nsz arcp contract afn double %86, 2.000000e+00
  %88 = fdiv reassoc ninf nsz arcp contract afn double %87, %77
  %89 = tail call reassoc ninf nsz arcp contract afn double @llvm.pow.f64(double %88, double 2.250000e+00)
  %90 = fadd reassoc ninf nsz arcp contract afn double %54, %73
  %91 = fadd reassoc ninf nsz arcp contract afn double %90, %89
  %92 = icmp eq i64 %58, %31
  br i1 %92, label %93, label %52

93:                                               ; preds = %52
  %94 = phi double [ %91, %52 ]
  %95 = icmp eq i64 %48, %30
  br i1 %95, label %96, label %42

96:                                               ; preds = %93
  %97 = phi double [ %94, %93 ]
  %98 = icmp eq i64 %38, %29
  br i1 %98, label %99, label %35

99:                                               ; preds = %96
  %100 = phi double [ %97, %96 ]
  %101 = add nuw nsw i32 %33, 1
  %102 = icmp eq i32 %101, %28
  br i1 %102, label %103, label %32

103:                                              ; preds = %99
  %104 = phi double [ %100, %99 ]
  %105 = getelementptr inbounds [4 x i8], [4 x i8]* %7, i64 0, i64 0
  store i8 48, i8* %105, align 1
  %106 = getelementptr inbounds [4 x i8], [4 x i8]* %7, i64 0, i64 1
  store i8 1, i8* %106, align 1
  %107 = getelementptr inbounds [4 x i8], [4 x i8]* %7, i64 0, i64 2
  store i8 1, i8* %107, align 1
  %108 = getelementptr inbounds [4 x i8], [4 x i8]* %7, i64 0, i64 3
  store i8 0, i8* %108, align 1
  %109 = getelementptr inbounds { double }, { double }* %8, i64 0, i32 0
  store double %104, double* %109, align 8
  %110 = bitcast [8 x i64]* %6 to i8*
  %111 = bitcast { double }* %8 to i8*
  %112 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %110, i32 -1, i64 1239157112576, i8* nonnull %105, i8* nonnull %111) #4
  br label %113

113:                                              ; preds = %103, %5
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare double @llvm.pow.f64(double, double) #2

; Function Attrs: nofree
declare i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr #3

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { nofree "intel-lang"="fortran" }
attributes #4 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
