; RUN: opt -hir-ssa-deconstruction -hir-general-unroll -print-after=hir-general-unroll -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll,print<hir>" -disable-output < %s 2>&1 | FileCheck %s


; Check that General Unroll does not transform remainder loop into a switch for deeply nested loops.
; Although it's not clear that register pressure increases, transforming it may cause degradations
; in certain benchmarks. TODO: Ideally the innerloop would be vectorized but the loop structure is
; complex.

; BEGIN REGION { }
;      + DO i1 = 0, %tmp1848 + -2, 1   <DO_LOOP>  <MAX_TC_EST = 150>
;      |   %tmp1856 = 1;
;      |   %tmp1857 = 0;
;      |
;      |   + DO i2 = 0, %tmp1849 + -2, 1   <DO_LOOP>
;      |   |   %tmp1857.out = %tmp1857;
;      |   |   %tmp1856.out = %tmp1856;
;      |   |   %tmp1859 = (bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.8, i64 0, i64 2523600) to i32*))[i2];
;      |   |   %tmp1857 = %tmp1859 + 1  +  %tmp1857;
;      |   |   if (%tmp1857 >= %tmp1856)
;      |   |   {
;      |   |      %tmp1866 = (bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.8, i64 0, i64 2523720) to double*))[i2];
;      |   |
;      |   |      + DO i3 = 0, zext.i32.i64((1 + (-1 * %tmp1856.out) + %tmp1859 + %tmp1857.out)), 1   <DO_LOOP>  <MAX_TC_EST = 150>
;      |   |      |   %tmp1892 = 0.000000e+00;
;      |   |      |   if (%tmp1859 >= 0)
;      |   |      |   {
;      |   |      |      %tmp1876 = 0.000000e+00;
;      |   |      |
;      |   |      |      + DO i4 = 0, sext.i32.i64((1 + %tmp1859)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 150>
;      |   |      |      |   %tmp1880 = (bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.8, i64 0, i64 2484000) to double*))[i4][i3 + sext.i32.i64(%tmp1856.out) + -1];
;      |   |      |      |   %tmp1885 = (bitcast ([1092056 x i8]* @global.10 to double*))[i1][i4 + %tmp1856.out];
;      |   |      |      |   %tmp1886 = %tmp1885  *  %tmp1880;
;      |   |      |      |   %tmp1876 = %tmp1886  +  %tmp1876;
;      |   |      |      + END LOOP
;      |   |      |
;      |   |      |      %tmp1892 = %tmp1876;
;      |   |      |   }
;      |   |      |   %tmp1893 = %tmp1866  *  %tmp1892;
;      |   |      |   (@global.59)[0][i1][i3 + sext.i32.i64(%tmp1856.out) + -1] = %tmp1893;
;      |   |      + END LOOP
;      |   |   }
;      |   |   %tmp1856 = %tmp1859 + 1  +  %tmp1856;
;      |   + END LOOP
;      + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK; DO i4 = 0, %tgu + -1, 1
; CHECK: DO i4 = 8 * %tgu, sext.i32.i64((1 + %tmp1859)) + -1, 1
; CHECK-NOT: switch

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global.8 = external hidden unnamed_addr global [2523968 x i8], align 32
@global.10 = external hidden unnamed_addr global [1092056 x i8], align 32
@global.59 = external hidden unnamed_addr global [150 x [150 x double]], align 16

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #0

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #0

; Function Attrs: nofree nounwind uwtable
define dso_local void @bar.bb1850(i64 %tmp1849, i64 %tmp1848) #1 {
newFuncRoot:
  br label %bb1850

bb1906.exitStub:                                  ; preds = %bb1903
  ret void

bb1850:                                           ; preds = %newFuncRoot, %bb1903
  %tmp1851 = phi i64 [ 1, %newFuncRoot ], [ %tmp1904, %bb1903 ]
  %tmp1852 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 1200, double* nonnull bitcast ([1092056 x i8]* @global.10 to double*), i64 %tmp1851) #2
  %tmp1853 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 1200, double* nonnull getelementptr inbounds ([150 x [150 x double]], [150 x [150 x double]]* @global.59, i64 0, i64 0, i64 0), i64 %tmp1851) #2
  br label %bb1854

bb1854:                                           ; preds = %bb1899, %bb1850
  %tmp1855 = phi i64 [ 1, %bb1850 ], [ %tmp1901, %bb1899 ]
  %tmp1856 = phi i32 [ 1, %bb1850 ], [ %tmp1900, %bb1899 ]
  %tmp1857 = phi i32 [ 0, %bb1850 ], [ %tmp1861, %bb1899 ]
  %tmp1858 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.8, i64 0, i64 2523600) to i32*), i64 %tmp1855) #2
  %tmp1859 = load i32, i32* %tmp1858, align 1, !alias.scope !3, !noalias !6
  %tmp1860 = add i32 %tmp1859, 1
  %tmp1861 = add i32 %tmp1860, %tmp1857
  %tmp1862 = icmp slt i32 %tmp1861, %tmp1856
  br i1 %tmp1862, label %bb1899, label %bb1863

bb1863:                                           ; preds = %bb1854
  %tmp1864 = icmp slt i32 %tmp1859, 0
  %tmp1865 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.8, i64 0, i64 2523720) to double*), i64 %tmp1855) #2
  %tmp1866 = load double, double* %tmp1865, align 1, !alias.scope !13, !noalias !14
  %tmp1867 = sext i32 %tmp1856 to i64
  %tmp1868 = add i32 %tmp1857, 2
  %tmp1869 = add i32 %tmp1868, %tmp1859
  %tmp1870 = sext i32 %tmp1860 to i64
  br label %bb1871

bb1871:                                           ; preds = %bb1891, %bb1863
  %tmp1872 = phi i64 [ %tmp1867, %bb1863 ], [ %tmp1895, %bb1891 ]
  br i1 %tmp1864, label %bb1891, label %bb1873

bb1873:                                           ; preds = %bb1871
  br label %bb1874

bb1874:                                           ; preds = %bb1874, %bb1873
  %tmp1875 = phi i64 [ %tmp1877, %bb1874 ], [ 0, %bb1873 ]
  %tmp1876 = phi double [ %tmp1887, %bb1874 ], [ 0.000000e+00, %bb1873 ]
  %tmp1877 = add nuw nsw i64 %tmp1875, 1
  %tmp1878 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 1200, double* nonnull bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.8, i64 0, i64 2484000) to double*), i64 %tmp1877) #2
  %tmp1879 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %tmp1878, i64 %tmp1872) #2
  %tmp1880 = load double, double* %tmp1879, align 1, !alias.scope !15, !noalias !16
  %tmp1881 = trunc i64 %tmp1875 to i32
  %tmp1882 = add nsw i32 %tmp1856, %tmp1881
  %tmp1883 = sext i32 %tmp1882 to i64
  %tmp1884 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %tmp1852, i64 %tmp1883) #2
  %tmp1885 = load double, double* %tmp1884, align 1, !alias.scope !17, !noalias !18
  %tmp1886 = fmul fast double %tmp1885, %tmp1880
  %tmp1887 = fadd fast double %tmp1886, %tmp1876
  %tmp1888 = icmp eq i64 %tmp1877, %tmp1870
  br i1 %tmp1888, label %bb1889, label %bb1874

bb1889:                                           ; preds = %bb1874
  %tmp1890 = phi double [ %tmp1887, %bb1874 ]
  br label %bb1891

bb1891:                                           ; preds = %bb1889, %bb1871
  %tmp1892 = phi double [ 0.000000e+00, %bb1871 ], [ %tmp1890, %bb1889 ]
  %tmp1893 = fmul fast double %tmp1866, %tmp1892
  %tmp1894 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %tmp1853, i64 %tmp1872) #2
  store double %tmp1893, double* %tmp1894, align 1, !alias.scope !19, !noalias !20
  %tmp1895 = add nsw i64 %tmp1872, 1
  %tmp1896 = trunc i64 %tmp1895 to i32
  %tmp1897 = icmp eq i32 %tmp1869, %tmp1896
  br i1 %tmp1897, label %bb1898, label %bb1871

bb1898:                                           ; preds = %bb1891
  br label %bb1899

bb1899:                                           ; preds = %bb1898, %bb1854
  %tmp1900 = add i32 %tmp1860, %tmp1856
  %tmp1901 = add nuw nsw i64 %tmp1855, 1
  %tmp1902 = icmp eq i64 %tmp1901, %tmp1849
  br i1 %tmp1902, label %bb1903, label %bb1854

bb1903:                                           ; preds = %bb1899
  %tmp1904 = add nuw nsw i64 %tmp1851, 1
  %tmp1905 = icmp eq i64 %tmp1904, %tmp1848
  br i1 %tmp1905, label %bb1906.exitStub, label %bb1850
}

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
!3 = !{!4}
!4 = distinct !{!4, !5, !"derivx_: %derivx_$NP"}
!5 = distinct !{!5, !"derivx_"}
!6 = !{!7, !8, !9, !10, !11, !12}
!7 = distinct !{!7, !5, !"derivx_: %derivx_$D"}
!8 = distinct !{!8, !5, !"derivx_: %derivx_$U"}
!9 = distinct !{!9, !5, !"derivx_: %derivx_$UX"}
!10 = distinct !{!10, !5, !"derivx_: %derivx_$AL"}
!11 = distinct !{!11, !5, !"derivx_: %derivx_$ND"}
!12 = distinct !{!12, !5, !"derivx_: %derivx_$M"}
!13 = !{!10}
!14 = !{!7, !8, !9, !4, !11, !12}
!15 = !{!7}
!16 = !{!8, !9, !10, !4, !11, !12}
!17 = !{!8}
!18 = !{!7, !9, !10, !4, !11, !12}
!19 = !{!9}
!20 = !{!7, !8, !10, !4, !11, !12}
