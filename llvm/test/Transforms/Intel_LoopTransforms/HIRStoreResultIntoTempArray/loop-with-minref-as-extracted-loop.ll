; Check that we use the loop with minref to create the extracted loop when we do bulk loop carried scalar replacement
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-store-result-into-temp-array,print<hir>" -hir-create-function-level-region -disable-output 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Store Result Into Temp Array ***
;Function: jacobian_
;
;<0>          BEGIN REGION { }
;<13>                  %33 = 0.000000e+00;
;<179>              + DO i1 = 0, sext.i32.i64(%15) + -1, 1   <DO_LOOP>
;<181>              |   + DO i2 = 0, sext.i32.i64(%10) + -1, 1   <DO_LOOP>
;<23>               |   |   %41 = i2 + 3  %  %10;
;<182>              |   |
;<182>              |   |   + DO i3 = 0, sext.i32.i64(%9) + -1, 1   <DO_LOOP>
;<33>               |   |   |   %51 = i3 + 4  %  %9;
;<37>               |   |   |   %55 = (%0)[i1 + 2][i2 + 2][i3 + 2][0];
;<40>               |   |   |   %58 = (%0)[i1 + 2][i2 + 2][i3 + 2][1]  /  %55;
;<43>               |   |   |   %61 = (%0)[i1 + 2][i2 + 2][i3 + 2][2]  /  %55;
;<44>               |   |   |   %62 = %58  *  %58;
;<45>               |   |   |   %63 = %61  *  %61;
;<46>               |   |   |   %64 = %62  +  %63;
;<47>               |   |   |   %65 = %64  *  2.000000e+00;
;<48>               |   |   |   %66 = %65  /  %55;
;<49>               |   |   |   %67 = @llvm.pow.f64(%66,  2.250000e+00);
;<53>               |   |   |   %71 = (%0)[i1 + 2][%41][%51][0];
;<56>               |   |   |   %74 = (%0)[i1 + 2][%41][%51][1]  /  %71;
;<59>               |   |   |   %77 = (%0)[i1 + 2][%41][%51][2]  /  %71;
;<60>               |   |   |   %78 = %74  *  %74;
;<61>               |   |   |   %79 = %77  *  %77;
;<62>               |   |   |   %80 = %78  +  %79;
;<63>               |   |   |   %81 = %80  *  2.000000e+00;
;<64>               |   |   |   %82 = %81  /  %71;
;<65>               |   |   |   %83 = @llvm.pow.f64(%82,  2.250000e+00);
;<66>               |   |   |   %84 = %33  +  %67;
;<67>               |   |   |   %33 = %84  +  %83;
;<182>              |   |   + END LOOP
;<181>              |   + END LOOP
;<179>              + END LOOP
;<179>
;<183>
;<89>                  %98 = %33;
;<183>              + DO i1 = 0, sext.i32.i64(%15) + -1, 1   <DO_LOOP>
;<184>              |   + DO i2 = 0, sext.i32.i64(%10) + -1, 1   <DO_LOOP>
;<99>               |   |   %106 = i2 + 3  %  %10 + 1;
;<185>              |   |
;<185>              |   |   + DO i3 = 0, sext.i32.i64(%9) + -1, 1   <DO_LOOP>
;<109>              |   |   |   %116 = i3 + 4  %  %9 + 1;
;<113>              |   |   |   %120 = (%0)[i1 + 1][i2 + 2][i3 + 2][0];
;<116>              |   |   |   %123 = (%0)[i1 + 1][i2 + 2][i3 + 2][1]  /  %120;
;<119>              |   |   |   %126 = (%0)[i1 + 1][i2 + 2][i3 + 2][2]  /  %120;
;<120>              |   |   |   %127 = %123  *  %123;
;<121>              |   |   |   %128 = %126  *  %126;
;<122>              |   |   |   %129 = %127  +  %128;
;<123>              |   |   |   %130 = %129  *  2.000000e+00;
;<124>              |   |   |   %131 = %130  /  %120;
;<125>              |   |   |   %132 = @llvm.pow.f64(%131,  2.250000e+00);
;<129>              |   |   |   %136 = (%0)[i1 + 1][%106][%116][0];
;<132>              |   |   |   %139 = (%0)[i1 + 1][%106][%116][1]  /  %136;
;<135>              |   |   |   %142 = (%0)[i1 + 1][%106][%116][2]  /  %136;
;<136>              |   |   |   %143 = %139  *  %139;
;<137>              |   |   |   %144 = %142  *  %142;
;<138>              |   |   |   %145 = %143  +  %144;
;<139>              |   |   |   %146 = %145  *  2.000000e+00;
;<140>              |   |   |   %147 = %146  /  %136;
;<141>              |   |   |   %148 = @llvm.pow.f64(%147,  2.250000e+00);
;<142>              |   |   |   %149 = %98  +  %132;
;<143>              |   |   |   %98 = %149  +  %148;
;<185>              |   |   + END LOOP
;<184>              |   + END LOOP
;<183>              + END LOOP
;<164>                 (%7)[0][0] = 48;
;<166>                 (%7)[0][1] = 1;
;<168>                 (%7)[0][2] = 1;
;<170>                 (%7)[0][3] = 0;
;<172>                 (%8)[0].0 = %98;
;<175>                 %167 = @for_write_seq_lis(&((i8*)(%6)[0]),  -1,  1239157112576,  &((%7)[0][0]),  &((i8*)(%8)[0]));
;<183>
;<178>              ret ;
;<0>          END REGION
;
;*** IR Dump After HIR Store Result Into Temp Array ***
;Function: jacobian_
;
; CHECK:      BEGIN REGION { modified }
; CHECK:           %array_size = sext.i32.i64(%10) + 1  *  sext.i32.i64(%9) + 1;
; CHECK:           %array_size7 = sext.i32.i64(%15) + 1  *  %array_size;
; CHECK:           %TempArray = alloca %array_size7;
;
; CHECK:           + DO i1 = 0, sext.i32.i64(%15), 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, sext.i32.i64(%10), 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, sext.i32.i64(%9), 1   <DO_LOOP>
; CHECK:           |   |   |   %120 = (%0)[i1 + 1][i2 + 2][i3 + 2][0];
; CHECK:           |   |   |   %123 = (%0)[i1 + 1][i2 + 2][i3 + 2][1]  /  %120;
; CHECK:           |   |   |   %126 = (%0)[i1 + 1][i2 + 2][i3 + 2][2]  /  %120;
; CHECK:           |   |   |   %127 = %123  *  %123;
; CHECK:           |   |   |   %128 = %126  *  %126;
; CHECK:           |   |   |   %129 = %127  +  %128;
; CHECK:           |   |   |   %130 = %129  *  2.000000e+00;
; CHECK:           |   |   |   %131 = %130  /  %120;
; CHECK:           |   |   |   (%TempArray)[i1][i2][i3] = @llvm.pow.f64(%131,  2.250000e+00);
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
;
;
; CHECK:              %33 = 0.000000e+00;
; CHECK:           + DO i1 = 0, sext.i32.i64(%15) + -1, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, sext.i32.i64(%10) + -1, 1   <DO_LOOP>
; CHECK:           |   |   %41 = i2 + 3  %  %10;
; CHECK:           |   |
; CHECK:           |   |   + DO i3 = 0, sext.i32.i64(%9) + -1, 1   <DO_LOOP>
; CHECK:           |   |   |   %51 = i3 + 4  %  %9;
; CHECK:           |   |   |   %67 = (%TempArray)[i1 + 1][i2][i3];
; CHECK:           |   |   |   %83 = (%TempArray)[i1 + 1][zext.i32.i64(%41) + -2][zext.i32.i64(%51) + -2];
; CHECK:           |   |   |   %84 = %33  +  %67;
; CHECK:           |   |   |   %33 = %84  +  %83;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
;
;
; CHECK:              %98 = %33;
; CHECK:           + DO i1 = 0, sext.i32.i64(%15) + -1, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, sext.i32.i64(%10) + -1, 1   <DO_LOOP>
; CHECK:           |   |   %106 = i2 + 3  %  %10 + 1;
; CHECK:           |   |
; CHECK:           |   |   + DO i3 = 0, sext.i32.i64(%9) + -1, 1   <DO_LOOP>
; CHECK:           |   |   |   %116 = i3 + 4  %  %9 + 1;
; CHECK:           |   |   |   %132 = (%TempArray)[i1][i2][i3];
; CHECK:           |   |   |   %148 = (%TempArray)[i1][zext.i32.i64(%106) + -2][zext.i32.i64(%116) + -2];
; CHECK:           |   |   |   %149 = %98  +  %132;
; CHECK:           |   |   |   %98 = %149  +  %148;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @jacobian_(ptr noalias nocapture readonly dereferenceable(8) %0, ptr noalias nocapture readonly dereferenceable(4) %1, ptr noalias nocapture readonly dereferenceable(4) %2, ptr noalias nocapture readonly dereferenceable(4) %3, ptr noalias nocapture readonly dereferenceable(4) %4) local_unnamed_addr #0 {
  %6 = alloca [8 x i64], align 16
  %7 = alloca [4 x i8], align 1
  %8 = alloca { double }, align 8
  %9 = load i32, ptr %1, align 1
  %10 = load i32, ptr %2, align 1
  %11 = sext i32 %9 to i64
  %12 = mul nsw i64 %11, 40
  %13 = sext i32 %10 to i64
  %14 = mul nsw i64 %12, %13
  %15 = load i32, ptr %3, align 1
  %16 = icmp slt i32 %15, 1
  %17 = icmp slt i32 %10, 1
  %18 = or i1 %16, %17
  %19 = icmp slt i32 %9, 1
  %20 = or i1 %18, %19
  %21 = load i32, ptr %4, align 1
  %22 = icmp slt i32 %21, 1
  %23 = or i1 %20, %22
  br i1 %23, label %164, label %24

24:                                               ; preds = %5
  %25 = add nuw nsw i32 %9, 1
  %26 = add nuw nsw i32 %10, 1
  %27 = add nuw nsw i32 %15, 1
  %28 = sext i32 %27 to i64
  %29 = sext i32 %26 to i64
  %30 = sext i32 %25 to i64
  br label %31

31:                                               ; preds = %90, %24
  %32 = phi i64 [ 1, %24 ], [ %92, %90 ]
  %33 = phi double [ 0.000000e+00, %24 ], [ %91, %90 ]
  %34 = add nuw nsw i64 %32, 2
  %35 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %14, ptr elementtype(double) nonnull %0, i64 %34)
  br label %36

36:                                               ; preds = %87, %31
  %37 = phi i64 [ %42, %87 ], [ 1, %31 ]
  %38 = phi double [ %88, %87 ], [ %33, %31 ]
  %39 = trunc i64 %37 to i32
  %40 = add i32 %39, 2
  %41 = srem i32 %40, %10
  %42 = add nuw nsw i64 %37, 1
  %43 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %12, ptr elementtype(double) nonnull %35, i64 %42)
  %44 = zext i32 %41 to i64
  %45 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %12, ptr elementtype(double) nonnull %35, i64 %44)
  br label %46

46:                                               ; preds = %46, %36
  %47 = phi i64 [ 1, %36 ], [ %52, %46 ]
  %48 = phi double [ %38, %36 ], [ %85, %46 ]
  %49 = trunc i64 %47 to i32
  %50 = add i32 %49, 3
  %51 = srem i32 %50, %9
  %52 = add nuw nsw i64 %47, 1
  %53 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %43, i64 %52)
  %54 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %53, i64 1)
  %55 = load double, ptr %54, align 1
  %56 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %53, i64 2)
  %57 = load double, ptr %56, align 1
  %58 = fdiv reassoc ninf nsz arcp contract afn double %57, %55
  %59 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %53, i64 3)
  %60 = load double, ptr %59, align 1
  %61 = fdiv reassoc ninf nsz arcp contract afn double %60, %55
  %62 = fmul reassoc ninf nsz arcp contract afn double %58, %58
  %63 = fmul reassoc ninf nsz arcp contract afn double %61, %61
  %64 = fadd reassoc ninf nsz arcp contract afn double %62, %63
  %65 = fmul reassoc ninf nsz arcp contract afn double %64, 2.000000e+00
  %66 = fdiv reassoc ninf nsz arcp contract afn double %65, %55
  %67 = tail call reassoc ninf nsz arcp contract afn double @llvm.pow.f64(double %66, double 2.250000e+00)
  %68 = zext i32 %51 to i64
  %69 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %45, i64 %68)
  %70 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %69, i64 1)
  %71 = load double, ptr %70, align 1
  %72 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %69, i64 2)
  %73 = load double, ptr %72, align 1
  %74 = fdiv reassoc ninf nsz arcp contract afn double %73, %71
  %75 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %69, i64 3)
  %76 = load double, ptr %75, align 1
  %77 = fdiv reassoc ninf nsz arcp contract afn double %76, %71
  %78 = fmul reassoc ninf nsz arcp contract afn double %74, %74
  %79 = fmul reassoc ninf nsz arcp contract afn double %77, %77
  %80 = fadd reassoc ninf nsz arcp contract afn double %78, %79
  %81 = fmul reassoc ninf nsz arcp contract afn double %80, 2.000000e+00
  %82 = fdiv reassoc ninf nsz arcp contract afn double %81, %71
  %83 = tail call reassoc ninf nsz arcp contract afn double @llvm.pow.f64(double %82, double 2.250000e+00)
  %84 = fadd reassoc ninf nsz arcp contract afn double %48, %67
  %85 = fadd reassoc ninf nsz arcp contract afn double %84, %83
  %86 = icmp eq i64 %52, %30
  br i1 %86, label %87, label %46

87:                                               ; preds = %46
  %88 = phi double [ %85, %46 ]
  %89 = icmp eq i64 %42, %29
  br i1 %89, label %90, label %36

90:                                               ; preds = %87
  %91 = phi double [ %88, %87 ]
  %92 = add nuw nsw i64 %32, 1
  %93 = icmp eq i64 %92, %28
  br i1 %93, label %94, label %31

94:                                               ; preds = %90
  %95 = phi double [ %91, %90 ]
  br label %96

96:                                               ; preds = %94, %155
  %97 = phi i64 [ %99, %155 ], [ 1, %94 ]
  %98 = phi double [ %156, %155 ], [ %95, %94 ]
  %99 = add nuw nsw i64 %97, 1
  %100 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %14, ptr elementtype(double) nonnull %0, i64 %99)
  br label %101

101:                                              ; preds = %152, %96
  %102 = phi i64 [ %107, %152 ], [ 1, %96 ]
  %103 = phi double [ %153, %152 ], [ %98, %96 ]
  %104 = trunc i64 %102 to i32
  %105 = add i32 %104, 2
  %106 = srem i32 %105, %26
  %107 = add nuw nsw i64 %102, 1
  %108 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %12, ptr elementtype(double) nonnull %100, i64 %107)
  %109 = zext i32 %106 to i64
  %110 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %12, ptr elementtype(double) nonnull %100, i64 %109)
  br label %111

111:                                              ; preds = %111, %101
  %112 = phi i64 [ 1, %101 ], [ %117, %111 ]
  %113 = phi double [ %103, %101 ], [ %150, %111 ]
  %114 = trunc i64 %112 to i32
  %115 = add i32 %114, 3
  %116 = srem i32 %115, %25
  %117 = add nuw nsw i64 %112, 1
  %118 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %108, i64 %117)
  %119 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %118, i64 1)
  %120 = load double, ptr %119, align 1
  %121 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %118, i64 2)
  %122 = load double, ptr %121, align 1
  %123 = fdiv reassoc ninf nsz arcp contract afn double %122, %120
  %124 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %118, i64 3)
  %125 = load double, ptr %124, align 1
  %126 = fdiv reassoc ninf nsz arcp contract afn double %125, %120
  %127 = fmul reassoc ninf nsz arcp contract afn double %123, %123
  %128 = fmul reassoc ninf nsz arcp contract afn double %126, %126
  %129 = fadd reassoc ninf nsz arcp contract afn double %127, %128
  %130 = fmul reassoc ninf nsz arcp contract afn double %129, 2.000000e+00
  %131 = fdiv reassoc ninf nsz arcp contract afn double %130, %120
  %132 = tail call reassoc ninf nsz arcp contract afn double @llvm.pow.f64(double %131, double 2.250000e+00)
  %133 = zext i32 %116 to i64
  %134 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) nonnull %110, i64 %133)
  %135 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %134, i64 1)
  %136 = load double, ptr %135, align 1
  %137 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %134, i64 2)
  %138 = load double, ptr %137, align 1
  %139 = fdiv reassoc ninf nsz arcp contract afn double %138, %136
  %140 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %134, i64 3)
  %141 = load double, ptr %140, align 1
  %142 = fdiv reassoc ninf nsz arcp contract afn double %141, %136
  %143 = fmul reassoc ninf nsz arcp contract afn double %139, %139
  %144 = fmul reassoc ninf nsz arcp contract afn double %142, %142
  %145 = fadd reassoc ninf nsz arcp contract afn double %143, %144
  %146 = fmul reassoc ninf nsz arcp contract afn double %145, 2.000000e+00
  %147 = fdiv reassoc ninf nsz arcp contract afn double %146, %136
  %148 = tail call reassoc ninf nsz arcp contract afn double @llvm.pow.f64(double %147, double 2.250000e+00)
  %149 = fadd reassoc ninf nsz arcp contract afn double %113, %132
  %150 = fadd reassoc ninf nsz arcp contract afn double %149, %148
  %151 = icmp eq i64 %117, %30
  br i1 %151, label %152, label %111

152:                                              ; preds = %111
  %153 = phi double [ %150, %111 ]
  %154 = icmp eq i64 %107, %29
  br i1 %154, label %155, label %101

155:                                              ; preds = %152
  %156 = phi double [ %153, %152 ]
  %157 = icmp eq i64 %99, %28
  br i1 %157, label %158, label %96

158:                                              ; preds = %155
  %159 = phi double [ %156, %155 ]
  store i8 48, ptr %7, align 1
  %160 = getelementptr inbounds [4 x i8], ptr %7, i64 0, i64 1
  store i8 1, ptr %160, align 1
  %161 = getelementptr inbounds [4 x i8], ptr %7, i64 0, i64 2
  store i8 1, ptr %161, align 1
  %162 = getelementptr inbounds [4 x i8], ptr %7, i64 0, i64 3
  store i8 0, ptr %162, align 1
  store double %159, ptr %8, align 8
  %163 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %6, i32 -1, i64 1239157112576, ptr nonnull %7, ptr nonnull %8) #4
  br label %164

164:                                              ; preds = %158, %5
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare double @llvm.pow.f64(double, double) #2

; Function Attrs: nofree
declare i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #3

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
