; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-temp-array-transpose"  -print-after=hir-temp-array-transpose -disable-output 2>&1 | FileCheck %s


; Check that we transpose the array with following dims: [i4][i3 + sext.i32.i64(%tmp1884) + -1].
; Even though i3 is not standalone and %tmp1884 is non-linear, the dimsizes are constant,
; which allows us to copy the contents of the array and use the transposed copy.

; Note that there are 3 uses in 3 loops where we reuse the copied transpose temparray.

; Before Transformation
;    BEGIN REGION { }
;          + DO i1 = 0, zext.i32.i64(%tmp1866) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 150>  <LEGAL_MAX_TC = 2147483647>
;          |      %tmp1884 = 1;
;          |      %tmp1885 = 0;
;          |   + DO i2 = 0, sext.i32.i64(%tmp1871) + -1, 1   <DO_LOOP>
;          |   |   %tmp1885.out = %tmp1885;
;          |   |   %tmp1887 = (bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523600) to i32*))[i2];
;          |   |   %tmp1885 = %tmp1887 + %tmp1885  +  1;
;          |   |   if (%tmp1885 >= %tmp1884)
;          |   |   {
;          |   |      %tmp1894 = (bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523720) to double*))[i2];
;          |   |
;          |   |      + DO i3 = 0, zext.i32.i64((1 + (-1 * %tmp1884) + %tmp1887 + %tmp1885.out)), 1
;  <MAX_TC_EST = 150>
;          |   |      |   %tmp1919 = 0.000000e+00;
;          |   |      |
;          |   |      |      %tmp1905 = 0.000000e+00;
;          |   |      |   + DO i4 = 0, sext.i32.i64(%tmp1887), 1   <DO_LOOP>  <MAX_TC_EST = 150>
;          |   |      |   |   %tmp1913 = (bitcast ([1092056 x i8]* @global.42 to double*))[i1][i4 + sext.i32.i64(%tmp1884) + -1]  *  (bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2484000) to double*))[i4][i3 + sext.i32.i64(%tmp1884) + -1];
;          |   |      |   |   %tmp1905 = %tmp1913  +  %tmp1905;
;          |   |      |   + END LOOP
;          |   |      |      %tmp1919 = %tmp1905;
;          |   |      |
;          |   |      |   %tmp1920 = %tmp1894  *  %tmp1919;
;          |   |      |   (@global.50)[0][i1][i3 + sext.i32.i64(%tmp1884) + -1] = %tmp1920;
;          |   |      + END LOOP
;          |   |   }
;          |   |   %tmp1884 = %tmp1887 + %tmp1884  +  1;
;          |   + END LOOP
;          + END LOOP
;
;
;          + DO i1 = 0, zext.i32.i64(%tmp1866) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 150>  <LEGAL_MAX_TC = 2147483647>
;          |      %tmp1943 = 1;
;          |      %tmp1944 = 0;
;          |   + DO i2 = 0, sext.i32.i64(%tmp1871) + -1, 1   <DO_LOOP>
;          |   |   %tmp1944.out = %tmp1944;
;          |   |   %tmp1946 = (bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523600) to i32*))[i2];
;          |   |   %tmp1944 = %tmp1946 + %tmp1944  +  1;
;          |   |   if (%tmp1944 >= %tmp1943)
;          |   |   {
;          |   |      %tmp1953 = (bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523720) to double*))[i2];
;          |   |
;          |   |      + DO i3 = 0, zext.i32.i64((1 + (-1 * %tmp1943) + %tmp1946 + %tmp1944.out)), 1   <DO_LOOP>  <MAX_TC_EST = 150>
;          |   |      |   %tmp1978 = 0.000000e+00;
;          |   |      |
;          |   |      |      %tmp1964 = 0.000000e+00;
;          |   |      |   + DO i4 = 0, sext.i32.i64(%tmp1946), 1   <DO_LOOP>  <MAX_TC_EST = 150>
;          |   |      |   |   %tmp1972 = (bitcast (i8* getelementptr inbounds ([1092056 x i8], [1092056 x i8]* @global.42, i64 0, i64 180000) to double*))[i1][i4 + sext.i32.i64(%tmp1943) + -1]  *  (bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2484000) to double*))[i4][i3 + sext.i32.i64(%tmp1943) + -1];
;          |   |      |   |   %tmp1964 = %tmp1972  +  %tmp1964;
;          |   |      |   + END LOOP
;          |   |      |      %tmp1978 = %tmp1964;
;          |   |      |
;          |   |      |   %tmp1979 = %tmp1953  *  %tmp1978;
;          |   |      |   (@global.48)[0][i1][i3 + sext.i32.i64(%tmp1943) + -1] = %tmp1979;
;          |   |      + END LOOP
;          |   |   }
;          |   |   %tmp1943 = %tmp1946 + %tmp1943  +  1;
;          |   + END LOOP
;          + END LOOP
;
;
;          + DO i1 = 0, zext.i32.i64(%tmp1866) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 150>  <LEGAL_MAX_TC = 2147483647>
;          |      %tmp2002 = 1;
;          |      %tmp2003 = 0;
;          |   + DO i2 = 0, sext.i32.i64(%tmp1871) + -1, 1   <DO_LOOP>
;          |   |   %tmp2003.out = %tmp2003;
;          |   |   %tmp2005 = (bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523600) to i32*))[i2];
;          |   |   %tmp2003 = %tmp2005 + %tmp2003  +  1;
;          |   |   if (%tmp2003 >= %tmp2002)
;          |   |   {
;          |   |      %tmp2012 = (bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523720) to double*))[i2];
;          |   |
;          |   |      + DO i3 = 0, zext.i32.i64((1 + (-1 * %tmp2002) + %tmp2005 + %tmp2003.out)), 1   <DO_LOOP>  <MAX_TC_EST = 150>
;          |   |      |   %tmp2037 = 0.000000e+00;
;          |   |      |
;          |   |      |      %tmp2023 = 0.000000e+00;
;          |   |      |   + DO i4 = 0, sext.i32.i64(%tmp2005), 1   <DO_LOOP>  <MAX_TC_EST = 150>
;          |   |      |   |   %tmp2031 = (bitcast (i8* getelementptr inbounds ([1092056 x i8], [1092056 x i8]* @global.42, i64 0, i64 720000) to double*))[i1][i4 + sext.i32.i64(%tmp2002) + -1]  *  (bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2484000) to double*))[i4][i3 + sext.i32.i64(%tmp2002) + -1];
;          |   |      |   |   %tmp2023 = %tmp2031  +  %tmp2023;
;          |   |      |   + END LOOP
;          |   |      |      %tmp2037 = %tmp2023;
;          |   |      |
;          |   |      |   %tmp2038 = %tmp2012  *  %tmp2037;
;          |   |      |   (@global.46)[0][i1][i3 + sext.i32.i64(%tmp2002) + -1] = %tmp2038;
;          |   |      + END LOOP
;          |   |   }
;          |   |   %tmp2002 = %tmp2005 + %tmp2002  +  1;
;          |   + END LOOP
;          + END LOOP
;    END REGION

; Region After Transpose:
; CHECK:  BEGIN REGION { modified }
; CHECK:        %call = @llvm.stacksave();
; CHECK:        %TranspTmpArr = alloca 39600;
;
; CHECK:        + DO i1 = 0, 149, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 32, 1   <DO_LOOP>
; CHECK:        |   |   (%TranspTmpArr)[i1][i2] = (bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2484000) to double*))[i2][i1];
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP
;
;
; CHECK:        + DO i1
; CHECK:        |   + DO i2
; CHECK:        |   |      + DO i3
; CHECK:        |   |      |   + DO i4
; CHECK:        |   |      |   |   %tmp1913 = (bitcast ([1092056 x i8]* @global.42 to double*))[i1][i4 + sext.i32.i64(%tmp1884) + -1]  *  (%TranspTmpArr)[i3 + sext.i32.i64(%tmp1884) + -1][i4];

; CHECK:        + DO i1
; CHECK:        |   + DO i2
; CHECK:        |   |      + DO i3
; CHECK:        |   |      |   + DO i4
; CHECK:        |   |      |   |   %tmp1972 = (bitcast (i8* getelementptr inbounds ([1092056 x i8], [1092056 x i8]* @global.42, i64 0, i64 180000) to double*))[i1][i4 + sext.i32.i64(%tmp1943) + -1]  *  (%TranspTmpArr)[i3 + sext.i32.i64(%tmp1943) + -1][i4];

; CHECK:        + DO i1
; CHECK:        |   + DO i2
; CHECK:        |   |      + DO i3
; CHECK:        |   |      |   + DO i4
; CHECK:        |   |      |   |   %tmp2031 = (bitcast (i8* getelementptr inbounds ([1092056 x i8], [1092056 x i8]* @global.42, i64 0, i64 720000) to double*))[i1][i4 + sext.i32.i64(%tmp2002) + -1]  *  (%TranspTmpArr)[i3 + sext.i32.i64(%tmp2002) + -1][i4];

; CHECK:        @llvm.stackrestore(&((%call)[0]));
; CHECK:  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global.40 = external hidden unnamed_addr global [2523968 x i8], align 32
@global.41 = external hidden unnamed_addr global [2523968 x i8], align 32
@global.42 = external hidden unnamed_addr global [1092056 x i8], align 32
@global.46 = external hidden unnamed_addr global [150 x [150 x double]], align 16, !llfort.type_idx !0
@global.48 = external hidden unnamed_addr global [150 x [150 x double]], align 16, !llfort.type_idx !0
@global.50 = external hidden unnamed_addr global [150 x [150 x double]], align 16, !llfort.type_idx !0
@global.103 = external hidden global [150 x [150 x double]], align 16, !llfort.type_idx !0
@global.104 = external hidden global [150 x [150 x double]], align 16, !llfort.type_idx !0
@global.105 = external hidden global [150 x [150 x double]], align 16, !llfort.type_idx !0
@global.106 = external hidden global [150 x [150 x double]], align 16, !llfort.type_idx !0

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #0

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #0

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare void @llvm.experimental.noalias.scope.decl(metadata) #1

; Function Attrs: nofree nosync nounwind uwtable
declare hidden fastcc void @wobble(i32, i32, double* noalias nocapture writeonly dereferenceable(8), double* noalias nocapture writeonly dereferenceable(8), double* noalias nocapture writeonly dereferenceable(8), double* noalias nocapture writeonly dereferenceable(8)) unnamed_addr #2

; Function Attrs: nofree nosync nounwind uwtable
declare hidden fastcc void @pluto(i32, double* noalias nocapture writeonly dereferenceable(8), double* noalias nocapture writeonly dereferenceable(8), double* noalias nocapture writeonly dereferenceable(8), double* noalias nocapture writeonly dereferenceable(8)) unnamed_addr #2

; Function Attrs: nofree nosync nounwind uwtable
declare hidden fastcc void @spam(i32, i32, double* noalias nocapture writeonly dereferenceable(8), double* noalias nocapture writeonly dereferenceable(8), double* noalias nocapture writeonly dereferenceable(8), double* noalias nocapture writeonly dereferenceable(8)) unnamed_addr #2

; Function Attrs: nofree nosync nounwind uwtable
declare hidden fastcc void @wombat(i32, i32, double* noalias nocapture writeonly dereferenceable(8), double* noalias nocapture writeonly dereferenceable(8), double* noalias nocapture writeonly dereferenceable(8), double* noalias nocapture writeonly dereferenceable(8)) unnamed_addr #2

; Function Attrs: nounwind uwtable
define dso_local i1 @zot.bb1863(i32* %tmp1866.out, i1* %tmp1867.out, i32* %tmp2055.out) #3 {
newFuncRoot:
  br label %bb1863

bb1863:                                           ; preds = %newFuncRoot
  call fastcc void @wobble(i32 1, i32 1, double* getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.106, i64 0, i64 0, i64 0), double* getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.105, i64 0, i64 0, i64 0), double* getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.104, i64 0, i64 0, i64 0), double* getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.103, i64 0, i64 0, i64 0)) #4
  call fastcc void @pluto(i32 1, double* getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.106, i64 0, i64 0, i64 0), double* getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.105, i64 0, i64 0, i64 0), double* getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.104, i64 0, i64 0, i64 0), double* getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.103, i64 0, i64 0, i64 0)) #4
  call fastcc void @spam(i32 1, i32 1, double* getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.106, i64 0, i64 0, i64 0), double* getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.105, i64 0, i64 0, i64 0), double* getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.104, i64 0, i64 0, i64 0), double* getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.103, i64 0, i64 0, i64 0)) #4
  %tmp1864 = load i32, i32* bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.41, i64 0, i64 2523964) to i32*), align 4, !tbaa !4, !llfort.type_idx !9
  %tmp1865 = add nsw i32 %tmp1864, -1
  call fastcc void @wombat(i32 2, i32 %tmp1865, double* getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.106, i64 0, i64 0, i64 0), double* getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.105, i64 0, i64 0, i64 0), double* getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.104, i64 0, i64 0, i64 0), double* getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.103, i64 0, i64 0, i64 0)) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !10) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !13) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !15) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !17) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !19) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !21) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !23) #4
  %tmp1866 = load i32, i32* bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523964) to i32*), align 4, !tbaa !25, !alias.scope !23, !noalias !30, !llfort.type_idx !31
  store i32 %tmp1866, i32* %tmp1866.out, align 4
  %tmp1867 = icmp slt i32 %tmp1866, 1
  store i1 %tmp1867, i1* %tmp1867.out, align 1
  br i1 %tmp1867, label %bb1868, label %bb1870

bb1868:                                           ; preds = %bb1863
  %tmp1869 = load i32, i32* bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523960) to i32*), align 8, !tbaa !32
  br label %bb2054

bb1870:                                           ; preds = %bb1863
  %tmp1871 = load i32, i32* bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523960) to i32*), align 8, !tbaa !34, !alias.scope !21, !noalias !36, !llfort.type_idx !37
  %tmp1872 = icmp slt i32 %tmp1871, 1
  %tmp1873 = add nuw nsw i32 %tmp1871, 1
  %tmp1874 = add nuw nsw i32 %tmp1866, 1
  %tmp1875 = zext i32 %tmp1874 to i64
  %tmp1876 = sext i32 %tmp1873 to i64
  br label %bb1877

bb1877:                                           ; preds = %bb1932, %bb1870
  %tmp1878 = phi i64 [ 1, %bb1870 ], [ %tmp1933, %bb1932 ]
  br i1 %tmp1872, label %bb1932, label %bb1879

bb1879:                                           ; preds = %bb1877
  %tmp1880 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 1200, double* nonnull elementtype(double) getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.50, i64 0, i64 0, i64 0), i64 %tmp1878) #4
  %tmp1881 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 1200, double* nonnull elementtype(double) bitcast ([1092056 x i8]* @global.42 to double*), i64 %tmp1878) #4
  br label %bb1882

bb1882:                                           ; preds = %bb1926, %bb1879
  %tmp1883 = phi i64 [ 1, %bb1879 ], [ %tmp1929, %bb1926 ]
  %tmp1884 = phi i32 [ 1, %bb1879 ], [ %tmp1928, %bb1926 ]
  %tmp1885 = phi i32 [ 0, %bb1879 ], [ %tmp1889, %bb1926 ]
  %tmp1886 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523600) to i32*), i64 %tmp1883) #4, !llfort.type_idx !38, !ifx.array_extent !39
  %tmp1887 = load i32, i32* %tmp1886, align 1, !tbaa !40, !alias.scope !19, !noalias !42
  %tmp1888 = add nsw i32 %tmp1885, %tmp1887
  %tmp1889 = add nsw i32 %tmp1888, 1
  %tmp1890 = icmp slt i32 %tmp1889, %tmp1884
  br i1 %tmp1890, label %bb1926, label %bb1891

bb1891:                                           ; preds = %bb1882
  %tmp1892 = icmp slt i32 %tmp1887, 0
  %tmp1893 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523720) to double*), i64 %tmp1883) #4, !llfort.type_idx !43, !ifx.array_extent !39
  %tmp1894 = load double, double* %tmp1893, align 1, !tbaa !44, !alias.scope !17, !noalias !46, !llfort.type_idx !47
  %tmp1895 = sext i32 %tmp1884 to i64
  %tmp1896 = add nuw nsw i32 %tmp1887, 1
  %tmp1897 = add i32 %tmp1885, 2
  %tmp1898 = add nsw i32 %tmp1897, %tmp1887
  %tmp1899 = sext i32 %tmp1896 to i64
  br label %bb1900

bb1900:                                           ; preds = %bb1918, %bb1891
  %tmp1901 = phi i64 [ %tmp1895, %bb1891 ], [ %tmp1922, %bb1918 ]
  br i1 %tmp1892, label %bb1918, label %bb1902

bb1902:                                           ; preds = %bb1900
  br label %bb1903

bb1903:                                           ; preds = %bb1903, %bb1902
  %tmp1904 = phi i64 [ %tmp1906, %bb1903 ], [ 0, %bb1902 ]
  %tmp1905 = phi double [ %tmp1914, %bb1903 ], [ 0.000000e+00, %bb1902 ]
  %tmp1906 = add nuw nsw i64 %tmp1904, 1
  %tmp1907 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 1200, double* nonnull elementtype(double) bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2484000) to double*), i64 %tmp1906) #4, !llfort.type_idx !48, !ifx.array_extent !49
  %tmp1908 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %tmp1907, i64 %tmp1901) #4, !llfort.type_idx !50
  %tmp1909 = load double, double* %tmp1908, align 1, !tbaa !51, !alias.scope !10, !noalias !53, !llfort.type_idx !54
  %tmp1910 = add nsw i64 %tmp1904, %tmp1895
  %tmp1911 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %tmp1881, i64 %tmp1910) #4, !llfort.type_idx !55
  %tmp1912 = load double, double* %tmp1911, align 1, !tbaa !56, !alias.scope !13, !noalias !58, !llfort.type_idx !59
  %tmp1913 = fmul fast double %tmp1912, %tmp1909
  %tmp1914 = fadd fast double %tmp1913, %tmp1905
  %tmp1915 = icmp eq i64 %tmp1906, %tmp1899
  br i1 %tmp1915, label %bb1916, label %bb1903

bb1916:                                           ; preds = %bb1903
  %tmp1917 = phi double [ %tmp1914, %bb1903 ]
  br label %bb1918

bb1918:                                           ; preds = %bb1916, %bb1900
  %tmp1919 = phi double [ 0.000000e+00, %bb1900 ], [ %tmp1917, %bb1916 ]
  %tmp1920 = fmul fast double %tmp1894, %tmp1919
  %tmp1921 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %tmp1880, i64 %tmp1901) #4, !llfort.type_idx !60
  store double %tmp1920, double* %tmp1921, align 1, !tbaa !61, !alias.scope !15, !noalias !63
  %tmp1922 = add nsw i64 %tmp1901, 1
  %tmp1923 = trunc i64 %tmp1922 to i32
  %tmp1924 = icmp eq i32 %tmp1898, %tmp1923
  br i1 %tmp1924, label %bb1925, label %bb1900

bb1925:                                           ; preds = %bb1918
  br label %bb1926

bb1926:                                           ; preds = %bb1925, %bb1882
  %tmp1927 = add nsw i32 %tmp1884, %tmp1887
  %tmp1928 = add nsw i32 %tmp1927, 1
  %tmp1929 = add nuw nsw i64 %tmp1883, 1
  %tmp1930 = icmp eq i64 %tmp1929, %tmp1876
  br i1 %tmp1930, label %bb1931, label %bb1882

bb1931:                                           ; preds = %bb1926
  br label %bb1932

bb1932:                                           ; preds = %bb1931, %bb1877
  %tmp1933 = add nuw nsw i64 %tmp1878, 1
  %tmp1934 = icmp eq i64 %tmp1933, %tmp1875
  br i1 %tmp1934, label %bb1935, label %bb1877

bb1935:                                           ; preds = %bb1932
  call void @llvm.experimental.noalias.scope.decl(metadata !64) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !67) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !69) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !71) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !73) #4
  br label %bb1936

bb1936:                                           ; preds = %bb1991, %bb1935
  %tmp1937 = phi i64 [ 1, %bb1935 ], [ %tmp1992, %bb1991 ]
  br i1 %tmp1872, label %bb1991, label %bb1938

bb1938:                                           ; preds = %bb1936
  %tmp1939 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 1200, double* nonnull elementtype(double) getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.48, i64 0, i64 0, i64 0), i64 %tmp1937) #4
  %tmp1940 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 1200, double* nonnull elementtype(double) bitcast (i8* getelementptr inbounds ([1092056 x i8], [1092056 x i8]* @global.42, i64 0, i64 180000) to double*), i64 %tmp1937) #4
  br label %bb1941

bb1941:                                           ; preds = %bb1985, %bb1938
  %tmp1942 = phi i64 [ 1, %bb1938 ], [ %tmp1988, %bb1985 ]
  %tmp1943 = phi i32 [ 1, %bb1938 ], [ %tmp1987, %bb1985 ]
  %tmp1944 = phi i32 [ 0, %bb1938 ], [ %tmp1948, %bb1985 ]
  %tmp1945 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523600) to i32*), i64 %tmp1942) #4, !llfort.type_idx !38, !ifx.array_extent !39
  %tmp1946 = load i32, i32* %tmp1945, align 1, !tbaa !75, !alias.scope !73, !noalias !80
  %tmp1947 = add nsw i32 %tmp1944, %tmp1946
  %tmp1948 = add nsw i32 %tmp1947, 1
  %tmp1949 = icmp slt i32 %tmp1948, %tmp1943
  br i1 %tmp1949, label %bb1985, label %bb1950

bb1950:                                           ; preds = %bb1941
  %tmp1951 = icmp slt i32 %tmp1946, 0
  %tmp1952 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523720) to double*), i64 %tmp1942) #4, !llfort.type_idx !43, !ifx.array_extent !39
  %tmp1953 = load double, double* %tmp1952, align 1, !tbaa !83, !alias.scope !71, !noalias !85, !llfort.type_idx !47
  %tmp1954 = sext i32 %tmp1943 to i64
  %tmp1955 = add nuw nsw i32 %tmp1946, 1
  %tmp1956 = add i32 %tmp1944, 2
  %tmp1957 = add nsw i32 %tmp1956, %tmp1946
  %tmp1958 = sext i32 %tmp1955 to i64
  br label %bb1959

bb1959:                                           ; preds = %bb1977, %bb1950
  %tmp1960 = phi i64 [ %tmp1954, %bb1950 ], [ %tmp1981, %bb1977 ]
  br i1 %tmp1951, label %bb1977, label %bb1961

bb1961:                                           ; preds = %bb1959
  br label %bb1962

bb1962:                                           ; preds = %bb1962, %bb1961
  %tmp1963 = phi i64 [ %tmp1965, %bb1962 ], [ 0, %bb1961 ]
  %tmp1964 = phi double [ %tmp1973, %bb1962 ], [ 0.000000e+00, %bb1961 ]
  %tmp1965 = add nuw nsw i64 %tmp1963, 1
  %tmp1966 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 1200, double* nonnull elementtype(double) bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2484000) to double*), i64 %tmp1965) #4, !llfort.type_idx !48, !ifx.array_extent !49
  %tmp1967 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %tmp1966, i64 %tmp1960) #4, !llfort.type_idx !50
  %tmp1968 = load double, double* %tmp1967, align 1, !tbaa !86, !alias.scope !64, !noalias !88, !llfort.type_idx !54
  %tmp1969 = add nsw i64 %tmp1963, %tmp1954
  %tmp1970 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %tmp1940, i64 %tmp1969) #4, !llfort.type_idx !55
  %tmp1971 = load double, double* %tmp1970, align 1, !tbaa !89, !alias.scope !67, !noalias !91, !llfort.type_idx !59
  %tmp1972 = fmul fast double %tmp1971, %tmp1968
  %tmp1973 = fadd fast double %tmp1972, %tmp1964
  %tmp1974 = icmp eq i64 %tmp1965, %tmp1958
  br i1 %tmp1974, label %bb1975, label %bb1962

bb1975:                                           ; preds = %bb1962
  %tmp1976 = phi double [ %tmp1973, %bb1962 ]
  br label %bb1977

bb1977:                                           ; preds = %bb1975, %bb1959
  %tmp1978 = phi double [ 0.000000e+00, %bb1959 ], [ %tmp1976, %bb1975 ]
  %tmp1979 = fmul fast double %tmp1953, %tmp1978
  %tmp1980 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %tmp1939, i64 %tmp1960) #4, !llfort.type_idx !60
  store double %tmp1979, double* %tmp1980, align 1, !tbaa !92, !alias.scope !69, !noalias !94
  %tmp1981 = add nsw i64 %tmp1960, 1
  %tmp1982 = trunc i64 %tmp1981 to i32
  %tmp1983 = icmp eq i32 %tmp1957, %tmp1982
  br i1 %tmp1983, label %bb1984, label %bb1959

bb1984:                                           ; preds = %bb1977
  br label %bb1985

bb1985:                                           ; preds = %bb1984, %bb1941
  %tmp1986 = add nsw i32 %tmp1943, %tmp1946
  %tmp1987 = add nsw i32 %tmp1986, 1
  %tmp1988 = add nuw nsw i64 %tmp1942, 1
  %tmp1989 = icmp eq i64 %tmp1988, %tmp1876
  br i1 %tmp1989, label %bb1990, label %bb1941

bb1990:                                           ; preds = %bb1985
  br label %bb1991

bb1991:                                           ; preds = %bb1990, %bb1936
  %tmp1992 = add nuw nsw i64 %tmp1937, 1
  %tmp1993 = icmp eq i64 %tmp1992, %tmp1875
  br i1 %tmp1993, label %bb1994, label %bb1936

bb1994:                                           ; preds = %bb1991
  call void @llvm.experimental.noalias.scope.decl(metadata !95) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !98) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !100) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !102) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !104) #4
  br label %bb1995

bb1995:                                           ; preds = %bb2050, %bb1994
  %tmp1996 = phi i64 [ 1, %bb1994 ], [ %tmp2051, %bb2050 ]
  br i1 %tmp1872, label %bb2050, label %bb1997

bb1997:                                           ; preds = %bb1995
  %tmp1998 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 1200, double* nonnull elementtype(double) getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.46, i64 0, i64 0, i64 0), i64 %tmp1996) #4
  %tmp1999 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 1200, double* nonnull elementtype(double) bitcast (i8* getelementptr inbounds ([1092056 x i8], [1092056 x i8]* @global.42, i64 0, i64 720000) to double*), i64 %tmp1996) #4
  br label %bb2000

bb2000:                                           ; preds = %bb2044, %bb1997
  %tmp2001 = phi i64 [ 1, %bb1997 ], [ %tmp2047, %bb2044 ]
  %tmp2002 = phi i32 [ 1, %bb1997 ], [ %tmp2046, %bb2044 ]
  %tmp2003 = phi i32 [ 0, %bb1997 ], [ %tmp2007, %bb2044 ]
  %tmp2004 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523600) to i32*), i64 %tmp2001) #4, !llfort.type_idx !38, !ifx.array_extent !39
  %tmp2005 = load i32, i32* %tmp2004, align 1, !tbaa !106, !alias.scope !104, !noalias !111
  %tmp2006 = add nsw i32 %tmp2003, %tmp2005
  %tmp2007 = add nsw i32 %tmp2006, 1
  %tmp2008 = icmp slt i32 %tmp2007, %tmp2002
  br i1 %tmp2008, label %bb2044, label %bb2009

bb2009:                                           ; preds = %bb2000
  %tmp2010 = icmp slt i32 %tmp2005, 0
  %tmp2011 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523720) to double*), i64 %tmp2001) #4, !llfort.type_idx !43, !ifx.array_extent !39
  %tmp2012 = load double, double* %tmp2011, align 1, !tbaa !114, !alias.scope !102, !noalias !116, !llfort.type_idx !47
  %tmp2013 = sext i32 %tmp2002 to i64
  %tmp2014 = add nuw nsw i32 %tmp2005, 1
  %tmp2015 = add i32 %tmp2003, 2
  %tmp2016 = add nsw i32 %tmp2015, %tmp2005
  %tmp2017 = sext i32 %tmp2014 to i64
  br label %bb2018

bb2018:                                           ; preds = %bb2036, %bb2009
  %tmp2019 = phi i64 [ %tmp2013, %bb2009 ], [ %tmp2040, %bb2036 ]
  br i1 %tmp2010, label %bb2036, label %bb2020

bb2020:                                           ; preds = %bb2018
  br label %bb2021

bb2021:                                           ; preds = %bb2021, %bb2020
  %tmp2022 = phi i64 [ %tmp2024, %bb2021 ], [ 0, %bb2020 ]
  %tmp2023 = phi double [ %tmp2032, %bb2021 ], [ 0.000000e+00, %bb2020 ]
  %tmp2024 = add nuw nsw i64 %tmp2022, 1
  %tmp2025 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 1200, double* nonnull elementtype(double) bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2484000) to double*), i64 %tmp2024) #4, !llfort.type_idx !48, !ifx.array_extent !49
  %tmp2026 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %tmp2025, i64 %tmp2019) #4, !llfort.type_idx !50
  %tmp2027 = load double, double* %tmp2026, align 1, !tbaa !117, !alias.scope !95, !noalias !119, !llfort.type_idx !54
  %tmp2028 = add nsw i64 %tmp2022, %tmp2013
  %tmp2029 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %tmp1999, i64 %tmp2028) #4, !llfort.type_idx !55
  %tmp2030 = load double, double* %tmp2029, align 1, !tbaa !120, !alias.scope !98, !noalias !122, !llfort.type_idx !59
  %tmp2031 = fmul fast double %tmp2030, %tmp2027
  %tmp2032 = fadd fast double %tmp2031, %tmp2023
  %tmp2033 = icmp eq i64 %tmp2024, %tmp2017
  br i1 %tmp2033, label %bb2034, label %bb2021

bb2034:                                           ; preds = %bb2021
  %tmp2035 = phi double [ %tmp2032, %bb2021 ]
  br label %bb2036

bb2036:                                           ; preds = %bb2034, %bb2018
  %tmp2037 = phi double [ 0.000000e+00, %bb2018 ], [ %tmp2035, %bb2034 ]
  %tmp2038 = fmul fast double %tmp2012, %tmp2037
  %tmp2039 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %tmp1998, i64 %tmp2019) #4, !llfort.type_idx !60
  store double %tmp2038, double* %tmp2039, align 1, !tbaa !123, !alias.scope !100, !noalias !125
  %tmp2040 = add nsw i64 %tmp2019, 1
  %tmp2041 = trunc i64 %tmp2040 to i32
  %tmp2042 = icmp eq i32 %tmp2016, %tmp2041
  br i1 %tmp2042, label %bb2043, label %bb2018

bb2043:                                           ; preds = %bb2036
  br label %bb2044

bb2044:                                           ; preds = %bb2043, %bb2000
  %tmp2045 = add nsw i32 %tmp2002, %tmp2005
  %tmp2046 = add nsw i32 %tmp2045, 1
  %tmp2047 = add nuw nsw i64 %tmp2001, 1
  %tmp2048 = icmp eq i64 %tmp2047, %tmp1876
  br i1 %tmp2048, label %bb2049, label %bb2000

bb2049:                                           ; preds = %bb2044
  br label %bb2050

bb2050:                                           ; preds = %bb2049, %bb1995
  %tmp2051 = add nuw nsw i64 %tmp1996, 1
  %tmp2052 = icmp eq i64 %tmp2051, %tmp1875
  br i1 %tmp2052, label %bb2053, label %bb1995

bb2053:                                           ; preds = %bb2050
  br label %bb2054

bb2054:                                           ; preds = %bb2053, %bb1868
  %tmp2055 = phi i32 [ %tmp1869, %bb1868 ], [ %tmp1871, %bb2053 ]
  store i32 %tmp2055, i32* %tmp2055.out, align 4
  %tmp2056 = icmp slt i32 %tmp2055, 2
  br i1 %tmp2056, label %bb2093.exitStub, label %bb2057.exitStub

bb2093.exitStub:                                  ; preds = %bb2054
  ret i1 true

bb2057.exitStub:                                  ; preds = %bb2054
  ret i1 false
}

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { nofree nosync nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!1, !2, !3}

!0 = !{i64 16}
!1 = !{i32 1, !"ThinLTO", i32 0}
!2 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!3 = !{i32 1, !"LTOPostLink", i32 1}
!4 = !{!5, !5, i64 0}
!5 = !{!"ifx$unique_sym$109", !6, i64 0}
!6 = !{!"Fortran Data Symbol", !7, i64 0}
!7 = !{!"Generic Fortran Symbol", !8, i64 0}
!8 = !{!"ifx$root$1$MAIN__"}
!9 = !{i64 113}
!10 = !{!11}
!11 = distinct !{!11, !12, !"derivx_: %derivx_$D"}
!12 = distinct !{!12, !"derivx_"}
!13 = !{!14}
!14 = distinct !{!14, !12, !"derivx_: %derivx_$U"}
!15 = !{!16}
!16 = distinct !{!16, !12, !"derivx_: %derivx_$UX"}
!17 = !{!18}
!18 = distinct !{!18, !12, !"derivx_: %derivx_$AL"}
!19 = !{!20}
!20 = distinct !{!20, !12, !"derivx_: %derivx_$NP"}
!21 = !{!22}
!22 = distinct !{!22, !12, !"derivx_: %derivx_$ND"}
!23 = !{!24}
!24 = distinct !{!24, !12, !"derivx_: %derivx_$M"}
!25 = !{!26, !26, i64 0}
!26 = !{!"ifx$unique_sym$603$4$12", !27, i64 0}
!27 = !{!"Fortran Data Symbol", !28, i64 0}
!28 = !{!"Generic Fortran Symbol", !29, i64 0}
!29 = !{!"ifx$root$8$derivx_$4$12"}
!30 = !{!11, !14, !16, !18, !20, !22}
!31 = !{i64 6851}
!32 = !{!33, !33, i64 0}
!33 = !{!"ifx$unique_sym$62", !6, i64 0}
!34 = !{!35, !35, i64 0}
!35 = !{!"ifx$unique_sym$607$4$12", !27, i64 0}
!36 = !{!11, !14, !16, !18, !20, !24}
!37 = !{i64 6850}
!38 = !{i64 6861}
!39 = !{i64 30}
!40 = !{!41, !41, i64 0}
!41 = !{!"ifx$unique_sym$609$4$12", !27, i64 0}
!42 = !{!11, !14, !16, !18, !22, !24}
!43 = !{i64 6875}
!44 = !{!45, !45, i64 0}
!45 = !{!"ifx$unique_sym$615$4$12", !27, i64 0}
!46 = !{!11, !14, !16, !20, !22, !24}
!47 = !{i64 6876}
!48 = !{i64 6867}
!49 = !{i64 33}
!50 = !{i64 6868}
!51 = !{!52, !52, i64 0}
!52 = !{!"ifx$unique_sym$613$4$12", !27, i64 0}
!53 = !{!14, !16, !18, !20, !22, !24}
!54 = !{i64 6869}
!55 = !{i64 6872}
!56 = !{!57, !57, i64 0}
!57 = !{!"ifx$unique_sym$614$4$12", !27, i64 0}
!58 = !{!11, !16, !18, !20, !22, !24}
!59 = !{i64 6873}
!60 = !{i64 6879}
!61 = !{!62, !62, i64 0}
!62 = !{!"ifx$unique_sym$616$4$12", !27, i64 0}
!63 = !{!11, !14, !18, !20, !22, !24}
!64 = !{!65}
!65 = distinct !{!65, !66, !"derivx_: %derivx_$D"}
!66 = distinct !{!66, !"derivx_"}
!67 = !{!68}
!68 = distinct !{!68, !66, !"derivx_: %derivx_$U"}
!69 = !{!70}
!70 = distinct !{!70, !66, !"derivx_: %derivx_$UX"}
!71 = !{!72}
!72 = distinct !{!72, !66, !"derivx_: %derivx_$AL"}
!73 = !{!74}
!74 = distinct !{!74, !66, !"derivx_: %derivx_$NP"}
!75 = !{!76, !76, i64 0}
!76 = !{!"ifx$unique_sym$609$4$13", !77, i64 0}
!77 = !{!"Fortran Data Symbol", !78, i64 0}
!78 = !{!"Generic Fortran Symbol", !79, i64 0}
!79 = !{!"ifx$root$8$derivx_$4$13"}
!80 = !{!65, !68, !70, !72, !81, !82}
!81 = distinct !{!81, !66, !"derivx_: %derivx_$ND"}
!82 = distinct !{!82, !66, !"derivx_: %derivx_$M"}
!83 = !{!84, !84, i64 0}
!84 = !{!"ifx$unique_sym$615$4$13", !77, i64 0}
!85 = !{!65, !68, !70, !74, !81, !82}
!86 = !{!87, !87, i64 0}
!87 = !{!"ifx$unique_sym$613$4$13", !77, i64 0}
!88 = !{!68, !70, !72, !74, !81, !82}
!89 = !{!90, !90, i64 0}
!90 = !{!"ifx$unique_sym$614$4$13", !77, i64 0}
!91 = !{!65, !70, !72, !74, !81, !82}
!92 = !{!93, !93, i64 0}
!93 = !{!"ifx$unique_sym$616$4$13", !77, i64 0}
!94 = !{!65, !68, !72, !74, !81, !82}
!95 = !{!96}
!96 = distinct !{!96, !97, !"derivx_: %derivx_$D"}
!97 = distinct !{!97, !"derivx_"}
!98 = !{!99}
!99 = distinct !{!99, !97, !"derivx_: %derivx_$U"}
!100 = !{!101}
!101 = distinct !{!101, !97, !"derivx_: %derivx_$UX"}
!102 = !{!103}
!103 = distinct !{!103, !97, !"derivx_: %derivx_$AL"}
!104 = !{!105}
!105 = distinct !{!105, !97, !"derivx_: %derivx_$NP"}
!106 = !{!107, !107, i64 0}
!107 = !{!"ifx$unique_sym$609$4$14", !108, i64 0}
!108 = !{!"Fortran Data Symbol", !109, i64 0}
!109 = !{!"Generic Fortran Symbol", !110, i64 0}
!110 = !{!"ifx$root$8$derivx_$4$14"}
!111 = !{!96, !99, !101, !103, !112, !113}
!112 = distinct !{!112, !97, !"derivx_: %derivx_$ND"}
!113 = distinct !{!113, !97, !"derivx_: %derivx_$M"}
!114 = !{!115, !115, i64 0}
!115 = !{!"ifx$unique_sym$615$4$14", !108, i64 0}
!116 = !{!96, !99, !101, !105, !112, !113}
!117 = !{!118, !118, i64 0}
!118 = !{!"ifx$unique_sym$613$4$14", !108, i64 0}
!119 = !{!99, !101, !103, !105, !112, !113}
!120 = !{!121, !121, i64 0}
!121 = !{!"ifx$unique_sym$614$4$14", !108, i64 0}
!122 = !{!96, !101, !103, !105, !112, !113}
!123 = !{!124, !124, i64 0}
!124 = !{!"ifx$unique_sym$616$4$14", !108, i64 0}
!125 = !{!96, !99, !103, !105, !112, !113}
