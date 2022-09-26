; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -hir-details-dims -disable-output  2>&1 | FileCheck %s

; Check that the dimsize for outermost dimension is available from the "ifx.array_extent" metadata.

; The key values are for: [0:i1:8(double*:30)] in %tmp1953 and %tmp1946, as well as
; [0:i3:1200(double*:33)] from %tmp1968

;         + DO i1 = 0, %tmp1876 + -2, 1   <DO_LOOP>
;         |   %tmp1944.out = %tmp1944;
;         |   %tmp1943.out = %tmp1943;
; CHECK:  |   %tmp1946 = (bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523600) to i32*))[0:i1:4(i32*:30)];
;         |   %tmp1944 = %tmp1946 + %tmp1944.out  +  1;
;         |   if (%tmp1944 >= %tmp1943)
;         |   {
; CHECK:  |      %tmp1953 = (bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523720) to double*))[0:i1:8(double*:30)];
;         |
;         |      + DO i2 = 0, zext.i32.i64((1 + (-1 * %tmp1943.out) + %tmp1946 + %tmp1944.out)), 1   <DO_LOOP>
;         |      |   %tmp1978 = 0.000000e+00;
;         |      |
;         |      |      %tmp1964 = 0.000000e+00;
;         |      |   + DO i3 = 0, sext.i32.i64(%tmp1946), 1   <DO_LOOP>
; CHECK:  |      |   |   %tmp1968 = (bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2484000) to double*))[0:i3:1200(double*:33)][0:i2 + sext.i32.i64(%tmp1943.out) + -1:8(double*:150)];
;         |      |   |   %tmp1971 = (%tmp1940)[0:i3 + sext.i32.i64(%tmp1943.out) + -1:8(double*:0)];
;         |      |   |   %tmp1972 = %tmp1971  *  %tmp1968;
;         |      |   |   %tmp1964 = %tmp1972  +  %tmp1964;
;         |      |   + END LOOP
;         |      |      %tmp1978 = %tmp1964;
;         |      |
;         |      |   %tmp1979 = %tmp1953  *  %tmp1978;
;         |      |   (%tmp1939)[0:i2 + sext.i32.i64(%tmp1943.out) + -1:8(double*:0)] = %tmp1979;
;         |      + END LOOP
;         |
;         |   }
;         |   %tmp1943 = %tmp1946 + %tmp1943.out  +  1;
;         + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global.40 = external hidden unnamed_addr global [2523968 x i8], align 32

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #0

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #0

; Function Attrs: nounwind uwtable
define dso_local void @zot.bb1941(double* %tmp1940, double* %tmp1939, i64 %tmp1876) #1 {
newFuncRoot:
  br label %bb1941

bb1941:                                           ; preds = %newFuncRoot, %bb1985
  %tmp1942 = phi i64 [ 1, %newFuncRoot ], [ %tmp1988, %bb1985 ]
  %tmp1943 = phi i32 [ 1, %newFuncRoot ], [ %tmp1987, %bb1985 ]
  %tmp1944 = phi i32 [ 0, %newFuncRoot ], [ %tmp1948, %bb1985 ]
  %tmp1945 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523600) to i32*), i64 %tmp1942) #2, !llfort.type_idx !3, !ifx.array_extent !4
  %tmp1946 = load i32, i32* %tmp1945, align 1, !tbaa !5, !alias.scope !10, !noalias !13
  %tmp1947 = add nsw i32 %tmp1944, %tmp1946
  %tmp1948 = add nsw i32 %tmp1947, 1
  %tmp1949 = icmp slt i32 %tmp1948, %tmp1943
  br i1 %tmp1949, label %bb1985, label %bb1950

bb1950:                                           ; preds = %bb1941
  %tmp1951 = icmp slt i32 %tmp1946, 0
  %tmp1952 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2523720) to double*), i64 %tmp1942) #2, !llfort.type_idx !20, !ifx.array_extent !4
  %tmp1953 = load double, double* %tmp1952, align 1, !tbaa !21, !alias.scope !23, !noalias !24, !llfort.type_idx !25
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
  %tmp1966 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 1200, double* nonnull elementtype(double) bitcast (i8* getelementptr inbounds ([2523968 x i8], [2523968 x i8]* @global.40, i64 0, i64 2484000) to double*), i64 %tmp1965) #2, !llfort.type_idx !26, !ifx.array_extent !27
  %tmp1967 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %tmp1966, i64 %tmp1960) #2, !llfort.type_idx !28
  %tmp1968 = load double, double* %tmp1967, align 1, !tbaa !29, !alias.scope !31, !noalias !32, !llfort.type_idx !33
  %tmp1969 = add nsw i64 %tmp1963, %tmp1954
  %tmp1970 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %tmp1940, i64 %tmp1969) #2, !llfort.type_idx !34
  %tmp1971 = load double, double* %tmp1970, align 1, !tbaa !35, !alias.scope !37, !noalias !38, !llfort.type_idx !39
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
  %tmp1980 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %tmp1939, i64 %tmp1960) #2, !llfort.type_idx !40
  store double %tmp1979, double* %tmp1980, align 1, !tbaa !41, !alias.scope !43, !noalias !44
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
  br i1 %tmp1989, label %bb1990.exitStub, label %bb1941

bb1990.exitStub:                                  ; preds = %bb1985
  ret void
}

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
!3 = !{i64 6861}
!4 = !{i64 30}
!5 = !{!6, !6, i64 0}
!6 = !{!"ifx$unique_sym$609$4$13", !7, i64 0}
!7 = !{!"Fortran Data Symbol", !8, i64 0}
!8 = !{!"Generic Fortran Symbol", !9, i64 0}
!9 = !{!"ifx$root$8$derivx_$4$13"}
!10 = !{!11}
!11 = distinct !{!11, !12, !"derivx_: %derivx_$NP"}
!12 = distinct !{!12, !"derivx_"}
!13 = !{!14, !15, !16, !17, !18, !19}
!14 = distinct !{!14, !12, !"derivx_: %derivx_$D"}
!15 = distinct !{!15, !12, !"derivx_: %derivx_$U"}
!16 = distinct !{!16, !12, !"derivx_: %derivx_$UX"}
!17 = distinct !{!17, !12, !"derivx_: %derivx_$AL"}
!18 = distinct !{!18, !12, !"derivx_: %derivx_$ND"}
!19 = distinct !{!19, !12, !"derivx_: %derivx_$M"}
!20 = !{i64 6875}
!21 = !{!22, !22, i64 0}
!22 = !{!"ifx$unique_sym$615$4$13", !7, i64 0}
!23 = !{!17}
!24 = !{!14, !15, !16, !11, !18, !19}
!25 = !{i64 6876}
!26 = !{i64 6867}
!27 = !{i64 33}
!28 = !{i64 6868}
!29 = !{!30, !30, i64 0}
!30 = !{!"ifx$unique_sym$613$4$13", !7, i64 0}
!31 = !{!14}
!32 = !{!15, !16, !17, !11, !18, !19}
!33 = !{i64 6869}
!34 = !{i64 6872}
!35 = !{!36, !36, i64 0}
!36 = !{!"ifx$unique_sym$614$4$13", !7, i64 0}
!37 = !{!15}
!38 = !{!14, !16, !17, !11, !18, !19}
!39 = !{i64 6873}
!40 = !{i64 6879}
!41 = !{!42, !42, i64 0}
!42 = !{!"ifx$unique_sym$616$4$13", !7, i64 0}
!43 = !{!16}
!44 = !{!14, !15, !17, !11, !18, !19}
